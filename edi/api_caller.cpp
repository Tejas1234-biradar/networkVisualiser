#include "api_caller.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

ApiCaller::ApiCaller(QObject* parent) : QObject(parent) {
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
            this, &ApiCaller::onReplyFinished);
}

void ApiCaller::get(const QString& url) {
    QNetworkRequest request{ QUrl(url) };
    manager->get(request);
}

void ApiCaller::post(const QString& url, const QByteArray& payload) {
    QNetworkRequest request{ QUrl(url) };
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->post(request, payload);
}
void ApiCaller::post(const QString &url, const QString &jsonData)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = manager->post(request, jsonData.toUtf8());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QString response = reply->readAll();
            emit responseReceived(response);
        } else {
            qWarning() << "POST Error:" << reply->errorString();
        }
        reply->deleteLater();
    });
}
void ApiCaller::put(const QString& url, const QByteArray& payload) {
    QNetworkRequest request{ QUrl(url) };
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->put(request, payload);
}

void ApiCaller::onReplyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        emit responseReceived(reply->readAll());
    } else {
        emit errorOccurred(reply->errorString());
    }
    reply->deleteLater();
}
