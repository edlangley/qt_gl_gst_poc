//#include <QtGui/QApplication>
#include <QtGui>
//#include "mainwindow.h"

#include "demoscene.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    QPixmapCache::setCacheLimit(100 * 1024); // 100 MB

    DemoScene *scene = new DemoScene();

    DemoView view;
    view.resize(800, 600);
    view.setScene(scene);
    view.show();

    return a.exec();
}
