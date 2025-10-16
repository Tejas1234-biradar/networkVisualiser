#include "websocket_client.h"
#include <QDebug>

WebSocketClient::WebSocketClient(const QUrl& url, QObject* parent)
    : QObject(parent), serverUrl(url)
{
    connect(&socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(&socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(&socket, &QWebSocket::textMessageReceived,
            this, &WebSocketClient::onTextMessageReceived);

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
    emit messageReceived(message);
}

void WebSocketClient::sendMessage(const QString& message) {
    if (socket.state() == QAbstractSocket::ConnectedState)
        socket.sendTextMessage(message);
}
