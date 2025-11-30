#include "graphNode.h"
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QRadialGradient>
#include <QCursor>

GraphNode::GraphNode(const QString &nodeId, const QPointF &pos,
                     double size, const QColor &color,
                     const QString &displayText,
                     QGraphicsItem *parent)
    : QGraphicsEllipseItem(-size, -size, size * 2, size * 2, parent)
    , nodeId(nodeId)
    , isDragging(false)
    , baseSize(size)
    , baseColor(color)
{
    setPos(pos);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setCursor(QCursor(Qt::OpenHandCursor));
    setZValue(10);  // Draw on top of edges

    // Gradient fill for modern look
    QRadialGradient gradient(0, 0, size);
    gradient.setColorAt(0, color.lighter(120));
    gradient.setColorAt(1, color);
    setBrush(QBrush(gradient));
    setPen(QPen(QColor(255, 255, 255), 2));

    // Create label
    label = new QGraphicsTextItem(this);
    QFont font;
    font.setPointSize(baseSize > 15 ? 10 : 9);
    font.setBold(true);
    label->setFont(font);
    label->setDefaultTextColor(Qt::white);

    QString text = displayText.isEmpty() ? nodeId : displayText;
    if (text.length() > 12 && displayText.isEmpty()) {
        text = text.left(10) + "...";
    }
    label->setPlainText(text);

    // Center the label
    QRectF labelRect = label->boundingRect();
    label->setPos(-labelRect.width()/2, -labelRect.height()/2);

    // Add shadow effect for depth
    label->setDefaultTextColor(QColor(255, 255, 255));
}

void GraphNode::setNodePosition(const QPointF &pos)
{
    setPos(pos);
}

QPointF GraphNode::getCenter() const
{
    return pos();
}

void GraphNode::setHighlight(bool highlight)
{
    if (highlight) {
        double highlightSize = baseSize * 1.3;
        setRect(-highlightSize, -highlightSize, highlightSize * 2, highlightSize * 2);

        QRadialGradient gradient(0, 0, highlightSize);
        gradient.setColorAt(0, QColor(255, 200, 100));
        gradient.setColorAt(1, QColor(255, 150, 50));
        setBrush(QBrush(gradient));
        setPen(QPen(QColor(255, 255, 255), 3));
        setZValue(100);  // Bring to front when highlighted
    } else {
        setRect(-baseSize, -baseSize, baseSize * 2, baseSize * 2);

        QRadialGradient gradient(0, 0, baseSize);
        gradient.setColorAt(0, baseColor.lighter(120));
        gradient.setColorAt(1, baseColor);
        setBrush(QBrush(gradient));
        setPen(QPen(QColor(255, 255, 255), 2));
        setZValue(10);
    }
}

void GraphNode::updateLabel()
{
    QRectF labelRect = label->boundingRect();
    label->setPos(-labelRect.width()/2, -labelRect.height()/2);
}

void GraphNode::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setHighlight(true);
    setCursor(QCursor(Qt::OpenHandCursor));
    QGraphicsEllipseItem::hoverEnterEvent(event);
}

void GraphNode::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!isDragging) {
        setHighlight(false);
    }
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

void GraphNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        dragStartPos = pos();
        setCursor(QCursor(Qt::ClosedHandCursor));
        setHighlight(true);
    }
    QGraphicsEllipseItem::mousePressEvent(event);
}

void GraphNode::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (isDragging) {
        QGraphicsEllipseItem::mouseMoveEvent(event);
    }
}

void GraphNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        setCursor(QCursor(Qt::OpenHandCursor));
        if (!isUnderMouse()) {
            setHighlight(false);
        }
    }
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}
