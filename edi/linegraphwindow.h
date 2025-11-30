#ifndef LINEGRAPHWINDOW_H
#define LINEGRAPHWINDOW_H

#include <QWidget>
#include <QVector>
#include <QPainter>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPaintEvent>
#include <QDateTime>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "api_caller.h"

struct PacketData {
    QString dst_ip;
    int dst_port;
    int length;
    QString protocol;
    QString src_ip;
    int src_port;
    struct {
        int ACK;
        int FIN;
        int PSH;
        int RST;
        int SYN;
        int URG;
    } tcp_flags;
    double timestamp;
};

class GraphWidget : public QWidget {
    Q_OBJECT

public:
    explicit GraphWidget(QWidget *parent = nullptr);
    void setData(const QVector<PacketData> &data);
    void setMetric(const QString &metric);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<PacketData> m_data;
    QString m_currentMetric;

    void drawGraph(QPainter &painter);
    void drawAxes(QPainter &painter);
    void drawGridLines(QPainter &painter);
    void drawDataPoints(QPainter &painter);
    double getValueForMetric(const PacketData &packet);
    QString getMetricLabel();
};

class LineGraphWindow : public QWidget {
    Q_OBJECT

public:
    explicit LineGraphWindow(QWidget *parent = nullptr);
    ~LineGraphWindow();

    void addPacketData(const PacketData &packet);
    void clearData();
    void setMaxDataPoints(int max);
    void startPeriodicFetch(const QString &apiUrl, int intervalMs = 2500);
    void stopPeriodicFetch();

private slots:
    void onMetricChanged(int index);
    void onApiResponse(const QString& response);
    void onApiError(const QString& error);
    void fetchPacketData();

private:
    void setupUI();
    void updateGraph();
    void updateStatistics();
    void parsePacketJson(const QString &jsonData);

    GraphWidget *m_graphWidget;
    QComboBox *m_metricSelector;
    QLabel *m_statsLabel;
    QLabel *m_titleLabel;
    QLabel *m_statusLabel;

    QVector<PacketData> m_packetData;
    int m_maxDataPoints;

    ApiCaller *m_apiCaller;
    QTimer *m_fetchTimer;
    QString m_apiUrl;
};

#endif // LINEGRAPHWINDOW_H
