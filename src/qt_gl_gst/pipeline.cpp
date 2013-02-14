
#include "pipeline.h"

Pipeline::Pipeline(int vidIx,
                   const QString &videoLocation,
                   QObject *parent)
  : QObject(parent),
    m_vidIx(vidIx),
    m_videoLocation(videoLocation),
    m_colFormat(ColFmt_Unknown),
    m_vidInfoValid(false)
{
}

Pipeline::~Pipeline()
{
}
