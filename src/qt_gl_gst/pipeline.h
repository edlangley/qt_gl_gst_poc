#ifndef PIPELINE_H
#define PIPELINE_H

#include <QWidget>

#include "AsyncQueue.h"

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
        QObject *parent);
    ~Pipeline();

    virtual void configure() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void unconfigure() = 0;
    void notifyNewFrame() {emit newFrameReady(m_vidIx);}

    int getVidIx() { return m_vidIx; }
    int getWidth() { return m_width; }
    int getHeight() { return m_height; }
    ColFormat getColourFormat() { return m_colFormat; }

    //AsyncQueue<GstBuffer*> queue_input_buf;
    //AsyncQueue<GstBuffer*> queue_output_buf;
    AsyncQueue<void*> queue_input_buf;
    AsyncQueue<void*> queue_output_buf;

Q_SIGNALS:
    void newFrameReady(int vidIx);
    void stopRequested();

protected:
    int m_vidIx;
    const QString m_videoLocation;
    int m_width;
    int m_height;
    ColFormat m_colFormat;
    bool m_vidInfoValid;
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


