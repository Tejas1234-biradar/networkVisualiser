#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "navbarwidget.h"
#include "verbosewidget.h"
#include "linegraphwindow.h"
#include "graphWindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Create the navbar widget
    NavbarWidget *navbar = new NavbarWidget(this);
    setCentralWidget(navbar);

    // Create placeholder pages
    createDashboardPage(navbar);
    createAnalyticsPage(navbar);
    createVerbosePage(navbar);
    createGraphPage(navbar);

    // Connect navbar signals
    connect(navbar, &NavbarWidget::dashboardClicked, this, &MainWindow::switchLayoutDashboard);
    connect(navbar, &NavbarWidget::analyticsClicked, this, &MainWindow::switchLayoutAnalytics);
    connect(navbar, &NavbarWidget::verboseClicked, this, &MainWindow::switchLayoutVerbose);
    connect(navbar, &NavbarWidget::graphClicked, this, &MainWindow::switchLayoutGraph);

    // Store navbar reference
    m_navbar = navbar;

    // Show dashboard by default
    switchLayoutDashboard();
}

void MainWindow::createDashboardPage(NavbarWidget *navbar) {
    QWidget *dashboardPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(dashboardPage);

    // Top Row
    layout->addWidget(new QLabel("Public IP | Private IP | Hostname | NIC"));

    // Middle Row
    layout->addWidget(new QLabel("Network Topography Graph"));

    // Bottom Box
    layout->addWidget(new QLabel("Anomaly Detection"));

    m_dashboardIndex = navbar->addPage(dashboardPage);
}

void MainWindow::createAnalyticsPage(NavbarWidget *navbar) {
    QWidget *analyticsPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(analyticsPage);

    // Top area: Port Map
    layout->addWidget(new QLabel("Packet Info"));

    // Bottom area: Heat Map/Pie chart
    QHBoxLayout *bottomRow = new QHBoxLayout;
    bottomRow->addWidget(new QLabel("Heat Map/Pie chart (Protocol)"));
    layout->addLayout(bottomRow);

    m_analyticsIndex = navbar->addPage(analyticsPage);
}

void MainWindow::createVerbosePage(NavbarWidget *navbar) {
    // Create the verbose widget (table + graph)
    VerboseWidget *verboseWidget = new VerboseWidget();

    // Start monitoring packets every 2.5 seconds
    verboseWidget->startMonitoring("http://localhost:5000/api/packets/stream", 2500);

    m_verboseIndex = navbar->addPage(verboseWidget);
    m_verboseWidget = verboseWidget;
}

void MainWindow::createGraphPage(NavbarWidget *navbar) {
    // Create a container widget for the graph page
    QWidget *graphPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(graphPage);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Create the GraphWindow but don't show it as a separate window
    GraphWindow *graphWindow = new GraphWindow(graphPage);

    // Add the central widget of GraphWindow to our layout
    layout->addWidget(graphWindow->centralWidget());

    // Initialize the graph window
    graphWindow->initialize();

    // Store reference if needed for later access
    m_graphWidget = graphWindow;

    // Add the page to navbar
    m_graphIndex = navbar->addPage(graphPage);
}

void MainWindow::switchLayoutDashboard() {
    qDebug() << "Switching to Dashboard";
    if (m_navbar) {
        m_navbar->switchToPage(m_dashboardIndex);
    }
}

void MainWindow::switchLayoutAnalytics() {
    qDebug() << "Switching to Analytics";
    if (m_navbar) {
        m_navbar->switchToPage(m_analyticsIndex);
    }
}

void MainWindow::switchLayoutVerbose() {
    qDebug() << "Switching to Verbose";
    if (m_navbar) {
        m_navbar->switchToPage(m_verboseIndex);
    }
}

void MainWindow::switchLayoutGraph() {
    qDebug() << "Switching to Graph";
    if (m_navbar) {
        m_navbar->switchToPage(m_graphIndex);
    }
}

MainWindow::~MainWindow() {
    delete ui;
}
