#include "tigstpipeline.h"

TIGStreamerPipeline::TIGStreamerPipeline(int vidIx,
                   const QString &videoLocation,
                   QObject *parent)
  : GStreamerPipeline(vidIx, videoLocation, parent)
{
    this->configure();
}

TIGStreamerPipeline::~TIGStreamerPipeline()
{
}

void TIGStreamerPipeline::configure()
{
    gst_init (NULL, NULL);

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
        g_object_set (G_OBJECT (this->m_source), "location", m_videoLocation.toAscii().constData(), NULL);
    }

    // gst-launch -v filesrc location=sample.mp4 ! qtdemux name=demux demux.audio_00 !
    //  queue max-size-buffers=8000 max-size-time=0 max-size-bytes=0 ! TIAuddec1 !
    //  alsasink demux.video_00 ! queue ! TIViddec2 ! TIDmaiVideoSink videoStd=VGA videoOutput=LCD


    this->m_qtdemux = gst_element_factory_make ("qtdemux", "demux");
    this->m_tividdecode = gst_element_factory_make ("TIViddec2", "tividdecode");
    this->m_tiaudiodecode = gst_element_factory_make ("TIAuddec1", "tiaudiodecode");
    this->m_audioqueue = gst_element_factory_make ("queue", "audioqueue");
    this->m_videoqueue = gst_element_factory_make ("queue", "videoqueue");
    this->m_videosink = gst_element_factory_make ("fakesink", "videosink");
    this->m_audiosink = gst_element_factory_make ("alsasink", "audiosink");

    g_object_set(G_OBJECT(this->m_audioqueue),
                 "max-size-buffers", 8000,
                 "max-size-time", 0,
                 "max-size-bytes", 0,
                 NULL);


    if (this->m_pipeline == NULL || this->m_source == NULL || this->m_qtdemux == NULL ||
        this->m_tividdecode == NULL || this->m_tiaudiodecode == NULL ||
        this->m_audioqueue == NULL || this->m_videoqueue == NULL ||
        this->m_videosink == NULL || this->m_audiosink == NULL)
        g_critical ("One of the GStreamer decoding elements is missing");

    /* Setup the pipeline */
    gst_bin_add_many (GST_BIN(this->m_pipeline), this->m_source, this->m_qtdemux, this->m_tividdecode,
                      this->m_tiaudiodecode, this->m_audioqueue, this->m_videoqueue, this->m_videosink,
                      this->m_audiosink, NULL);

    // Like all Gstreamer elements qtdemux inherits from gstelement which provides the
    // pad-added signal:
    g_signal_connect (this->m_qtdemux, "pad-added", G_CALLBACK (on_new_pad), this);

    /* Link the elements */
    gst_element_link (this->m_source, this->m_qtdemux);
    gst_element_link (this->m_audioqueue, this->m_tiaudiodecode);
    gst_element_link (this->m_tiaudiodecode, this->m_audiosink);
    gst_element_link (this->m_videoqueue, this->m_tividdecode);
    gst_element_link (this->m_tividdecode, this->m_videosink);

    m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    gst_bus_add_watch(m_bus, (GstBusFunc) bus_call, this);
    gst_object_unref(m_bus);

    gst_element_set_state (this->m_pipeline, GST_STATE_PAUSED);

}

void TIGStreamerPipeline::on_new_pad(GstElement *element,
                     GstPad *pad,
                     TIGStreamerPipeline* p)
{
    GstPad *sinkpad;
    GstCaps *caps;
    GstStructure *str;

    caps = gst_pad_get_caps (pad);
    str = gst_caps_get_structure (caps, 0);

    // DEBUG:
    const gchar *checkName = gst_structure_get_name (str);
    g_print("New pad on qtdemux, pad caps structure name: %s\n", checkName);

    if (g_strrstr (gst_structure_get_name (str), "video"))
    {
        sinkpad = gst_element_get_pad (p->m_videoqueue, "sink");

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

#if 0
/* fakesink handoff callback */
void TIGStreamerPipeline::on_gst_buffer(GstElement * element,
                        GstBuffer * buf,
                        GstPad * pad,
                        GStreamerPipeline* p)
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
#endif
