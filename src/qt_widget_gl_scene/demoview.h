#ifndef DEMOVIEW_H
#define DEMOVIEW_H

#include "demoscene.h"
#include <QGraphicsView>

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

#endif // DEMOVIEW_H
