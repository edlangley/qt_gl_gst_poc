#include "demoscene.h"

#include <QCheckBox>
#include <QPushButton>
#include <QGraphicsWebView>
#include <QVBoxLayout>


DemoView::DemoView()
    : m_scene(0)
    , m_firstPaint(true)
{
}

void DemoView::setScene(DemoScene *scene)
{
    QGraphicsView::setScene(scene);

    m_scene = scene;
//    m_scene->viewResized(this);
}

void DemoView::resizeEvent(QResizeEvent *)
{
    resetMatrix();
    qreal factor = width() / 4.0;
    scale(factor, factor);

//    if (m_scene)
//        m_scene->viewResized(this);
}

void DemoView::paintEvent(QPaintEvent *event)
{
    if (m_firstPaint)
    {
        static_cast<DemoScene *>(scene())->setAcceleratedViewport();

        m_firstPaint = false;
    }

    QGraphicsView::paintEvent(event);
}


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
    m_webviewitem->setVisible(true);
    m_formitem = new DemoItem(this, QPointF(3, 3), QPointF(3, 2), 1);
    addItem(m_formitem);
    m_formitem->setVisible(true);

    setSceneRect(-1, -1, 2, 2);

    updateTransforms();

}

void DemoScene::updateTransforms()
{
    // obscuring non-visible items, don't bother for now:
/*
    Span span;
    span.item = 0;
    span.sx1 = -std::numeric_limits<float>::infinity();
    span.sx2 =  std::numeric_limits<float>::infinity();
    span.cy = std::numeric_limits<float>::infinity();

    QList<Span> visibleList;
    visibleList << span;

    QTransform rotation;
    rotation *= QTransform().translate(-m_camera.pos().x(), -m_camera.pos().y());
    rotation *= rotatingTransform(m_camera.yaw());

    // first add all opaque items
    foreach (ProjectedItem *item, m_projectedItems) {
        if (item->isOpaque()) {
            item->setObscured(true);
            insertProjectedItem(visibleList, item, rotation, false);
        }
    }

    // mark visible opaque items
    for (int i = 0; i < visibleList.size(); ++i)
        if (visibleList.at(i).item)
            visibleList.at(i).item->setObscured(false);

    // now add all non-opaque items
    foreach (ProjectedItem *item, m_projectedItems) {
        if (!item->isOpaque())
            item->setObscured(!insertProjectedItem(visibleList, item, rotation, true));
    }

    foreach (ProjectedItem *item, m_projectedItems)
        item->updateTransform(m_camera);

    setFocusItem(0); // setVisible(true) might give focus to one of the items
    update();
*/

    // just call updateTransform(m_camera); on some items?
    // Handle items here
    m_webviewitem->updateTransform(m_camera);
    m_formitem->updateTransform(m_camera);

    update();
}

void DemoScene::move()
{
//    QSet<Entity *> movedEntities;
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
            //if (tryMove(pos, walkingDelta)) {

            pos.setX(pos.x() + walkingDelta.x());
            pos.setY(pos.y() + walkingDelta.y());

            walking = true;
            m_camera.setPos(pos);
            //}
        }

        if (m_strafingVelocity != 0) {
            QPointF walkingDelta = QLineF::fromPolar(m_strafingVelocity, m_camera.yaw()).p2();
            QPointF pos = m_camera.pos();
            //if (tryMove(pos, walkingDelta)) {

            pos.setX(pos.x() + walkingDelta.x());
            pos.setY(pos.y() + walkingDelta.y());

            walking = true;
            m_camera.setPos(pos);
            //}
        }

        walked = walked || walking;

        if (walking)
            m_walkTime += stepSize;
        m_simulationTime += stepSize;

/*        foreach (Entity *entity, m_entities) {
            if (entity->move(this))
                movedEntities.insert(entity);
        }
*/
    }

    m_camera.setTime(m_walkTime * 0.001);

    if (walked || m_deltaYaw != 0 || m_deltaPitch != 0) {
        updateTransforms();
    }
/*    else {
        foreach (Entity *entity, movedEntities)
            entity->updateTransform(m_camera);
    }
*/
    if (steps) {
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
    foreach (WallItem *item, m_walls) {
        QGraphicsProxyWidget *proxy = item->childItem();
        if (proxy && proxy->widget() == widget)
            item->childResized();
    }
*/
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

DemoItem::DemoItem(DemoScene *scene, const QPointF &a, const QPointF &b, int type)
    : m_bounds(QRectF(-0.5, -0.5, 1.0, 1.0)),
      m_obscured(false)
{
    m_a = a;
    m_b = b;

    m_childItem = 0;
    QWidget *childWidget = 0;

    // derive classes instead later:
    switch(type)
    {
    case 0:
        {
            QGraphicsWebView *view = new QGraphicsWebView(this);
            view->setCacheMode(QGraphicsItem::ItemCoordinateCache);
            view->setResizesToContents(false);
            view->setGeometry(QRectF(0, 0, 800, 600));
            view->setUrl(QUrl(QLatin1String("http://www.google.com")));

            QRectF rect = view->boundingRect();
            QPointF center = rect.center();
            qreal scale = qMin(m_scale / rect.width(), m_scale / rect.height());
            view->translate(0, -0.05);
            view->scale(scale, scale);
            view->translate(-center.x(), -center.y());
        }
        break;
    default:
    case 1:
        {
            QWidget *widget = new QWidget();
            QPushButton *button = new QPushButton("Button", widget);
            QCheckBox *checkBox = new QCheckBox("Tickbox", widget);
            widget->setLayout(new QVBoxLayout);
            widget->layout()->addWidget(button);
            widget->layout()->addWidget(checkBox);
            childWidget = widget;
        }
        break;

    }

    if (!childWidget)
        return;

    childWidget->installEventFilter(scene);

    m_childItem = new QGraphicsProxyWidget(this);
//    m_childItem = new ProxyWidget(this);
    m_childItem->setWidget(childWidget);
    m_childItem->setCacheMode(QGraphicsItem::ItemCoordinateCache);

    QRectF rect = m_childItem->boundingRect();
    QPointF center = rect.center();

    qreal scale = qMin(m_scale / rect.width(), m_scale / rect.height());
    m_childItem->translate(0, -0.05);
    m_childItem->scale(scale, scale);
    m_childItem->translate(-center.x(), -center.y());

    scene->addItem(m_childItem);
}

void DemoItem::updateTransform(const Camera &camera)
{
    if (!m_obscured) {
        QTransform rotation;
        rotation *= QTransform().translate(-camera.pos().x(), -camera.pos().y());
        rotation *= rotatingTransform(camera.yaw());
        QPointF center = (m_a + m_b) / 2;

        QPointF ca = rotation.map(m_a);
        QPointF cb = rotation.map(m_b);

        if (ca.y() > 0 || cb.y() > 0) {
            QMatrix4x4 m = camera.viewProjectionMatrix();
            m.translate(center.x(), 0, center.y());
            m *= fromRotation(-QLineF(m_b, m_a).angle(), Qt::YAxis);

            qreal zm = QLineF(camera.pos(), center).length();

            setVisible(true);
            setZValue(-zm);
            setTransform(m.toTransform(0));
            return;
        }
    }

    // hide the item by placing it far outside the scene
    // we could use setVisible() but that causes unnecessary
    // update to cahced items
/*
    QTransform transform;
    transform.translate(-1000, -1000);
    setTransform(transform);
*/
}

void DemoItem::childResized()
{
    QRectF rect = m_childItem->boundingRect();

    QPointF center = rect.center();

    qreal scale = qMin(m_scale / rect.width(), m_scale / rect.height());
    m_childItem->resetMatrix();
    m_childItem->translate(0, -0.05);
    m_childItem->scale(scale, scale);
    m_childItem->translate(-center.x(), -center.y());

    // refresh cache size
    m_childItem->setCacheMode(QGraphicsItem::NoCache);
    m_childItem->setCacheMode(QGraphicsItem::ItemCoordinateCache);

}

void DemoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // Just let children draw themselves


    painter->drawRect(-1,-1,2,2);
    if(m_childItem)
        m_childItem->update();
}
