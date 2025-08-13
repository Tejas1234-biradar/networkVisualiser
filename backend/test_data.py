#!/usr/bin/env python3
import json
import time
import random

# Generate fake packet data for testing
def generate_test_packets():
    src_ips = ["192.168.1.1", "10.0.0.1", "172.16.0.1", "8.8.8.8"]
    dst_ips = ["192.168.1.100", "10.0.0.100", "172.16.0.100", "1.1.1.1"]
    ports = [80, 443, 22, 8080, 3000, 5000]
    protocols = ["TCP", "UDP", "ICMP"]
    
    while True:
        packet = {
            "timestamp": time.time(),
            "src_ip": random.choice(src_ips),
            "dst_ip": random.choice(dst_ips),
            "protocol": random.choice(protocols),
            "length": random.randint(64, 1500)
        }
        
        if packet["protocol"] in ["TCP", "UDP"]:
            packet["src_port"] = random.choice(ports)
            packet["dst_port"] = random.choice(ports)
        
        print(json.dumps(packet))
        time.sleep(0.5)  # Send packet every 500ms

if __name__ == "__main__":
    generate_test_packets()