#include "navbarwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

NavbarWidget::NavbarWidget(QWidget *parent)
    : QWidget(parent), m_activeButton(nullptr) {
    setupUI();
}

NavbarWidget::~NavbarWidget() {
}

void NavbarWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create navbar container
    QWidget *navbarContainer = new QWidget(this);
    navbarContainer->setFixedHeight(60);
    navbarContainer->setStyleSheet(
        "background-color: #1e1e1e;"
        "border-bottom: 2px solid #3b3b3b;"
        );

    QHBoxLayout *navbarLayout = new QHBoxLayout(navbarContainer);
    navbarLayout->setContentsMargins(10, 5, 10, 5);
    navbarLayout->setSpacing(5);

    // App title/logo
    QLabel *appTitle = new QLabel("Network Visualizer", navbarContainer);
    appTitle->setStyleSheet(
        "color: #00ff00;"
        "font-size: 18px;"
        "font-weight: bold;"
        "padding: 10px;"
        );
    navbarLayout->addWidget(appTitle);

    navbarLayout->addStretch();

    // Create navigation buttons
    QString buttonStyle =
        "QPushButton {"
        "    background-color: #2b2b2b;"
        "    color: #ffffff;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #3b3b3b;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #4b4b4b;"
        "}";

    QString activeButtonStyle =
        "QPushButton {"
        "    background-color: #00ff00;"
        "    color: #000000;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #00dd00;"
        "}";

    m_dashboardBtn = new QPushButton("Dashboard", navbarContainer);
    m_dashboardBtn->setStyleSheet(buttonStyle);
    m_dashboardBtn->setProperty("activeStyle", activeButtonStyle);
    m_dashboardBtn->setProperty("normalStyle", buttonStyle);
    connect(m_dashboardBtn, &QPushButton::clicked, this, &NavbarWidget::onDashboardClicked);

    m_analyticsBtn = new QPushButton("Analytics", navbarContainer);
    m_analyticsBtn->setStyleSheet(buttonStyle);
    m_analyticsBtn->setProperty("activeStyle", activeButtonStyle);
    m_analyticsBtn->setProperty("normalStyle", buttonStyle);
    connect(m_analyticsBtn, &QPushButton::clicked, this, &NavbarWidget::onAnalyticsClicked);

    m_verboseBtn = new QPushButton("Verbose", navbarContainer);
    m_verboseBtn->setStyleSheet(buttonStyle);
    m_verboseBtn->setProperty("activeStyle", activeButtonStyle);
    m_verboseBtn->setProperty("normalStyle", buttonStyle);
    connect(m_verboseBtn, &QPushButton::clicked, this, &NavbarWidget::onVerboseClicked);

    m_graphBtn = new QPushButton("Graph", navbarContainer);
    m_graphBtn->setStyleSheet(buttonStyle);
    m_graphBtn->setProperty("activeStyle", activeButtonStyle);
    m_graphBtn->setProperty("normalStyle", buttonStyle);
    connect(m_graphBtn, &QPushButton::clicked, this, &NavbarWidget::onGraphClicked);

    navbarLayout->addWidget(m_dashboardBtn);
    navbarLayout->addWidget(m_analyticsBtn);
    navbarLayout->addWidget(m_verboseBtn);
    navbarLayout->addWidget(m_graphBtn);

    // Create content area (stacked widget)
    m_contentArea = new QStackedWidget(this);
    m_contentArea->setStyleSheet("background-color: #0a0a0a;");

    // Add widgets to main layout
    mainLayout->addWidget(navbarContainer);
    mainLayout->addWidget(m_contentArea, 1);

    // Set default active button
    setActiveButton(m_dashboardBtn);
}

void NavbarWidget::setActiveButton(QPushButton *activeBtn) {
    // Reset all buttons to normal style
    if (m_activeButton) {
        m_activeButton->setStyleSheet(m_activeButton->property("normalStyle").toString());
    }

    // Set new active button
    m_activeButton = activeBtn;
    if (m_activeButton) {
        m_activeButton->setStyleSheet(m_activeButton->property("activeStyle").toString());
    }
}

QStackedWidget* NavbarWidget::getContentArea() {
    return m_contentArea;
}

int NavbarWidget::addPage(QWidget *page) {
    return m_contentArea->addWidget(page);
}

void NavbarWidget::switchToPage(int index) {
    if (index >= 0 && index < m_contentArea->count()) {
        m_contentArea->setCurrentIndex(index);
    }
}

void NavbarWidget::onDashboardClicked() {
    setActiveButton(m_dashboardBtn);
    emit dashboardClicked();
}

void NavbarWidget::onAnalyticsClicked() {
    setActiveButton(m_analyticsBtn);
    emit analyticsClicked();
}

void NavbarWidget::onVerboseClicked() {
    setActiveButton(m_verboseBtn);
    emit verboseClicked();
}

void NavbarWidget::onGraphClicked() {
    setActiveButton(m_graphBtn);
    emit graphClicked();
}
