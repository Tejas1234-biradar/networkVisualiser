#include <QApplication>
#include "mainwindow.h"
#include "testwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    TestWindow t;
    t.show();
    return app.exec();
}
