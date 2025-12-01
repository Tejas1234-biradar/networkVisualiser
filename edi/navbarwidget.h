#ifndef NAVBARWIDGET_H
#define NAVBARWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>

class NavbarWidget : public QWidget {
    Q_OBJECT

public:
    explicit NavbarWidget(QWidget *parent = nullptr);
    ~NavbarWidget();

    // Get the stacked widget to add pages
    QStackedWidget* getContentArea();

    // Add a page to the content area and return its index
    int addPage(QWidget *page);

    // Switch to a specific page
    void switchToPage(int index);

signals:
    void analyticsClicked();
    void verboseClicked();
    void graphClicked();

private slots:
    void onAnalyticsClicked();
    void onVerboseClicked();
    void onGraphClicked();

private:
    void setupUI();
    void setActiveButton(QPushButton *activeBtn);

    // Navbar buttons
    QPushButton *m_analyticsBtn;
    QPushButton *m_verboseBtn;
    QPushButton *m_graphBtn;

    // Content area
    QStackedWidget *m_contentArea;

    // Track active button
    QPushButton *m_activeButton;
};

#endif // NAVBARWIDGET_H
