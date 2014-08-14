#-------------------------------------------------
#
# Project created by QtCreator 2012-03-03T06:40:34
#
#-------------------------------------------------

QT       += core gui opengl
CONFIG   += console


TARGET = qt_gl_gst
TEMPLATE = app

DEFINES += UNIX OMAP3530 VIDUYVY_SHADERS_NEEDED IMGTEX_EXT_NEEDED \
    HIDE_GL_WHEN_MODAL_OPEN TEXCOORDS_ALREADY_NORMALISED

SOURCES += main.cpp \
    mainwindow.cpp \
    glwidget.cpp \
    pipeline.cpp \
    gstpipeline.cpp \
    tigstpipeline.cpp \
    shaderlists.cpp \
    model.cpp \
    yuvdebugwindow.cpp \
    controlsform.cpp \
    glpowervrwidget.cpp \
    applogger.cpp

HEADERS  += mainwindow.h \
    glwidget.h \
    pipeline.h \
    gstpipeline.h \
    tigstpipeline.h \
    asyncwaitingqueue.h \
    shaderlists.h \
    model.h \
    yuvdebugwindow.h \
    controlsform.h \
    glpowervrwidget.h \
    applogger.h

FORMS += \
    controlsform.ui

# Gstreamer:
CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-0.10

# Model loading using Assimp:
PKGCONFIG += assimp

# TI CMem lib:
INCLUDEPATH += $$(SDK_PATH)/../linuxutils_2_25_05_11/packages/ti/sdo/linuxutils/cmem/include/
LIBS += -L$$(SDK_PATH)/../linuxutils_2_25_05_11/packages/ti/sdo/linuxutils/cmem/lib -l:cmem.a470MV


# Run on remote Linux device parameters:
target.path += /opt/qt_gl_gst-omap3530evm
INSTALLS += target
