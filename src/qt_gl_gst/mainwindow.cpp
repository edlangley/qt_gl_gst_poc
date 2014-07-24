#include "mainwindow.h"
#include "applogger.h"

#ifdef OMAP3530
 #include "glpowervrwidget.h"
#else
 #include "glwidget.h"
#endif
#include "controlsform.h"

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent) :
    QMainWindow(parent)
{
    GlobalLog.SetModuleLogLevel(LOG_GL, Logger::Info);
    GlobalLog.SetModuleLogLevel(LOG_GLSHADERS, Logger::Info);
    GlobalLog.SetModuleLogLevel(LOG_OBJLOADER, Logger::Info);
    GlobalLog.SetModuleLogLevel(LOG_VIDPIPELINE, Logger::Info);

#ifdef OMAP3530
    GLWidget *glWidget = new GLPowerVRWidget(argc, argv, this);
#else
    GLWidget *glWidget = new GLWidget(argc, argv, this);
#endif
    glWidget->initVideo();

    ControlsForm *controlsForm = new ControlsForm(glWidget, this);
    setCentralWidget(controlsForm);
}
