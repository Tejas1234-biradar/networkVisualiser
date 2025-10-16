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
    auto* wsBtn = new QPushButton("Send WS Message", this);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(getBtn);
    layout->addWidget(postBtn);
    layout->addWidget(putBtn);
    layout->addWidget(wsBtn);

    api = new ApiCaller(this);
    connect(api, &ApiCaller::responseReceived, this, [&](const QString& data) {
        label->setText(data.left(400));
    });
    connect(api, &ApiCaller::errorOccurred, this, [&](const QString& err) {
        label->setText("Error: " + err);
    });

    connect(getBtn, &QPushButton::clicked, [this]() {
        api->get("https://jsonplaceholder.typicode.com/posts/1");
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

    auto* ws = new WebSocketClient(QUrl("wss://echo.websocket.events"), this);
    connect(wsBtn, &QPushButton::clicked, [ws]() {
        ws->sendMessage("Ping from Qt6!");
    });
    connect(ws, &WebSocketClient::messageReceived, this, [&](const QString& msg) {
        label->setText("WebSocket: " + msg);
    });
}
