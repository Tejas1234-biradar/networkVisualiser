


# Network Visualizer - Getting Started

This guide will help you set up and run the Network Visualizer project, including the packet sniffer and traceroute components.
````markdown
---

## Requirements

- **Operating System**: Primarily tested on Arch Linux.  
  - For other OS (Windows/macOS), use **WSL** or a **Linux VM**.
- **Compiler**: `g++` (with C++17 support)
- **Python**: `python3`
- **Python packages**:
  - `nlohmann` (for JSON support in C++)
  - Other required packages: (list if any)
- **Libraries**:  
  - `libpcap` (for packet capturing)  
  - `pthread` (for multi-threading)

---

## Building the Sniffer

1. Open a terminal in the project directory.
2. Compile the C++ code:

```bash
g++ -std=c++17 sniffer.cpp Traceroute.cpp main.cpp -lpcap -pthread -o packet_sniffer
````

This will create the executable `packet_sniffer`.

---

## Running the Sniffer and Server

Run the sniffer and pipe output to the Python app:

```bash
sudo ./packet_sniffer | python3 app.py
```

**Notes**:

* `sudo` is required for packet capturing privileges.
* Make sure Python dependencies are installed before running (`pip3 install <package>`).

---

## Testing the Server

You can test the server endpoint using `curl`:

```bash
curl http://localhost:5000/stats
```

For pretty JSON output (requires `jq`):

```bash
curl http://localhost:5000/stats | jq
```

---

## Troubleshooting

* If `libpcap` is missing:

  ```bash
  sudo pacman -Syu libpcap
  ```
* If running on non-Arch systems, ensure `g++`, `libpcap-dev`, and Python are installed in your environment.

---

## Additional Notes

* Ensure the sniffer and traceroute executables are in the same directory as `app.py`.
* For development, any changes to `sniffer.cpp` or `Traceroute.cpp` require recompilation.

---

