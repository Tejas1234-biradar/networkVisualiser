#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QWidget>
#include "api_caller.h"

class QLabel;
class QPushButton;
class ApiCaller;
class WebSocketClient;

class TestWindow : public QWidget {
    Q_OBJECT
public:
    explicit TestWindow(QWidget* parent = nullptr);

private:
    QLabel* label;
    ApiCaller* api;
    WebSocketClient* ws;
    QPushButton* wsBtn;  // Make sure this line is here!
};

#endif // TESTWINDOW_H
