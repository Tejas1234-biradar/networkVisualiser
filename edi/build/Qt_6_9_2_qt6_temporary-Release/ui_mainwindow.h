/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
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
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *DashboardB;
    QPushButton *AnalyticsB;
    QPushButton *VerboseB;
    QPushButton *GraphB;
    QWidget *LayoutArea;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(902, 469);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
        centralwidget->setSizePolicy(sizePolicy1);
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
        DashboardB = new QPushButton(centralwidget);
        DashboardB->setObjectName("DashboardB");

        horizontalLayout->addWidget(DashboardB);

        AnalyticsB = new QPushButton(centralwidget);
        AnalyticsB->setObjectName("AnalyticsB");

        horizontalLayout->addWidget(AnalyticsB);

        VerboseB = new QPushButton(centralwidget);
        VerboseB->setObjectName("VerboseB");

        horizontalLayout->addWidget(VerboseB);

        GraphB = new QPushButton(centralwidget);
        GraphB->setObjectName("GraphB");

        horizontalLayout->addWidget(GraphB);


        verticalLayout->addLayout(horizontalLayout);

        LayoutArea = new QWidget(centralwidget);
        LayoutArea->setObjectName("LayoutArea");

        verticalLayout->addWidget(LayoutArea);

        MainWindow->setCentralWidget(centralwidget);

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
