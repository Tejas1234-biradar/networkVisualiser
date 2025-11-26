#ifndef DATASERVICE_H
#define DATASERVICE_H

#include <QObject>
#include <QtQml/qqmlregistration.h>

class DataService : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit DataService(QObject *parent = nullptr);

private:
    int counter = 0 ;

private slots:
    void processOneThing();

signals:
    // Signal emitted when the value changes
    void counterChange(int newValue);


};

#endif // DATASERVICE_H
