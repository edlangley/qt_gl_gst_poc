/*
 * GStreamer
 * Copyright (C) 2009 Julien Isorce <julien.isorce@gmail.com>
 * Copyright (C) 2009 Andrey Nechypurenko <andreynech@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef PIPELINE_H
#define PIPELINE_H

//#include <QtGui>
#include <QWidget>

#include <gst/gst.h>

//#include "glcontextid.h"
#include "AsyncQueue.h"
//#include "GstGLBufferDef.h"

typedef enum _ColFormat
{
    // these relate to fourCC codes, but abstract gstreamer from outside:
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
        //const GLContextID &ctx,
        const QString &videoLocation,
        QObject *parent);
    ~Pipeline();

    void configure();
    void start();
    void notifyNewFrame() {emit newFrameReady(m_vidIx);}
    void stop();
    void unconfigure();

    //AsyncQueue<GstGLBuffer*> queue_input_buf;
    //AsyncQueue<GstGLBuffer*> queue_output_buf;
    AsyncQueue<GstBuffer*> queue_input_buf;
    AsyncQueue<GstBuffer*> queue_output_buf;

    int getWidth() { return m_width; }
    int getHeight() { return m_height; }
    ColFormat getColourFormat() { return m_colFormat; }

    // bit lazy just making these public for gst callbacks, but it'll do for now
    GstElement *m_source;
    GstElement *m_decodebin;
    GstElement *m_videosink;
    GstElement *m_audiosink;
    GstElement *m_audioconvert;
    GstElement *m_audioqueue;
 //   GstElement *videoqueue;

Q_SIGNALS:
    void newFrameReady(int vidIx);
    void stopRequested();

private:
    int m_vidIx;
//    const GLContextID m_glctx;
    const QString m_videoLocation;
    GMainLoop* m_loop;
    GstBus* m_bus;
    //GstPipeline* m_pipeline;
    GstElement* m_pipeline;

    int m_width;
    int m_height;
    ColFormat m_colFormat;
    bool m_vidInfoValid;

    static void on_gst_buffer(GstElement * element, GstBuffer * buf, GstPad * pad, Pipeline* p);
    //static void on_new_pad(GstElement *element, GstPad *pad, gpointer data);
    static void on_new_pad(GstElement *element, GstPad *pad, Pipeline* p);
    static gboolean bus_call (GstBus *bus, GstMessage *msg, Pipeline* p);
    static ColFormat discoverColFormat(GstBuffer * buf);
};

#endif
