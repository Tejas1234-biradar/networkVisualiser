#ifndef PACKETSNIFFER_H
#define PACKETSNIFFER_H

#include <pcap.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp> // or nlohmann/json.hpp depending on your JSON library
#include <set>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>

using json = nlohmann::json; // Adjust based on your JSON library

// Structure for traceroute hop response
struct HopResponse {
    std::string ip;
    std::vector<double> rtts; // Multiple RTTs for this IP
};

// Structure for traceroute hop (can have multiple responses)
struct Hop {
    int ttl;
    std::vector<HopResponse> responses;
};

// Traceroute task for thread pool
struct TracerouteTask {
    std::string dstIP;
};

class Traceroute {
private:
    std::string target;
    int maxHops;
    int timeout;

    struct ProbeResult {
        bool success;
        double rtt;
        std::string responseIP;
    };

    ProbeResult sendProbe(int sockfd, const struct sockaddr_in &targetAddr, int ttl, int probeNum);
    Hop processProbesForHop(int ttl, const std::vector<ProbeResult> &probes);
    unsigned short checksum(unsigned short *buf, int len);

public:
    Traceroute(const std::string &targetHost, int maxHops = 30, int timeout = 1000);
    std::vector<Hop> performTrace();
};

class PacketSniffer {
private:
    static const int MAX_CONCURRENT_TRACES = 4;
    
    std::string interface;
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    
    // Packet storage
    json packets;
    int packetCount;
    std::string baseSessionName;
    int chunkIndex;
    
    // Traceroute thread pool
    std::vector<std::thread> tracerThreads;
    std::queue<TracerouteTask> taskQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;
    bool stopTracerThreads = false;
    std::set<std::string> tracedIPs;
    
    void processPacket(const struct pcap_pkthdr *header, const u_char *packet);
    void saveToFile();
    void runTracerouteAsync(const std::string &dstIP);
    void tracerThreadFunc();
    
    static void packetHandler(u_char *userData, const struct pcap_pkthdr *header, const u_char *packet);

public:
    PacketSniffer(const std::string &interfaceName);
    ~PacketSniffer();
    
    bool start();
    void stop();
};

#endif // PACKETSNIFFER_H