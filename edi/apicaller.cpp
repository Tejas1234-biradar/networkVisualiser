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

void ApiCaller::put(const QString& url, const QByteArray& payload) {
    QNetworkRequest request{ QUrl(url) };
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->put(request, payload);
}

void ApiCaller::onReplyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        emit responseReceived(reply->readAll());  // This is already QByteArray
    } else {
        emit errorOccurred(reply->errorString());
    }
    reply->deleteLater();
}
