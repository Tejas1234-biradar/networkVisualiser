/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.17
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *txtFirstNum;
    QLineEdit *txtSecondNum;
    QPushButton *btnAdd;
    QPushButton *btnSub;
    QPushButton *btnDivide;
    QPushButton *btnMultipy;
    QLabel *label_3;
    QLineEdit *txtResultNum;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(513, 378);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(50, 31, 151, 61));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(50, 110, 151, 22));
        txtFirstNum = new QLineEdit(centralwidget);
        txtFirstNum->setObjectName(QString::fromUtf8("txtFirstNum"));
        txtFirstNum->setGeometry(QRect(190, 50, 221, 29));
        txtSecondNum = new QLineEdit(centralwidget);
        txtSecondNum->setObjectName(QString::fromUtf8("txtSecondNum"));
        txtSecondNum->setGeometry(QRect(190, 100, 221, 29));
        btnAdd = new QPushButton(centralwidget);
        btnAdd->setObjectName(QString::fromUtf8("btnAdd"));
        btnAdd->setGeometry(QRect(50, 220, 81, 30));
        btnSub = new QPushButton(centralwidget);
        btnSub->setObjectName(QString::fromUtf8("btnSub"));
        btnSub->setGeometry(QRect(140, 220, 81, 30));
        btnDivide = new QPushButton(centralwidget);
        btnDivide->setObjectName(QString::fromUtf8("btnDivide"));
        btnDivide->setGeometry(QRect(240, 220, 81, 30));
        btnMultipy = new QPushButton(centralwidget);
        btnMultipy->setObjectName(QString::fromUtf8("btnMultipy"));
        btnMultipy->setGeometry(QRect(340, 220, 81, 30));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(90, 260, 151, 61));
        txtResultNum = new QLineEdit(centralwidget);
        txtResultNum->setObjectName(QString::fromUtf8("txtResultNum"));
        txtResultNum->setGeometry(QRect(170, 268, 221, 61));
        txtResultNum->setReadOnly(true);
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "First Number", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Second Number", nullptr));
        btnAdd->setText(QCoreApplication::translate("MainWindow", "+", nullptr));
        btnSub->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        btnDivide->setText(QCoreApplication::translate("MainWindow", "/", nullptr));
        btnMultipy->setText(QCoreApplication::translate("MainWindow", "X", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Result", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
