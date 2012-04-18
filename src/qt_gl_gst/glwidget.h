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

typedef enum
{
    VidShaderFirst = 0,
    VidShaderNormal = 0,
    VidShaderColourHilight = 1,
    VidShaderAlphaMask = 2,
    VidShaderLast = 2,
} VidShaderEffectType;

typedef struct _VidTextureInfo
{
    GLuint texId;
    GstBuffer *buffer;
    bool texInfoValid;
    int width;
    int height;
    ColFormat colourFormat;
//    GLuint textureUnit;
    QGLShaderProgram *shader;
    VidShaderEffectType effect;

} VidTextureInfo;


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
    //void drawSky();
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
    void setAppropriateVidShader(int vidIx);
    void setVidShaderVars(int vidIx, bool printErrors);
    int loadShaderFile(QString fileName, QString &shaderSource);
    int setupShader(QGLShaderProgram *prog, QString baseFileName, bool vertNeeded, bool fragNeeded);
    int setupShader(QGLShaderProgram *prog, const char *shaderList[], int listLen);
    int printOpenGLError(const char *file, int line);
    void nextClearColor(void);

    // Camera:
    // Implement position later if a sky box is desired, and perhaps FPS mode
//    bool cameraCirclingMode;
//    float xPos;
//    float yPos;
//    float zPos;
    float xRot;
    float yRot;
    float zRot;
    QPoint lastPos;
    int Rotate;
    int xLastIncr;
    int yLastIncr;
    float fXInertia;
    float fYInertia;
    float fXInertiaOld;
    float fYInertiaOld;
    // Deprecate:
    GLfloat fScale; // replace with changing zPos

    EModelType currentModel;
    int clearColor;
    bool stackVidQuads;

    int getCallingGstVecIx(int vidIx);
    QVector<QString> videoLoc;
    QVector<GstThread*>gstThreads;
    bool closing;
    QVector<VidTextureInfo> vidTextures;

    QGLShaderProgram brickProg;
    QGLShaderProgram I420NoEffect;
    QGLShaderProgram I420ColourHilight;
    QGLShaderProgram I420AlphaMask;

    // video shader effects vars - for simplicitys sake make them general to all vids
    QVector4D ColourHilightRangeMin;
    QVector4D ColourHilightRangeMax;
    GLuint alphaTextureId;
    bool alphaTextureLoaded;
    GLuint alphaTexWidth;
    GLuint alphaTexHeight;

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

public slots:
    void animate();

};

#endif // GLWIDGET_H
