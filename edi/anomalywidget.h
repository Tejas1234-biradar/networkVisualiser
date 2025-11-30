#ifndef ANOMALYWIDGET_H
#define ANOMALYWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QTimer>
#include <QPushButton>
#include <QProgressBar>
#include "api_caller.h"

struct Packet {
    QString srcIp;
    QString dstIp;
    int srcPort;
    int dstPort;
    int length;
    QString protocol;
    int synFlag;
    int ackFlag;
    int finFlag;
    int pshFlag;
    int rstFlag;
    int urgFlag;
    double timestamp;
    double duration;  // Calculated from consecutive packets
};

struct PredictionResult {
    bool isMalicious;
    double confidence;
    Packet packet;
};

class AnomalyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnomalyWidget(QWidget *parent = nullptr);
    void startMonitoring(const QString &apiUrl, int intervalMs = 3000);
    void stopMonitoring();

private slots:
    void fetchPackets();
    void onPacketsReceived(const QString &data);
    void onPredictionReceived(const QString &data);
    void clearResults();

private:
    void setupUI();
    void analyzePacket(const Packet &packet);
    void updateStatistics();
    void addPredictionToTable(const PredictionResult &result);
    int protocolToNumber(const QString &protocol);

    ApiCaller *packetApi;
    ApiCaller *predictionApi;
    QTimer *fetchTimer;
    QString apiUrl;

    // UI Components
    QWidget *statsPanel;
    QLabel *totalPacketsLabel;
    QLabel *maliciousCountLabel;
    QLabel *benignCountLabel;
    QLabel *accuracyLabel;

    QTableWidget *resultsTable;
    QPushButton *clearButton;
    QPushButton *pauseButton;
    QProgressBar *analysisProgress;
    QLabel *statusLabel;

    // Statistics
    int totalAnalyzed;
    int maliciousCount;
    int benignCount;
    QList<Packet> packetBuffer;
    QMap<QString, double> lastPacketTime;  // Track timing between packets

    bool isPaused;
    Packet currentAnalyzingPacket;
};

#endif // ANOMALYWIDGET_H
