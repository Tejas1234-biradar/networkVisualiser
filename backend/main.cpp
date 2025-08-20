#include "packetSniffer.h"
#include <iostream>
#include <signal.h>
#include <cstdlib>

using namespace std;

PacketSniffer *globalSniffer = nullptr;

void signalHandler(int signum) {
    cerr << "\n🛑 Interrupt signal (" << signum << ") received.\n";
    if (globalSniffer) {
        globalSniffer->stop();
    }
    exit(signum);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <interface>\n";
        cerr << "Example: " << argv[0] << " eth0\n";
        return 1;
    }

    string interface = argv[1];
    
    // ALL status messages to stderr
    cerr << "🚀 Starting network packet sniffer..." << endl;
    cerr << "📁 Output will be saved to packets/ directory" << endl;

    PacketSniffer sniffer(interface);
    globalSniffer = &sniffer;

    // Set up signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    if (!sniffer.start()) {
        cerr << "❌ Failed to start packet sniffer\n";
        return 1;
    }

    return 0;
}
