# Real-Time Network Visualization and Packet Analysis Tool

A powerful, currently tested on **Linux** (planned support for Windows & macOS) network packet analysis tool built using **Qt (C++)** with a **Flask-based backend** for extended processing. The project focuses on deep packet inspection, real-time network visualization, traceroute mapping, and anomaly detection using machine learning.

---

## 🚀 Features

### 🔍 **1. Packet Sniffing & Analysis (C++)**

* Captures network packets in real time.
* Supports multiple protocols: **TCP, UDP, ICMP**, and more.
* Automatic detection and logging of **TCP handshakes** (SYN, SYN/ACK, ACK).
* Uniform JSON export of packet data (every fixed interval / batch).
* Built with raw sockets / pcap (depending on OS).

### 🌐 **2. Flask Server for Packet Processing (Python)**

* Receives packet data from the Qt client in JSON format.

* Performs:
  ✔ Packet classification
  ✔ Header extraction
  ✔ Content-based analysis

* Sends structured results back to the Qt GUI.

### 🤖 **3. Anomaly Detection (Random Forests)**

* Detects suspicious patterns such as:

  * Abnormal packet bursts
  * Unusual port activity
  * Strange IP behavior
  * Protocol anomalies
* Model trained using scikit-learn.
* Results displayed in the UI with alerts.

### 🧭 **4. Traceroute Mapping**

* Performs IP-level route discovery.
* Visualizes the path each packet takes across the network.
* Helps identify routing issues and network hops.

### 📊 **5. Visualization Dashboard (Qt)**

* **Network Graph View**

  * Displays devices as nodes.
  * Shows connections as edges.

* **Line Graph View and Tabular View**

  * Real-time packet rate monitoring.
  * Bandwidth / protocol usage.
  * Full packet breakdown.
  * Live updates from backend.

* **Anomaly Detection View**

  * Analyzes each packet and queries the server to check if it is malicious.
  * Displays the AI model’s Random Forest output in a structured tabular format.

---

## ⚙️ Tech Stack

| Component       | Technology                   |
| --------------- | ---------------------------- |
| GUI             | **Qt (C++)**                 |
| Packet Sniffing | C++ raw sockets / pcap       |
| Backend         | **Flask (Python)**           |
| ML Model        | Random Forest (scikit-learn) |
| Visualization   | QtCharts / custom graphs     |

---

## 🏗️ Architecture Overview

The system operates through a clean multi-stage pipeline, separating packet capture, analysis, backend processing, and frontend visualization.

### **System Flow**

1. **Packet Sniffer (Pure C++)**

   * Uses Linux's native networking capabilities.
   * Accesses raw network traffic via libraries like **libpcap**.
   * Preprocesses & structures packet data.

2. **Communication to Flask Backend**

   * Packet sniffer sends JSON payloads directly to the Flask server.
   * Flask acts as a lightweight processing layer.

3. **Flask Server → Qt Application**

   * Flask exposes a REST API.
   * The Qt client polls these endpoints for real-time updates.

4. **Qt Visualization Layer**

   * Polling-based refresh for graphs, tables, and alerts.

```
+-----------------------+         JSON Packets          +-----------------------+
|  Packet Sniffer (C++) | ============================> |     Flask Server      |
|  (libpcap / raw caps) |                               | (REST API + Analysis)  |
+-----------------------+                                +-----------+-----------+
                                                                      |
                                                                      | REST Endpoints
                                                                      v
                                                         +-------------------------+
                                                         |   ML Engine (Python)    |
                                                         | Random Forest Anomaly   |
                                                         | Detection (REST API)    |
                                                         +-----------+-------------+
                                                                     |
                                                                     | REST Endpoints
                                                                     v
                                                          +-----------------------+
                                                          |     Qt Application    |
                                                          | (Polling UI: Graphs,  |
                                                          |  Tables, Alerts)      |
                                                          +-----------------------+
```

---

## 📦 Installation

 Refer to [documentation/getting_started.md](documentation/getting_started.md) for installation.

---

## 🧪 Running the Application

1. Start the Flask backend:

```
sudo ./sniffer | python3 app.py
```

2. Launch the Qt application.
3. Start packet capture (root/admin may be required on some OS).
4. View network graphs, tables, and traceroute.
5. Check anomaly alerts in real time.

---

## 🧠 Additional Notes

* Currently confirmed working on ** Linux**. Windows & macOS testing is planned..
* Designed to be modular: sniffer, analysis engine, and ML components can be swapped independently.
* Emphasis on real-time visualization and clarity.

---



## ⭐ Acknowledgements

Thanks to the open-source community, Qt contributors, Python ecosystem, and pcap developers for making powerful tooling accessible.

