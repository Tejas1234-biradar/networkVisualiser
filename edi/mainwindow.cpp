#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtQuickWidgets/QQuickWidget>
#include <QQmlContext>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QStackedWidget>
#include <QtCharts/QLineSeries>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow) {
    ui->setupUi(this);
    connect(ui->DashboardB,&QPushButton::clicked,this,&MainWindow::switchLayoutDashboard);
    connect(ui->AnalyticsB,&QPushButton::clicked,this,&MainWindow::switchLayoutAnalytics);
    connect(ui->VerboseB,&QPushButton::clicked,this,&MainWindow::switchLayoutVerbose);
    connect(ui->GraphB,&QPushButton::clicked,this,&MainWindow::switchLayoutGraph);
    switchLayoutDashboard();

}

void MainWindow::clearLayout(QLayout* layout){
    if (!layout) return;
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->setParent(nullptr);  // Detach so Qt can manage correctly
            widget->deleteLater();       // Schedule for deletion, avoids crash
        } else if (QLayout* childLayout = item->layout()) {
            clearLayout(childLayout); // Recursively clear
        }
        delete item;  // always delete the QLayoutItem
    }
}

void MainWindow::switchLayoutDashboard(){
    //Destroy Old Layout
    QLayout* oldLayout = ui->LayoutArea->layout();
    if (oldLayout) {
        clearLayout(oldLayout);
        delete oldLayout;
    }

    QVBoxLayout* mainLayout = new QVBoxLayout;
    //Top Row:
    mainLayout->addWidget(new QLabel("Public IP | Private IP | Hostname | NIC"));

    //Middle Row:
    QHBoxLayout* middleRow = new QHBoxLayout;

    //Left Box of Middle Row:
    QVBoxLayout* leftColumn= new QVBoxLayout;
    leftColumn->addWidget(new QLabel("Port Map"));
    middleRow->addLayout(leftColumn);

    //RIght Box of Middle Row:
    QVBoxLayout* rightColumn = new QVBoxLayout;
    rightColumn->addWidget(new QLabel("Network Topography Graph"));
    middleRow->addLayout(rightColumn);

    //Adding Middle Row to mainLayout:
    mainLayout->addLayout(middleRow);

    //Bottom Box:
    mainLayout->addWidget(new QLabel("Anamoly Detection"));

    ui->LayoutArea->setLayout(mainLayout);
}

void MainWindow::switchLayoutAnalytics(){
    //Destroy Old Layout
    QLayout* oldLayout = ui->LayoutArea->layout();
    if (oldLayout) {
        clearLayout(oldLayout);
        delete oldLayout;
    }
    QVBoxLayout* mainLayout = new QVBoxLayout;

    // Top area: Port Map
    mainLayout->addWidget(new QLabel("Port Map"));

    // Bottom area: Heat Map/Pie chart
    QHBoxLayout* bottomRow = new QHBoxLayout;
    bottomRow->addWidget(new QLabel("Heat Map/Pie chart(Protocol)"));
    // Insert chart or placeholder here if needed
    mainLayout->addLayout(bottomRow);

    ui->LayoutArea->setLayout(mainLayout);
}

void MainWindow::switchLayoutVerbose(){
    //Destroy Old Layout
    QLayout* oldLayout = ui->LayoutArea->layout();
    if (oldLayout) {
        clearLayout(oldLayout);
        delete oldLayout;
    }
    QVBoxLayout* mainLayout = new QVBoxLayout;

    mainLayout->addWidget(new QLabel("Packet Info"));
    mainLayout->addWidget(new QLabel("Line Graph"));
    lineGraph = new QQuickWidget(this);
    lineGraph->setResizeMode(QQuickWidget::SizeRootObjectToView);
    lineGraph->setSource(QUrl(QStringLiteral("qrc:/main.qml")));


    // Add to layout
    mainLayout->addWidget(lineGraph);
    ui->LayoutArea->setLayout(mainLayout);
}

void MainWindow::switchLayoutGraph(){
    //Destroy Old Layout
    QLayout* oldLayout = ui->LayoutArea->layout();
    if (oldLayout) {
        clearLayout(oldLayout);
        delete oldLayout;
    }
    QVBoxLayout* newLayout = new QVBoxLayout;
    newLayout->addWidget(new QLabel("Graph", ui->LayoutArea));
    ui->LayoutArea->setLayout(newLayout);
}

MainWindow::~MainWindow() {
    delete ui;
}
