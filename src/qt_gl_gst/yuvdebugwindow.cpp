#include "yuvdebugwindow.h"

YuvDebugWindow::YuvDebugWindow(QWidget *parent) :
    QDialog(parent)
{
    m_imageLabel = new QLabel(this);
#if 0
    QImage yuvImage("./alphamasks/fade.jpg");
    if(yuvImage.isNull())
    {
        qDebug("yuvImage not loaded");
    }
    else
    {

        m_imageLabel->setPixmap(QPixmap::fromImage(yuvImage));
    }
#endif
    m_imageLabel->setText("Video will be shown here");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_imageLabel);
    setLayout(layout);
}
