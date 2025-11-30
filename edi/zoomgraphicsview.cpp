#include "zoomgraphicsview.h"
#include <QScrollBar>
#include <QMouseEvent>
#include <QtMath>

ZoomGraphicsView::ZoomGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , currentZoom(1.0)
    , minZoom(0.2)
    , maxZoom(5.0)
    , zoomFactor(1.15)
    , numScheduledScalings(0)
    , isPanning(false)
{
    setupZoom();
}

ZoomGraphicsView::ZoomGraphicsView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
    , currentZoom(1.0)
    , minZoom(0.2)
    , maxZoom(5.0)
    , zoomFactor(1.15)
    , numScheduledScalings(0)
    , isPanning(false)
{
    setupZoom();
}

void ZoomGraphicsView::setupZoom()
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::NoDrag);

    // Enable smooth scrolling
    zoomTimeLine = new QTimeLine(200, this);
    zoomTimeLine->setUpdateInterval(20);

    connect(zoomTimeLine, &QTimeLine::valueChanged, this, &ZoomGraphicsView::scalingTime);
    connect(zoomTimeLine, &QTimeLine::finished, this, &ZoomGraphicsView::animFinished);
}

void ZoomGraphicsView::setZoomRange(double minZoom, double maxZoom)
{
    this->minZoom = minZoom;
    this->maxZoom = maxZoom;
}

void ZoomGraphicsView::resetZoom()
{
    resetTransform();
    currentZoom = 1.0;
    centerOn(sceneRect().center());
}

void ZoomGraphicsView::wheelEvent(QWheelEvent *event)
{
    // Get the number of degrees the wheel was rotated
    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;

    numScheduledScalings += numSteps;

    if (numScheduledScalings * numSteps < 0) {
        numScheduledScalings = numSteps;
    }

    // Start animation
    if (zoomTimeLine->state() != QTimeLine::Running) {
        zoomTimeLine->start();
    }

    event->accept();
}

void ZoomGraphicsView::scalingTime(qreal value)
{
    Q_UNUSED(value);

    if (numScheduledScalings == 0) {
        return;
    }

    double factor = numScheduledScalings > 0 ? zoomFactor : 1.0 / zoomFactor;
    double newZoom = currentZoom * factor;

    // Clamp zoom level
    if (newZoom < minZoom) {
        factor = minZoom / currentZoom;
        newZoom = minZoom;
        numScheduledScalings = 0;
    } else if (newZoom > maxZoom) {
        factor = maxZoom / currentZoom;
        newZoom = maxZoom;
        numScheduledScalings = 0;
    }

    if (numScheduledScalings > 0) {
        numScheduledScalings--;
    } else {
        numScheduledScalings++;
    }

    currentZoom = newZoom;
    scale(factor, factor);
}

void ZoomGraphicsView::animFinished()
{
    if (numScheduledScalings > 0) {
        numScheduledScalings--;
    } else {
        numScheduledScalings++;
    }
}

void ZoomGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton ||
        (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier)) {
        isPanning = true;
        lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void ZoomGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (isPanning) {
        QPointF delta = mapToScene(lastPanPoint) - mapToScene(event->pos());
        lastPanPoint = event->pos();

        // Pan the view
        setTransformationAnchor(QGraphicsView::NoAnchor);
        translate(delta.x(), delta.y());
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void ZoomGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton ||
        (event->button() == Qt::LeftButton && isPanning)) {
        isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void ZoomGraphicsView::zoom(double factor, QPointF centerPoint)
{
    double newZoom = currentZoom * factor;

    // Clamp zoom
    if (newZoom < minZoom || newZoom > maxZoom) {
        return;
    }

    currentZoom = newZoom;

    // Zoom with animation
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    scale(factor, factor);
}
