#ifndef GLWIDGET_H
#define GLWIDGET_H

#define ENABLE_YUV_WINDOW       1

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
#include <QTime>
#include <QPaintEvent>

#include <iostream>

#include "vidthread.h"
#include "pipeline.h"

#include "model.h"

#ifdef ENABLE_YUV_WINDOW
#include "yuvdebugwindow.h"
#endif

#define INERTIA_THRESHOLD       1.0f
#define INERTIA_FACTOR          0.5f
#define SCALE_FACTOR            0.01f
#define SCALE_INCREMENT         0.5f

#define DFLT_OBJ_MODEL_FILE_NAME    "models/glm-data/sphere.obj"
#define MODEL_BOUNDARY_SIZE     2.0f

typedef enum
{
    ModelEffectFirst = 0,
    ModelEffectBrick = 0,
    ModelEffectVideo = 1,
    ModelEffectLast = 1,
} ModelEffectType;

typedef enum
{
    VidShaderFirst = 0,
    VidShaderNoEffect = 0,
    VidShaderColourHilight = 1,
    VidShaderColourHilightSwap = 2,
    VidShaderAlphaMask = 3,
    VidShaderLast = 3,
    // Any shaders after last should not be toggled through with "next shader" key:
    VidShaderNoEffectNormalisedTexCoords = 4,
    VidShaderLitNormalisedTexCoords = 5,
} VidShaderEffectType;

#define NUM_VIDTEXTURE_VERTICES_X    2
#define NUM_VIDTEXTURE_VERTICES_Y    2
#define VIDTEXTURE_LEFT_X            -1.3f
#define VIDTEXTURE_RIGHT_X           1.3f
#define VIDTEXTURE_TOP_Y             1.0f
#define VIDTEXTURE_BOT_Y             -1.0f

typedef struct _VidTextureInfo
{
    GLuint texId;
    void *buffer;
    bool texInfoValid;
    int width;
    int height;
    ColFormat colourFormat;
    QGLShaderProgram *shader;
    VidShaderEffectType effect;

    QVector2D triStripVertices[NUM_VIDTEXTURE_VERTICES_X * NUM_VIDTEXTURE_VERTICES_Y];
    QVector2D triStripTexCoords[NUM_VIDTEXTURE_VERTICES_X * NUM_VIDTEXTURE_VERTICES_Y];
    QVector2D triStripAlphaTexCoords[NUM_VIDTEXTURE_VERTICES_X * NUM_VIDTEXTURE_VERTICES_Y];
} VidTextureInfo;

typedef struct _GLShaderModule
{
    const char *sourceFileName;
    QGLShader::ShaderType type;
} GLShaderModule;

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
    void paintEvent(QPaintEvent *event);
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
    int setupShader(QGLShaderProgram *prog, GLShaderModule shaderList[], int listLen);
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

    QMatrix4x4 modelViewMatrix;
    QMatrix4x4 projectionMatrix;

    int clearColor;
    bool stackVidQuads;
    ModelEffectType currentModelEffect;

    int getCallingGstVecIx(int vidIx);
    QVector<QString> videoLoc;
    QVector<VidThread*>vidThreads;
    bool closing;
    QVector<VidTextureInfo> vidTextures;

    QGLShaderProgram brickProg;
    QGLShaderProgram I420NoEffectNormalised;
    QGLShaderProgram I420LitNormalised;
    QGLShaderProgram I420NoEffect;
    QGLShaderProgram I420ColourHilight;
    QGLShaderProgram I420ColourHilightSwap;
    QGLShaderProgram I420AlphaMask;

    QGLShaderProgram UYVYNoEffectNormalised;
    QGLShaderProgram UYVYLitNormalised;
    QGLShaderProgram UYVYNoEffect;
    QGLShaderProgram UYVYColourHilight;
    QGLShaderProgram UYVYColourHilightSwap;
    QGLShaderProgram UYVYAlphaMask;

    // Video shader effects vars - for simplicitys sake make them general to all vids
    QVector4D ColourHilightRangeMin;
    QVector4D ColourHilightRangeMax;
    QVector4D ColourComponentSwapR;
    QVector4D ColourComponentSwapG;
    QVector4D ColourComponentSwapB;
    bool ColourSwapDirUpwards;
    GLuint alphaTextureId;
    bool alphaTextureLoaded;
    GLuint alphaTexWidth;
    GLuint alphaTexHeight;

    Model *model;

    // FPS counter
    int frames;
    QTime frameTime;

#ifdef ENABLE_YUV_WINDOW
    YuvDebugWindow *yuvWindow;
    QVector<QRgb> colourMap;
#endif

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

public slots:
    void animate();

};

#endif // GLWIDGET_H
