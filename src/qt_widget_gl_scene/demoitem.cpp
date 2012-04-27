#include "demoscene.h"
#include "demoitem.h"
#include "demoview.h"


TransformItem::TransformItem(DemoScene *scene, const QPointF &a, const QPointF &b)
//    : m_bounds(QRectF(-0.5, -0.5, 1.0, 1.0))
{
    m_a = a;
    m_b = b;

    m_bounds = QRectF(-0.5, -0.5, 1.0, 1.0);
}

void TransformItem::updateTransform(const Camera &camera)
{
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

DemoItem::DemoItem(DemoScene *scene, const QPointF &a, const QPointF &b, int type)
    : m_bounds(QRectF(-0.5, -0.5, 1.0, 1.0)),
      m_obscured(false)
{
    m_a = a;
    m_b = b;
    m_scale = 0.8;

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

#if 1
    m_childItem = new QGraphicsProxyWidget(this);
    m_childItem->setWidget(childWidget);
    m_childItem->setCacheMode(QGraphicsItem::ItemCoordinateCache);

    QRectF rect = m_childItem->boundingRect();
    QPointF center = rect.center();

    qreal scale = qMin(m_scale / rect.width(), m_scale / rect.height());
    m_childItem->translate(0, -0.05);
    m_childItem->scale(scale, scale);
    m_childItem->translate(-center.x(), -center.y());

    //scene->addItem(m_childItem);
#else
    setWidget(childWidget);
    setCacheMode(QGraphicsItem::ItemCoordinateCache);

    scene->addItem(this);
#endif
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


void DemoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    // Just let children draw themselves
    QGraphicsProxyWidget::paint(painter, item, widget);

    painter->drawRect(-1,-1,2,2);
    if(m_childItem)
        m_childItem->update();
}

