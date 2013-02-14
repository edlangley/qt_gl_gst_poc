#include "yuvdebugwindow.h"

YuvDebugWindow::YuvDebugWindow(QWidget *parent) :
    QDialog(parent)
{
    imageLabel = new QLabel(this);
#if 0
    QImage yuvImage("./alphamasks/fade.jpg");
    if(yuvImage.isNull())
    {
        qDebug("yuvImage not loaded");
    }
    else
    {

        imageLabel->setPixmap(QPixmap::fromImage(yuvImage));
    }
#endif
    imageLabel->setText("Video will be shown here");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(imageLabel);
    setLayout(layout);
}
