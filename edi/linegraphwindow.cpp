#include "linegraphwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <algorithm>
#include <cmath>

// GraphWidget Implementation
GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent), m_currentMetric("length") {
    setMinimumSize(600, 400);
    setStyleSheet("background-color: #2b2b2b;");
}

void GraphWidget::setData(const QVector<PacketData> &data) {
    m_data = data;
    update();
}

void GraphWidget::setMetric(const QString &metric) {
    m_currentMetric = metric;
    update();
}

void GraphWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawGraph(painter);
}

void GraphWidget::drawGraph(QPainter &painter) {
    if (m_data.isEmpty()) {
        painter.setPen(QPen(QColor(150, 150, 150), 1));
        painter.drawText(rect(), Qt::AlignCenter, "No data available");
        return;
    }

    drawGridLines(painter);
    drawAxes(painter);
    drawDataPoints(painter);
}

void GraphWidget::drawAxes(QPainter &painter) {
    int margin = 60;
    int graphWidth = width() - 2 * margin;
    int graphHeight = height() - 2 * margin;

    // Draw axes
    painter.setPen(QPen(QColor(200, 200, 200), 2));
    painter.drawLine(margin, height() - margin, width() - margin, height() - margin); // X-axis
    painter.drawLine(margin, margin, margin, height() - margin); // Y-axis

    // Draw labels
    painter.setPen(QColor(180, 180, 180));
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);

    // Y-axis label
    painter.save();
    painter.translate(15, height() / 2);
    painter.rotate(-90);
    painter.drawText(0, 0, getMetricLabel());
    painter.restore();

    // X-axis label
    painter.drawText(width() / 2 - 50, height() - 15, "Time");

    // Draw Y-axis values
    if (!m_data.isEmpty()) {
        double maxVal = 0;
        for (const auto &packet : m_data) {
            double val = getValueForMetric(packet);
            if (val > maxVal) maxVal = val;
        }

        if (maxVal > 0) {
            int numTicks = 5;
            for (int i = 0; i <= numTicks; i++) {
                double value = (maxVal / numTicks) * i;
                int y = height() - margin - (graphHeight * i / numTicks);
                painter.drawText(5, y + 5, QString::number(value, 'f', 0));
            }
        }
    }
}

void GraphWidget::drawGridLines(QPainter &painter) {
    int margin = 60;
    int graphHeight = height() - 2 * margin;

    painter.setPen(QPen(QColor(60, 60, 60), 1, Qt::DotLine));

    // Horizontal grid lines
    int numLines = 5;
    for (int i = 0; i <= numLines; i++) {
        int y = margin + (graphHeight * i / numLines);
        painter.drawLine(margin, y, width() - margin, y);
    }
}

void GraphWidget::drawDataPoints(QPainter &painter) {
    if (m_data.size() < 2) return;

    int margin = 60;
    int graphWidth = width() - 2 * margin;
    int graphHeight = height() - 2 * margin;

    // Find max value for scaling
    double maxVal = 0;
    for (const auto &packet : m_data) {
        double val = getValueForMetric(packet);
        if (val > maxVal) maxVal = val;
    }

    if (maxVal == 0) maxVal = 1;

    // Draw line
    painter.setPen(QPen(QColor(100, 200, 255), 2));
    painter.setBrush(QColor(100, 200, 255));

    for (int i = 0; i < m_data.size() - 1; i++) {
        double val1 = getValueForMetric(m_data[i]);
        double val2 = getValueForMetric(m_data[i + 1]);

        int x1 = margin + (graphWidth * i / (m_data.size() - 1));
        int y1 = height() - margin - (graphHeight * val1 / maxVal);

        int x2 = margin + (graphWidth * (i + 1) / (m_data.size() - 1));
        int y2 = height() - margin - (graphHeight * val2 / maxVal);

        painter.drawLine(x1, y1, x2, y2);
        painter.drawEllipse(QPoint(x1, y1), 3, 3);
    }

    // Draw last point
    int lastIdx = m_data.size() - 1;
    double lastVal = getValueForMetric(m_data[lastIdx]);
    int lastX = width() - margin;
    int lastY = height() - margin - (graphHeight * lastVal / maxVal);
    painter.drawEllipse(QPoint(lastX, lastY), 3, 3);
}

double GraphWidget::getValueForMetric(const PacketData &packet) {
    if (m_currentMetric == "length") return packet.length;
    if (m_currentMetric == "dst_port") return packet.dst_port;
    if (m_currentMetric == "src_port") return packet.src_port;
    if (m_currentMetric == "tcp_ack") return packet.tcp_flags.ACK;
    if (m_currentMetric == "tcp_syn") return packet.tcp_flags.SYN;
    if (m_currentMetric == "tcp_fin") return packet.tcp_flags.FIN;
    if (m_currentMetric == "tcp_psh") return packet.tcp_flags.PSH;
    return 0;
}

QString GraphWidget::getMetricLabel() {
    if (m_currentMetric == "length") return "Packet Count";
    if (m_currentMetric == "dst_port") return "Destination Port";
    if (m_currentMetric == "src_port") return "Source Port";
    if (m_currentMetric == "tcp_ack") return "TCP ACK Flag";
    if (m_currentMetric == "tcp_syn") return "TCP SYN Flag";
    if (m_currentMetric == "tcp_fin") return "TCP FIN Flag";
    if (m_currentMetric == "tcp_psh") return "TCP PSH Flag";
    return "Value";
}

// LineGraphWindow Implementation
LineGraphWindow::LineGraphWindow(QWidget *parent)
    : QWidget(parent),
    m_maxDataPoints(100),
    m_apiCaller(nullptr),
    m_fetchTimer(nullptr),
    m_graphWidget(nullptr),
    m_metricSelector(nullptr),
    m_statsLabel(nullptr),
    m_titleLabel(nullptr),
    m_statusLabel(nullptr) {

    // Initialize API caller
    m_apiCaller = new ApiCaller(this);
    connect(m_apiCaller, &ApiCaller::responseReceived, this, &LineGraphWindow::onApiResponse);
    connect(m_apiCaller, &ApiCaller::errorOccurred, this, &LineGraphWindow::onApiError);

    // Initialize timer
    m_fetchTimer = new QTimer(this);
    connect(m_fetchTimer, &QTimer::timeout, this, &LineGraphWindow::fetchPacketData);

    setupUI();
    setWindowTitle("Network Packet Visualization");
    resize(800, 600);
}

LineGraphWindow::~LineGraphWindow() {
    stopPeriodicFetch();
}

void LineGraphWindow::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Title
    m_titleLabel = new QLabel("Network Packet Data - Live Stream", this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("color: #ffffff; padding: 10px;");

    // Metric selector
    QHBoxLayout *controlLayout = new QHBoxLayout();
    QLabel *metricLabel = new QLabel("Metric:", this);
    metricLabel->setStyleSheet("color: #ffffff;");

    m_metricSelector = new QComboBox(this);
    m_metricSelector->addItem("Packet Count", "length");
    m_metricSelector->addItem("Destination Port", "dst_port");
    m_metricSelector->addItem("Source Port", "src_port");
    m_metricSelector->addItem("TCP ACK Flag", "tcp_ack");
    m_metricSelector->addItem("TCP SYN Flag", "tcp_syn");
    m_metricSelector->addItem("TCP FIN Flag", "tcp_fin");
    m_metricSelector->addItem("TCP PSH Flag", "tcp_psh");
    m_metricSelector->setStyleSheet(
        "QComboBox { background-color: #3b3b3b; color: #ffffff; padding: 5px; }"
        "QComboBox::drop-down { border: none; }"
        );

    controlLayout->addWidget(metricLabel);
    controlLayout->addWidget(m_metricSelector);
    controlLayout->addStretch();

    // Graph widget
    m_graphWidget = new GraphWidget(this);

    // Statistics label
    m_statsLabel = new QLabel("Statistics: No data", this);
    m_statsLabel->setStyleSheet("color: #ffffff; padding: 10px; background-color: #3b3b3b;");

    // Status label
    m_statusLabel = new QLabel("Status: Idle", this);
    m_statusLabel->setStyleSheet("color: #00ff00; padding: 5px; background-color: #2b2b2b;");
    m_statusLabel->setAlignment(Qt::AlignCenter);

    // Add to main layout
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_graphWidget, 1);
    mainLayout->addWidget(m_statsLabel);

    setStyleSheet("background-color: #1e1e1e;");

    // Connect signals
    connect(m_metricSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LineGraphWindow::onMetricChanged);
}

void LineGraphWindow::addPacketData(const PacketData &packet) {
    m_packetData.append(packet);

    // Limit data points
    if (m_packetData.size() > m_maxDataPoints) {
        m_packetData.removeFirst();
    }

    updateGraph();
    updateStatistics();
}

void LineGraphWindow::clearData() {
    m_packetData.clear();
    updateGraph();
    updateStatistics();
}

void LineGraphWindow::setMaxDataPoints(int max) {
    m_maxDataPoints = max;
}

void LineGraphWindow::onMetricChanged(int index) {
    QString metric = m_metricSelector->currentData().toString();
    m_graphWidget->setMetric(metric);
    updateStatistics();
}

void LineGraphWindow::updateGraph() {
    m_graphWidget->setData(m_packetData);
}

void LineGraphWindow::updateStatistics() {
    if (m_packetData.isEmpty()) {
        m_statsLabel->setText("Statistics: No data");
        return;
    }

    QString metric = m_metricSelector->currentData().toString();
    QVector<double> values;

    for (const auto &packet : m_packetData) {
        if (metric == "length") values.append(packet.length);
        else if (metric == "dst_port") values.append(packet.dst_port);
        else if (metric == "src_port") values.append(packet.src_port);
        else if (metric == "tcp_ack") values.append(packet.tcp_flags.ACK);
        else if (metric == "tcp_syn") values.append(packet.tcp_flags.SYN);
        else if (metric == "tcp_fin") values.append(packet.tcp_flags.FIN);
        else if (metric == "tcp_psh") values.append(packet.tcp_flags.PSH);
    }

    if (values.isEmpty()) {
        m_statsLabel->setText("Statistics: No data");
        return;
    }

    double sum = 0;
    double min = values[0];
    double max = values[0];

    for (double val : values) {
        sum += val;
        if (val < min) min = val;
        if (val > max) max = val;
    }

    double avg = sum / values.size();

    QString stats = QString("Total Packets: %1 | Min: %2 | Max: %3 | Avg: %4")
                        .arg(m_packetData.size())
                        .arg(min, 0, 'f', 2)
                        .arg(max, 0, 'f', 2)
                        .arg(avg, 0, 'f', 2);

    m_statsLabel->setText(stats);
}

void LineGraphWindow::startPeriodicFetch(const QString &apiUrl, int intervalMs) {
    m_apiUrl = apiUrl;

    // Fetch immediately
    fetchPacketData();

    // Start periodic fetching
    m_fetchTimer->start(intervalMs);

    m_statusLabel->setText(QString("Status: Fetching every %1ms").arg(intervalMs));
    m_statusLabel->setStyleSheet("color: #00ff00; padding: 5px; background-color: #2b2b2b;");
}

void LineGraphWindow::stopPeriodicFetch() {
    if (m_fetchTimer && m_fetchTimer->isActive()) {
        m_fetchTimer->stop();
        m_statusLabel->setText("Status: Stopped");
        m_statusLabel->setStyleSheet("color: #ff8800; padding: 5px; background-color: #2b2b2b;");
    }
}

void LineGraphWindow::fetchPacketData() {
    if (!m_apiUrl.isEmpty() && m_apiCaller) {
        m_apiCaller->get(m_apiUrl);
        m_statusLabel->setText("Status: Fetching...");
        m_statusLabel->setStyleSheet("color: #ffff00; padding: 5px; background-color: #2b2b2b;");
    }
}

void LineGraphWindow::onApiResponse(const QString& response) {
    m_statusLabel->setText("Status: Connected");
    m_statusLabel->setStyleSheet("color: #00ff00; padding: 5px; background-color: #2b2b2b;");

    parsePacketJson(response);
}

void LineGraphWindow::onApiError(const QString& error) {
    m_statusLabel->setText(QString("Status: Error - %1").arg(error));
    m_statusLabel->setStyleSheet("color: #ff0000; padding: 5px; background-color: #2b2b2b;");

    qDebug() << "API Error:" << error;
}

void LineGraphWindow::parsePacketJson(const QString &jsonData) {
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());

    if (!doc.isObject()) {
        qDebug() << "Invalid JSON format - expected object";
        return;
    }

    QJsonObject root = doc.object();

    // Check if this is a single packet response (from /api/packets/stream)
    if (root.contains("src_ip") && root.contains("dst_ip")) {
        PacketData packet;
        packet.src_ip = root["src_ip"].toString();
        packet.src_port = root["src_port"].toInt();
        packet.dst_ip = root["dst_ip"].toString();
        packet.dst_port = root["dst_port"].toInt();
        packet.length = root["length"].toInt();
        packet.protocol = root["protocol"].toString();
        packet.timestamp = root["timestamp"].toDouble();

        // Parse TCP flags
        if (root.contains("tcp_flags") && root["tcp_flags"].isObject()) {
            QJsonObject flags = root["tcp_flags"].toObject();
            packet.tcp_flags.ACK = flags["ACK"].toInt();
            packet.tcp_flags.FIN = flags["FIN"].toInt();
            packet.tcp_flags.PSH = flags["PSH"].toInt();
            packet.tcp_flags.RST = flags["RST"].toInt();
            packet.tcp_flags.SYN = flags["SYN"].toInt();
            packet.tcp_flags.URG = flags["URG"].toInt();
        }

        addPacketData(packet);
        qDebug() << "Parsed single packet from" << packet.src_ip << "to" << packet.dst_ip;
        return;
    }

    // Check if this is a packets array response (from /api/packets/recent)
    if (root.contains("packets") && root["packets"].isArray()) {
        QJsonArray packetsArray = root["packets"].toArray();

        qDebug() << "Parsing" << packetsArray.size() << "packets";

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

            // Parse TCP flags
            if (packetObj.contains("tcp_flags") && packetObj["tcp_flags"].isObject()) {
                QJsonObject flags = packetObj["tcp_flags"].toObject();
                packet.tcp_flags.ACK = flags["ACK"].toInt();
                packet.tcp_flags.FIN = flags["FIN"].toInt();
                packet.tcp_flags.PSH = flags["PSH"].toInt();
                packet.tcp_flags.RST = flags["RST"].toInt();
                packet.tcp_flags.SYN = flags["SYN"].toInt();
                packet.tcp_flags.URG = flags["URG"].toInt();
            }

            addPacketData(packet);
        }

        qDebug() << "Successfully parsed" << packetsArray.size() << "packets";
        return;
    }

    qDebug() << "Unknown JSON format";
}
