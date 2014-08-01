#ifndef PIPELINE_H
#define PIPELINE_H

#include <QWidget>
#include "asyncwaitingqueue.h"

#define COLFMT_FOUR_CC(a,b,c,d) \
    ((unsigned long) ((a) | (b)<<8 | (c)<<16 | (d)<<24))

typedef enum _ColFormat
{
    // these relate to fourCC codes, but abstract video framework system from outside:
    ColFmt_I420 = COLFMT_FOUR_CC('I', '4', '2', '0'),
    ColFmt_IYUV = COLFMT_FOUR_CC('I', 'Y', 'U', 'V'),
    ColFmt_YV12 = COLFMT_FOUR_CC('Y', 'V', '1', '2'),
    ColFmt_YUYV = COLFMT_FOUR_CC('Y', 'U', 'Y', 'V'),
    ColFmt_YUY2 = COLFMT_FOUR_CC('Y', 'U', 'Y', '2'),
    ColFmt_V422 = COLFMT_FOUR_CC('V', '4', '2', '2'),
    ColFmt_YUNV = COLFMT_FOUR_CC('Y', 'U', 'N', 'V'),
    ColFmt_UYVY = COLFMT_FOUR_CC('U', 'Y', 'V', 'Y'),
    ColFmt_Y422 = COLFMT_FOUR_CC('Y', '4', '2', '2'),
    ColFmt_UYNV = COLFMT_FOUR_CC('U', 'Y', 'N', 'V'),

    // Also capture RGBs in the same enum
    ColFmt_RGB888 = COLFMT_FOUR_CC('R', 'G', 'B', '8'),
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


