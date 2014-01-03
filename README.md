qt_gl_gst_poc
=============

This is a proof of concept demo using a media framework (Currently Gstreamer) to stream video frames as textures. The videos can then be rendered with different shaders, blended with alphamasks, rendered on obj models and any other effects to be added as desired. Qt is used to drive the application and provides a basic UI as well as GL|ES compatibility. Currently runs on a native desktop build and the Texas Intstruments OMAP3 using TI's bc-cat driver for the PowerVR streaming texture extensions.
