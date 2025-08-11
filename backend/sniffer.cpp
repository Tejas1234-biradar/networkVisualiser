#include "packetSniffer.h"
#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <netinet/ip_icmp.h>
#include <nlohmann/json.hpp>
/*
TODO:To train the Anomaly model
TODO Make a python file which extracts data using pandas and adds it to a csv file
!we require the following parameters 
?avg_packet_size
?unique_src_ips
?unique_dst_ips
?protocol_distribution (percentage TCP, UDP, ICMP, etc.)
?packet_rate (packets per second)
?port_scan_signals:
?Count of unique destination ports from same source in short time
?tcp_flag_counts:
?SYNs without ACKs → SYN flood
*/
using namespace std;
using json = nlohmann::json;
PacketSniffer::PacketSniffer(const string& interfaceName)//here we use the PacketSniffer:: to acess the class in our header and implement the functions
: interface(interfaceName), handle(nullptr), packetCount(0), chunkIndex(1) {
        /**
         * Clears the error buffer by setting all bytes in errbuf to zero using memset.
         * This ensures that the error buffer does not contain any garbage values before use.
         */
    memset(errbuf, 0, PCAP_ERRBUF_SIZE);
    
    // Create packets directory if it doesn't exist
    filesystem::create_directory("packets");
    
    // Initialize the session file
    baseSessionName = "packets/session_" + to_string(time(nullptr)) + ".json";
    packets = json::array();//define a json array
}

PacketSniffer::~PacketSniffer() {//destructor
    if (handle != nullptr) {
        pcap_close(handle);//handle is the current capture session 
    }
}

bool PacketSniffer::start() {
    handle = pcap_open_live(interface.c_str(), BUFSIZ, 1, 1000, errbuf);//returns a pcap struct
    if (handle == nullptr) {
        cerr << "pcap_open_live failed: " << errbuf << "\n";
        return false;
    }

    cout << "🔍 Listening on " << interface << "...\nPress Ctrl+C to stop.\n";

    // Start the packet capture loop
    if (pcap_loop(handle, 0, packetHandler, reinterpret_cast<u_char*>(this)) < 0) {
        cerr << "pcap_loop error\n";
        return false;
    }

    return true;
}

void PacketSniffer::stop() {
    if (handle != nullptr) {
        pcap_breakloop(handle);
        saveToFile();  // Save remaining packets
    }
}

void PacketSniffer::packetHandler(u_char *userData, const struct pcap_pkthdr *header, const u_char *packet) {
    PacketSniffer* sniffer = reinterpret_cast<PacketSniffer*>(userData);
    sniffer->processPacket(header, packet);
}

void PacketSniffer::processPacket(const struct pcap_pkthdr *header, const u_char *packet) {
    const struct ip *ipHeader = (struct ip *)(packet + 14);
    int ipHeaderLen = ipHeader->ip_hl * 4;

    string srcIP = inet_ntoa(ipHeader->ip_src);
    string dstIP = inet_ntoa(ipHeader->ip_dst);

    json packetData;//initalize a sinngle json element
    packetData["timestamp"] = header->ts.tv_sec + header->ts.tv_usec / 1e6;
    packetData["src_ip"] = srcIP;
    packetData["dst_ip"] = dstIP;
    packetData["length"] = header->len;

    if (ipHeader->ip_p == IPPROTO_TCP) {
        const struct tcphdr *tcpHeader = (struct tcphdr *)(packet + 14 + ipHeaderLen);
        packetData["protocol"] = "TCP";
        packetData["src_port"] = ntohs(tcpHeader->th_sport);
        packetData["dst_port"] = ntohs(tcpHeader->th_dport);
        uint8_t flags=tcpHeader->th_flags;
packetData["tcp_flags"] = {
    {"FIN", flags & TH_FIN},
    {"SYN", flags & TH_SYN},
    {"RST", flags & TH_RST},
    {"PSH", flags & TH_PUSH},
    {"ACK", flags & TH_ACK},
    {"URG", flags & TH_URG}
};
    } else if (ipHeader->ip_p == IPPROTO_UDP) {
        const struct udphdr *udpHeader = (struct udphdr *)(packet + 14 + ipHeaderLen);
        packetData["protocol"] = "UDP";
        packetData["src_port"] = ntohs(udpHeader->uh_sport);
        packetData["dst_port"] = ntohs(udpHeader->uh_dport);
    } else if (ipHeader->ip_p == IPPROTO_ICMP) {
        packetData["protocol"] = "ICMP";
        const struct icmp *icmpHeader = (struct icmp *)(packet + 14 + ipHeaderLen);
        packetData["type"] = (int)icmpHeader->icmp_type;
        packetData["code"] = (int)icmpHeader->icmp_code;
    } else {
        packetData["protocol"] = "Other";
        packetData["protocol_number"] = (int)ipHeader->ip_p;
    }

    packets.push_back(packetData);//Json array
    packetCount++;

    // Save to file every 40 packets
    if (packetCount % 40 == 0) {
        saveToFile();
    }

    // Still print to console for real-time monitoring
    cout << "\n📦 Packet:\n";
    cout << "  Src IP: " << srcIP << "\n";
    cout << "  Dst IP: " << dstIP << "\n";
    cout << "  Protocol: ";

    if (ipHeader->ip_p == IPPROTO_TCP) {
        const struct tcphdr *tcpHeader = (struct tcphdr *)(packet + 14 + ipHeaderLen);
        cout << "TCP\n";
        cout << "  Src Port: " << ntohs(tcpHeader->th_sport) << "\n";
        cout << "  Dst Port: " << ntohs(tcpHeader->th_dport) << "\n";
    } else if (ipHeader->ip_p == IPPROTO_UDP) {
        const struct udphdr *udpHeader = (struct udphdr *)(packet + 14 + ipHeaderLen);
        cout << "UDP\n";
        cout << "  Src Port: " << ntohs(udpHeader->uh_sport) << "\n";
        cout << "  Dst Port: " << ntohs(udpHeader->uh_dport) << "\n";
    } else {
        cout << "Other (" << (int)ipHeader->ip_p << ")\n";
    }

    cout << "  Length: " << header->len << " bytes\n";
}

void PacketSniffer::saveToFile() {
    string fileName = baseSessionName + "_chunk_" + to_string(chunkIndex++) + ".json";
    ofstream file(fileName);
    file << packets.dump(2);
    file.close();
    packets.clear();  // Clear packets after saving
}
