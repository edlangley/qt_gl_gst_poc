#include "mainwindow.h"

#include "glwidget.h"
#include "controlsform.h"

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent) :
    QMainWindow(parent)
{
    GLWidget *glWidget = new GLWidget(argc, argv, this);
    ControlsForm *controlsForm = new ControlsForm(glWidget, this);
    setCentralWidget(controlsForm);
}
