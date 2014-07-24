#ifndef PIPELINE_H
#define PIPELINE_H

#include <QWidget>
#include "asyncwaitingqueue.h"


typedef enum _ColFormat
{
    // these relate to fourCC codes, but abstract video framework system from outside:
    ColFmt_I420,
    ColFmt_IYUV,
    ColFmt_YV12,
    ColFmt_YUYV,
    ColFmt_YUY2,
    ColFmt_V422,
    ColFmt_YUNV,
    ColFmt_UYVY,
    ColFmt_Y422,
    ColFmt_UYNV,

    ColFmt_RGB888,
    ColFmt_BGR888,
    ColFmt_ARGB8888,
    ColFmt_BGRA8888,

    ColFmt_Unknown
} ColFormat;

class Pipeline : public QObject
{
    Q_OBJECT

public:
    Pipeline(int vidIx,
        const QString &videoLocation,
        const char *renderer_slot,
        QObject *parent);
    ~Pipeline();

    virtual void Configure() = 0;
    virtual void Start() = 0;
    void NotifyNewFrame() { emit newFrameReady(m_vidIx); }

    int getVidIx() { return m_vidIx; }
    int getWidth() { return m_width; }
    int getHeight() { return m_height; }
    ColFormat getColourFormat() { return m_colFormat; }
    quint32 getFourCC() { return m_fourCC; }
    virtual unsigned char *bufToVidDataStart(void *buf) = 0;

    bool isFinished() { return this->m_finished; }

    AsyncQueue<void*> m_incomingBufQueue;
    AsyncQueue<void*> m_outgoingBufQueue;

Q_SIGNALS:
    void newFrameReady(int vidIx);
    void finished(int vidIx);

public slots:
    virtual void Stop() = 0;

protected:
    int m_vidIx;
    const QString m_videoLocation;
    int m_width;
    int m_height;
    ColFormat m_colFormat;
    quint32 m_fourCC;
    bool m_vidInfoValid;
    bool m_finished;
};

#if defined OMAP3530

 // Don't include derived classes if this include is from the base class
 // header file due to the MOC. In that case, base class is not defined
 // yet but inclusion guard is defined.
 #ifndef GSTPIPELINE_H
  #include "gstpipeline.h"
  #ifndef TIGSTPIPELINE_H
   #include "tigstpipeline.h"
  #endif
 #endif

#elif defined UNIX
 #include "gstpipeline.h"
#endif

#endif // PIPELINE_H


