#-------------------------------------------------
#
# Project created by QtCreator 2012-03-03T06:40:34
#
#-------------------------------------------------

QT       += core gui opengl
CONFIG   += console


TARGET = qt_gl_gst
TEMPLATE = app

DEFINES += UNIX OMAP3530 GLES2

SOURCES += main.cpp \
    glwidget.cpp \
    pipeline.cpp \
    gstthread.cpp \
    model.cpp

HEADERS  += \
    glwidget.h \
    pipeline.h \
    gstthread.h \
    AsyncQueue.h \
    model.h

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += gstreamer-0.10
}

# Model loading using Assimp:
LIBS += -L. -lassimp
INCLUDEPATH += /home/elangley/3d_existing_code/assimp--2.0.863-sdk/include/

# DVSDK doesn't have GFX SDK lib files in linux-devkit directory:
LIBS += -L/mnt/data/ti-dvsdk_omap3530-evm_4_01_00_09/omap35x_graphics_sdk_4.00.00.01/GFX_Linux_SDK/OGLES2/SDKPackage/Builds/OGLES2/LinuxOMAP3/lib \
    -lIMGegl \
    -lEGL \
    -lGLESv2 \
    -lOpenVG \
    -lsrv_um 

#libffgen.a     libglslcompiler.so  libOpenVGU.so  libpvrPVR2D_FLIPWSEGL.so   libpvrPVR2D_X11WSEGL.so      libusc.a
#libGLES_CM.so  libpvr2d.so    libpvrPVR2D_FRONTWSEGL.so  libPVRScopeServices.so       libuseasm.a
# pvrPVR2D_BLITWSEGL.so   libpvrPVR2D_LINUXFBWSEGL.so libusp.a


#LIBS += -L/mnt/data/ti-dvsdk_omap3530-evm_4_01_00_09/omap35x_graphics_sdk_4.00.00.01/gfx_rel_es3.x
