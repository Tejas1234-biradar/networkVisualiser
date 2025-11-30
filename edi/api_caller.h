#ifndef API_CALLER_H
#define API_CALLER_H
#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class ApiCaller : public QObject {
    Q_OBJECT
public:
    explicit ApiCaller(QObject* parent = nullptr);

    void get(const QString& url);
    void post(const QString& url, const QByteArray& payload);
    void put(const QString& url, const QByteArray& payload);

signals:
    void responseReceived(const QString& data);
    void errorOccurred(const QString& message);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* manager;
};


#endif // API_CALLER_H
