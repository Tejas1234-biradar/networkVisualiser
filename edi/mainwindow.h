#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class NavbarWidget;
class VerboseWidget;
class LineGraphWindow;
class GraphWindow;  // Add forward declaration

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void switchLayoutDashboard();
    void switchLayoutAnalytics();
    void switchLayoutVerbose();
    void switchLayoutGraph();

private:
    void createDashboardPage(NavbarWidget *navbar);
    void createAnalyticsPage(NavbarWidget *navbar);
    void createVerbosePage(NavbarWidget *navbar);
    void createGraphPage(NavbarWidget *navbar);

    Ui::MainWindow *ui;
    NavbarWidget *m_navbar;

    // Page indices
    int m_dashboardIndex;
    int m_analyticsIndex;
    int m_verboseIndex;
    int m_graphIndex;

    // Widget references (if needed)
    VerboseWidget *m_verboseWidget;
    LineGraphWindow *m_lineGraphWidget;  // Fixed variable name
    GraphWindow *m_graphWidget;  // Add GraphWindow reference
};

#endif // MAINWINDOW_H
