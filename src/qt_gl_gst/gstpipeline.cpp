
#include "gstpipeline.h"


GStreamerPipeline::GStreamerPipeline(int vidIx,
                   const QString &videoLocation,
                   const char *renderer_slot,
                   QObject *parent)
  : Pipeline(vidIx, videoLocation, renderer_slot, parent),
    m_source(NULL),
    m_decodebin(NULL),
    m_videosink(NULL),
    m_audiosink(NULL),
    m_audioconvert(NULL),
    m_audioqueue(NULL),
    m_loop(NULL),
    m_bus(NULL),
    m_pipeline(NULL)
{
    PIPELINE_DEBUG("GStreamerPipeline constructor entered");

    m_incomingBufThread = new GstIncomingBufThread(this, this);
    m_outgoingBufThread = new GstOutgoingBufThread(this, this);

    QObject::connect(m_incomingBufThread, SIGNAL(finished()), this, SLOT(cleanUp()));

}

GStreamerPipeline::~GStreamerPipeline()
{
}

void GStreamerPipeline::Configure()
{
    PIPELINE_DEBUG("GStreamerPipeline::Configure entered");

    gst_init (NULL, NULL);

#ifdef Q_WS_WIN
    m_loop = g_main_loop_new (NULL, FALSE);
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
        g_object_set (G_OBJECT (this->m_source), "location", /*"video.avi"*/ m_videoLocation.toUtf8().constData(), NULL);
    }
    this->m_decodebin = gst_element_factory_make ("decodebin2", "decodebin");
    this->m_videosink = gst_element_factory_make ("fakesink", "videosink");
    this->m_audiosink = gst_element_factory_make ("alsasink", "audiosink");
    this->m_audioconvert = gst_element_factory_make ("audioconvert", "audioconvert");
    this->m_audioqueue = gst_element_factory_make ("queue", "audioqueue");

    if (this->m_pipeline == NULL || this->m_source == NULL || this->m_decodebin == NULL ||
        this->m_videosink == NULL || this->m_audiosink == NULL || this->m_audioconvert == NULL || this->m_audioqueue == NULL)
        g_critical ("One of the GStreamer decoding elements is missing");

    /* Setup the pipeline */
    gst_bin_add_many (GST_BIN (this->m_pipeline), this->m_source, this->m_decodebin, this->m_videosink,
                      this->m_audiosink, this->m_audioconvert, this->m_audioqueue, /*videoqueue,*/ NULL);
    g_signal_connect (this->m_decodebin, "pad-added", G_CALLBACK (on_new_pad), this);

    /* Link the elements */
    gst_element_link (this->m_source, this->m_decodebin);
    gst_element_link (this->m_audioqueue, this->m_audioconvert);
    gst_element_link (this->m_audioconvert, this->m_audiosink);

    m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    gst_bus_add_watch(m_bus, (GstBusFunc) bus_call, this);
    gst_object_unref(m_bus);

    gst_element_set_state (this->m_pipeline, GST_STATE_PAUSED);

}

void GStreamerPipeline::Start()
{
    GstStateChangeReturn ret = gst_element_set_state(GST_ELEMENT(this->m_pipeline), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        qDebug("Failed to start up pipeline!");

        /* check if there is an error message with details on the bus */
        GstMessage* msg = gst_bus_poll(this->m_bus, GST_MESSAGE_ERROR, 0);
        if (msg)
        {
            GError *err = NULL;
            gst_message_parse_error (msg, &err, NULL);
            qCritical ("ERROR: %s", err->message);
            g_error_free (err);
            gst_message_unref (msg);
        }
        return;
    }

    // Start the threads:
    m_incomingBufThread->start();
    m_outgoingBufThread->start();
}

void GStreamerPipeline::Stop()
{
#ifdef Q_WS_WIN
    g_main_loop_quit(m_loop);
#else
    emit stopRequested();
#endif
}

void GStreamerPipeline::cleanUp()
{
    gst_element_set_state(GST_ELEMENT(this->m_pipeline), GST_STATE_NULL);

    // Wait for both threads to finish up
    m_incomingBufThread->wait(QUEUE_CLEANUP_WAITTIME_MS);
    m_outgoingBufThread->wait(QUEUE_CLEANUP_WAITTIME_MS);

    GstBuffer *buf;
    while(this->m_incomingBufQueue.size())
    {
        this->m_incomingBufQueue.get((void**)(&buf));
        gst_buffer_unref(buf);
    }
    while(this->m_outgoingBufQueue.size())
    {
        this->m_outgoingBufQueue.get((void**)(&buf));
        gst_buffer_unref(buf);
    }

    gst_object_unref(m_pipeline);

    // Done
    m_finished = true;
    emit finished(m_vidIx);
}

void GStreamerPipeline::on_new_pad(GstElement *element,
                     GstPad *pad,
                     GStreamerPipeline* p)
{
    GstPad *sinkpad;
    GstCaps *caps;
    GstStructure *str;

    Q_UNUSED(element);

    caps = gst_pad_get_caps (pad);
    str = gst_caps_get_structure (caps, 0);

    if (g_strrstr (gst_structure_get_name (str), "video"))
    {
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
void GStreamerPipeline::on_gst_buffer(GstElement * element,
                        GstBuffer * buf,
                        GstPad * pad,
                        GStreamerPipeline* p)
{
    PIPELINE_DEBUG("on_gst_buffer: vid %d, element=%p, buf=%p, pad=%p, p=%p, bufdata=%p\n",
                   p->getVidIx(), element, buf, pad, p, GST_BUFFER_DATA(buf));

    Q_UNUSED(pad)
    Q_UNUSED(element)

    if(p->m_vidInfoValid == false)
    {
        PIPELINE_DEBUG("GStreamerPipeline: Received first frame of vid %d", p->getVidIx());

        GstCaps *caps = gst_pad_get_negotiated_caps (pad);
        if (caps)
        {
            GstStructure *structure = gst_caps_get_structure (caps, 0);
            gst_structure_get_int (structure, "width", &(p->m_width));
            gst_structure_get_int (structure, "height", &(p->m_height));
        }
        else
            qCritical("on_gst_buffer() - Could not get caps for pad!\n");

        p->m_colFormat = discoverColFormat(buf);

        p->m_vidInfoValid = true;
    }

    /* ref then push buffer to use it in qt */
    gst_buffer_ref(buf);
    p->m_incomingBufQueue.put(buf);
    PIPELINE_DEBUG("GStreamerPipeline: vid %d pushed buffer %p to incoming queue", p->getVidIx(), buf);

    PIPELINE_DEBUG("GStreamerPipeline: vid %d sending buf to GLES", p->getVidIx());
    p->NotifyNewFrame();
}

gboolean GStreamerPipeline::bus_call(GstBus *bus, GstMessage *msg, GStreamerPipeline* p)
{
  Q_UNUSED(bus)

    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
            qDebug("End-of-stream received. Stopping.");
            p->Stop();
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
            qDebug("Debug details: %s", debug);
            g_free(debug);
            }
            p->Stop();
            break;
        }

        default:
            break;
    }

    return TRUE;
}

ColFormat GStreamerPipeline::discoverColFormat(GstBuffer * buf)
{
    // Edit for consistent style later
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
    {
            g_print ("Unsupported caps name %s",
                     gst_structure_get_name (pStructure));
    }

    gst_caps_unref (pCaps);
    pCaps = NULL;

    return ret;
}

quint32 GStreamerPipeline::discoverFourCC(GstBuffer * buf)
{
    guint32       uiFourCC = 0;
    GstCaps*      pCaps	 = NULL;
    GstStructure* pStructure = NULL;

    pCaps = gst_buffer_get_caps (buf);
    pStructure = gst_caps_get_structure (pCaps, 0);

    if (gst_structure_has_name (pStructure, "video/x-raw-yuv"))
    {
        gst_structure_get_fourcc (pStructure, "format", &uiFourCC);
    }

    return (quint32)uiFourCC;
}

void GstIncomingBufThread::run()
{
    PIPELINE_DEBUG("GStreamerPipeline: vid %d incoming buf thread started", m_pipelinePtr->getVidIx());

#ifndef Q_WS_WIN
    //works like the gmainloop on linux (GstEvent are handled)
    QObject::connect(m_pipelinePtr, SIGNAL(stopRequested()), this, SLOT(quit()));
    exec();
#else
    g_main_loop_run(m_loop);
#endif

    // Incoming handling is all done in the static on_gst_buffer callback

    PIPELINE_DEBUG("GStreamerPipeline: vid %d incoming buf thread finished", m_pipelinePtr->getVidIx());
}


void GstOutgoingBufThread::run()
{
    PIPELINE_DEBUG("GStreamerPipeline: vid %d outgoing buf thread started", m_pipelinePtr->getVidIx());

    QObject::connect(m_pipelinePtr, SIGNAL(stopRequested()), this, SLOT(quit()));

    while(m_keepRunningOutgoingThread)
    {
        /* Pop then unref buffer we have finished using in qt,
           block here if queue is empty */
        GstBuffer *buf_old = NULL;
        if(m_pipelinePtr->m_outgoingBufQueue.get((void**)(&buf_old), QUEUE_THREADBLOCK_WAITTIME_MS))
        {
            if (buf_old)
            {
                gst_buffer_unref(buf_old);
                PIPELINE_DEBUG("GStreamerPipeline: vid %d popped buffer %p from outgoing queue", m_pipelinePtr->getVidIx(), buf_old);
                PIPELINE_DEBUG("GStreamerPipeline: vid %d m_outgoingBufQueue size is = %d", m_pipelinePtr->getVidIx(), m_pipelinePtr->m_outgoingBufQueue.size());
            }
        } 
    }

    PIPELINE_DEBUG("GStreamerPipeline: vid %d outgoing buf thread finished", m_pipelinePtr->getVidIx());
}


