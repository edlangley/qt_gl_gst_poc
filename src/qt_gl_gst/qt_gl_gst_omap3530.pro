#-------------------------------------------------
#
# Project created by QtCreator 2012-03-03T06:40:34
#
#-------------------------------------------------

QT       += core gui opengl
CONFIG   += console


TARGET = qt_gl_gst
TEMPLATE = app

DEFINES += UNIX OMAP3530

SOURCES += main.cpp \
    mainwindow.cpp \
    glwidget.cpp \
    pipeline.cpp \
    gstpipeline.cpp \
    tigstpipeline.cpp \
    shaderlists.cpp \
    model.cpp \
    yuvdebugwindow.cpp \
    controlsform.cpp

HEADERS  += mainwindow.h \
    glwidget.h \
    pipeline.h \
    gstpipeline.h \
    tigstpipeline.h \
    asyncwaitingqueue.h \
    shaderlists.h \
    model.h \
    yuvdebugwindow.h \
    controlsform.h

FORMS += \
    controlsform.ui

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += gstreamer-0.10
}

# Model loading using Assimp:
LIBS += -L. -lassimp
INCLUDEPATH += /mnt/data/not_backedup/3d_existing_code/assimp--2.0.863-sdk/include/

target.path += /opt/qt_gl_gst-omap3530evm
INSTALLS += target
