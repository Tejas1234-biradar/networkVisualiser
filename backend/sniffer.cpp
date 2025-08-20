#include "packetSniffer.h"
#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <chrono>

using namespace std;

PacketSniffer::PacketSniffer(const string &interfaceName)
    : interface(interfaceName), handle(nullptr), packetCount(0), chunkIndex(1) {

    memset(errbuf, 0, PCAP_ERRBUF_SIZE);
    filesystem::create_directory("packets");
    baseSessionName = "packets/session_" + to_string(time(nullptr));
    packets = json::array();

    // Start traceroute threads
    for (int i = 0; i < MAX_CONCURRENT_TRACES; ++i)
        tracerThreads.emplace_back(&PacketSniffer::tracerThreadFunc, this);
}

PacketSniffer::~PacketSniffer() {
    if (handle) pcap_close(handle);

    // Stop tracer threads
    {
        lock_guard<mutex> lock(queueMutex);
        stopTracerThreads = true;
    }
    queueCV.notify_all();

    for (auto &t : tracerThreads)
        if (t.joinable()) t.join();
}

bool PacketSniffer::start() {
    handle = pcap_open_live(interface.c_str(), BUFSIZ, 1, 1000, errbuf);
    if (!handle) {
        cerr << "pcap_open_live failed: " << errbuf << "\n";
        return false;
    }

    cerr << "🔍 Listening on " << interface << "...\nPress Ctrl+C to stop.\n";

    if (pcap_loop(handle, 0, packetHandler, reinterpret_cast<u_char *>(this)) < 0) {
        cerr << "pcap_loop error\n";
        return false;
    }
    return true;
}

void PacketSniffer::stop() {
    if (handle) {
        pcap_breakloop(handle);
        saveToFile();
    }
}

void PacketSniffer::packetHandler(u_char *userData, const struct pcap_pkthdr *header, const u_char *packet) {
    PacketSniffer *sniffer = reinterpret_cast<PacketSniffer *>(userData);
    sniffer->processPacket(header, packet);
}

void PacketSniffer::processPacket(const struct pcap_pkthdr *header, const u_char *packet) {
    if (header->len < 34) return; // Too small for IP packet
    
    const struct iphdr *ipHeader = (struct iphdr *)(packet + 14);
    int ipHeaderLen = ipHeader->ihl * 4;

    struct in_addr src_addr, dst_addr;
    src_addr.s_addr = ipHeader->saddr;
    dst_addr.s_addr = ipHeader->daddr;
    
    string srcIP = inet_ntoa(src_addr);
    string dstIP = inet_ntoa(dst_addr);

    json packetData;
    packetData["timestamp"] = header->ts.tv_sec + header->ts.tv_usec / 1e6;
    packetData["src_ip"] = srcIP;
    packetData["dst_ip"] = dstIP;
    packetData["length"] = header->len;

    if (ipHeader->protocol == IPPROTO_TCP) {
        if (header->len < 14 + ipHeaderLen + sizeof(struct tcphdr)) return;
        
        const struct tcphdr *tcpHeader = (struct tcphdr *)(packet + 14 + ipHeaderLen);
        packetData["protocol"] = "TCP";
        packetData["src_port"] = ntohs(tcpHeader->source);
        packetData["dst_port"] = ntohs(tcpHeader->dest);
        
        packetData["tcp_flags"] = {
            {"FIN", tcpHeader->fin},
            {"SYN", tcpHeader->syn},
            {"RST", tcpHeader->rst},
            {"PSH", tcpHeader->psh},
            {"ACK", tcpHeader->ack},
            {"URG", tcpHeader->urg}
        };
    }
    else if (ipHeader->protocol == IPPROTO_UDP) {
        if (header->len < 14 + ipHeaderLen + sizeof(struct udphdr)) return;
        
        const struct udphdr *udpHeader = (struct udphdr *)(packet + 14 + ipHeaderLen);
        packetData["protocol"] = "UDP";
        packetData["src_port"] = ntohs(udpHeader->source);
        packetData["dst_port"] = ntohs(udpHeader->dest);
    }
    else if (ipHeader->protocol == IPPROTO_ICMP) {
        if (header->len < 14 + ipHeaderLen + sizeof(struct icmphdr)) return;
        
        const struct icmphdr *icmpHeader = (struct icmphdr *)(packet + 14 + ipHeaderLen);
        packetData["protocol"] = "ICMP";
        packetData["type"] = (int)icmpHeader->type;
        packetData["code"] = (int)icmpHeader->code;
    }
    else {
        packetData["protocol"] = "Other";
        packetData["protocol_number"] = (int)ipHeader->protocol;
    }

    packets.push_back(packetData);
    packetCount++;

    if (packetCount % 40 == 0) saveToFile();

    cout << packetData.dump() << endl;
    cout.flush();

    runTracerouteAsync(dstIP);
}

void PacketSniffer::saveToFile() {
    if (packets.empty()) return;
    
    string fileName = baseSessionName + "_chunk_" + to_string(chunkIndex++) + ".json";
    ofstream file(fileName);
    if (file.is_open()) {
        file << packets.dump(2);
        file.close();
    }
    packets.clear();
}

void PacketSniffer::runTracerouteAsync(const std::string &dstIP) {
    if (dstIP.empty() || dstIP == "127.0.0.1" || dstIP == "0.0.0.0") return;

    lock_guard<mutex> lock(queueMutex);
    if (tracedIPs.find(dstIP) != tracedIPs.end()) return;

    tracedIPs.insert(dstIP);
    taskQueue.push({dstIP});
    queueCV.notify_one();
}

void PacketSniffer::tracerThreadFunc() {
    while (true) {
        TracerouteTask task;
        {
            unique_lock<mutex> lock(queueMutex);
            queueCV.wait(lock, [this] { return !taskQueue.empty() || stopTracerThreads; });

            if (stopTracerThreads && taskQueue.empty()) return;

            task = taskQueue.front();
            taskQueue.pop();
        }

        try {
            Traceroute tracer(task.dstIP, 20, 1000);
            auto hops = tracer.performTrace();

            // Create traceroute JSON output matching standard traceroute format
            if (!hops.empty()) {
                json tracerouteData;
                tracerouteData["dst_ip"] = task.dstIP;
                tracerouteData["protocol"] = "TRACEROUTE";
                tracerouteData["timestamp"] = time(nullptr);
                
                json hopsArray = json::array();
                for (const auto &hop : hops) {
                    json hopData;
                    hopData["ttl"] = hop.ttl;
                    
                    json responsesArray = json::array();
                    for (const auto &response : hop.responses) {
                        json responseData;
                        responseData["ip"] = response.ip;
                        
                        json rttsArray = json::array();
                        for (double rtt : response.rtts) {
                            if (rtt > 0) {
                                rttsArray.push_back(rtt);
                            } else {
                                rttsArray.push_back("*"); // Timeout
                            }
                        }
                        responseData["rtts"] = rttsArray;
                        responsesArray.push_back(responseData);
                    }
                    hopData["responses"] = responsesArray;
                    hopsArray.push_back(hopData);
                }
                tracerouteData["hops"] = hopsArray;

                cout << tracerouteData.dump() << endl;
                cout.flush();
            }
        }
        catch (const std::exception &e) {
            cerr << "[TRACEROUTE ERROR] " << e.what() << " for IP " << task.dstIP << "\n";
        }
    }
}
