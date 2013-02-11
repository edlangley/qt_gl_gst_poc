#include "yuvdebugwindow.h"

YuvDebugWindow::YuvDebugWindow(QWidget *parent) :
    QDialog(parent)
{
    imageLabel = new QLabel(this);

    QImage yuvImage("./alphamasks/fade.jpg");
    if(yuvImage.isNull())
    {
        qDebug("yuvImage not loaded");
    }
    else
    {

        imageLabel->setPixmap(QPixmap::fromImage(yuvImage));
    }

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(imageLabel);
    setLayout(layout);
}
