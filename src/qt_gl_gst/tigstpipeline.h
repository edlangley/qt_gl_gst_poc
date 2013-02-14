#ifndef TIGSTPIPELINE_H
#define TIGSTPIPELINE_H

// Re-include base class header here to keep the MOC happy:
#include "gstpipeline.h"

class TIGStreamerPipeline : public GStreamerPipeline
{
    Q_OBJECT

public:
    TIGStreamerPipeline(int vidIx,
        const QString &videoLocation,
        QObject *parent);
    ~TIGStreamerPipeline();

    void Configure();

    // bit lazy just making these public for gst callbacks, but it'll do for now
    GstElement *m_qtdemux;
    GstElement *m_tividdecode;
    GstElement *m_tiaudiodecode;
    GstElement *m_videoqueue;

protected:
    // Can use base class version?:
    //static void on_gst_buffer(GstElement * element, GstBuffer * buf, GstPad * pad, TIGStreamerPipeline* p);

    static void on_new_pad(GstElement *element, GstPad *pad, TIGStreamerPipeline* p);
};

#endif // TIGSTPIPELINE_H
