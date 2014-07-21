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
    LOG(LOG_GL, Logger::Debug1, "Test Debug1 log at default level, no format args");
    LOG(LOG_GL, Logger::Debug2, "Test Debug2 log at default level, with an arg %d", 5);
    LOG(LOG_GL, Logger::Warning, "Test Warning log at default level");
    LOG(LOG_GL, Logger::Error, "Test Error log at default level, with some args %d, %s, %lu", 6, "Hello!", 12345678);
    GlobalLog.SetModuleLogLevel(LOG_GL, Logger::Debug2);
    LOG(LOG_GL, Logger::Debug2, "Test Debug2 log at Debug2 level");
    LOG(LOG_GL, Logger::Info, "Test Info log at Debug2 level");
    LOG(LOG_GL, Logger::Error, "Test Error log at Debug2 level, with an arg %d", 6);

#ifdef OMAP3530
    GLWidget *glWidget = new GLPowerVRWidget(argc, argv, this);
#else
    GLWidget *glWidget = new GLWidget(argc, argv, this);
#endif
    glWidget->initVideo();

    ControlsForm *controlsForm = new ControlsForm(glWidget, this);
    setCentralWidget(controlsForm);
}
