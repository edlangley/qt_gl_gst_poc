#include "mainwindow.h"

#ifdef OMAP3530
 #include "glpowervrwidget.h"
#else
 #include "glwidget.h"
#endif
#include "controlsform.h"

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent) :
    QMainWindow(parent)
{
#ifdef OMAP3530
    GLWidget *glWidget = new GLPowerVRWidget(argc, argv, this);
#else
    GLWidget *glWidget = new GLWidget(argc, argv, this);
#endif
    ControlsForm *controlsForm = new ControlsForm(glWidget, this);
    setCentralWidget(controlsForm);
}
