#ifndef TESTWINDOW_H
#define TESTWINDOW_H
#pragma once
#include <QWidget>
#include "api_caller.h"

class QLabel;
class QPushButton;

class TestWindow : public QWidget {
    Q_OBJECT
public:
    explicit TestWindow(QWidget* parent = nullptr);


private:
    QLabel* label;
    QPushButton* fetchButton;
    ApiCaller* api;
};
#endif // TESTWINDOW_H
