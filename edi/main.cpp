#include <QApplication>
#include "mainwindow.h"
#include "testwindow.h"
#include "welcomescreen.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    //MainWindow* showMainWindow= w;
    WelcomeScreen ws(nullptr, &w);
    //TestWindow t;

    ws.show();
    //w.show();
    //t.show();
    return app.exec();
}
