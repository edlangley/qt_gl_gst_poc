#-------------------------------------------------
#
# Project created by QtCreator 2012-03-03T06:40:34
#
#-------------------------------------------------

QT       += core gui opengl
CONFIG   += console


TARGET = qt_gl_gst
TEMPLATE = app

DEFINES += UNIX OMAP3530 IMGTEX_EXT_NEEDED

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
    glpowervrwidget.cpp

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
    glpowervrwidget.h

FORMS += \
    controlsform.ui

# Gstreamer:
CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-0.10

# Model loading using Assimp:
LIBS += -L. -lassimp
INCLUDEPATH += /mnt/data/not_backedup/3d_existing_code/assimp--2.0.863-sdk/include/

# Imagination Technologies PowerVR Streaming texture driver:
INCLUDEPATH += /home/elangley/not_backedup/bc-cat-0.2.0/module/

# TI CMem lib:
INCLUDEPATH += /home/elangley/not_backedup/ti-dvsdk_omap3530-evm_4_01_00_09/linuxutils_2_25_05_11/packages/ti/sdo/linuxutils/cmem/include/
LIBS += -l:cmem.a470MV

# Run on remote Linux device parameters:
target.path += /opt/qt_gl_gst-omap3530evm
INSTALLS += target
