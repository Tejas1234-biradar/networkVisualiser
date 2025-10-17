#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QWidget>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QLabel;
class QPushButton;
class QTextEdit;
class WebSocketClient;

class TestWindow : public QWidget {
    Q_OBJECT
public:
    explicit TestWindow(QWidget* parent = nullptr);

private slots:
    void handleGraphData(const QString& message);
    void fetchInitialGraph();
    void onInitialGraphReceived(QNetworkReply* reply);

private:
    void processGraphData(const QJsonObject& data, const QString& source);
    void log(const QString& message);

    QLabel* statusLabel;
    QTextEdit* logOutput;
    QPushButton* reconnectBtn;
    QPushButton* refreshBtn;
    QPushButton* clearLogBtn;
    WebSocketClient* ws;
    QNetworkAccessManager* networkManager;
};

#endif // TESTWINDOW_H
