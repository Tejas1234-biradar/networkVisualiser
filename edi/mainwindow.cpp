#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    // --- Create stacked widget for pages ---
    auto *stack = new QStackedWidget;

    // Function to create Page 1 (full layout)
    auto createPage1 = [&]() {
        auto *page = new QWidget;
        auto *layout = new QVBoxLayout(page);

        // --- Top bar 1 ---
        auto *topBar1 = new QFrame;
        topBar1->setFrameShape(QFrame::Box);
        topBar1->setFixedHeight(50);

        // --- Top bar 2 with 4 buttons ---
        auto *topBar2 = new QFrame;
        topBar2->setFrameShape(QFrame::Box);
        topBar2->setFixedHeight(40);
        auto *btnLayout = new QHBoxLayout(topBar2);

        auto *btn1 = new QPushButton("Page 1");
        auto *btn2 = new QPushButton("Page 2");
        auto *btn3 = new QPushButton("Page 3");
        auto *btn4 = new QPushButton("Page 4");

        btnLayout->addWidget(btn1);
        btnLayout->addWidget(btn2);
        btnLayout->addWidget(btn3);
        btnLayout->addWidget(btn4);

        // --- Middle row ---
        auto *middleLayout = new QHBoxLayout;
        auto *midLeft = new QFrame;
        auto *midRight = new QFrame;
        midLeft->setFrameShape(QFrame::Box);
        midRight->setFrameShape(QFrame::Box);
        middleLayout->addWidget(midLeft, 1);
        middleLayout->addWidget(midRight, 1);

        // --- Bottom ---
        auto *bottom = new QFrame;
        bottom->setFrameShape(QFrame::Box);

        // Add widgets to page layout
        layout->addWidget(topBar1);
        layout->addWidget(topBar2);
        layout->addLayout(middleLayout);
        layout->addWidget(bottom);

        // Navigation connections
        connect(btn1, &QPushButton::clicked, [=]() { stack->setCurrentIndex(0); });
        connect(btn2, &QPushButton::clicked, [=]() { stack->setCurrentIndex(1); });
        connect(btn3, &QPushButton::clicked, [=]() { stack->setCurrentIndex(2); });
        connect(btn4, &QPushButton::clicked, [=]() { stack->setCurrentIndex(3); });

        return page;
    };

    // Function to create Pages 2,3,4 (simpler layout)
    auto createSimplePage = [&](const QString &name) {
        auto *page = new QWidget;
        auto *layout = new QVBoxLayout(page);

        // --- Top bar 1 ---
        auto *topBar1 = new QFrame;
        topBar1->setFrameShape(QFrame::Box);
        topBar1->setFixedHeight(50);

        // --- Top bar 2 with 4 buttons ---
        auto *topBar2 = new QFrame;
        topBar2->setFrameShape(QFrame::Box);
        topBar2->setFixedHeight(40);
        auto *btnLayout = new QHBoxLayout(topBar2);

        auto *btn1 = new QPushButton("Page 1");
        auto *btn2 = new QPushButton("Page 2");
        auto *btn3 = new QPushButton("Page 3");
        auto *btn4 = new QPushButton("Page 4");

        btnLayout->addWidget(btn1);
        btnLayout->addWidget(btn2);
        btnLayout->addWidget(btn3);
        btnLayout->addWidget(btn4);

        // --- Single bottom box ---
        auto *bottom = new QFrame;
        bottom->setFrameShape(QFrame::Box);

        // Add widgets to layout
        layout->addWidget(topBar1);
        layout->addWidget(topBar2);
        layout->addWidget(bottom);

        // Navigation connections
        connect(btn1, &QPushButton::clicked, [=]() { stack->setCurrentIndex(0); });
        connect(btn2, &QPushButton::clicked, [=]() { stack->setCurrentIndex(1); });
        connect(btn3, &QPushButton::clicked, [=]() { stack->setCurrentIndex(2); });
        connect(btn4, &QPushButton::clicked, [=]() { stack->setCurrentIndex(3); });

        return page;
    };

    // Add all 4 pages
    stack->addWidget(createPage1());
    stack->addWidget(createSimplePage("Page 2"));
    stack->addWidget(createSimplePage("Page 3"));
    stack->addWidget(createSimplePage("Page 4"));

    // Add stacked widget to main layout
    mainLayout->addWidget(stack);
    setCentralWidget(central);
}

MainWindow::~MainWindow() {}
