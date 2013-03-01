#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow mainWindow(argc, argv);
    //mainWindow.show();
    mainWindow.showFullScreen();

    return a.exec();
}
