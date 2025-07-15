#include "packetSniffer.h"

int main() {
    PacketSniffer sniffer("wlo1"); // or "wlan0", "eth0" based on your device

    if (!sniffer.start()) {
        return 1;
    }

    return 0;
}
