#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QStackedWidget>



MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow) {
    ui->setupUi(this);
    connect(ui->AnalyticsB,&QPushButton::clicked,this,&MainWindow::switchLayoutAnalytics);

}

void MainWindow::switchLayoutAnalytics(){
    QVBoxLayout* newLayout = new QVBoxLayout;
    newLayout->addWidget(new QLabel("New Content Here", ui->LayoutArea));
    ui->LayoutArea->setLayout(newLayout);
}

MainWindow::~MainWindow() {
    delete ui;
}
