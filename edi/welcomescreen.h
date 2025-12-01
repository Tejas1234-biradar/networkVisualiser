#ifndef WELCOMESCREEN_H
#define WELCOMESCREEN_H

#include <QMainWindow>
#include <QProcess>
#include <QWidget>
#include <mainwindow.h>

class WelcomeScreen : public QWidget {
    Q_OBJECT
public:
    WelcomeScreen(QWidget *parent= nullptr, MainWindow *mainWindow=nullptr);

private:
    MainWindow * mainWindow;
    QProcess* process= new QProcess;
    QString StdOut;
    QString StdError;
    void setupUi();
    void showMainWindow();
};

#endif // WELCOMESCREEN_H
