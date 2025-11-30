#include "graphWindow.h"
#include "zoomgraphicsview.h"
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QtMath>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QPainterPath>
#include <QGraphicsPathItem>
#include <QGroupBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

GraphWindow::GraphWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentTopology(CIRCLE_TOPOLOGY)
    , showTraceRoute(false)
{
    // Create central widget with layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Create stats panel
    statsPanel = new QWidget(this);
    QHBoxLayout *statsLayout = new QHBoxLayout(statsPanel);
    statsPanel->setStyleSheet(
        "QWidget {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 rgba(102, 126, 234, 180), stop:1 rgba(118, 75, 162, 180));"
        "  border-radius: 15px;"
        "  padding: 15px;"
        "}"
        );

    // Create stat items
    auto createStatItem = [](const QString &label, QLabel *&valueLabel) {
        QWidget *item = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(item);
        layout->setAlignment(Qt::AlignCenter);

        valueLabel = new QLabel("0");
        valueLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: white;");
        valueLabel->setAlignment(Qt::AlignCenter);

        QLabel *labelWidget = new QLabel(label);
        labelWidget->setStyleSheet("font-size: 14px; color: rgba(255, 255, 255, 200);");
        labelWidget->setAlignment(Qt::AlignCenter);

        layout->addWidget(valueLabel);
        layout->addWidget(labelWidget);
        return item;
    };

    statsLayout->addWidget(createStatItem("Nodes", nodeCountLabel));
    statsLayout->addWidget(createStatItem("Connections", edgeCountLabel));
    statsLayout->addWidget(createStatItem("Traced Paths", pathCountLabel));
    statsPanel->setLayout(statsLayout);

    // Create control panel
    controlPanel = new QWidget(this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlPanel);
    controlPanel->setStyleSheet(
        "QWidget { background: rgba(255, 255, 255, 230); border-radius: 10px; padding: 10px; }"
        );

    topologyButton = new QPushButton("Toggle Layout", this);
    resetButton = new QPushButton("Reset Zoom", this);
    refreshButton = new QPushButton("Refresh Data", this);
    traceRouteCheckbox = new QCheckBox("Show Trace Routes", this);

    QString buttonStyle =
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #667eea, stop:1 #764ba2);"
        "  color: white;"
        "  border: none;"
        "  padding: 10px 20px;"
        "  border-radius: 20px;"
        "  font-weight: bold;"
        "  min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #7c93f5, stop:1 #8a5cb8);"
        "}"
        "QPushButton:pressed {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #5568d3, stop:1 #653a8f);"
        "}";

    topologyButton->setStyleSheet(buttonStyle);
    resetButton->setStyleSheet(buttonStyle);
    refreshButton->setStyleSheet(buttonStyle);
    traceRouteCheckbox->setStyleSheet(
        "QCheckBox { color: #333; font-weight: bold; spacing: 8px; }"
        );

    controlLayout->addWidget(topologyButton);
    controlLayout->addWidget(resetButton);
    controlLayout->addWidget(refreshButton);
    controlLayout->addWidget(traceRouteCheckbox);
    controlLayout->addStretch();

    // Connection status indicator
    statusLabel = new QLabel("● Connecting...");
    statusLabel->setStyleSheet(
        "QLabel { "
        "  background: rgba(0, 0, 0, 180); "
        "  color: #FFA726; "
        "  padding: 8px 15px; "
        "  border-radius: 15px; "
        "  font-weight: bold;"
        "}"
        );
    controlLayout->addWidget(statusLabel);
    controlPanel->setLayout(controlLayout);

    // Create scene and view
    scene = new QGraphicsScene(this);
    view = new ZoomGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    view->setBackgroundBrush(QBrush(QColor(21, 25, 64)));
    view->setStyleSheet(
        "QGraphicsView {"
        "  border: 2px solid rgba(102, 126, 234, 100);"
        "  border-radius: 15px;"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(102, 126, 234, 30), stop:1 rgba(118, 75, 162, 30));"
        "}"
        );

    // Create legend panel
    legendPanel = new QWidget(this);
    QVBoxLayout *legendLayout = new QVBoxLayout(legendPanel);
    legendPanel->setStyleSheet(
        "QWidget {"
        "  background: rgba(0, 0, 0, 200);"
        "  border-radius: 10px;"
        "  padding: 15px;"
        "}"
        "QLabel { color: white; font-size: 11px; }"
        );

    auto createLegendItem = [](const QColor &color, const QString &text, bool isLine = false) {
        QWidget *item = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(item);
        layout->setContentsMargins(0, 2, 0, 2);

        QLabel *colorBox = new QLabel();
        if (isLine) {
            colorBox->setFixedSize(20, 3);
            colorBox->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        } else {
            colorBox->setFixedSize(15, 15);
            colorBox->setStyleSheet(QString("background-color: %1; border-radius: 7px;").arg(color.name()));
        }

        QLabel *label = new QLabel(text);
        layout->addWidget(colorBox);
        layout->addWidget(label);
        layout->addStretch();
        return item;
    };

    QLabel *legendTitle = new QLabel("Legend");
    legendTitle->setStyleSheet("font-weight: bold; font-size: 12px; color: white; margin-bottom: 5px;");
    legendLayout->addWidget(legendTitle);
    legendLayout->addWidget(createLegendItem(QColor(255, 107, 107), "Local Machine"));
    legendLayout->addWidget(createLegendItem(QColor(78, 205, 196), "Routers"));
    legendLayout->addWidget(createLegendItem(QColor(69, 183, 209), "Destinations"));
    legendLayout->addWidget(createLegendItem(QColor(150, 206, 180), "Direct Connection", true));
    legendLayout->addWidget(createLegendItem(QColor(254, 202, 87), "Traceroute Path", true));
    legendLayout->addStretch();
    legendPanel->setLayout(legendLayout);
    legendPanel->setFixedWidth(180);

    // Position legend on top of view
    legendPanel->setParent(view);
    legendPanel->move(20, 20);
    legendPanel->raise();

    // Add widgets to layout
    mainLayout->addWidget(statsPanel);
    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(view, 1);

    setCentralWidget(centralWidget);
    setWindowTitle("🌐 Network Topology Visualizer");
    resize(1400, 900);

    // Set main window style
    setStyleSheet(
        "QMainWindow {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 #667eea, stop:1 #764ba2);"
        "}"
        );

    // Initialize API caller
    api = new ApiCaller(this);
    connect(api, &ApiCaller::responseReceived, this, &GraphWindow::onApiResponse);

    // Setup polling timer for periodic updates
    pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, this, &GraphWindow::fetchGraphData);

    // Connect button signals
    connect(topologyButton, &QPushButton::clicked, this, &GraphWindow::toggleTopology);
    connect(resetButton, &QPushButton::clicked, this, &GraphWindow::resetView);
    connect(refreshButton, &QPushButton::clicked, this, &GraphWindow::refreshData);
    connect(traceRouteCheckbox, &QCheckBox::toggled, this, &GraphWindow::toggleTraceRoute);
}

void GraphWindow::initialize()
{
    // Initial fetch
    fetchGraphData();

    // Start polling every 3 seconds (3000ms)
    pollTimer->start(3000);

    updateConnectionStatus(true);
}

void GraphWindow::fetchGraphData()
{
    api->get("http://localhost:5000/api/graph");
}

void GraphWindow::onApiResponse(const QString &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "Unexpected format, got non-object JSON";
        updateConnectionStatus(false);
        return;
    }

    QJsonObject root = doc.object();

    // Parse nodes if available
    if (root.contains("nodes") && root["nodes"].isArray()) {
        QJsonArray nodesArray = root["nodes"].toArray();
        for (const auto &val : nodesArray) {
            if (!val.isObject()) continue;
            QJsonObject obj = val.toObject();

            NodeData node;
            node.id = obj["id"].toString();
            node.ip = obj["ip"].toString();
            node.type = obj["type"].toString();
            node.isLocal = obj["is_local"].toBool();
            node.packetCount = obj["packet_count"].toInt();

            if (obj["protocols"].isArray()) {
                for (auto p : obj["protocols"].toArray())
                    node.protocols.append(p.toString());
            }

            nodeDataMap[node.id] = node;
        }
    }

    // Parse edges
    if (!root.contains("edges") || !root["edges"].isArray()) {
        qWarning() << "Missing 'edges' array in response";
        updateConnectionStatus(false);
        return;
    }

    QJsonArray arr = root["edges"].toArray();
    adjacency.clear();
    edges.clear();

    for (const auto &val : arr) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        Edge e;
        e.source = obj["source"].toString();
        e.target = obj["target"].toString();
        e.type   = obj["type"].toString();
        e.packetCount = obj["packet_count"].toInt();
        e.ageSeconds  = obj["age_seconds"].toDouble();
        e.firstSeen   = obj["first_seen"].toDouble();
        e.lastSeen    = obj["last_seen"].toDouble();

        if (obj["protocols"].isArray()) {
            for (auto p : obj["protocols"].toArray())
                e.protocols.append(p.toString());
        }

        addEdge(e);
        updateNodeData(e.source);
        updateNodeData(e.target);
    }

    qDebug() << "Parsed edges:" << edges.size() << "nodes:" << nodeDataMap.size();
    updateStatistics();
    drawGraph();
    updateConnectionStatus(true);
}

void GraphWindow::addEdge(const Edge &edge)
{
    adjacency[edge.source].insert(edge.target);
    edges.append(edge);
}

void GraphWindow::updateNodeData(const QString &nodeId)
{
    if (!nodeDataMap.contains(nodeId)) {
        NodeData node;
        node.id = nodeId;
        node.ip = nodeId;
        node.type = "destination";
        node.isLocal = false;
        node.packetCount = 0;
        nodeDataMap[nodeId] = node;
    }
}

void GraphWindow::toggleTopology()
{
    if (currentTopology == CIRCLE_TOPOLOGY) {
        currentTopology = RADIAL_TOPOLOGY;

        QTimer::singleShot(0, this, [this]() {
            applyRadialLayout();
        });
    } else if (currentTopology == RADIAL_TOPOLOGY) {
        currentTopology = FREE_TOPOLOGY;

        QTimer::singleShot(0, this, [this]() {
            applyForceDirectedLayout();
        });
    } else {
        currentTopology = CIRCLE_TOPOLOGY;

        QTimer::singleShot(0, this, [this]() {
            drawGraph();
        });
    }
}

void GraphWindow::toggleTraceRoute(bool enabled)
{
    showTraceRoute = enabled;
    drawGraph();
}

void GraphWindow::resetView()
{
    view->resetZoom();
}

void GraphWindow::refreshData()
{
    fetchGraphData();
}

void GraphWindow::updateStatistics()
{
    nodeCountLabel->setText(QString::number(nodeDataMap.size()));
    edgeCountLabel->setText(QString::number(edges.size()));

    // Count traceroute paths (edges with type != "direct")
    int pathCount = 0;
    for (const Edge &e : edges) {
        if (e.type != "direct") pathCount++;
    }
    pathCountLabel->setText(QString::number(pathCount));
}

void GraphWindow::updateConnectionStatus(bool connected)
{
    if (connected) {
        statusLabel->setText("● Connected");
        statusLabel->setStyleSheet(
            "QLabel { "
            "  background: rgba(0, 0, 0, 180); "
            "  color: #4CAF50; "
            "  padding: 8px 15px; "
            "  border-radius: 15px; "
            "  font-weight: bold;"
            "}"
            );
    } else {
        statusLabel->setText("● Disconnected");
        statusLabel->setStyleSheet(
            "QLabel { "
            "  background: rgba(0, 0, 0, 180); "
            "  color: #f44336; "
            "  padding: 8px 15px; "
            "  border-radius: 15px; "
            "  font-weight: bold;"
            "}"
            );
    }
}

void GraphWindow::applyRadialLayout()
{
    QStringList nodes = adjacency.keys();
    if (nodes.isEmpty()) return;

    double centerX = view->width() / 2;
    double centerY = view->height() / 2;

    // Categorize nodes
    QStringList localNodes, routerNodes, otherNodes;
    for (const QString &node : nodes) {
        if (nodeDataMap.contains(node)) {
            if (nodeDataMap[node].isLocal) {
                localNodes.append(node);
            } else if (nodeDataMap[node].type == "router") {
                routerNodes.append(node);
            } else {
                otherNodes.append(node);
            }
        } else {
            otherNodes.append(node);
        }
    }

    // Position local nodes in center
    for (int i = 0; i < localNodes.size(); ++i) {
        nodePositions[localNodes[i]] = QPointF(centerX, centerY);
    }

    // Position routers in inner circle
    double innerRadius = 150;
    for (int i = 0; i < routerNodes.size(); ++i) {
        double angle = (2 * M_PI * i) / qMax(routerNodes.size(), 1);
        nodePositions[routerNodes[i]] = QPointF(
            centerX + innerRadius * qCos(angle),
            centerY + innerRadius * qSin(angle)
            );
    }

    // Position other nodes in outer circle
    double outerRadius = 250;
    for (int i = 0; i < otherNodes.size(); ++i) {
        double angle = (2 * M_PI * i) / qMax(otherNodes.size(), 1);
        nodePositions[otherNodes[i]] = QPointF(
            centerX + outerRadius * qCos(angle),
            centerY + outerRadius * qSin(angle)
            );
    }

    drawGraph();
}

void GraphWindow::applyForceDirectedLayout()
{
    QStringList nodes = adjacency.keys();
    if (nodes.isEmpty()) return;

    // Initialize random positions if not already set
    for (const QString &node : nodes) {
        if (!nodePositions.contains(node)) {
            nodePositions[node] = QPointF(
                rand() % 600 + 100,
                rand() % 400 + 100
                );
        }
    }

    // Run force-directed algorithm iterations
    const int iterations = 100;
    for (int iter = 0; iter < iterations; ++iter) {
        QMap<QString, QPointF> forces;

        // Initialize forces
        for (const QString &node : nodes) {
            forces[node] = QPointF(0, 0);
        }

        // Repulsion between all nodes
        for (int i = 0; i < nodes.size(); ++i) {
            for (int j = i + 1; j < nodes.size(); ++j) {
                QPointF delta = nodePositions[nodes[i]] - nodePositions[nodes[j]];
                double distance = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
                if (distance < 1) distance = 1;

                double force = REPULSION_STRENGTH / (distance * distance);
                QPointF direction = delta / distance;

                forces[nodes[i]] += direction * force;
                forces[nodes[j]] -= direction * force;
            }
        }

        // Attraction along edges
        for (const Edge &edge : edges) {
            QPointF delta = nodePositions[edge.target] - nodePositions[edge.source];
            double distance = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
            if (distance < 1) distance = 1;

            double force = ATTRACTION_STRENGTH * distance;
            QPointF direction = delta / distance;

            forces[edge.source] += direction * force;
            forces[edge.target] -= direction * force;
        }

        // Apply forces
        for (const QString &node : nodes) {
            nodePositions[node] += forces[node] * DAMPING;

            // Keep within bounds
            nodePositions[node].setX(qBound(50.0, nodePositions[node].x(), 1100.0));
            nodePositions[node].setY(qBound(50.0, nodePositions[node].y(), 700.0));
        }
    }

    drawGraph();
}

QColor GraphWindow::getNodeColor(const QString &nodeId)
{
    if (nodeDataMap.contains(nodeId)) {
        const NodeData &node = nodeDataMap[nodeId];
        if (node.isLocal) return QColor(255, 107, 107);
        if (node.type == "router") return QColor(78, 205, 196);
    }
    return QColor(69, 183, 209);
}

double GraphWindow::getNodeSize(const QString &nodeId)
{
    if (nodeDataMap.contains(nodeId)) {
        const NodeData &node = nodeDataMap[nodeId];
        if (node.isLocal) return 20.0;
        if (node.type == "router") return 12.0;
    }
    return 15.0;
}

QColor GraphWindow::getEdgeColor(const Edge &edge)
{
    if (edge.type == "direct") {
        return QColor(150, 206, 180, 180);
    } else {
        return QColor(254, 202, 87, 180);
    }
}

void GraphWindow::drawGraph()
{
    scene->setBackgroundBrush(QBrush(QColor(41, 41, 87)));

    scene->clear();
    nodeItems.clear();

    QStringList nodes = adjacency.keys();
    if (nodes.isEmpty()) return;

    // Position nodes based on current topology
    if (currentTopology == CIRCLE_TOPOLOGY) {
        double radius = qMin(view->width(), view->height()) * 0.35;
        QPointF center(view->width() / 2, view->height() / 2);

        for (int i = 0; i < nodes.size(); ++i) {
            double angle = (2 * M_PI * i) / nodes.size();
            QPointF pos(center.x() + radius * qCos(angle),
                        center.y() + radius * qSin(angle));
            nodePositions[nodes[i]] = pos;
        }
    }

    // Filter edges based on showTraceRoute
    QList<Edge> visibleEdges;
    for (const Edge &edge : edges) {
        if (showTraceRoute || edge.type == "direct") {
            visibleEdges.append(edge);
        }
    }

    // Draw edges first
    for (const Edge &edge : visibleEdges) {
        drawEdge(edge.source, edge.target, edge);
    }

    // Draw nodes
    for (const QString &node : nodes) {
        drawNode(node, nodePositions[node]);
    }

    scene->setSceneRect(scene->itemsBoundingRect().adjusted(-100, -100, 100, 100));
}

void GraphWindow::drawNode(const QString &node, const QPointF &pos)
{
    QColor color = getNodeColor(node);
    double size = getNodeSize(node);

    QString displayText = node;
    if (nodeDataMap.contains(node) && nodeDataMap[node].isLocal) {
        displayText = "LOCAL";
    } else if (node.length() > 12) {
        displayText = node.left(10) + "...";
    }

    GraphNode *graphNode = new GraphNode(node, pos, size, color, displayText);

    if (nodeDataMap.contains(node)) {
        const NodeData &data = nodeDataMap[node];
        QString tooltip = QString("IP: %1\nType: %2\nPackets: %3")
                              .arg(data.ip)
                              .arg(data.type)
                              .arg(data.packetCount);
        if (!data.protocols.isEmpty()) {
            tooltip += "\nProtocols: " + data.protocols.join(", ");
        }
        graphNode->setToolTip(tooltip);
    }

    scene->addItem(graphNode);
    nodeItems[node] = graphNode;
}

void GraphWindow::drawEdge(const QString &source, const QString &target, const Edge &edgeData)
{
    if (!nodePositions.contains(source) || !nodePositions.contains(target))
        return;

    QPointF src = nodePositions[source];
    QPointF dst = nodePositions[target];

    QColor edgeColor = getEdgeColor(edgeData);

    double thickness = qMin(1.0 + qLn(edgeData.packetCount + 1) * 2, 8.0);

    QPen pen(edgeColor, thickness);
    pen.setCapStyle(Qt::RoundCap);

    QGraphicsLineItem *line = scene->addLine(QLineF(src, dst), pen);
    line->setZValue(-1);
}

GraphWindow::~GraphWindow()
{
    // Stop the timer when destroying the window
    if (pollTimer) {
        pollTimer->stop();
    }
}
