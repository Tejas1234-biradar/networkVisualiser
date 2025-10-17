#include "websocket_client.h"
#include <QDebug>

WebSocketClient::WebSocketClient(const QUrl& url, QObject* parent)
    : QObject(parent), serverUrl(url)
{
    connect(&socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(&socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(&socket, &QWebSocket::textMessageReceived,
            this, &WebSocketClient::onTextMessageReceived);

    // Add error handling
    connect(&socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred),
            this, &WebSocketClient::onError);

    qDebug() << "Attempting to connect to:" << serverUrl;
    socket.open(serverUrl);
}

void WebSocketClient::onConnected() {
    qDebug() << "WebSocket connected to" << serverUrl;
    emit connectionStatusChanged(true);
}

void WebSocketClient::onDisconnected() {
    qDebug() << "WebSocket disconnected";
    emit connectionStatusChanged(false);
}

void WebSocketClient::onTextMessageReceived(const QString& message) {
    qDebug() << "Message received:" << message;
    emit messageReceived(message);
}

// In websocket_client.cpp, update onError method:
void WebSocketClient::onError(QAbstractSocket::SocketError error) {
    QString errorString = socket.errorString();
    qDebug() << "WebSocket error code:" << error;
    qDebug() << "WebSocket error:" << errorString;
    qDebug() << "Socket state:" << socket.state();
    qDebug() << "URL:" << serverUrl;
    emit errorOccurred(errorString);
}

void WebSocketClient::sendMessage(const QString& message) {
    if (socket.state() == QAbstractSocket::ConnectedState) {
        qDebug() << "Sending message:" << message;
        socket.sendTextMessage(message);
    } else {
        qDebug() << "Cannot send message - socket not connected. State:" << socket.state();
        emit errorOccurred("Socket not connected");
    }
}

void WebSocketClient::reconnect() {
    if (socket.state() == QAbstractSocket::ConnectedState) {
        socket.close();
    }
    socket.open(serverUrl);
}

bool WebSocketClient::isConnected() const {
    return socket.state() == QAbstractSocket::ConnectedState;
}
