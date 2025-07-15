#include "mainwindow.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent){
    QWidget* central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout();
    QLabel* label=new QLabel("Hello World",this);
    label->setAlignment(Qt::AlignCenter);
    QPushButton* btn=new QPushButton("Show Ip");
    QLabel* ipLabel=new QLabel("");
    ipLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    layout->addWidget(btn);
    layout->addWidget(ipLabel);
    central->setLayout(layout);
    setCentralWidget(central);
    setWindowTitle("Qt6 Hello World");
    resize(400,300);
    connect(btn,&QPushButton::clicked,this,[=](){
        ipLabel->setText("127.0.0.1");
    });
}


MainWindow:: ~MainWindow(){}
