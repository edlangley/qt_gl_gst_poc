#ifndef DEMOSCENE_H
#define DEMOSCENE_H

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QPaintEngine>
#include <QTimer>
#include <QTime>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGLWidget>
#include <QGraphicsProxyWidget>


#include "camera.h"

class DemoItem;

class DemoScene : public QGraphicsScene
{
    Q_OBJECT
public:
    DemoScene();

    void setAcceleratedViewport();


protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    bool eventFilter(QObject *target, QEvent *event);

    bool handleKey(int key, bool pressed);

public slots:
    void move();
    void updateRenderer();

private:
    void updateTransforms();

    Camera m_camera;

    qreal m_walkingVelocity;
    qreal m_strafingVelocity;
    qreal m_turningSpeed;
    qreal m_pitchSpeed;

    qreal m_deltaYaw;
    qreal m_deltaPitch;

    QTime m_time;
    long m_simulationTime;
    long m_walkTime;

    // Couple of example items:
    DemoItem *m_webviewitem;
    DemoItem *m_formitem;
};


class DemoView: public QGraphicsView
{
    Q_OBJECT
public:
    explicit DemoView();
    void resizeEvent(QResizeEvent *event);
    void setScene(DemoScene *scene);
    void paintEvent(QPaintEvent *event);

private:
    DemoScene *m_scene;
    bool m_firstPaint;

};


class DemoItem : public QGraphicsItem
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



#endif // DEMOSCENE_H
