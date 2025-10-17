#include "testwindow.h"
#include "websocket_client.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QNetworkRequest>
#include <QTimer>
#include <QDebug>

TestWindow::TestWindow(QWidget* parent)
    : QWidget(parent), logOutput(nullptr), ws(nullptr), networkManager(nullptr)
{
    setWindowTitle("Network Sniffer - Graph Visualizer");
    resize(800, 600);

    statusLabel = new QLabel("Status: Initializing...", this);
    statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #f0f0f0; font-size: 14px; }");
    statusLabel->setWordWrap(true);

    reconnectBtn = new QPushButton("Reconnect WebSocket", this);
    refreshBtn = new QPushButton("Refresh Graph Data", this);
    clearLogBtn = new QPushButton("Clear Log", this);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(reconnectBtn);
    buttonLayout->addWidget(refreshBtn);
    buttonLayout->addWidget(clearLogBtn);

    // Initialize log output
    logOutput = new QTextEdit(this);
    logOutput->setReadOnly(true);
    logOutput->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #d4d4d4; font-family: 'Courier New'; font-size: 10px; }");

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(statusLabel);
    layout->addLayout(buttonLayout);
    layout->addWidget(logOutput);
    layout->addStretch();

    // Setup Network Manager for HTTP requests
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &TestWindow::onInitialGraphReceived);

    // Clear log button
    connect(clearLogBtn, &QPushButton::clicked, this, [this]() {
        logOutput->clear();
        log("Log cleared");
    });

    // Fetch initial graph data via HTTP GET
    fetchInitialGraph();

    // Connect to WebSocket for real-time updates (after 1 second delay)
    QTimer::singleShot(1000, this, [this]() {
       ws = new WebSocketClient(QUrl("ws://localhost:5000/socket.io/?EIO=4&transport=websocket"), this);

        connect(ws, &WebSocketClient::connectionStatusChanged, this, [this](bool connected) {
            if (connected) {
                statusLabel->setText(statusLabel->text() + "\n✓ WebSocket Connected - Real-time updates active");
                statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #90EE90; color: #006400; font-size: 14px; }");
                log("✓ WebSocket connected successfully");
            } else {
                statusLabel->setText("✗ WebSocket Disconnected - No real-time updates");
                statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #FFB6C1; color: #8B0000; font-size: 14px; }");
                log("✗ WebSocket disconnected");
            }
        });

        connect(ws, &WebSocketClient::messageReceived, this, &TestWindow::handleGraphData);

        connect(ws, &WebSocketClient::errorOccurred, this, [this](const QString& error) {
            statusLabel->setText("WebSocket Error: " + error);
            statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #FFB6C1; color: #8B0000; font-size: 14px; }");
            log("⚠ WebSocket error: " + error);
        });
    });

    // Button connections
    connect(reconnectBtn, &QPushButton::clicked, [this]() {
        if (ws) {
            statusLabel->setText("Reconnecting WebSocket...");
            statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #FFE4B5; font-size: 14px; }");
            log("Attempting to reconnect WebSocket...");
            ws->reconnect();
        }
    });

    connect(refreshBtn, &QPushButton::clicked, this, &TestWindow::fetchInitialGraph);
}

void TestWindow::log(const QString& message) {
    qDebug() << message;

    if (logOutput) {
        logOutput->append(message);

        // Auto-scroll to bottom
        QTextCursor cursor = logOutput->textCursor();
        cursor.movePosition(QTextCursor::End);
        logOutput->setTextCursor(cursor);
    }
}

void TestWindow::fetchInitialGraph() {
    log("\n--- Fetching Initial Graph (HTTP GET) ---");
    log("URL: http://localhost:5000/api/graph/detailed");
    statusLabel->setText("Loading initial graph data from API...");
    statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #FFE4B5; font-size: 14px; }");

    QNetworkRequest request(QUrl("http://localhost:5000/api/graph/detailed"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    networkManager->get(request);
}

void TestWindow::onInitialGraphReceived(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);

        if (!doc.isNull() && doc.isObject()) {
            QJsonObject data = doc.object();
            log("\n========================================");
            log("INITIAL GRAPH DATA LOADED (HTTP GET)");
            log("========================================");
            processGraphData(data, "HTTP GET");

            statusLabel->setText("✓ Initial graph loaded successfully");
            statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #ADD8E6; color: #00008B; font-size: 14px; font-weight: bold; }");
        } else {
            log("✗ Invalid JSON response from API");
            statusLabel->setText("✗ Error: Invalid response from API");
            statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #FFB6C1; font-size: 14px; }");
        }
    } else {
        log("✗ HTTP Error: " + reply->errorString());
        statusLabel->setText("✗ Error loading graph: " + reply->errorString());
        statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #FFB6C1; color: #8B0000; font-size: 14px; font-weight: bold; }");
    }

    reply->deleteLater();
}

void TestWindow::handleGraphData(const QString& message) {
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());

    if (doc.isNull()) {
        log("⚠ Received non-JSON WebSocket message: " + message);
        return;
    }

    QJsonObject obj = doc.object();

    log("\n========================================");
    log("REAL-TIME UPDATE (WebSocket)");
    log("========================================");

    processGraphData(obj, "WebSocket");
}

void TestWindow::processGraphData(const QJsonObject& obj, const QString& source) {
    QString type = obj["type"].toString();

    if (type.isEmpty()) {
        type = "full";  // Default for HTTP response
    }

    log("Source: " + source);
    log("Type: " + type);

    if (type == "graph_update" || type == "full" || obj.contains("nodes")) {
        // Full graph update
        QJsonArray nodes = obj["nodes"].toArray();
        QJsonArray edges = obj["edges"].toArray();
        QJsonObject paths = obj["traceroute_paths"].toObject();

        log("\n--- GRAPH SUMMARY ---");
        log(QString("Nodes count: %1").arg(nodes.size()));
        log(QString("Edges count: %1").arg(edges.size()));
        log(QString("Traceroute paths: %1").arg(paths.size()));

        log("\n--- NODES ---");
        for (const QJsonValue& nodeVal : nodes) {
            QJsonObject node = nodeVal.toObject();
            QString id = node["id"].toString();
            if (id.isEmpty()) id = node["ip"].toString();
            QString ip = node["ip"].toString();
            bool isLocal = node["is_local"].toBool(false);
            QString nodeType = node["type"].toString("unknown");
            int packetCount = node["packet_count"].toInt(0);

            QJsonArray protocols = node["protocols"].toArray();
            QStringList protoList;
            for (const QJsonValue& p : protocols) {
                protoList << p.toString();
            }

            log(QString("  [%1] %2").arg(isLocal ? "LOCAL" : nodeType.toUpper()).arg(ip));
            log(QString("      Packets: %1 | Protocols: %2").arg(packetCount).arg(protoList.join(", ")));
        }

        log("\n--- EDGES ---");
        for (const QJsonValue& edgeVal : edges) {
            QJsonObject edge = edgeVal.toObject();
            QString source = edge["source"].toString();
            QString target = edge["target"].toString();
            QString edgeType = edge["type"].toString("direct");
            int packetCount = edge["packet_count"].toInt(1);

            log(QString("  %1 --[%2 packets]--> %3 (type: %4)")
                    .arg(source).arg(packetCount).arg(target).arg(edgeType));
        }

        if (source == "HTTP GET") {
            statusLabel->setText(QString("✓ Loaded: %1 nodes, %2 edges, %3 paths")
                                     .arg(nodes.size()).arg(edges.size()).arg(paths.size()));
        } else {
            statusLabel->setText(QString("✓ Updated: %1 nodes, %2 edges")
                                     .arg(nodes.size()).arg(edges.size()));
        }
    }
    else if (type == "new_node") {
        QString ip = obj["ip"].toString();
        QString nodeType = obj["node_type"].toString("unknown");
        bool isLocal = obj["is_local"].toBool(false);

        log("\n--- NEW NODE DETECTED ---");
        log("  IP: " + ip);
        log("  Type: " + nodeType);
        log("  Local: " + QString(isLocal ? "Yes" : "No"));

        statusLabel->setText(QString("✓ New Node: %1 (%2)").arg(ip).arg(nodeType));
    }
    else if (type == "new_edge" || type == "edge") {
        QString source = obj["source"].toString();
        QString target = obj["target"].toString();
        int packets = obj["packets"].toInt(1);
        int bytes = obj["bytes"].toInt(0);

        log("\n--- NEW EDGE DETECTED ---");
        log("  From: " + source);
        log("  To: " + target);
        log(QString("  Packets: %1").arg(packets));
        if (bytes > 0) {
            log(QString("  Bytes: %1").arg(bytes));
        }

        statusLabel->setText(QString("✓ New Edge: %1 → %2 (%3 packets)")
                                 .arg(source).arg(target).arg(packets));
    }
    else {
        log("⚠ Unknown message type: " + type);
        log("Full data: " + QString(QJsonDocument(obj).toJson(QJsonDocument::Compact)));
    }

    log("========================================\n");
}
