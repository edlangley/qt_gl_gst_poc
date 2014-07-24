#include "yuvdebugwindow.h"

YuvDebugWindow::YuvDebugWindow(QWidget *parent) :
    QDialog(parent)
{
    m_imageLabel = new QLabel(this);
    m_imageLabel->setText("Video will be shown here");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_imageLabel);
    setLayout(layout);
}
