#ifndef DEMOITEM_H
#define DEMOITEM_H

#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>

#include <QCheckBox>
#include <QPushButton>
#include <QGraphicsWebView>
#include <QVBoxLayout>

class TransformItem : public QGraphicsProxyWidget
{
public:

    explicit TransformItem(DemoScene *scene, const QPointF &a, const QPointF &b);

    virtual void updateTransform(const Camera &camera);

    QRectF boundingRect() const { return m_bounds; }
    void setPosition(const QPointF &a, const QPointF &b) { m_a = a; m_b = b; }
    QPointF a() const { return m_a; }
    QPointF b() const { return m_b; }

private:
    QPointF m_a;
    QPointF m_b;

    QRectF m_bounds;
};

class DemoItem : public QGraphicsProxyWidget
//class DemoItem : public QGraphicsItem
{
public:
    explicit DemoItem(DemoScene *scene, const QPointF &a, const QPointF &b, int type);

    virtual void updateTransform(const Camera &camera);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void childResized();

    QRectF boundingRect() const { return m_bounds; }
    QGraphicsProxyWidget *childItem() const { return m_childItem; }
    void setPosition(const QPointF &a, const QPointF &b) { m_a = a; m_b = b; }
    QPointF a() const { return m_a; }
    QPointF b() const { return m_b; }

private:
    QPointF m_a;
    QPointF m_b;
    QRectF m_bounds;
    bool m_obscured;
    qreal m_scale;


    // move to derived?:
    QGraphicsProxyWidget *m_childItem;


};

#endif // DEMOITEM_H
