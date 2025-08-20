#!/usr/bin/env python3
"""
Test data generator for network packet sniffer server
Simulates both regular packets and traceroute data
"""

import json
import time
import random
import sys
from datetime import datetime

# Sample data pools
LOCAL_IPS = ["192.168.1.100", "10.0.0.50"]
ROUTER_IPS = [
    "192.168.1.1",
    "10.129.66.120", 
    "192.168.29.10",
    "192.168.28.13",
    "192.168.28.9",
    "192.168.31.23",
    "192.168.31.33",
    "125.20.207.126",
    "116.119.36.22",
    "116.119.161.198"
]

DESTINATION_IPS = [
    "8.8.8.8",
    "1.1.1.1", 
    "142.250.238.200",
    "192.178.110.244",
    "74.125.253.167",
    "157.240.22.35",
    "31.13.64.35"
]

WEB_PORTS = [80, 443, 8080, 8443]
PROTOCOLS = ["TCP", "UDP", "ICMP"]

def generate_regular_packet():
    """Generate a regular network packet"""
    local_ip = random.choice(LOCAL_IPS)
    dest_ip = random.choice(DESTINATION_IPS)
    
    # Randomly choose direction (outgoing vs incoming)
    if random.random() < 0.7:  # 70% outgoing
        src_ip, dst_ip = local_ip, dest_ip
    else:  # 30% incoming
        src_ip, dst_ip = dest_ip, local_ip
    
    protocol = random.choice(PROTOCOLS)
    
    packet = {
        "timestamp": time.time(),
        "src_ip": src_ip,
        "dst_ip": dst_ip,
        "length": random.randint(64, 1500),
        "protocol": protocol
    }
    
    if protocol in ["TCP", "UDP"]:
        packet["src_port"] = random.randint(1024, 65535)
        packet["dst_port"] = random.choice(WEB_PORTS) if random.random() < 0.6 else random.randint(1024, 65535)
        
        if protocol == "TCP":
            # Add TCP flags
            packet["tcp_flags"] = {
                "FIN": random.choice([0, 1]) if random.random() < 0.1 else 0,
                "SYN": 1 if random.random() < 0.3 else 0,
                "RST": 0,
                "PSH": 1 if random.random() < 0.5 else 0,
                "ACK": 1 if random.random() < 0.8 else 0,
                "URG": 0
            }
    
    elif protocol == "ICMP":
        packet["type"] = random.choice([0, 3, 8, 11])  # Echo reply, dest unreachable, echo request, time exceeded
        packet["code"] = 0
    
    return packet

def generate_traceroute_packet():
    """Generate a traceroute packet with realistic hop sequence"""
    dst_ip = random.choice(DESTINATION_IPS)
    
    # Build a realistic path from local -> routers -> destination
    num_hops = random.randint(3, 8)
    path_ips = [random.choice(LOCAL_IPS)] + random.sample(ROUTER_IPS, min(num_hops-2, len(ROUTER_IPS))) + [dst_ip]
    
    hops = []
    for ttl, hop_ip in enumerate(path_ips[1:], 1):  # Skip local IP, start TTL at 1
        # Simulate multiple probes per hop (like real traceroute)
        responses = []
        
        # Sometimes multiple IPs respond at same TTL (load balancing)
        num_responses = 1 if random.random() < 0.8 else random.randint(2, 3)
        
        for _ in range(num_responses):
            # Add some variation in response IPs
            response_ip = hop_ip
            if num_responses > 1 and random.random() < 0.5:
                # Simulate load balancer with different IPs
                response_ip = f"{'.'.join(hop_ip.split('.')[:-1])}.{random.randint(1, 254)}"
            
            # Generate realistic RTTs (increase with distance)
            base_rtt = ttl * random.uniform(10, 25) + random.uniform(5, 20)
            rtts = []
            
            for probe in range(3):  # 3 probes per response
                if random.random() < 0.95:  # 95% success rate
                    rtt = base_rtt + random.uniform(-5, 15)
                    rtts.append(round(rtt, 3))
                else:
                    rtts.append("*")  # Timeout
            
            responses.append({
                "ip": response_ip,
                "rtts": rtts
            })
        
        hops.append({
            "ttl": ttl,
            "responses": responses
        })
    
    traceroute_packet = {
        "dst_ip": dst_ip,
        "protocol": "TRACEROUTE",
        "timestamp": int(time.time()),
        "hops": hops
    }
    
    return traceroute_packet

def print_status(message):
    """Print status message to stderr (like the C++ sniffer)"""
    print(message, file=sys.stderr)

def main():
    print_status("🚀 Starting test data generator...")
    print_status("📁 Generating sample network data")
    print_status("🔍 Simulating packets on test interface...")
    print_status("Press Ctrl+C to stop.")
    
    try:
        packet_count = 0
        while True:
            # Generate mostly regular packets with occasional traceroutes
            if random.random() < 0.15:  # 15% chance of traceroute
                packet = generate_traceroute_packet()
                print_status(f"[DEBUG] Generated traceroute to {packet['dst_ip']}")
            else:
                packet = generate_regular_packet()
            
            # Output JSON to stdout (like the C++ sniffer)
            print(json.dumps(packet))
            sys.stdout.flush()
            
            packet_count += 1
            
            # Vary the timing
            if packet['protocol'] == 'TRACEROUTE':
                time.sleep(random.uniform(2, 8))  # Traceroutes take longer
            else:
                time.sleep(random.uniform(0.1, 2.0))  # Regular packets are frequent
            
            # Print periodic stats
            if packet_count % 50 == 0:
                print_status(f"[DEBUG] Generated {packet_count} packets")
    
    except KeyboardInterrupt:
        print_status(f"\n🛑 Test data generator stopped. Generated {packet_count} packets.")
        sys.exit(0)

def generate_sample_batch():
    """Generate a small batch of sample data for testing"""
    print_status("🔧 Generating sample batch...")
    
    # Generate a few regular packets
    for _ in range(5):
        packet = generate_regular_packet()
        print(json.dumps(packet))
    
    # Generate one traceroute
    packet = generate_traceroute_packet()
    print(json.dumps(packet))
    
    print_status("✅ Sample batch complete!")

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--batch":
        generate_sample_batch()
    else:
        main()