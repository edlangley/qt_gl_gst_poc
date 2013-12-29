
#ifndef GSTPIPELINE_H
#define GSTPIPELINE_H

#include <QWidget>
#include <QThread>

#include <gst/gst.h>

// Re-include base class header here to keep the MOC happy:
#include "pipeline.h"

//#define PIPELINE_BUFFER_VID_DATA_START    GST_BUFFER_DATA

#define QUEUE_CLEANUP_WAITTIME_MS         200
#define QUEUE_THREADBLOCK_WAITTIME_MS     50

class GStreamerPipeline;

// The incoming buffer thread is really only needed in Windows
// to run g_main_loop_run()
class GstIncomingBufThread : public QThread
{
    Q_OBJECT

public:
    GstIncomingBufThread(GStreamerPipeline *pipelinePtr, QObject *parent = 0)
        : QThread(parent),
          m_pipelinePtr(pipelinePtr) { }
    void run();

private:
    GStreamerPipeline *m_pipelinePtr;
};


class GstOutgoingBufThread : public QThread
{
    Q_OBJECT

public:
    GstOutgoingBufThread(GStreamerPipeline *pipelinePtr, QObject *parent = 0)
        : QThread(parent),
          m_pipelinePtr(pipelinePtr),
          m_keepRunningOutgoingThread(true) { }
    void run();

public slots:
    void quit() { m_keepRunningOutgoingThread = false; } // Not using an event loop for this thread

private:
    GStreamerPipeline *m_pipelinePtr;
    bool m_keepRunningOutgoingThread;
};


class GStreamerPipeline : public Pipeline
{
    Q_OBJECT

public:
    GStreamerPipeline(int vidIx,
        const QString &videoLocation,
        const char *renderer_slot,
        QObject *parent);
    ~GStreamerPipeline();

    void Configure();
    void Start();
    unsigned char *bufToVidDataStart(void *buf) { return GST_BUFFER_DATA(buf); }

    // bit lazy just making these public for gst callbacks, but it'll do for now
    GstElement *m_source;
    GstElement *m_decodebin;
    GstElement *m_videosink;
    GstElement *m_audiosink;
    GstElement *m_audioconvert;
    GstElement *m_audioqueue;

signals:
    void stopRequested();

public Q_SLOTS:
    void Stop();

private slots:
    void cleanUp();

protected:
    GMainLoop* m_loop;
    GstBus* m_bus;
    GstElement* m_pipeline;

    GstIncomingBufThread *m_incomingBufThread;
    GstOutgoingBufThread *m_outgoingBufThread;
    friend class GstIncomingBufThread;
    friend class GstOutgoingBufThread;

    static void on_gst_buffer(GstElement * element, GstBuffer * buf, GstPad * pad, GStreamerPipeline* p);
    static void on_new_pad(GstElement *element, GstPad *pad, GStreamerPipeline* p);
    static gboolean bus_call (GstBus *bus, GstMessage *msg, GStreamerPipeline* p);
    static ColFormat discoverColFormat(GstBuffer * buf);
    static quint32 discoverFourCC(GstBuffer * buf);
};

#endif
