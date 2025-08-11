import json
import glob
import csv
from collections import defaultdict, Counter

def load_packets(json_files_pattern="packets/session_*.json_chunk_*.json"):
    packets = []
    for filename in glob.glob(json_files_pattern):
        with open(filename, 'r') as f:
            chunk_packets = json.load(f)
            packets.extend(chunk_packets)
    return packets

def aggregate_packets(packets, window_size=1):
    """
    Aggregates packets into time windows of `window_size` seconds.
    Returns a dict: window_start_time -> list of packets in that window
    """
    windows = defaultdict(list)
    for pkt in packets:
        ts = pkt["timestamp"]
        window_start = (ts // window_size) * window_size
        windows[window_start].append(pkt)
    return windows

def calculate_features(packets_in_window):
    packet_count = len(packets_in_window)
    if packet_count == 0:
        return None

    total_size = sum(pkt.get("length", 0) for pkt in packets_in_window)
    avg_packet_size = total_size / packet_count

    unique_src_ips = set(pkt.get("src_ip") for pkt in packets_in_window if "src_ip" in pkt)
    unique_dst_ips = set(pkt.get("dst_ip") for pkt in packets_in_window if "dst_ip" in pkt)
    unique_src_ports = set()
    
    protocol_counter = Counter()
    port_scan_dict = defaultdict(set)  # src_ip -> set of dst_ports
    tcp_flag_counter = Counter()
    tcp_packet_count = 0

    for pkt in packets_in_window:
        proto = pkt.get("protocol", "Other")
        protocol_counter[proto] += 1

        # For port scan signals (count unique dst ports per src IP)
        src_ip = pkt.get("src_ip")
        dst_port = pkt.get("dst_port")

        if src_ip and dst_port is not None:
            port_scan_dict[src_ip].add(dst_port)

        if proto == "TCP":
            tcp_packet_count += 1
            # Count TCP flags from packet["tcp_flags"]
            flags = pkt.get("tcp_flags", {})
            for flag, val in flags.items():
                if val:
                    tcp_flag_counter[flag] += 1
            
            # Track unique src ports for TCP packets
            src_port = pkt.get("src_port")
            if src_port is not None:
                unique_src_ports.add(src_port)
        elif proto == "UDP":
            src_port = pkt.get("src_port")
            if src_port is not None:
                unique_src_ports.add(src_port)

    # Protocol distribution (percentage)
    protocol_distribution = {k: v / packet_count for k, v in protocol_counter.items()}

    # Packet rate = packet_count per window (window size is 1 second by default)
    packet_rate = packet_count / 1

    # Port scan signals: count of unique dst ports from same source in the window
    port_scan_signals = sum(len(ports) for ports in port_scan_dict.values())

    # TCP flag counts
    syn_count = tcp_flag_counter.get("SYN", 0)
    ack_count = tcp_flag_counter.get("ACK", 0)
    syn_without_ack = max(syn_count - ack_count, 0)  # SYNs without ACKs

    # TCP flag ratios
    syn_without_ack_ratio = syn_without_ack / tcp_packet_count if tcp_packet_count > 0 else 0

    features = {
        "avg_packet_size": avg_packet_size,
        "unique_src_ips": len(unique_src_ips),
        "unique_dst_ips": len(unique_dst_ips),
        "unique_src_ports": len(unique_src_ports),
        "protocol_tcp": protocol_distribution.get("TCP", 0),
        "protocol_udp": protocol_distribution.get("UDP", 0),
        "protocol_icmp": protocol_distribution.get("ICMP", 0),
        "protocol_other": protocol_distribution.get("Other", 0),
        "packet_rate": packet_rate,
        "port_scan_signals": port_scan_signals,
        "tcp_flag_syn": syn_count,
        "tcp_flag_ack": ack_count,
        "tcp_flag_syn_without_ack": syn_without_ack,
        "tcp_flag_syn_without_ack_ratio": syn_without_ack_ratio
    }
    return features

def save_to_csv(features_per_window, output_file="network_features.csv"):
    if not features_per_window:
        print("No features to save.")
        return

    with open(output_file, mode='w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=["window_start"] + list(next(iter(features_per_window.values())).keys()))
        writer.writeheader()
        for window_start, features in sorted(features_per_window.items()):
            row = {"window_start": window_start}
            row.update(features)
            writer.writerow(row)
    print(f"Saved features to {output_file}")

def main():
    print("Loading packets...")
    packets = load_packets()
    print(f"Loaded {len(packets)} packets")

    print("Aggregating packets into time windows...")
    windows = aggregate_packets(packets, window_size=0.2)  # 1 second windows

    print("Calculating features for each window...")
    features_per_window = {}
    for window_start, pkts in windows.items():
        features = calculate_features(pkts)
        if features:
            features_per_window[window_start] = features

    save_to_csv(features_per_window)

if __name__ == "__main__":
    main()
