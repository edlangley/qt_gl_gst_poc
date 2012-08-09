#include "mainwindow.h"

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent) :
    QMainWindow(parent)
{
    GLWidget *glWidget = new GLWidget(argc, argv, this);

    setCentralWidget(glWidget);
}
