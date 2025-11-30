#include "anomalywidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QGroupBox>
#include <QtMath>

AnomalyWidget::AnomalyWidget(QWidget *parent)
    : QWidget(parent)
    , totalAnalyzed(0)
    , maliciousCount(0)
    , benignCount(0)
    , isPaused(false)
{
    qDebug() << "=== AnomalyWidget Constructor ===";

    packetApi = new ApiCaller(this);
    predictionApi = new ApiCaller(this);
    fetchTimer = new QTimer(this);

    connect(packetApi, &ApiCaller::responseReceived, this, &AnomalyWidget::onPacketsReceived);
    connect(predictionApi, &ApiCaller::responseReceived, this, &AnomalyWidget::onPredictionReceived);
    connect(fetchTimer, &QTimer::timeout, this, &AnomalyWidget::fetchPackets);

    qDebug() << "ApiCaller connections established";

    setupUI();

    qDebug() << "=== AnomalyWidget Constructor Complete ===\n";
}

void AnomalyWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel *titleLabel = new QLabel("🛡️ Malicious Packet Detection");
    titleLabel->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;"
        );
    mainLayout->addWidget(titleLabel);

    // Statistics Panel
    statsPanel = new QWidget();
    QHBoxLayout *statsLayout = new QHBoxLayout(statsPanel);
    statsPanel->setStyleSheet(
        "QWidget {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 rgba(52, 152, 219, 200), stop:1 rgba(41, 128, 185, 200));"
        "  border-radius: 12px;"
        "  padding: 20px;"
        "}"
        );

    auto createStatCard = [](const QString &label, QLabel *&valueLabel, const QString &color) {
        QWidget *card = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setAlignment(Qt::AlignCenter);

        valueLabel = new QLabel("0");
        valueLabel->setStyleSheet(QString("font-size: 36px; font-weight: bold; color: %1;").arg(color));
        valueLabel->setAlignment(Qt::AlignCenter);

        QLabel *labelWidget = new QLabel(label);
        labelWidget->setStyleSheet("font-size: 14px; color: white;");
        labelWidget->setAlignment(Qt::AlignCenter);

        layout->addWidget(valueLabel);
        layout->addWidget(labelWidget);
        return card;
    };

    statsLayout->addWidget(createStatCard("Total Analyzed", totalPacketsLabel, "white"));
    statsLayout->addWidget(createStatCard("Malicious", maliciousCountLabel, "#e74c3c"));
    statsLayout->addWidget(createStatCard("Benign", benignCountLabel, "#2ecc71"));
    statsLayout->addWidget(createStatCard("Detection Rate", accuracyLabel, "#f39c12"));

    mainLayout->addWidget(statsPanel);

    // Control Panel
    QWidget *controlPanel = new QWidget();
    QHBoxLayout *controlLayout = new QHBoxLayout(controlPanel);
    controlPanel->setStyleSheet(
        "QWidget { background: rgba(236, 240, 241, 255); border-radius: 10px; padding: 15px; }"
        );

    QString buttonStyle =
        "QPushButton {"
        "  background: #3498db;"
        "  color: white;"
        "  border: none;"
        "  padding: 10px 25px;"
        "  border-radius: 8px;"
        "  font-weight: bold;"
        "  min-width: 100px;"
        "}"
        "QPushButton:hover { background: #2980b9; }"
        "QPushButton:pressed { background: #21618c; }";

    pauseButton = new QPushButton("⏸ Pause");
    clearButton = new QPushButton("🗑 Clear");
    pauseButton->setStyleSheet(buttonStyle);
    clearButton->setStyleSheet(buttonStyle);

    connect(pauseButton, &QPushButton::clicked, this, [this]() {
        isPaused = !isPaused;
        pauseButton->setText(isPaused ? "▶ Resume" : "⏸ Pause");
        if (isPaused) {
            fetchTimer->stop();
            statusLabel->setText("⏸ Analysis Paused");
        } else {
            fetchTimer->start();
            statusLabel->setText("🔄 Analyzing packets...");
        }
    });
    connect(clearButton, &QPushButton::clicked, this, &AnomalyWidget::clearResults);

    statusLabel = new QLabel("🔄 Ready to analyze");
    statusLabel->setStyleSheet("font-weight: bold; color: #34495e;");

    analysisProgress = new QProgressBar();
    analysisProgress->setStyleSheet(
        "QProgressBar {"
        "  border: 2px solid #bdc3c7;"
        "  border-radius: 5px;"
        "  text-align: center;"
        "  background: white;"
        "}"
        "QProgressBar::chunk {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #3498db, stop:1 #2980b9);"
        "  border-radius: 3px;"
        "}"
        );
    analysisProgress->setMaximum(0);
    analysisProgress->setMinimum(0);
    analysisProgress->setVisible(false);

    controlLayout->addWidget(pauseButton);
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(statusLabel);
    controlLayout->addStretch();
    controlLayout->addWidget(analysisProgress);

    mainLayout->addWidget(controlPanel);

    // Results Table
    QGroupBox *resultsGroup = new QGroupBox("Detection Results");
    resultsGroup->setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  font-size: 16px;"
        "  border: 2px solid #bdc3c7;"
        "  border-radius: 8px;"
        "  margin-top: 10px;"
        "  padding-top: 15px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 5px;"
        "}"
        );
    QVBoxLayout *resultsLayout = new QVBoxLayout(resultsGroup);

    resultsTable = new QTableWidget(0, 8);
    resultsTable->setHorizontalHeaderLabels({
        "Timestamp", "Source", "Destination", "Port", "Protocol",
        "Size", "Status", "Confidence"
    });
    resultsTable->horizontalHeader()->setStretchLastSection(true);
    resultsTable->setAlternatingRowColors(true);
    resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultsTable->setStyleSheet(
        "QTableWidget {"
        "  gridline-color: #ecf0f1;"
        "  border: 1px solid #bdc3c7;"
        "  border-radius: 5px;"
        "}"
        "QHeaderView::section {"
        "  background: #34495e;"
        "  color: white;"
        "  padding: 8px;"
        "  border: none;"
        "  font-weight: bold;"
        "}"
        "QTableWidget::item:selected {"
        "  background: #3498db;"
        "  color: white;"
        "}"
        );

    resultsLayout->addWidget(resultsTable);
    mainLayout->addWidget(resultsGroup, 1);

    setLayout(mainLayout);
}

void AnomalyWidget::startMonitoring(const QString &url, int intervalMs)
{
    qDebug() << "\n=== Starting Anomaly Monitoring ===";
    qDebug() << "API URL:" << url;
    qDebug() << "Interval:" << intervalMs << "ms";

    apiUrl = url;
    fetchTimer->start(intervalMs);
    statusLabel->setText("🔄 Analyzing packets...");

    qDebug() << "Timer started, fetching initial data...";
    fetchPackets();  // Immediate first fetch

    qDebug() << "=== Monitoring Started ===\n";
}

void AnomalyWidget::stopMonitoring()
{
    fetchTimer->stop();
    statusLabel->setText("⏹ Monitoring stopped");
}

void AnomalyWidget::fetchPackets()
{
    if (isPaused) {
        qDebug() << "Fetch skipped - paused";
        return;
    }

    qDebug() << "Fetching packets from:" << apiUrl;
    analysisProgress->setVisible(true);
    packetApi->get(apiUrl);
}

void AnomalyWidget::onPacketsReceived(const QString &data)
{
    analysisProgress->setVisible(false);

    qDebug() << "=== Packet Response Received ===";
    qDebug() << "Data length:" << data.length();
    qDebug() << "First 200 chars:" << data.left(200);

    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "ERROR: Not a JSON object!";
        return;
    }

    QJsonObject root = doc.object();
    qDebug() << "Root keys:" << root.keys();

    // Check if it's a single packet or an array of packets
    QJsonArray packets;

    if (root.contains("packets") && root["packets"].isArray()) {
        // Format: {"packets": [{...}, {...}]}
        packets = root["packets"].toArray();
        qDebug() << "Found 'packets' array with" << packets.size() << "packets";
    } else if (root.contains("src_ip") && root.contains("dst_ip")) {
        // Format: Single packet object directly
        qDebug() << "Detected single packet format (no 'packets' wrapper)";
        packets.append(root);  // Wrap single packet in array
    } else {
        qWarning() << "ERROR: Unrecognized packet format!";
        qWarning() << "Available keys:" << root.keys();
        return;
    }

    qDebug() << "Number of packets to process:" << packets.size();

    // Process only the latest packets (limit to avoid overload)
    int count = 0;
    for (const QJsonValue &val : packets) {
        if (count >= 5) {
            qDebug() << "Limiting to 5 packets, skipping rest";
            break;
        }

        if (!val.isObject()) {
            qWarning() << "Packet" << count << "is not an object, skipping";
            continue;
        }

        QJsonObject pkt = val.toObject();
        qDebug() << "\n--- Processing packet" << count << "---";
        qDebug() << "Packet keys:" << pkt.keys();

        Packet packet;
        packet.srcIp = pkt["src_ip"].toString();
        packet.dstIp = pkt["dst_ip"].toString();
        packet.srcPort = pkt["src_port"].toInt();
        packet.dstPort = pkt["dst_port"].toInt();
        packet.length = pkt["length"].toInt();
        packet.protocol = pkt["protocol"].toString();
        packet.timestamp = pkt["timestamp"].toDouble();

        qDebug() << "  src:" << packet.srcIp << ":" << packet.srcPort;
        qDebug() << "  dst:" << packet.dstIp << ":" << packet.dstPort;
        qDebug() << "  protocol:" << packet.protocol << "length:" << packet.length;

        // Parse TCP flags
        if (pkt.contains("tcp_flags") && pkt["tcp_flags"].isObject()) {
            QJsonObject flags = pkt["tcp_flags"].toObject();
            packet.synFlag = flags["SYN"].toInt();
            packet.ackFlag = flags["ACK"].toInt();
            packet.finFlag = flags["FIN"].toInt();
            packet.pshFlag = flags["PSH"].toInt();
            packet.rstFlag = flags["RST"].toInt();
            packet.urgFlag = flags["URG"].toInt();
            qDebug() << "  TCP flags - SYN:" << packet.synFlag << "ACK:" << packet.ackFlag;
        } else {
            packet.synFlag = packet.ackFlag = packet.finFlag = 0;
            packet.pshFlag = packet.rstFlag = packet.urgFlag = 0;
            qDebug() << "  No TCP flags found";
        }

        // Calculate duration from last packet with same src/dst pair
        QString pairKey = packet.srcIp + ":" + packet.dstIp;
        if (lastPacketTime.contains(pairKey)) {
            packet.duration = packet.timestamp - lastPacketTime[pairKey];
            qDebug() << "  Duration calculated:" << packet.duration;
        } else {
            packet.duration = 0.1;  // Default duration for first packet
            qDebug() << "  Duration (first packet):" << packet.duration;
        }
        lastPacketTime[pairKey] = packet.timestamp;

        qDebug() << "  Calling analyzePacket()...";
        analyzePacket(packet);
        count++;
    }

    qDebug() << "=== Finished processing" << count << "packets ===\n";
}

void AnomalyWidget::analyzePacket(const Packet &packet)
{
    qDebug() << "\n>>> ANALYZE PACKET CALLED <<<";

    // Store current packet for prediction response
    currentAnalyzingPacket = packet;

    // Handle port 0 (common for ICMP) - use a default port
    int dstPort = packet.dstPort;
    if (dstPort == 0) {
        // For ICMP or packets without ports, use a safe default
        dstPort = 80;  // Use port 80 as default for analysis
        qDebug() << "  Port 0 detected, using default port 80 for analysis";
    }

    // Build prediction request
    QJsonObject predictionRequest;
    predictionRequest["packet_size"] = packet.length;
    predictionRequest["duration"] = qMax(0.1, packet.duration);  // Ensure positive
    predictionRequest["dst_port"] = dstPort;
    predictionRequest["protocol"] = protocolToNumber(packet.protocol);
    predictionRequest["syn_flag"] = packet.synFlag;

    QJsonDocument doc(predictionRequest);
    QString jsonString = doc.toJson(QJsonDocument::Compact);

    qDebug() << "Prediction JSON:" << jsonString;
    qDebug() << "Posting to: http://localhost:5000/predict/malicious-packet";

    // POST to prediction endpoint (note: no /api prefix based on your Flask routes)
    predictionApi->post("http://localhost:5000/predict/malicious-packet", jsonString);

    qDebug() << "POST request sent!\n";
}

void AnomalyWidget::onPredictionReceived(const QString &data)
{
    qDebug() << "\n<<< PREDICTION RESPONSE RECEIVED >>>";
    qDebug() << "Response:" << data;

    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "ERROR: Invalid prediction response (not JSON object):" << data;
        return;
    }

    QJsonObject response = doc.object();
    qDebug() << "Response keys:" << response.keys();

    PredictionResult result;
    result.packet = currentAnalyzingPacket;

    // Parse prediction result (adjust based on your API response format)
    if (response.contains("prediction")) {
        QString pred = response["prediction"].toString();
        int predInt = response["prediction"].toInt(-1);
        result.isMalicious = (pred == "malicious" || pred == "1" || predInt == 1);
        qDebug() << "Prediction field found:" << pred << "isMalicious:" << result.isMalicious;
    } else if (response.contains("is_malicious")) {
        result.isMalicious = response["is_malicious"].toBool();
        qDebug() << "is_malicious field found:" << result.isMalicious;
    } else {
        qWarning() << "ERROR: Unknown response format - no prediction field found!";
        qWarning() << "Available keys:" << response.keys();
        return;
    }

    result.confidence = response.contains("confidence") ?
                            response["confidence"].toDouble() : 0.85;
    qDebug() << "Confidence:" << result.confidence;

    totalAnalyzed++;
    if (result.isMalicious) {
        maliciousCount++;
        qDebug() << "MALICIOUS packet detected!";
    } else {
        benignCount++;
        qDebug() << "Benign packet";
    }

    addPredictionToTable(result);
    updateStatistics();

    qDebug() << "Stats - Total:" << totalAnalyzed << "Malicious:" << maliciousCount << "Benign:" << benignCount;
    qDebug() << "<<< PREDICTION PROCESSING COMPLETE >>>\n";
}

void AnomalyWidget::addPredictionToTable(const PredictionResult &result)
{
    int row = resultsTable->rowCount();
    resultsTable->insertRow(row);

    // Timestamp
    QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(result.packet.timestamp));
    resultsTable->setItem(row, 0, new QTableWidgetItem(dt.toString("hh:mm:ss")));

    // Source
    resultsTable->setItem(row, 1, new QTableWidgetItem(
                                      QString("%1:%2").arg(result.packet.srcIp).arg(result.packet.srcPort)
                                      ));

    // Destination
    resultsTable->setItem(row, 2, new QTableWidgetItem(result.packet.dstIp));

    // Port
    resultsTable->setItem(row, 3, new QTableWidgetItem(QString::number(result.packet.dstPort)));

    // Protocol
    resultsTable->setItem(row, 4, new QTableWidgetItem(result.packet.protocol));

    // Size
    resultsTable->setItem(row, 5, new QTableWidgetItem(QString::number(result.packet.length)));

    // Status
    QString status = result.isMalicious ? "⚠ MALICIOUS" : "✓ Benign";
    QTableWidgetItem *statusItem = new QTableWidgetItem(status);
    statusItem->setForeground(result.isMalicious ? QColor("#e74c3c") : QColor("#2ecc71"));
    statusItem->setFont(QFont("Arial", 10, QFont::Bold));
    resultsTable->setItem(row, 6, statusItem);

    // Confidence
    QString confidence = QString("%1%").arg(result.confidence * 100, 0, 'f', 1);
    resultsTable->setItem(row, 7, new QTableWidgetItem(confidence));

    // Scroll to latest
    resultsTable->scrollToBottom();

    // Keep table size manageable
    if (resultsTable->rowCount() > 100) {
        resultsTable->removeRow(0);
    }
}

void AnomalyWidget::updateStatistics()
{
    totalPacketsLabel->setText(QString::number(totalAnalyzed));
    maliciousCountLabel->setText(QString::number(maliciousCount));
    benignCountLabel->setText(QString::number(benignCount));

    if (totalAnalyzed > 0) {
        double detectionRate = (double)maliciousCount / totalAnalyzed * 100.0;
        accuracyLabel->setText(QString("%1%").arg(detectionRate, 0, 'f', 1));
    } else {
        accuracyLabel->setText("0%");
    }
}

void AnomalyWidget::clearResults()
{
    resultsTable->setRowCount(0);
    totalAnalyzed = 0;
    maliciousCount = 0;
    benignCount = 0;
    updateStatistics();
    statusLabel->setText("🗑 Results cleared");
}

int AnomalyWidget::protocolToNumber(const QString &protocol)
{
    QString proto = protocol.toUpper();
    if (proto == "TCP") return 6;
    if (proto == "UDP") return 17;
    if (proto == "ICMP") return 1;
    if (proto == "IGMP") return 2;
    return 0;  // Unknown
}
