#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtQuickWidgets/QQuickWidget>
#include <QQuickView>
#include <QDebug>
#include <QQmlContext>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QStackedWidget>
#include <QtCharts/QLineSeries>
#include "iostream"


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
    mainLayout->addWidget(new QLabel("Network Topography Graph"));

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
    mainLayout->addWidget(new QLabel("Packet Info"));

    // Bottom area: Heat Map/Pie chart
    QHBoxLayout* bottomRow = new QHBoxLayout;
    bottomRow->addWidget(new QLabel("Heat Map/Pie chart(Protocol)"));
    // Insert chart or placeholder here if needed
    mainLayout->addLayout(bottomRow);

    ui->LayoutArea->setLayout(mainLayout);
}

void MainWindow::switchLayoutVerbose(){
    std::cout << "In SwitchLayoutVerbose" << std::endl;
    //Destroy Old Layout
    QLayout* oldLayout = ui->LayoutArea->layout();
    std::cout << "In SwitchLayoutVerbose:oldLayout" << oldLayout << std::endl;
    if (oldLayout) {
        clearLayout(oldLayout);
        delete oldLayout;
    }
    QVBoxLayout* mainLayout = new QVBoxLayout;



    //lineGraphQmlWidget = new QQuickWidget(this);
    //lineGraphQmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    //lineGraphQmlWidget->setSource(QUrl(QStringLiteral("qrc:/main.qml")));

    //QQuickItem *qQuickItem = lineGraphQmlWidget->rootObject();


    // QVariantList points ;
    // points
    //     << QVariantMap {{ "x",0}, {"y",1}}
    //     << QVariantMap {{ "x",1}, {"y",3}}
    //     << QVariantMap {{ "x",2}, {"y",2}};

    // lineGraphQmlWidget->setProperty("dataPoints",points);


    // Using QQuickView
    // QQuickView view;
    // view.setSource(QUrl(QStringLiteral("qrc:/main.qml")));
    // view.show();
    // QObject *object = view.findChild("graphView");
    // QQuickItem *item = qobject_cast<QQuickItem*>(object);

    QVBoxLayout* LineGraph = new QVBoxLayout;
    QQuickWidget *qmlWidget = new QQuickWidget;
    qDebug() << "In Verbose Layout after QuickWidget Creation";
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/main.qml")));
    LineGraph->addWidget(new QLabel("LineGraph"));
    LineGraph->addWidget(qmlWidget);
    mainLayout->addLayout(LineGraph);
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
