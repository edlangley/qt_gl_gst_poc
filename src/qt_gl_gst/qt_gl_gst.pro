#-------------------------------------------------
#
# Project created by QtCreator 2012-03-03T06:40:34
#
#-------------------------------------------------

QT       += core gui opengl
CONFIG   += console


TARGET = qt_gl_gst
TEMPLATE = app

DEFINES += UNIX RECTTEX_EXT_NEEDED GLU_NEEDED

SOURCES += main.cpp \
    glwidget.cpp \
    model.cpp \
    gstpipeline.cpp \
    pipeline.cpp \
    shaderlists.cpp \
    tigstpipeline.cpp \
    mainwindow.cpp \
    yuvdebugwindow.cpp \
    controlsform.cpp

HEADERS  += \
    glwidget.h \
    asyncwaitingqueue.h \
    model.h \
    gstpipeline.h \
    pipeline.h \
    shaderlists.h \
    tigstpipeline.h \
    mainwindow.h \
    yuvdebugwindow.h \
    controlsform.h

LIBS += -lgstreamer-0.10 \
    -lgstinterfaces-0.10 \
    -lglib-2.0 \
    -lgmodule-2.0 \
    -lgobject-2.0 \
    -lgthread-2.0 \
    -lGLU \
    -lGL \
    -lGLEW

FORMS += \
    controlsform.ui

INCLUDEPATH += /usr/include/gstreamer-0.10 \
    /usr/local/include/gstreamer-0.10 \
    /usr/include/glib-2.0 \
    /usr/lib/i386-linux-gnu/glib-2.0/include \
    /usr/include/libxml2

# Model loading using Assimp:
LIBS += -L. -lassimp
INCLUDEPATH += /mnt/data/not_backedup/3d_existing_code/assimp--2.0.863-sdk/include/

