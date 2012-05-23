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

#include "gstthread.h"
#include "pipeline.h"

#include "model.h"

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
} VidShaderEffectType;

typedef struct _VidTextureInfo
{
    GLuint texId;
    GstBuffer *buffer;
    bool texInfoValid;
    int width;
    int height;
    ColFormat colourFormat;
    QGLShaderProgram *shader;
    VidShaderEffectType effect;

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
    void paintGL();
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
    QVector<GstThread*>gstThreads;
    bool closing;
    QVector<VidTextureInfo> vidTextures;

    QGLShaderProgram brickProg;
    QGLShaderProgram I420NoEffectNormalised;
    QGLShaderProgram I420NoEffect;
    QGLShaderProgram I420ColourHilight;
    QGLShaderProgram I420ColourHilightSwap;
    QGLShaderProgram I420AlphaMask;

    // Video shader effects vars - for simplicitys sake make them general to all vids
    QVector4D ColourHilightRangeMin;
    QVector4D ColourHilightRangeMax;
    QVector4D ColourComponentToSwap;
    GLuint alphaTextureId;
    bool alphaTextureLoaded;
    GLuint alphaTexWidth;
    GLuint alphaTexHeight;

    Model *model;

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

public slots:
    void animate();

};

#endif // GLWIDGET_H
