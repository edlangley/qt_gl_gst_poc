//#include <QtGui/QApplication>
#include <QtGui>
//#include "mainwindow.h"

#include "demoscene.h"
#include "demoitem.h"
#include "demoview.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    QPixmapCache::setCacheLimit(100 * 1024); // 100 MB

    DemoScene *scene = new DemoScene();
//    QGraphicsScene *scene = new QGraphicsScene();

/*
    QGroupBox *groupBox = new QGroupBox("Contact Details");
    QLabel *numberLabel = new QLabel("Telephone number");
    QLineEdit *numberEdit = new QLineEdit;
    QFormLayout *layout = new QFormLayout;
    layout->addRow(numberLabel, numberEdit);
    groupBox->setLayout(layout);
    scene->addWidget(groupBox);
*/

    DemoView view;
//    QGraphicsView view;
    view.resize(800, 600);
    view.setScene(scene);
    view.show();

    return a.exec();
}
