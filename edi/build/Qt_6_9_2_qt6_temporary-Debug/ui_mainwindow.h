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
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *DashboardB;
    QPushButton *AnalyticsB;
    QPushButton *VerboseB;
    QPushButton *GraphB;
    QWidget *LayoutArea;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(986, 463);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayoutWidget = new QWidget(centralwidget);
        horizontalLayoutWidget->setObjectName("horizontalLayoutWidget");
        horizontalLayoutWidget->setGeometry(QRect(0, 0, 991, 31));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        DashboardB = new QPushButton(horizontalLayoutWidget);
        DashboardB->setObjectName("DashboardB");

        horizontalLayout->addWidget(DashboardB);

        AnalyticsB = new QPushButton(horizontalLayoutWidget);
        AnalyticsB->setObjectName("AnalyticsB");

        horizontalLayout->addWidget(AnalyticsB);

        VerboseB = new QPushButton(horizontalLayoutWidget);
        VerboseB->setObjectName("VerboseB");

        horizontalLayout->addWidget(VerboseB);

        GraphB = new QPushButton(horizontalLayoutWidget);
        GraphB->setObjectName("GraphB");

        horizontalLayout->addWidget(GraphB);

        LayoutArea = new QWidget(centralwidget);
        LayoutArea->setObjectName("LayoutArea");
        LayoutArea->setGeometry(QRect(0, 30, 991, 431));
        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        DashboardB->setText(QCoreApplication::translate("MainWindow", "Dashboard", nullptr));
        AnalyticsB->setText(QCoreApplication::translate("MainWindow", "Analytics", nullptr));
        VerboseB->setText(QCoreApplication::translate("MainWindow", "Verbose", nullptr));
        GraphB->setText(QCoreApplication::translate("MainWindow", "Graph", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
