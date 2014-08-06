
#include "applogger.h"
#include "tigstpipeline.h"

TIGStreamerPipeline::TIGStreamerPipeline(int vidIx,
                   const QString &videoLocation,
                   const char *renderer_slot,
                   QObject *parent)
  : GStreamerPipeline(vidIx, videoLocation, renderer_slot, parent),
    m_qtdemux(NULL),
    m_tividdecode(NULL),
    m_tiaudiodecode(NULL),
    m_videoqueue(NULL)
{
    LOG(LOG_VIDPIPELINE, Logger::Debug1, "constructor entered");
}

TIGStreamerPipeline::~TIGStreamerPipeline()
{
}

void TIGStreamerPipeline::Configure()
{
    LOG(LOG_VIDPIPELINE, Logger::Debug1, "Configure entered");

    gst_init (NULL, NULL);

    /* Create the elements */
    this->m_pipeline = gst_pipeline_new (NULL);
    if(this->m_videoLocation.isEmpty())
    {
        LOG(LOG_VIDPIPELINE, Logger::Info, "No video file specified. Using video test source.");
        this->m_source = gst_element_factory_make ("videotestsrc", "testsrc");
    }
    else
    {
        this->m_source = gst_element_factory_make ("filesrc", "filesrc");
        g_object_set (G_OBJECT (this->m_source), "location", m_videoLocation.toUtf8().constData(), NULL);
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
    //this->m_audiosink = gst_element_factory_make ("alsasink", "audiosink");

    g_object_set(G_OBJECT(this->m_audioqueue),
                 "max-size-buffers", 8000,
                 "max-size-time", 0,
                 "max-size-bytes", 0,
                 NULL);

    if (this->m_pipeline == NULL || this->m_source == NULL || this->m_qtdemux == NULL ||
        this->m_tividdecode == NULL || this->m_tiaudiodecode == NULL ||
        this->m_audioqueue == NULL || this->m_videoqueue == NULL ||
        this->m_videosink == NULL)// || this->m_audiosink == NULL)
    {
        LOG(LOG_VIDPIPELINE, Logger::Error, "One of the GStreamer decoding elements is missing");
    }

    /* Setup the pipeline */
    gst_bin_add_many (GST_BIN(this->m_pipeline), this->m_source, this->m_qtdemux, this->m_tividdecode,
                      this->m_tiaudiodecode, this->m_audioqueue, this->m_videoqueue, this->m_videosink,
                      //this->m_audiosink,
                      NULL);

    // Like all Gstreamer elements qtdemux inherits from gstelement which provides the
    // pad-added signal:
    g_signal_connect (this->m_qtdemux, "pad-added", G_CALLBACK (on_new_pad), this);

    /* Link the elements */
    gst_element_link (this->m_source, this->m_qtdemux);
    gst_element_link (this->m_audioqueue, this->m_tiaudiodecode);
    //gst_element_link (this->m_tiaudiodecode, this->m_audiosink);
    gst_element_link (this->m_videoqueue, this->m_tividdecode);
#if 0
    /* Use caps filter to get I420 from video decoder */
    GstCaps *caps;
    caps = gst_caps_new_simple ("video/x-raw-yuv",
            "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('I', '4', '2', '0'),
            NULL);
    if(!gst_element_link_filtered (this->m_tividdecode, this->m_videosink, caps))
    {
        LOG(LOG_VIDPIPELINE, Logger::Error, "Failed to link viddecode and videosink");
    }
    gst_caps_unref (caps);
#else
    gst_element_link (this->m_tividdecode, this->m_videosink);
#endif
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

    Q_UNUSED(element);

    caps = gst_pad_get_caps (pad);
    str = gst_caps_get_structure (caps, 0);

    // DEBUG:
    const gchar *checkName = gst_structure_get_name (str);
    LOG(LOG_VIDPIPELINE, Logger::Debug1, "New pad on qtdemux, pad caps structure name: %s", checkName);

    if (g_strrstr (gst_structure_get_name (str), "video"))
    {
        LOG(LOG_VIDPIPELINE, Logger::Debug1, "Pad is for video");

        sinkpad = gst_element_get_pad (p->m_videoqueue, "sink");

        g_object_set (G_OBJECT (p->m_videosink),
                      "sync", TRUE,
                      "signal-handoffs", TRUE,
                      NULL);

        g_signal_connect (p->m_videosink,
                          "handoff",
                          G_CALLBACK(on_gst_buffer),
                          p);

    }
    else
    {
        LOG(LOG_VIDPIPELINE, Logger::Debug1, "Pad is for audio, ignoring for now");
        gst_caps_unref (caps);
        return;

        sinkpad = gst_element_get_pad (p->m_audioqueue, "sink");
    }

    gst_caps_unref (caps);

    gst_pad_link (pad, sinkpad);
    gst_object_unref (sinkpad);
}
