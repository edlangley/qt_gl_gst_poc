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

#include "pipeline.h"


Pipeline::Pipeline(int vidIx,
                   //const GLContextID &ctx,
                   const QString &videoLocation,
                   QObject *parent)
  : QObject(parent),
    m_vidIx(vidIx),
    //m_glctx(ctx),
    m_videoLocation(videoLocation),
    m_loop(NULL),
    m_bus(NULL),
    m_pipeline(NULL),
    m_vidInfoValid(false)
{
    this->configure();
}

Pipeline::~Pipeline()
{
}

void
Pipeline::configure()
{
    gst_init (NULL, NULL);

#ifdef Q_WS_WIN
    m_loop = g_main_loop_new (NULL, FALSE);
#endif
#if 0
    if(m_videoLocation.isEmpty())
    {
        qDebug("No video file specified. Using video test source.");
        m_pipeline =
            GST_PIPELINE (gst_parse_launch
                      ("videotestsrc ! "
                       "video/x-raw-yuv, width=640, height=480, "
                       "framerate=(fraction)30/1 ! "
                       //"glupload ! gleffects effect=5 ! fakesink sync=1",
                       "glupload ! fakesink sync=1",
                       NULL));
    }
    else
    {
        qDebug("Loading video: %s", m_videoLocation.toAscii().data());
        m_pipeline =
            GST_PIPELINE (gst_parse_launch
                      (QString("filesrc location=%1 ! decodebin2 ! "

                               "glupload ! gleffects ! "
                               "fakesink sync=1").arg(m_videoLocation).toAscii(),
                       //"glupload ! gleffects effect=5 ! "
                       NULL));
    }

    m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    gst_bus_add_watch(m_bus, (GstBusFunc) bus_call, this);
    gst_object_unref(m_bus);

    /* Retrieve the last gl element */
    GstElement *gl_element = gst_bin_get_by_name(GST_BIN(m_pipeline), "gleffects0");
    if(!gl_element)
    {
        qDebug ("gl element could not be found");
        return;
    }
    g_object_set(G_OBJECT (gl_element), "external-opengl-context",
               this->m_glctx.contextId, NULL);
    g_object_unref(gl_element);

    gst_element_set_state(GST_ELEMENT(this->m_pipeline), GST_STATE_PAUSED);
    GstState state = GST_STATE_PAUSED;
    if(gst_element_get_state(GST_ELEMENT(this->m_pipeline),
            &state, NULL, GST_CLOCK_TIME_NONE)
            != GST_STATE_CHANGE_SUCCESS)
    {
        qDebug("failed to pause pipeline");
        return;
    }

#endif




    /* Create the elements */
    this->m_pipeline = gst_pipeline_new (NULL);
    if(this->m_videoLocation.isEmpty())
    {
        qDebug("No video file specified. Using video test source.");
        this->m_source = gst_element_factory_make ("videotestsrc", "testsrc");
    }
    else
    {
        this->m_source = gst_element_factory_make ("filesrc", "filesrc");
        g_object_set (G_OBJECT (this->m_source), "location", /*"video.avi"*/ m_videoLocation.toAscii().constData(), NULL);
    }
    this->m_decodebin = gst_element_factory_make ("decodebin2", "decodebin");
    this->m_videosink = gst_element_factory_make ("fakesink", "videosink");
    this->m_audiosink = gst_element_factory_make ("alsasink", "audiosink");
    this->m_audioconvert = gst_element_factory_make ("audioconvert", "audioconvert");
    this->m_audioqueue = gst_element_factory_make ("queue", "audioqueue");
//    m_videoqueue = gst_element_factory_make ("queue", "videoqueue");

    if (this->m_pipeline == NULL || this->m_source == NULL || this->m_decodebin == NULL ||
        this->m_videosink == NULL || this->m_audiosink == NULL || this->m_audioconvert == NULL || this->m_audioqueue == NULL)
        //|| m_videoqueue == NULL)
        g_critical ("One of the GStreamer decoding elements is missing");

    /* Setup the pipeline */
    gst_bin_add_many (GST_BIN (this->m_pipeline), this->m_source, this->m_decodebin, this->m_videosink,
                      this->m_audiosink, this->m_audioconvert, this->m_audioqueue, /*videoqueue,*/ NULL);
    g_signal_connect (this->m_decodebin, "pad-added", G_CALLBACK (on_new_pad), this);

    /* Link the elements */
    gst_element_link (this->m_source, this->m_decodebin);
    gst_element_link (this->m_audioqueue, this->m_audioconvert);
    gst_element_link (this->m_audioconvert, this->m_audiosink);
//    gst_element_link (m_videoqueue, videosink);

    m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    gst_bus_add_watch(m_bus, (GstBusFunc) bus_call, this);
    gst_object_unref(m_bus);

    gst_element_set_state (this->m_pipeline, GST_STATE_PAUSED);

}

void
Pipeline::start()
{
#if 0
    // set a callback to retrieve the gst gl textures
    GstElement *fakesink = gst_bin_get_by_name(GST_BIN(this->m_pipeline),
        "fakesink0");
    g_object_set(G_OBJECT (fakesink), "signal-handoffs", TRUE, NULL);
        g_signal_connect(fakesink, "handoff", G_CALLBACK (on_gst_buffer), this);
    g_object_unref(fakesink);
#endif
    GstStateChangeReturn ret =
    gst_element_set_state(GST_ELEMENT(this->m_pipeline), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        qDebug("Failed to start up pipeline!");

        /* check if there is an error message with details on the bus */
        GstMessage* msg = gst_bus_poll(this->m_bus, GST_MESSAGE_ERROR, 0);
        if (msg)
        {
            GError *err = NULL;
            gst_message_parse_error (msg, &err, NULL);
            qDebug ("ERROR: %s", err->message);
            g_error_free (err);
            gst_message_unref (msg);
        }
        return;
    }

#ifdef Q_WS_WIN
    g_main_loop_run(m_loop);
#endif


}

void
Pipeline::on_new_pad(GstElement *element,
                     GstPad *pad,
                     Pipeline* p)//gpointer data)
{
    GstPad *sinkpad;
    GstCaps *caps;
    GstStructure *str;
    //Pipeline* p = (Pipeline*)data;

    caps = gst_pad_get_caps (pad);
    str = gst_caps_get_structure (caps, 0);

    if (g_strrstr (gst_structure_get_name (str), "video"))
    {
        //sinkpad = gst_element_get_pad (videoqueue, "sink");
        //gst_pad_add_buffer_probe (pad, G_CALLBACK (on_gst_buffer), this);
        sinkpad = gst_element_get_pad (p->m_videosink, "sink");

        g_object_set (G_OBJECT (p->m_videosink),
                      "sync", TRUE,
                      "signal-handoffs", TRUE,
                      NULL);
        g_signal_connect (p->m_videosink,
                          "preroll-handoff",
                          G_CALLBACK(on_gst_buffer),
                          p);
        g_signal_connect (p->m_videosink,
                          "handoff",
                          G_CALLBACK(on_gst_buffer),
                          p);

    }
    else
        sinkpad = gst_element_get_pad (p->m_audioqueue, "sink");

    gst_caps_unref (caps);

    gst_pad_link (pad, sinkpad);
    gst_object_unref (sinkpad);
}

/* fakesink handoff callback */
void
Pipeline::on_gst_buffer(GstElement * element,
                        GstBuffer * buf,
                        GstPad * pad,
                        Pipeline* p)
{
    Q_UNUSED(pad)
    Q_UNUSED(element)

    if(p->m_vidInfoValid == false)
    {
        GstCaps *caps = gst_pad_get_negotiated_caps (pad);
        if (caps)
        {
            GstStructure *structure = gst_caps_get_structure (caps, 0);
            gst_structure_get_int (structure, "width", &(p->m_width));
            gst_structure_get_int (structure, "height", &(p->m_height));
        }
        else
            g_print ("on_gst_buffer() - Could not get caps for pad!\n");

        p->m_colFormat = discoverColFormat(buf);

        p->m_vidInfoValid = true;
    }

    /* ref then push buffer to use it in qt */
    gst_buffer_ref(buf);
//    p->queue_input_buf.put((GstGLBuffer*)buf);
    p->queue_input_buf.put(buf);

    if (p->queue_input_buf.size() > 3)
        p->notifyNewFrame();

    /* pop then unref buffer we have finished to use in qt */
    if (p->queue_output_buf.size() > 3)
    {
        GstBuffer *buf_old = (GstBuffer*)(p->queue_output_buf.get());\
        if (buf_old)
            gst_buffer_unref(buf_old);
    }
}

void
Pipeline::stop()
{
#ifdef Q_WS_WIN
    g_main_loop_quit(m_loop);
#else
    emit stopRequested();
#endif
}

void
Pipeline::unconfigure()
{
    gst_element_set_state(GST_ELEMENT(this->m_pipeline), GST_STATE_NULL);

    GstBuffer *buf;
    while(this->queue_input_buf.size())
    {
        buf = (GstBuffer*)(this->queue_input_buf.get());
        gst_buffer_unref(buf);
    }
    while(this->queue_output_buf.size())
    {
        buf = (GstBuffer*)(this->queue_output_buf.get());
        gst_buffer_unref(buf);
    }

    gst_object_unref(m_pipeline);
}

gboolean
Pipeline::bus_call(GstBus *bus, GstMessage *msg, Pipeline* p)
{
  Q_UNUSED(bus)

    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
            qDebug("End-of-stream received. Stopping.");
            p->stop();
        break;

        case GST_MESSAGE_ERROR:
        {
            gchar *debug = NULL;
            GError *err = NULL;
            gst_message_parse_error(msg, &err, &debug);
            qDebug("Error: %s", err->message);
            g_error_free (err);
            if(debug)
            {
            qDebug("Debug deails: %s", debug);
            g_free(debug);
            }
            p->stop();
            break;
        }

        default:
            break;
    }

    return TRUE;
}

ColFormat Pipeline::discoverColFormat(GstBuffer * buf)
{
    // Stole this whole function, edit for consistent style later
    gchar*	      pTmp	 = NULL;
    GstCaps*      pCaps	 = NULL;
    GstStructure* pStructure = NULL;
    gint	      iDepth;
    gint	      iBitsPerPixel;
    gint	      iRedMask;
    gint	      iGreenMask;
    gint	      iBlueMask;
    gint	      iAlphaMask;
    ColFormat ret = ColFmt_Unknown;

    pTmp = gst_caps_to_string (GST_BUFFER_CAPS(buf));
    g_print ("%s\n", pTmp);
    g_free (pTmp);

    g_print ("buffer-size in bytes: %d\n", GST_BUFFER_SIZE (buf));

    pCaps = gst_buffer_get_caps (buf);
    pStructure = gst_caps_get_structure (pCaps, 0);

    if (gst_structure_has_name (pStructure, "video/x-raw-rgb"))
    {
            gst_structure_get_int (pStructure, "bpp", &iBitsPerPixel);
            gst_structure_get_int (pStructure, "depth", &iDepth);
            gst_structure_get_int (pStructure, "red_mask", &iRedMask);
            gst_structure_get_int (pStructure, "green_mask", &iGreenMask);
            gst_structure_get_int (pStructure, "blue_mask", &iBlueMask);

            switch (iDepth)
            {
                    case 24:
                            if (iRedMask   == 0x00ff0000 &&
                                iGreenMask == 0x0000ff00 &&
                                iBlueMask  == 0x000000ff)
                            {
                                    g_print ("format is RGB\n");
                                    ret = ColFmt_RGB888;
                            }
                            else if (iRedMask   == 0x000000ff &&
                                     iGreenMask == 0x0000ff00 &&
                                     iBlueMask  == 0x00ff0000)
                            {
                                    g_print ("format is BGR\n");
                                    ret = ColFmt_BGR888;
                            }
                            else
                                    g_print ("Unhandled 24 bit RGB-format");
                    break;

                    case 32:
                            gst_structure_get_int (pStructure,
                                                   "alpha_mask",
                                                   &iAlphaMask);
                            if (iRedMask   == 0xff000000 &&
                                iGreenMask == 0x00ff0000 &&
                                iBlueMask  == 0x0000ff00)
                            {
                                    g_print ("format is RGBA\n");
                                    ret = ColFmt_ARGB8888;
                            }
                            else if (iRedMask   == 0x00ff0000 &&
                                     iGreenMask == 0x0000ff00 &&
                                     iBlueMask  == 0x000000ff)
                            {
                                    g_print ("format is BGRA\n");
                                    ret = ColFmt_BGRA8888;
                            }
                            else
                                    g_print ("Unhandled 32 bit RGB-format");
                    break;

                    default :
                            g_print ("Unhandled RGB-format of depth %d\n",
                                     iDepth);
                    break;
            }
    }
    else if (gst_structure_has_name (pStructure, "video/x-raw-yuv"))
    {
            guint32 uiFourCC;

            gst_structure_get_fourcc (pStructure, "format", &uiFourCC);

            switch (uiFourCC)
            {
                    case GST_MAKE_FOURCC ('I', '4', '2', '0'):
                            g_print ("I420 (0x%X)\n", uiFourCC);
                            ret = ColFmt_I420;
                    break;

                    case GST_MAKE_FOURCC ('I', 'Y', 'U', 'V'):
                            g_print ("IYUV (0x%X)\n", uiFourCC);
                            ret = ColFmt_IYUV;
                    break;

                    case GST_MAKE_FOURCC ('Y', 'V', '1', '2'):
                            g_print ("YV12 (0x%X)\n", uiFourCC);
                            ret = ColFmt_YV12;
                    break;

                    case GST_MAKE_FOURCC ('Y', 'U', 'Y', 'V'):
                            g_print ("YUYV (0x%X)\n", uiFourCC);
                            ret = ColFmt_YUYV;
                    break;

                    case GST_MAKE_FOURCC ('Y', 'U', 'Y', '2'):
                            g_print ("YUY2 (0x%X)\n", uiFourCC);
                            ret = ColFmt_YUY2;
                    break;

                    case GST_MAKE_FOURCC ('V', '4', '2', '2'):
                            g_print ("V422 (0x%X)\n", uiFourCC);
                            ret = ColFmt_V422;
                    break;

                    case GST_MAKE_FOURCC ('Y', 'U', 'N', 'V'):
                            g_print ("YUNV (0x%X)\n", uiFourCC);
                            ret = ColFmt_YUNV;
                    break;

                    case GST_MAKE_FOURCC ('U', 'Y', 'V', 'Y'):
                            g_print ("UYVY (0x%X)\n", uiFourCC);
                            ret = ColFmt_UYVY;
                    break;

                    case GST_MAKE_FOURCC ('Y', '4', '2', '2'):
                            g_print ("Y422 (0x%X)\n", uiFourCC);
                            ret = ColFmt_Y422;
                    break;

                    case GST_MAKE_FOURCC ('U', 'Y', 'N', 'V'):
                            g_print ("UYNV (0x%X)\n", uiFourCC);
                            ret = ColFmt_YUNV;
                    break;

                    default :
                            g_print ("Unhandled YUV-format \n");
                    break;
            }
    }
    else
            g_print ("Unsupported caps name %s",
                     gst_structure_get_name (pStructure));

    gst_caps_unref (pCaps);
    pCaps = NULL;

    return ret;
}
