#include "demoscene.h"
#include "demoitem.h"
#include "demoview.h"

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


