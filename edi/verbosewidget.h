#ifndef VERBOSEWIDGET_H
#define VERBOSEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QTimer>
#include <QLabel>
#include "linegraphwindow.h"
#include "api_caller.h"

class VerboseWidget : public QWidget {
    Q_OBJECT

public:
    explicit VerboseWidget(QWidget *parent = nullptr);
    ~VerboseWidget();

    void startMonitoring(const QString &apiUrl, int intervalMs = 2500);
    void stopMonitoring();
    void clearAllData();

private slots:
    void onApiResponse(const QString& response);
    void onApiError(const QString& error);
    void fetchData();

private:
    void setupUI();
    void addPacketToTable(const PacketData &packet);
    void parseAndDisplayPackets(const QString &jsonData);
    QString getPacketType(const PacketData &packet);

    // UI Components
    QTableWidget *m_packetTable;
    LineGraphWindow *m_graphWindow;
    QLabel *m_tableTitle;
    QLabel *m_statusLabel;

    // API handling
    ApiCaller *m_apiCaller;
    QTimer *m_fetchTimer;
    QString m_apiUrl;

    // Settings
    int m_maxTableRows;
};

#endif // VERBOSEWIDGET_H
