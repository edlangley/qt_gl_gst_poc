#include "mainwindow.h"

#include "glwidget.h"
#include "controlsform.h"

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent) :
    QMainWindow(parent)
{
    //GLWidget *glWidget = new GLWidget(argc, argv, this);
    //setCentralWidget(glWidget);

//    GLWidget *glWidget = new GLWidget(argc, argv, this);
//    ControlsForm *controlsForm = new ControlsForm(this);
//    QVBoxLayout *layout = new QVBoxLayout;
//    layout->addWidget(glWidget);
//    layout->addWidget(controlsForm);
//    layout->setStretchFactor(glWidget, 10000);
//    layout->setStretchFactor(controlsForm, 0);
//    setLayout(layout);

    GLWidget *glWidget = new GLWidget(argc, argv, this);
    ControlsForm *controlsForm = new ControlsForm(glWidget, this);
    setCentralWidget(controlsForm);

}
