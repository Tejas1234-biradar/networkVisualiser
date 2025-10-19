#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQuickWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QQuickWidget *lineGraph;

private slots:
    void clearLayout(QLayout* layout);
    void switchLayoutDashboard();
    void switchLayoutAnalytics();
    void switchLayoutVerbose();
    void switchLayoutGraph();
    void on_MainWindow_iconSizeChanged(const QSize &iconSize);
};

class LineSeriesWrapper : public QLineSeries {
    Q_OBJECT
public:
    explicit LineSeriesWrapper(QObject* parent = nullptr) : QLineSeries(parent) {}

    Q_INVOKABLE void appendPoint(qreal x, qreal y) {
        QLineSeries::append(x, y);
    }
};
#endif // MAINWINDOW_H
