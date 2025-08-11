#pragma once
#include <pcap.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <string>
#include <nlohmann/json.hpp>
//declare and intialize the PacketSniffer class and declare the methods in advance (acts like an interface in java)
class PacketSniffer {
public:
    PacketSniffer(const std::string& interfaceName);//constructor
    ~PacketSniffer();//destructor
    //public attributes
    std::string baseSessionName;//the session name would be the timestamp of the first packet we have just initialized 
    int chunkIndex;
    //public methods
    void stop();
    bool start();

private:
    //private methods to be implemented in the sniffer class
    static void packetHandler(u_char *userData, const struct pcap_pkthdr *header, const u_char *packet);
    void processPacket(const struct pcap_pkthdr *header, const u_char *packet);
    void saveToFile();
    //interface name to refers to the device name like wlo(wifi),bl(bluetooth);
    std::string interface;
    pcap_t* handle;
    char errbuf[PCAP_ERRBUF_SIZE];

    std::string sessionFile;
    //this library is necessary to save files in Json format
    nlohmann::json packets;
    int packetCount;
};
