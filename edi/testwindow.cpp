#include "testwindow.h"
#include "websocket_client.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

TestWindow::TestWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("API + WebSocket Test");
    resize(600, 400);

    label = new QLabel("Press any button to test APIs or WebSocket", this);
    label->setWordWrap(true);

    auto* getBtn = new QPushButton("GET", this);
    auto* postBtn = new QPushButton("POST", this);
    auto* putBtn = new QPushButton("PUT", this);
    wsBtn = new QPushButton("Send WS Message", this);
    auto* reconnectBtn = new QPushButton("Reconnect WebSocket", this);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(getBtn);
    layout->addWidget(postBtn);
    layout->addWidget(putBtn);
    layout->addWidget(wsBtn);
    layout->addWidget(reconnectBtn);

    // API setup
    api = new ApiCaller(this);
    connect(api, &ApiCaller::responseReceived, this, [this](const QString& data) {
        label->setText(data.left(400));
    });

    connect(api, &ApiCaller::errorOccurred, this, [this](const QString& err) {
        label->setText("API Error: " + err);
    });

    connect(getBtn, &QPushButton::clicked, [this]() {
        api->get("http://localhost:5000/api/graph/detailed");
    });

    connect(postBtn, &QPushButton::clicked, [this]() {
        QJsonObject payload{{"title", "foo"}, {"body", "bar"}, {"userId", 1}};
        api->post("https://jsonplaceholder.typicode.com/posts",
                  QJsonDocument(payload).toJson());
    });

    connect(putBtn, &QPushButton::clicked, [this]() {
        QJsonObject payload{{"id", 1}, {"title", "updated"}, {"body", "new body"}, {"userId", 1}};
        api->put("https://jsonplaceholder.typicode.com/posts/1",
                 QJsonDocument(payload).toJson());
    });

    // WebSocket setup - create as member variable for proper lifetime
   ws = new WebSocketClient(QUrl("wss://ws.postman-echo.com/raw"), this);;

    connect(ws, &WebSocketClient::connectionStatusChanged, this, [this](bool connected) {
        if (connected) {
            label->setText("WebSocket: Connected!");
            wsBtn->setEnabled(true);
        } else {
            label->setText("WebSocket: Disconnected");
            wsBtn->setEnabled(false);
        }
    });

    connect(ws, &WebSocketClient::messageReceived, this, [this](const QString& msg) {
        label->setText("WebSocket received: " + msg);
    });

    connect(ws, &WebSocketClient::errorOccurred, this, [this](const QString& error) {
        label->setText("WebSocket Error: " + error);
    });

    connect(wsBtn, &QPushButton::clicked, [this]() {
        if (ws->isConnected()) {
            ws->sendMessage("Ping from Qt6!");
        } else {
            label->setText("WebSocket not connected!");
        }
    });

    connect(reconnectBtn, &QPushButton::clicked, [this]() {
        label->setText("Reconnecting WebSocket...");
        ws->reconnect();
    });

    // Initially disable send button until connected
    wsBtn->setEnabled(false);
}
