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

#include "pipeline.h"

#include "model.h"

#ifdef ENABLE_YUV_WINDOW
#include "yuvdebugwindow.h"
#endif

#ifdef IMGTEX_EXT_NEEDED
#include "GLES2/gl2ext.h"
#endif

// Handle texture extensions on different platforms with some generic
// definitions here:
#ifdef RECTTEX_EXT_NEEDED
 #define GL_RECT_TEXTURE_2D                  GL_TEXTURE_RECTANGLE_ARB
 #define GL_RECT_TEXTURE0                    GL_TEXTURE0_ARB
 #define GL_RECT_TEXTURE1                    GL_TEXTURE1_ARB
 #define GL_RECT_VID_TEXTURE_2D              GL_TEXTURE_RECTANGLE_ARB
 #define GL_RECT_VID_TEXTURE0                GL_TEXTURE0_ARB
 #define GL_RECT_VID_TEXTURE1                GL_TEXTURE1_ARB
 #define VIDCONV_FRAG_SHADER_SUFFIX          "-recttex"
#elif IMGTEX_EXT_NEEDED
 #define GL_RECT_TEXTURE_2D                  GL_TEXTURE_2D
 #define GL_RECT_TEXTURE0                    GL_TEXTURE0
 #define GL_RECT_TEXTURE1                    GL_TEXTURE1
 #define GL_RECT_VID_TEXTURE_2D              GL_TEXTURE_STREAM_IMG
 #define GL_RECT_VID_TEXTURE0                GL_TEXTURE0
 #define GL_RECT_VID_TEXTURE1                GL_TEXTURE1
 #define VIDCONV_FRAG_SHADER_SUFFIX          "-imgstream"
#else
 #define GL_RECT_TEXTURE_2D                  GL_TEXTURE_2D
 #define GL_RECT_TEXTURE0                    GL_TEXTURE0
 #define GL_RECT_TEXTURE1                    GL_TEXTURE1
 #define GL_RECT_VID_TEXTURE_2D              GL_TEXTURE_2D
 #define GL_RECT_VID_TEXTURE0                GL_TEXTURE0
 #define GL_RECT_VID_TEXTURE1                GL_TEXTURE1
 #define VIDCONV_FRAG_SHADER_SUFFIX          ""
#endif


#define INERTIA_THRESHOLD       1.0f
#define INERTIA_FACTOR          0.5f
#define SCALE_FACTOR            0.01f
#define SCALE_INCREMENT         0.5f

#define DATA_DIR_ENV_VAR_NAME   "QTGLGST_DATA_DIR"

#define DFLT_OBJ_MODEL_FILE_NAME    "models/sphere.obj"
#define MODEL_BOUNDARY_SIZE     2.0f

typedef enum
{
    ModelEffectFirst = 0,
    ModelEffectBrick = 0,
    ModelEffectVideo = 1,
    ModelEffectVideoLit = 2,
    ModelEffectLast = 2,
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
    VidShaderLit = 6,
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

    int frameCount;
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

    virtual void initVideo();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);

Q_SIGNALS:
    void closeRequested();
    void stackVidsStateChanged(bool newState);
    void rotateStateChanged(bool newState);
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

public Q_SLOTS:
    /* Video related */
    void newFrame(int vidIx);
    void pipelineFinished(int vidIx);
    /* Input event handlers */
    void cycleVidShaderSlot();
    void cycleModelShaderSlot();
    void showYUVWindowSlot();
    void loadVideoSlot();
    void loadModelSlot();
    void loadAlphaSlot();
    void rotateToggleSlot(bool toggleState);
    void stackVidsToggleSlot(int toggleState);
    void cycleBackgroundSlot();
    void resetPosSlot();
    void exitSlot();

    void animate();

protected:
    virtual void initializeGL();
    virtual Pipeline* createPipeline(int vidIx);
    void paintEvent(QPaintEvent *event);
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *e);
    void closeEvent(QCloseEvent* event);
    virtual bool loadNewTexture(int vidIx);
    int printOpenGLError(const char *file, int line);

    QVector<QString> m_videoLoc;
    QVector<Pipeline*> m_vidPipelines;
    QVector<VidTextureInfo> m_vidTextures;

private:
    void setAppropriateVidShader(int vidIx);
    void setVidShaderVars(int vidIx, bool printErrors);
    int loadShaderFile(QString fileName, QString &shaderSource);
    int setupShader(QGLShaderProgram *prog, QString baseFileName, bool vertNeeded, bool fragNeeded);
    int setupShader(QGLShaderProgram *prog, GLShaderModule shaderList[], int listLen);
    int getCallingGstVecIx(int vidIx);

    bool m_closing;
    QString m_dataFilesDir;

    // Camera:
    // Implement position later if a sky box is desired, and perhaps FPS mode
//    bool cameraCirclingMode;
//    float xPos;
//    float yPos;
//    float zPos;
    float m_xRot;
    float m_yRot;
    float m_zRot;
    QPoint m_lastPos;
    int m_rotateOn;
    int m_xLastIncr;
    int m_yLastIncr;
    float m_xInertia;
    float m_yInertia;
    float m_xInertiaOld;
    float m_yInertiaOld;
    // Deprecate:
    GLfloat m_scaleValue; // replace with changing zPos

    QMatrix4x4 m_modelViewMatrix;
    QMatrix4x4 m_projectionMatrix;

    int m_clearColorIndex;
    bool m_stackVidQuads;
    ModelEffectType m_currentModelEffectIndex;

    QGLShaderProgram m_brickProg;
#ifdef VIDI420_SHADERS_NEEDED
    QGLShaderProgram m_I420NoEffectNormalised;
    QGLShaderProgram m_I420LitNormalised;
    QGLShaderProgram m_I420NoEffect;
    QGLShaderProgram m_I420Lit;
    QGLShaderProgram m_I420ColourHilight;
    QGLShaderProgram m_I420ColourHilightSwap;
    QGLShaderProgram m_I420AlphaMask;
#endif
#ifdef VIDUYVY_SHADERS_NEEDED
    QGLShaderProgram m_UYVYNoEffectNormalised;
    QGLShaderProgram m_UYVYLitNormalised;
    QGLShaderProgram m_UYVYNoEffect;
    QGLShaderProgram m_UYVYLit;
    QGLShaderProgram m_UYVYColourHilight;
    QGLShaderProgram m_UYVYColourHilightSwap;
    QGLShaderProgram m_UYVYAlphaMask;
#endif

    // Video shader effects vars - for simplicitys sake make them general to all vids
    QVector4D m_colourHilightRangeMin;
    QVector4D m_colourHilightRangeMax;
    QVector4D m_colourComponentSwapR;
    QVector4D m_colourComponentSwapG;
    QVector4D m_colourComponentSwapB;
    bool m_colourSwapDirUpwards;
    GLuint m_alphaTextureId;
    bool m_alphaTextureLoaded;
    GLuint m_alphaTexWidth;
    GLuint m_alphaTexHeight;

    Model *m_model;

    // FPS counter
    int m_frames;
    QTime m_frameTime;

#ifdef ENABLE_YUV_WINDOW
    YuvDebugWindow *m_yuvWindow;
    QVector<QRgb> m_colourMap;
#endif

};

#endif // GLWIDGET_H
