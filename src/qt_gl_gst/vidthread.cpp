
#include "pipeline.h"
#include "vidthread.h"


VidThread::VidThread(int vidIx,
        const QString &videoLocation,
        const char *renderer_slot,
        QObject *parent):
    QThread(parent),
    m_vidIx(vidIx),
    m_videoLocation(videoLocation),
    m_chooseNew(false)
{
#if defined OMAP3530
    m_pipeline = new GstTIPipeline(vidIx, m_videoLocation, this);
#elif defined UNIX
    m_pipeline = new GStreamerPipeline(vidIx, m_videoLocation, this);
#endif

    QObject::connect(m_pipeline, SIGNAL(newFrameReady(int)), this->parent(), renderer_slot, Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(finished()), this, SLOT(reemitFinished()));
}

VidThread::~VidThread()
{
}

void VidThread::stop()
{
    if(m_pipeline)
      m_pipeline->stop();
}

void VidThread::run()
{
    qDebug("Starting gst pipeline");
    m_pipeline->start(); //it runs the gmainloop on win32

#ifndef Q_WS_WIN
    //works like the gmainloop on linux (GstEvent are handled)
    connect(m_pipeline, SIGNAL(stopRequested()), this, SLOT(quit()));
    exec();
#endif

    m_pipeline->unconfigure();

    m_pipeline = NULL;
    // This is not a memory leak. Pipeline will be deleted
    // when the parent object (this) will be destroyed.
    // We set m_pipeline to NULL to prevent further attempts
    // to stop already stopped pipeline
}
