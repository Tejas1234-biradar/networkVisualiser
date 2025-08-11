#include "mainwindow.h"
#include <QLabel>
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)//constructor
    : QMainWindow(parent)
    ,ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}


MainWindow:: ~MainWindow(){
    delete ui;
}
int firstNum,secondNum,result;

void MainWindow::on_btnAdd_clicked()
{
    firstNum=ui->txtFirstNum->text().toInt();
    secondNum=ui->txtSecondNum->text().toInt();
    result=firstNum+secondNum;
    ui->txtResultNum->setText(QString::number(result));
}


void MainWindow::on_btnSub_clicked()
{
    firstNum=ui->txtFirstNum->text().toInt();
    secondNum=ui->txtSecondNum->text().toInt();
    result=firstNum-secondNum;
    ui->txtResultNum->setText(QString::number(result));
}


void MainWindow::on_btnDivide_clicked()
{
    firstNum=ui->txtFirstNum->text().toInt();
    secondNum=ui->txtSecondNum->text().toInt();
    if(secondNum==0){
        ui->txtResultNum->setText(QString::fromStdString("PLease dont"));
    }
    else{
    result=firstNum/secondNum;
    }
    ui->txtResultNum->setText(QString::number(result));
}


void MainWindow::on_btnMultipy_clicked()
{
    firstNum=ui->txtFirstNum->text().toInt();
    secondNum=ui->txtSecondNum->text().toInt();
    result=firstNum*secondNum;
    ui->txtResultNum->setText(QString::number(result));
}

