
#include "pipeline.h"

Pipeline::Pipeline(int vidIx,
                   const QString &videoLocation,
                   const char *renderer_slot,
                   QObject *parent)
  : QObject(parent),
    m_vidIx(vidIx),
    m_videoLocation(videoLocation),
    m_colFormat(ColFmt_Unknown),
    m_vidInfoValid(false),
    m_finished(false)
{
    QObject::connect(this, SIGNAL(newFrameReady(int)), this->parent(), renderer_slot, Qt::QueuedConnection);
}

Pipeline::~Pipeline()
{
}
