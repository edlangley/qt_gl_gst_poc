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

SOURCES += \
    main.cpp \
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

FORMS += \
    controlsform.ui

# OpenGL support libraries:
LIBS += -lGLU \
    -lGL \
    -lGLEW

# Gstreamer:
CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-0.10

# Model loading using Assimp:
PKGCONFIG += assimp
