
#ifndef GSTPIPELINE_H
#define GSTPIPELINE_H

#include <QWidget>

#include <gst/gst.h>

// Re-include base class header here to keep the MOC happy:
#include "pipeline.h"

#define PIPELINE_BUFFER_VID_DATA_START    GST_BUFFER_DATA


class GStreamerPipeline : public Pipeline //public QObject
{
    Q_OBJECT

public:
    GStreamerPipeline(int vidIx,
        const QString &videoLocation,
        QObject *parent);
    ~GStreamerPipeline();

    void configure();
    void start();
    void stop();
    void unconfigure();

    // bit lazy just making these public for gst callbacks, but it'll do for now
    GstElement *m_source;
    GstElement *m_decodebin;
    GstElement *m_videosink;
    GstElement *m_audiosink;
    GstElement *m_audioconvert;
    GstElement *m_audioqueue;

private:
    GMainLoop* m_loop;
    GstBus* m_bus;
    GstElement* m_pipeline;

    static void on_gst_buffer(GstElement * element, GstBuffer * buf, GstPad * pad, GStreamerPipeline* p);
    static void on_new_pad(GstElement *element, GstPad *pad, GStreamerPipeline* p);
    static gboolean bus_call (GstBus *bus, GstMessage *msg, GStreamerPipeline* p);
    static ColFormat discoverColFormat(GstBuffer * buf);
};

#endif
