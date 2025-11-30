#include "verbosewidget.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QFont>
#include <QColor>

VerboseWidget::VerboseWidget(QWidget *parent)
    : QWidget(parent),
    m_packetTable(nullptr),
    m_graphWindow(nullptr),
    m_tableTitle(nullptr),
    m_statusLabel(nullptr),
    m_apiCaller(nullptr),
    m_fetchTimer(nullptr),
    m_maxTableRows(100) {

    // Initialize API caller
    m_apiCaller = new ApiCaller(this);
    connect(m_apiCaller, &ApiCaller::responseReceived, this, &VerboseWidget::onApiResponse);
    connect(m_apiCaller, &ApiCaller::errorOccurred, this, &VerboseWidget::onApiError);

    // Initialize timer
    m_fetchTimer = new QTimer(this);
    connect(m_fetchTimer, &QTimer::timeout, this, &VerboseWidget::fetchData);

    setupUI();
}

VerboseWidget::~VerboseWidget() {
    stopMonitoring();
}

void VerboseWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create splitter for resizable sections
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);

    // ===== TOP HALF: Packet Table =====
    QWidget *topWidget = new QWidget(this);
    QVBoxLayout *topLayout = new QVBoxLayout(topWidget);
    topLayout->setContentsMargins(5, 5, 5, 5);

    // Table title
    m_tableTitle = new QLabel("Packet Capture", topWidget);
    QFont titleFont = m_tableTitle->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    m_tableTitle->setFont(titleFont);
    m_tableTitle->setStyleSheet("color: #ffffff; padding: 5px;");

    // Status label
    m_statusLabel = new QLabel("Status: Idle", topWidget);
    m_statusLabel->setStyleSheet("color: #00ff00; padding: 3px; background-color: #2b2b2b;");
    m_statusLabel->setAlignment(Qt::AlignCenter);

    // Packet table
    m_packetTable = new QTableWidget(topWidget);
    m_packetTable->setColumnCount(4);
    m_packetTable->setHorizontalHeaderLabels({"Packet Type", "Destination IP", "Packet Length", "Protocol"});

    // Table styling
    m_packetTable->setStyleSheet(
        "QTableWidget {"
        "   background-color: #2b2b2b;"
        "   color: #ffffff;"
        "   gridline-color: #404040;"
        "   border: 1px solid #404040;"
        "}"
        "QTableWidget::item {"
        "   padding: 5px;"
        "   border-bottom: 1px solid #404040;"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: #0078d7;"
        "}"
        "QHeaderView::section {"
        "   background-color: #1e1e1e;"
        "   color: #ffffff;"
        "   padding: 8px;"
        "   border: 1px solid #404040;"
        "   font-weight: bold;"
        "}"
        );

    // Table properties
    m_packetTable->setAlternatingRowColors(true);
    m_packetTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_packetTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_packetTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_packetTable->horizontalHeader()->setStretchLastSection(true);
    m_packetTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_packetTable->verticalHeader()->setVisible(false);

    // Set column widths
    m_packetTable->setColumnWidth(0, 150);  // Packet Type
    m_packetTable->setColumnWidth(1, 200);  // Destination IP
    m_packetTable->setColumnWidth(2, 150);  // Packet Length
    m_packetTable->setColumnWidth(3, 100);  // Protocol

    topLayout->addWidget(m_tableTitle);
    topLayout->addWidget(m_statusLabel);
    topLayout->addWidget(m_packetTable);

    // ===== BOTTOM HALF: Line Graph =====
    m_graphWindow = new LineGraphWindow(this);

    // Add widgets to splitter
    splitter->addWidget(topWidget);
    splitter->addWidget(m_graphWindow);

    // Set initial sizes (50-50 split)
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter);

    setStyleSheet("background-color: #1e1e1e;");
}

void VerboseWidget::addPacketToTable(const PacketData &packet) {
    // Insert row at the top (most recent first)
    m_packetTable->insertRow(0);

    // Packet Type
    QString packetType = getPacketType(packet);
    QTableWidgetItem *typeItem = new QTableWidgetItem(packetType);

    // Color code by packet type
    if (packetType.contains("SYN")) {
        typeItem->setForeground(QColor(100, 200, 255));  // Light blue
    } else if (packetType.contains("ACK")) {
        typeItem->setForeground(QColor(100, 255, 100));  // Light green
    } else if (packetType.contains("FIN") || packetType.contains("RST")) {
        typeItem->setForeground(QColor(255, 100, 100));  // Light red
    } else {
        typeItem->setForeground(QColor(200, 200, 200));  // Light gray
    }
    m_packetTable->setItem(0, 0, typeItem);

    // Destination IP
    QTableWidgetItem *ipItem = new QTableWidgetItem(packet.dst_ip);
    ipItem->setForeground(QColor(200, 200, 200));
    m_packetTable->setItem(0, 1, ipItem);

    // Packet Length
    QTableWidgetItem *lengthItem = new QTableWidgetItem(QString::number(packet.length));
    lengthItem->setForeground(QColor(200, 200, 200));
    lengthItem->setTextAlignment(Qt::AlignCenter);
    m_packetTable->setItem(0, 2, lengthItem);

    // Protocol
    QTableWidgetItem *protocolItem = new QTableWidgetItem(packet.protocol);

    // Color code by protocol
    if (packet.protocol == "TCP") {
        protocolItem->setForeground(QColor(100, 200, 255));
    } else if (packet.protocol == "UDP") {
        protocolItem->setForeground(QColor(255, 200, 100));
    } else if (packet.protocol == "ICMP") {
        protocolItem->setForeground(QColor(255, 100, 255));
    } else {
        protocolItem->setForeground(QColor(200, 200, 200));
    }
    protocolItem->setTextAlignment(Qt::AlignCenter);
    m_packetTable->setItem(0, 3, protocolItem);

    // Limit table rows
    while (m_packetTable->rowCount() > m_maxTableRows) {
        m_packetTable->removeRow(m_packetTable->rowCount() - 1);
    }

    // Also add to graph
    m_graphWindow->addPacketData(packet);
}

QString VerboseWidget::getPacketType(const PacketData &packet) {
    QStringList flags;

    if (packet.tcp_flags.SYN) flags << "SYN";
    if (packet.tcp_flags.ACK) flags << "ACK";
    if (packet.tcp_flags.FIN) flags << "FIN";
    if (packet.tcp_flags.RST) flags << "RST";
    if (packet.tcp_flags.PSH) flags << "PSH";
    if (packet.tcp_flags.URG) flags << "URG";

    if (flags.isEmpty()) {
        return packet.protocol;
    } else {
        return packet.protocol + " [" + flags.join(",") + "]";
    }
}

void VerboseWidget::startMonitoring(const QString &apiUrl, int intervalMs) {
    m_apiUrl = apiUrl;

    // Fetch immediately
    fetchData();

    // Start periodic fetching
    m_fetchTimer->start(intervalMs);

    m_statusLabel->setText(QString("Status: Monitoring (fetch every %1ms)").arg(intervalMs));
    m_statusLabel->setStyleSheet("color: #00ff00; padding: 3px; background-color: #2b2b2b;");
}

void VerboseWidget::stopMonitoring() {
    if (m_fetchTimer && m_fetchTimer->isActive()) {
        m_fetchTimer->stop();
        m_statusLabel->setText("Status: Stopped");
        m_statusLabel->setStyleSheet("color: #ff8800; padding: 3px; background-color: #2b2b2b;");
    }
}

void VerboseWidget::clearAllData() {
    m_packetTable->setRowCount(0);
    m_graphWindow->clearData();
}

void VerboseWidget::fetchData() {
    if (!m_apiUrl.isEmpty() && m_apiCaller) {
        m_apiCaller->get(m_apiUrl);
        m_statusLabel->setText("Status: Fetching...");
        m_statusLabel->setStyleSheet("color: #ffff00; padding: 3px; background-color: #2b2b2b;");
    }
}

void VerboseWidget::onApiResponse(const QString& response) {
    m_statusLabel->setText("Status: Connected");
    m_statusLabel->setStyleSheet("color: #00ff00; padding: 3px; background-color: #2b2b2b;");

    parseAndDisplayPackets(response);
}

void VerboseWidget::onApiError(const QString& error) {
    m_statusLabel->setText(QString("Status: Error - %1").arg(error));
    m_statusLabel->setStyleSheet("color: #ff0000; padding: 3px; background-color: #2b2b2b;");

    qDebug() << "API Error:" << error;
}

void VerboseWidget::parseAndDisplayPackets(const QString &jsonData) {
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());

    if (!doc.isObject()) {
        qDebug() << "Invalid JSON format";
        return;
    }

    QJsonObject root = doc.object();

    // Handle single packet
    if (root.contains("src_ip") && root.contains("dst_ip")) {
        PacketData packet;
        packet.src_ip = root["src_ip"].toString();
        packet.src_port = root["src_port"].toInt();
        packet.dst_ip = root["dst_ip"].toString();
        packet.dst_port = root["dst_port"].toInt();
        packet.length = root["length"].toInt();
        packet.protocol = root["protocol"].toString();
        packet.timestamp = root["timestamp"].toDouble();

        if (root.contains("tcp_flags") && root["tcp_flags"].isObject()) {
            QJsonObject flags = root["tcp_flags"].toObject();
            packet.tcp_flags.ACK = flags["ACK"].toInt();
            packet.tcp_flags.FIN = flags["FIN"].toInt();
            packet.tcp_flags.PSH = flags["PSH"].toInt();
            packet.tcp_flags.RST = flags["RST"].toInt();
            packet.tcp_flags.SYN = flags["SYN"].toInt();
            packet.tcp_flags.URG = flags["URG"].toInt();
        }

        addPacketToTable(packet);
        return;
    }

    // Handle packet array
    if (root.contains("packets") && root["packets"].isArray()) {
        QJsonArray packetsArray = root["packets"].toArray();

        for (const auto &val : packetsArray) {
            if (!val.isObject()) continue;

            QJsonObject packetObj = val.toObject();

            PacketData packet;
            packet.src_ip = packetObj["src_ip"].toString();
            packet.src_port = packetObj["src_port"].toInt();
            packet.dst_ip = packetObj["dst_ip"].toString();
            packet.dst_port = packetObj["dst_port"].toInt();
            packet.length = packetObj["length"].toInt();
            packet.protocol = packetObj["protocol"].toString();
            packet.timestamp = packetObj["timestamp"].toDouble();

            if (packetObj.contains("tcp_flags") && packetObj["tcp_flags"].isObject()) {
                QJsonObject flags = packetObj["tcp_flags"].toObject();
                packet.tcp_flags.ACK = flags["ACK"].toInt();
                packet.tcp_flags.FIN = flags["FIN"].toInt();
                packet.tcp_flags.PSH = flags["PSH"].toInt();
                packet.tcp_flags.RST = flags["RST"].toInt();
                packet.tcp_flags.SYN = flags["SYN"].toInt();
                packet.tcp_flags.URG = flags["URG"].toInt();
            }

            addPacketToTable(packet);
        }
    }
}
