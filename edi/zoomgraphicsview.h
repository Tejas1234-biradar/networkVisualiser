#ifndef ZOOMGRAPHICSVIEW_H
#define ZOOMGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QTimeLine>

class ZoomGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ZoomGraphicsView(QWidget *parent = nullptr);
    explicit ZoomGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr);

    void setZoomRange(double minZoom, double maxZoom);
    double getCurrentZoom() const { return currentZoom; }
    void resetZoom();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void scalingTime(qreal value);
    void animFinished();

private:
    void setupZoom();
    void zoom(double factor, QPointF centerPoint);

    double currentZoom;
    double minZoom;
    double maxZoom;
    double zoomFactor;
    QTimeLine *zoomTimeLine;
    int numScheduledScalings;
    bool isPanning;
    QPoint lastPanPoint;
};

#endif // ZOOMGRAPHICSVIEW_H
