#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QApplication>
#include <QGLWidget>
#include <QGLShaderProgram>
#include <QMouseEvent>
#include <QTextStream>
#include <QFile>
#include <QTimer>
#include <QStringList>
#include <QFileDialog>
#include <QSignalMapper>

#include <iostream>

#include <GL/glut.h>

#include "gstthread.h"
#include "pipeline.h"

#define INERTIA_THRESHOLD       1.0f
#define INERTIA_FACTOR          0.5f
#define SCALE_FACTOR            0.01f
#define SCALE_INCREMENT         0.5f

typedef enum
{
    EModelFirst  = 0,
    EModelTeapot = 0,
    EModelSphere = 1,
    EModelTorus  = 2,
    EModelLast   = 2,
} EModelType;

typedef struct _VidTextureInfo VidTextureInfo;
struct _VidTextureInfo
{
    GLuint texId;
    GstBuffer *buffer;
    bool texInfoValid;
    int width;
    int height;
    ColFormat colourFormat;
//    GLuint textureUnit;
    QGLShaderProgram *shader;

};

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(int argc, char *argv[], QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);

protected:
    void initializeGL();
    void paintGL();
    void drawSky();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *e);
    void closeEvent(QCloseEvent* event);

Q_SIGNALS:
    void closeRequested();

public Q_SLOTS:
    void newFrame(int vidIx);
    void gstThreadFinished(int vidIx);

private:
    int loadShaderFile(QString fileName, QString &shaderSource);
    int setupShader(QGLShaderProgram *prog, QString baseFileName, bool vertNeeded, bool fragNeeded);
    int printOpenGLError(char *file, int line);
    void nextClearColor(void);
    // Camera:
//    int xRot;
//    int yRot;
//    int zRot;
    // FPS mode:
    float xRot;
    float yRot;
    float zRot;
    float xPos;
    float yPos;
    float zPos;

    // Circling mode:
    QPoint lastPos;
    EModelType gleModel;
    int Rotate;
    int xLastIncr;
    int yLastIncr;
    float fXInertia;
    float fYInertia;
    float fXInertiaOld;
    float fYInertiaOld;

    bool cameraCirclingMode;

    // Deprecate:
    GLfloat fScale; // replace with changing zPos

    int clearColor;

    int getCallingGstVecIx(int vidIx);
    QVector<QString> videoLoc;
    QVector<GstThread*>gstThreads;
    bool closing;
    QVector<VidTextureInfo> vidTextures;

    //make this generic shortly:
    // try storing as individual shaders, then link when needed?
    QGLShaderProgram brickProg;
    QGLShaderProgram I420ToRGB;

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

public slots:
    void animate();

};

#endif // GLWIDGET_H
