#include "dataservice.h"
#include "QChronoTimer"
#include <QDebug>
#include <QString>

using namespace std::chrono_literals;

DataService::DataService(QObject *parent)
    : QObject{parent}
{
    qDebug() <<  "** In Data Service construction *** "  ;

    QChronoTimer *timer = new QChronoTimer(1s,this);
    connect(timer, &QChronoTimer::timeout, this, &DataService::processOneThing);
    timer->start();
}

void DataService::processOneThing() {
    counter++;
    // cout << "I am in cout : counter" <<  counter  << std::endl;

    emit counterChange(counter);
    qDebug() <<  "I am in qDebug : counter" <<  counter  ;
}

