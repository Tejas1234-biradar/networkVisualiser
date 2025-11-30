#ifndef GRAPHNODE_H
#define GRAPHNODE_H

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

class GraphNode : public QGraphicsEllipseItem
{
public:
    GraphNode(const QString &nodeId, const QPointF &pos,
              double size = 15.0, const QColor &color = QColor(69, 183, 209),
              const QString &displayText = QString(),
              QGraphicsItem *parent = nullptr);

    void setNodePosition(const QPointF &pos);
    QPointF getCenter() const;
    void setHighlight(bool highlight);
    void updateLabel();
    QString getNodeId() const { return nodeId; }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QString nodeId;
    QGraphicsTextItem *label;
    bool isDragging;
    QPointF dragStartPos;
    double baseSize;
    QColor baseColor;
};

#endif // GRAPHNODE_H
