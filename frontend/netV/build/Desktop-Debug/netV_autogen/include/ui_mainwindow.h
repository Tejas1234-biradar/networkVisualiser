/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actioni_too_am_in_this_episode;
    QAction *actionyes_this_is_working;
    QWidget *centralwidget;
    QPushButton *pushButton;
    QMenuBar *menubar;
    QMenu *menuTesting_123;
    QMenu *menuhelno;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1500, 898);
        actioni_too_am_in_this_episode = new QAction(MainWindow);
        actioni_too_am_in_this_episode->setObjectName("actioni_too_am_in_this_episode");
        actionyes_this_is_working = new QAction(MainWindow);
        actionyes_this_is_working->setObjectName("actionyes_this_is_working");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(700, 430, 88, 26));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1500, 21));
        menuTesting_123 = new QMenu(menubar);
        menuTesting_123->setObjectName("menuTesting_123");
        menuhelno = new QMenu(menubar);
        menuhelno->setObjectName("menuhelno");
        MainWindow->setMenuBar(menubar);

        menubar->addAction(menuTesting_123->menuAction());
        menubar->addAction(menuhelno->menuAction());
        menuTesting_123->addAction(actionyes_this_is_working);
        menuhelno->addAction(actioni_too_am_in_this_episode);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actioni_too_am_in_this_episode->setText(QCoreApplication::translate("MainWindow", "i too am in this episode", nullptr));
        actionyes_this_is_working->setText(QCoreApplication::translate("MainWindow", "yes this is working", nullptr));
        pushButton->setText(QCoreApplication::translate("MainWindow", "HelloWorld", nullptr));
        menuTesting_123->setTitle(QCoreApplication::translate("MainWindow", "Testing 123", nullptr));
        menuhelno->setTitle(QCoreApplication::translate("MainWindow", "helno", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
