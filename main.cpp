#include <QtGui/QApplication>
#include "glwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GLWidget widget(argc, argv);
    //widget.show();
    widget.showFullScreen();

    return a.exec();
}
