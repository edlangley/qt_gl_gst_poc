#include <QApplication>
//#include "glwidget.h"
#include "mainwindow.h"

/*
#include <QMainWindow>
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow() {};
    MainWindow(int argc, char *argv[], QWidget *parent = 0);

private:
};

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent)
{
    GLWidget *widget = new GLWidget(argc, argv);

    setCentralWidget(widget);
}
*/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //GLWidget widget(argc, argv);
    //widget.show();
    //widget.showFullScreen();

    MainWindow mainWindow(argc, argv);
    mainWindow.show();

    return a.exec();
}
