#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <QObject>
#include <QWebSocket>

class WebSocketClient : public QObject {
    Q_OBJECT
public:
    explicit WebSocketClient(const QUrl& url, QObject* parent = nullptr);
    void sendMessage(const QString& message);
    void reconnect();
    bool isConnected() const;

signals:
    void messageReceived(const QString& message);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString& error);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onError(QAbstractSocket::SocketError error);

private:
    QWebSocket socket;
    QUrl serverUrl;
};

#endif // WEBSOCKET_CLIENT_H
