#ifndef GRAPHWINDOW_H
#define GRAPHWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QTimer>
#include <QMap>
#include <QSet>
#include <QPointF>
#include "api_caller.h"
#include "graphNode.h"

class ZoomGraphicsView;

struct Edge {
    QString source;
    QString target;
    QString type;
    int packetCount;
    double ageSeconds;
    double firstSeen;
    double lastSeen;
    QStringList protocols;
};

struct NodeData {
    QString id;
    QString ip;
    QString type;
    bool isLocal;
    int packetCount;
    QStringList protocols;
};

enum TopologyType {
    CIRCLE_TOPOLOGY,
    FREE_TOPOLOGY,
    RADIAL_TOPOLOGY
};

class GraphWindow : public QMainWindow
{
    Q_OBJECT

public:
    GraphWindow(QWidget *parent = nullptr);
    ~GraphWindow();
    void initialize();

private slots:
    void onApiResponse(const QString &data);
    void toggleTopology();
    void toggleTraceRoute(bool enabled);
    void resetView();
    void refreshData();
    void fetchGraphData();

private:
    void addEdge(const Edge &edge);
    void updateNodeData(const QString &nodeId);
    void drawGraph();
    void drawNode(const QString &node, const QPointF &pos);
    void drawEdge(const QString &source, const QString &target, const Edge &edgeData);
    void applyForceDirectedLayout();
    void applyRadialLayout();
    QColor getEdgeColor(const Edge &edge);
    QColor getNodeColor(const QString &nodeId);
    double getNodeSize(const QString &nodeId);
    void updateStatistics();
    void updateConnectionStatus(bool connected);

    QGraphicsScene *scene;
    ZoomGraphicsView *view;
    QWidget *controlPanel;
    QWidget *statsPanel;
    QWidget *legendPanel;
    QPushButton *topologyButton;
    QPushButton *resetButton;
    QPushButton *refreshButton;
    QCheckBox *traceRouteCheckbox;
    QLabel *nodeCountLabel;
    QLabel *edgeCountLabel;
    QLabel *pathCountLabel;
    QLabel *statusLabel;

    ApiCaller *api;
    QTimer *pollTimer;

    QMap<QString, QSet<QString>> adjacency;
    QList<Edge> edges;
    QMap<QString, NodeData> nodeDataMap;
    QMap<QString, QPointF> nodePositions;
    QMap<QString, GraphNode*> nodeItems;

    TopologyType currentTopology;
    bool showTraceRoute;

    const double REPULSION_STRENGTH = 5000.0;
    const double ATTRACTION_STRENGTH = 0.05;
    const double DAMPING = 0.5;
};

#endif // GRAPHWINDOW_H
