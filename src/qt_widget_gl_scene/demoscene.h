#ifndef DEMOSCENE_H
#define DEMOSCENE_H


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
class TransformItem;

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
    QGraphicsProxyWidget *m_groupBox;
    TransformItem *m_transformItem;
};









#endif // DEMOSCENE_H
