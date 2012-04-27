#include "demoscene.h"
#include "demoitem.h"
#include "demoview.h"


DemoScene::DemoScene()
    : m_walkingVelocity(0)
    , m_strafingVelocity(0)
    , m_turningSpeed(0)
    , m_pitchSpeed(0)
    , m_deltaYaw(0)
    , m_deltaPitch(0)
{
    m_camera.setPos(QPointF(1.5, 1.5));
    m_camera.setYaw(0.1);

    QTimer *timer = new QTimer(this);
    timer->setInterval(20);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(move()));

    m_time.start();

    m_webviewitem = NULL;
    m_formitem = NULL;
    m_webviewitem = new DemoItem(this, QPointF(0, 0), QPointF(1, 0), 0);
    addItem(m_webviewitem);

    m_formitem = new DemoItem(this, QPointF(3, 3), QPointF(3, 2), 1);
    addItem(m_formitem);

/*
    QGroupBox *groupBox = new QGroupBox("Contact Details");
    QLabel *numberLabel = new QLabel("Telephone number");
    QLineEdit *numberEdit = new QLineEdit;
    QFormLayout *layout = new QFormLayout;
    layout->addRow(numberLabel, numberEdit);
    groupBox->setLayout(layout);
//    m_groupBox = addWidget(groupBox);


    m_transformItem = new TransformItem(this, QPointF(2, 3), QPointF(3, 2));
    m_transformItem->setWidget(groupBox);
    addItem(m_transformItem);
*/

    setSceneRect(-1, -1, 2, 2);

    updateTransforms();

}

void DemoScene::updateTransforms()
{
    // Handle items here
    m_webviewitem->updateTransform(m_camera);
    m_formitem->updateTransform(m_camera);

//    m_transformItem->updateTransform(m_camera);

    update();
}

void DemoScene::move()
{
    long elapsed = m_time.elapsed();
    bool walked = false;

    const int stepSize = 5;
    int steps = (elapsed - m_simulationTime) / stepSize;

    if (steps) {
        m_deltaYaw /= steps;
        m_deltaPitch /= steps;

        m_deltaYaw += m_turningSpeed;
        m_deltaPitch += m_pitchSpeed;
    }

    qreal walkingVelocity = m_walkingVelocity;

    for (int i = 0; i < steps; ++i) {
        m_camera.setYaw(m_camera.yaw() + m_deltaYaw);
        m_camera.setPitch(m_camera.pitch() + m_deltaPitch);

        bool walking = false;
        if (walkingVelocity != 0) {
            QPointF walkingDelta = QLineF::fromPolar(walkingVelocity, m_camera.yaw() - 90).p2();
            QPointF pos = m_camera.pos();

            pos.setX(pos.x() + walkingDelta.x());
            pos.setY(pos.y() + walkingDelta.y());

            walking = true;
            m_camera.setPos(pos);
        }

        if (m_strafingVelocity != 0) {
            QPointF walkingDelta = QLineF::fromPolar(m_strafingVelocity, m_camera.yaw()).p2();
            QPointF pos = m_camera.pos();

            pos.setX(pos.x() + walkingDelta.x());
            pos.setY(pos.y() + walkingDelta.y());

            walking = true;
            m_camera.setPos(pos);
        }

        walked = walked || walking;

        if (walking)
            m_walkTime += stepSize;
        m_simulationTime += stepSize;

    }

    m_camera.setTime(m_walkTime * 0.001);

    if (walked || m_deltaYaw != 0 || m_deltaPitch != 0)
    {
        updateTransforms();
    }

    if (steps)
    {
        m_deltaYaw = 0;
        m_deltaPitch = 0;
    }
}

void DemoScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (focusItem()) {
        QGraphicsScene::mouseMoveEvent(event);
        return;
    }

    if (event->buttons() & Qt::LeftButton) {
        QPointF delta(event->scenePos() - event->lastScenePos());
        m_deltaYaw += delta.x() * 80;
        m_deltaPitch -= delta.y() * 80;
    }
}

void DemoScene::keyPressEvent(QKeyEvent *event)
{
    if (handleKey(event->key(), true)) {
        event->accept();
        return;
    }

    QGraphicsScene::keyPressEvent(event);
}

void DemoScene::keyReleaseEvent(QKeyEvent *event)
{
    if (handleKey(event->key(), false)) {
        event->accept();
        return;
    }

    QGraphicsScene::keyReleaseEvent(event);
}

bool DemoScene::handleKey(int key, bool pressed)
{
    if (focusItem())
        return false;

    switch (key) {
    case Qt::Key_Left:
    case Qt::Key_Q:
        m_turningSpeed = (pressed ? -0.5 : 0.0);
        return true;
    case Qt::Key_Right:
    case Qt::Key_E:
        m_turningSpeed = (pressed ? 0.5 : 0.0);
        return true;
    case Qt::Key_Down:
        m_pitchSpeed = (pressed ? 0.5 : 0.0);
        return true;
    case Qt::Key_Up:
        m_pitchSpeed = (pressed ? -0.5 : 0.0);
        return true;
    case Qt::Key_S:
        m_walkingVelocity = (pressed ? -0.01 : 0.0);
        return true;
    case Qt::Key_W:
        m_walkingVelocity = (pressed ? 0.01 : 0.0);
        return true;
    case Qt::Key_A:
        m_strafingVelocity = (pressed ? -0.01 : 0.0);
        return true;
    case Qt::Key_D:
        m_strafingVelocity = (pressed ? 0.01 : 0.0);
        return true;

    }

    return false;
}

bool DemoScene::eventFilter(QObject *target, QEvent *event)
{
    QWidget *widget = qobject_cast<QWidget *>(target);
    if (!widget || event->type() != QEvent::Resize)
        return false;

/*

    // Handle items here
    QGraphicsProxyWidget *proxy = m_webviewitem->childItem();
    if (proxy && proxy->widget() == widget)
        m_webviewitem->childResized();

    if(m_formitem) // this will be better when items are stored in a QVector or something
    {
        proxy = m_formitem->childItem();
        if (proxy && proxy->widget() == widget)
            m_formitem->childResized();
    }
*/
    return false;
}


// Called on first paint:
void DemoScene::setAcceleratedViewport()
{
    QTimer::singleShot(0, this, SLOT(updateRenderer()));
}


void DemoScene::updateRenderer()
{
    if (views().size() == 0)
    {
        qDebug() << "Scene has no viewport\n";
        return;
    }

    QGraphicsView *view = views().at(0);
    if (view)
    {
        view->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    }

    if (view->viewport()->inherits("QGLWidget"))
    {
        view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        qDebug() << "Viewport is accelerated\n";
    }
    else
    {
        view->setRenderHints(QPainter::Antialiasing);
        qDebug() << "Viewport is not accelerated\n";
    }

}

/*
void DemoScene::viewResized(QGraphicsView *view)
{
    QRectF bounds = m_walkingItem->sceneBoundingRect();
    QPointF bottomLeft = view->mapToScene(QPoint(5, view->height() - 5));

    m_walkingItem->setPos(bottomLeft.x(), bottomLeft.y() - bounds.height());
}
*/

class ProxyWidget : public QGraphicsProxyWidget
{
public:
    ProxyWidget(QGraphicsItem *parent = 0)
        : QGraphicsProxyWidget(parent)
    {
    }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant & value)
    {
        // we want the position of proxy widgets to stay at (0, 0)
        // so ignore the position changes from the native QWidget
        if (change == ItemPositionChange)
            return QPointF();
        else
            return QGraphicsProxyWidget::itemChange(change, value);
    }
};

