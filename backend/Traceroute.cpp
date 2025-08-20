#include "packetSniffer.h"
#include <iostream>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <thread>

using namespace std;

Traceroute::Traceroute(const std::string &targetHost, int maxHops, int timeout)
    : target(targetHost), maxHops(maxHops), timeout(timeout) {}

std::vector<Hop> Traceroute::performTrace() {
    std::vector<Hop> results;
    
    struct sockaddr_in targetAddr;
    memset(&targetAddr, 0, sizeof(targetAddr));
    targetAddr.sin_family = AF_INET;
    
    if (inet_aton(target.c_str(), &targetAddr.sin_addr) == 0) {
        throw std::runtime_error("Invalid IP address: " + target);
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create raw socket (need root privileges)");
    }

    bool reachedTarget = false;
    
    for (int ttl = 1; ttl <= maxHops && !reachedTarget; ++ttl) {
        // Send 3 probes per TTL (like standard traceroute)
        std::vector<ProbeResult> probes;
        
        for (int probe = 0; probe < 3; ++probe) {
            ProbeResult result = sendProbe(sockfd, targetAddr, ttl, probe);
            probes.push_back(result);
            
            // Check if we reached the target
            if (result.success && result.responseIP == target) {
                reachedTarget = true;
            }
            
            // Small delay between probes
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        // Process results for this TTL
        Hop hop = processProbesForHop(ttl, probes);
        if (!hop.responses.empty()) {
            results.push_back(hop);
        }
    }

    close(sockfd);
    return results;
}

Traceroute::ProbeResult Traceroute::sendProbe(int sockfd, const struct sockaddr_in &targetAddr, int ttl, int probeNum) {
    ProbeResult result;
    result.success = false;
    result.rtt = -1.0;
    result.responseIP = "";
    
    // Set TTL for this probe
    if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
        return result;
    }

    // Create ICMP echo request
    struct icmphdr icmp;
    memset(&icmp, 0, sizeof(icmp));
    icmp.type = ICMP_ECHO;
    icmp.code = 0;
    icmp.un.echo.id = getpid() & 0xFFFF;
    icmp.un.echo.sequence = (ttl << 8) | probeNum; // Unique sequence per probe
    icmp.checksum = 0;
    icmp.checksum = checksum((unsigned short *)&icmp, sizeof(icmp));

    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Send the probe
    if (sendto(sockfd, &icmp, sizeof(icmp), 0, (struct sockaddr *)&targetAddr, sizeof(targetAddr)) < 0) {
        return result;
    }

    // Wait for response
    fd_set readfds;
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    if (select(sockfd + 1, &readfds, nullptr, nullptr, &tv) > 0) {
        char buffer[1024];
        struct sockaddr_in fromAddr;
        socklen_t fromLen = sizeof(fromAddr);
        
        ssize_t bytesReceived = recvfrom(sockfd, buffer, sizeof(buffer), 0, 
                                       (struct sockaddr *)&fromAddr, &fromLen);
        
        if (bytesReceived > 0) {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            
            // Parse the IP header to get to ICMP
            struct iphdr *ipHdr = (struct iphdr *)buffer;
            int ipHeaderLen = ipHdr->ihl * 4;
            
            if (bytesReceived >= ipHeaderLen + sizeof(struct icmphdr)) {
                struct icmphdr *icmpReply = (struct icmphdr *)(buffer + ipHeaderLen);
                
                // Check if this is our packet
                if ((icmpReply->type == ICMP_TIME_EXCEEDED || icmpReply->type == ICMP_ECHOREPLY)) {
                    result.success = true;
                    result.rtt = duration.count() / 1000.0; // Convert to milliseconds
                    result.responseIP = inet_ntoa(fromAddr.sin_addr);
                }
            }
        }
    }
    
    return result;
}

Hop Traceroute::processProbesForHop(int ttl, const std::vector<ProbeResult> &probes) {
    Hop hop;
    hop.ttl = ttl;
    
    // Group responses by IP address (like standard traceroute)
    std::map<std::string, std::vector<double>> ipToRtts;
    
    for (const auto &probe : probes) {
        if (probe.success) {
            ipToRtts[probe.responseIP].push_back(probe.rtt);
        } else {
            ipToRtts["*"].push_back(-1.0); // Timeout
        }
    }
    
    // Create responses for JSON output
    for (const auto &pair : ipToRtts) {
        HopResponse response;
        response.ip = pair.first;
        response.rtts = pair.second;
        hop.responses.push_back(response);
    }
    
    return hop;
}

unsigned short Traceroute::checksum(unsigned short *buf, int len) {
    unsigned long sum = 0;
    
    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }
    
    if (len == 1) {
        sum += *(unsigned char *)buf;
    }
    
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    
    return (unsigned short)(~sum);
}