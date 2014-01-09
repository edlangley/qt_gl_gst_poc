#include <QMainWindow>
#include "glwidget.h"
#include "shaderlists.h"

#ifdef GLU_NEEDED
 #include "GL/glu.h"
#endif


GLWidget::GLWidget(int argc, char *argv[], QWidget *parent) :
    QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba), parent),
    m_closing(false),
    m_brickProg(this)
{
    m_xRot = 0;
    m_yRot = 0;
    m_zRot = 0;
    m_scaleValue = 1.0;
    m_lastPos = QPoint(0, 0);

    m_rotateOn = 1;
    m_xLastIncr = 0;
    m_yLastIncr = 0;
    m_xInertia = -0.5;
    m_yInertia = 0;

    m_clearColorIndex = 0;
    m_stackVidQuads = false;
    m_currentModelEffectIndex = ModelEffectFirst;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
    timer->start(20);

    grabKeyboard();

    // Video shader effects vars
    m_colourHilightRangeMin = QVector4D(0.0, 0.0, 0.0, 0.0);
    m_colourHilightRangeMax = QVector4D(0.2, 0.2, 1.0, 1.0); // show shades of blue as they are
    m_colourComponentSwapR = QVector4D(1.0, 1.0, 1.0, 0.0);
    m_colourComponentSwapG = QVector4D(1.0, 1.0, 0.0, 0.0);
    m_colourComponentSwapB = QVector4D(1.0, 1.0, 1.0, 0.0);
    m_colourSwapDirUpwards = true;
    m_alphaTextureLoaded = false;

    // Video pipeline
    for(int vidIx = 1; vidIx < argc; vidIx++)
    {
        m_videoLoc.push_back(QString(argv[vidIx]));
    }

    // Instantiate video pipeline for each filename specified
    for(int vidIx = 0; vidIx < this->m_videoLoc.size(); vidIx++)
    {
        this->m_vidPipelines.push_back(this->createPipeline(vidIx));

        if(this->m_vidPipelines[vidIx] == NULL)
        {
            qCritical("Error creating pipeline for vid %d", vidIx);
            return;
        }

        QObject::connect(this->m_vidPipelines[vidIx], SIGNAL(finished(int)),
                         this, SLOT(pipelineFinished(int)));
        QObject::connect(this, SIGNAL(closeRequested()),
                         this->m_vidPipelines[vidIx], SLOT(Stop()), Qt::QueuedConnection);

        this->m_vidPipelines[vidIx]->Configure();
    }

    m_model = NULL;

    m_frames = 0;
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoBufferSwap(false);
    setAutoFillBackground(false);

#ifdef ENABLE_YUV_WINDOW
    m_yuvWindow = new YuvDebugWindow(this);
    /* Build a colour map */
    for(int i = 0; i < 256; i++)
    {
        m_colourMap.push_back(qRgb(i, i, i));
    }
#endif

    m_dataFilesDir = QString(qgetenv(DATA_DIR_ENV_VAR_NAME));
    if(m_dataFilesDir.size() == 0)
    {
        m_dataFilesDir = QString("./");
    }
    else
    {
        m_dataFilesDir += "/";
    }
    qDebug("m_dataFilesDir = %s", m_dataFilesDir.toUtf8().constData());
}

GLWidget::~GLWidget()
{
}

void GLWidget::initializeGL()
{
    QString verStr((const char*)glGetString(GL_VERSION));
    qDebug("GL_VERSION: %s", verStr.toUtf8().constData());

    QStringList verNums = verStr.split(QRegExp("[ .]"));
    bool foundVerNum = false;
    for(int verNumIx = 0; verNumIx < verNums.length(); verNumIx++)
    {
        int verNum = verNums[verNumIx].toInt(&foundVerNum);
        if(foundVerNum)
        {
            if(verNum < 2)
            {
                qCritical("Support for OpenGL 2.0 is required for this demo...exiting\n");
                close();
            }
            break;
        }
    }
    if(!foundVerNum)
    {
        qCritical("Couldn't find OpenGL version number\n");
    }

    qDebug("Window is%s double buffered", ((this->format().doubleBuffer()) ? "": " not"));

    qglClearColor(QColor(Qt::black));

    setupShader(&m_brickProg, BrickGLESShaderList, NUM_SHADERS_BRICKGLES);
    // Set up initial uniform values
//    m_brickProg.setUniformValue("BrickColor", QVector3D(1.0, 0.3, 0.2));
//    m_brickProg.setUniformValue("MortarColor", QVector3D(0.85, 0.86, 0.84));
    m_brickProg.setUniformValue("BrickColor", QVector3D(0.0, 0.5, 1.0));
    m_brickProg.setUniformValue("MortarColor", QVector3D(0.0, 0.5, 1.0));
    m_brickProg.setUniformValue("BrickSize", QVector3D(0.30, 0.15, 0.30));
    m_brickProg.setUniformValue("BrickPct", QVector3D(0.90, 0.85, 0.90));
    m_brickProg.setUniformValue("LightPosition", QVector3D(0.0, 0.0, 4.0));
    m_brickProg.release();
    printOpenGLError(__FILE__, __LINE__);

#ifdef VIDI420_SHADERS_NEEDED
    setupShader(&m_I420NoEffectNormalised, VidI420NoEffectNormalisedShaderList, NUM_SHADERS_VIDI420_NOEFFECT_NORMALISED);
    setupShader(&m_I420LitNormalised, VidI420LitNormalisedShaderList, NUM_SHADERS_VIDI420_LIT_NORMALISED);
    setupShader(&m_I420NoEffect, VidI420NoEffectShaderList, NUM_SHADERS_VIDI420_NOEFFECT);
    setupShader(&m_I420ColourHilight, VidI420ColourHilightShaderList, NUM_SHADERS_VIDI420_COLOURHILIGHT);
    setupShader(&m_I420ColourHilightSwap, VidI420ColourHilightSwapShaderList, NUM_SHADERS_VIDI420_COLOURHILIGHTSWAP);
    setupShader(&m_I420AlphaMask, VidI420AlphaMaskShaderList, NUM_SHADERS_VIDI420_ALPHAMASK);
#endif

#ifdef VIDUYVY_SHADERS_NEEDED
    setupShader(&m_UYVYNoEffectNormalised, VidUYVYNoEffectNormalisedShaderList, NUM_SHADERS_VIDUYVY_NOEFFECT_NORMALISED);
    setupShader(&m_UYVYLitNormalised, VidUYVYLitNormalisedShaderList, NUM_SHADERS_VIDUYVY_LIT_NORMALISED);
    setupShader(&m_UYVYNoEffect, VidUYVYNoEffectShaderList, NUM_SHADERS_VIDUYVY_NOEFFECT);
    setupShader(&m_UYVYColourHilight, VidUYVYColourHilightShaderList, NUM_SHADERS_VIDUYVY_COLOURHILIGHT);
    setupShader(&m_UYVYColourHilightSwap, VidUYVYColourHilightSwapShaderList, NUM_SHADERS_VIDUYVY_COLOURHILIGHTSWAP);
    setupShader(&m_UYVYAlphaMask, VidUYVYAlphaMaskShaderList, NUM_SHADERS_VIDUYVY_ALPHAMASK);
#endif

    glTexParameteri(GL_RECT_VID_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_RECT_VID_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_RECT_VID_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_RECT_VID_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Set uniforms for vid shaders along with other stream details when first
    // frame comes through


    // Create entry in tex info vector for all pipelines
    for(int vidIx = 0; vidIx < this->m_vidPipelines.size(); vidIx++)
    {
        VidTextureInfo newInfo;
        glGenTextures(1, &newInfo.texId);
        newInfo.texInfoValid = false;
        newInfo.buffer = NULL;
        newInfo.effect = VidShaderNoEffect;
#ifdef ENABLE_FRAME_COUNT_DEBUG
        newInfo.frameCount = 0;
#endif

        this->m_vidTextures.push_back(newInfo);
    }

    m_model = new Model();
    if(m_model->Load(m_dataFilesDir + DFLT_OBJ_MODEL_FILE_NAME) != 0)
    {
        qCritical() << "Couldn't load obj model file " << m_dataFilesDir + DFLT_OBJ_MODEL_FILE_NAME;
    }
    m_model->SetScale(MODEL_BOUNDARY_SIZE);


    for(int vidIx = 0; vidIx < this->m_vidPipelines.size(); vidIx++)
    {
        this->m_vidPipelines[vidIx]->Start();
    }
}

Pipeline* GLWidget::createPipeline(int vidIx)
{
    return new GStreamerPipeline(vidIx, this->m_videoLoc[vidIx], SLOT(newFrame(int)), this);
}

void GLWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    makeCurrent();

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    this->m_modelViewMatrix = QMatrix4x4();
    this->m_modelViewMatrix.lookAt(QVector3D(0.0, 0.0, -5.0), QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 1.0, 0.0));
    this->m_modelViewMatrix.rotate(-m_zRot / 16.0, 0.0, 0.0, 1.0);
    this->m_modelViewMatrix.rotate(-m_xRot / 16.0, 1.0, 0.0, 0.0);
    this->m_modelViewMatrix.rotate(m_yRot / 16.0, 0.0, 1.0, 0.0);
    this->m_modelViewMatrix.scale(m_scaleValue);

    // Draw an object in the middle
    ModelEffectType enabledModelEffect = m_currentModelEffectIndex;
    QGLShaderProgram *currentShader = NULL;
    switch(enabledModelEffect)
    {
    case ModelEffectBrick:
        m_brickProg.bind();
        currentShader = &m_brickProg;
        break;
    case ModelEffectVideo:
        glActiveTexture(GL_RECT_VID_TEXTURE0);
        glBindTexture(GL_RECT_VID_TEXTURE_2D, this->m_vidTextures[0].texId);

        this->m_vidTextures[0].effect = VidShaderNoEffectNormalisedTexCoords;
        setAppropriateVidShader(0);
        this->m_vidTextures[0].shader->bind();
        setVidShaderVars(0, false);

        currentShader = this->m_vidTextures[0].shader;
        break;

    case ModelEffectVideoLit:
        glActiveTexture(GL_RECT_VID_TEXTURE0);
        glBindTexture(GL_RECT_VID_TEXTURE_2D, this->m_vidTextures[0].texId);

        this->m_vidTextures[0].effect = VidShaderLitNormalisedTexCoords;
        setAppropriateVidShader(0);
        this->m_vidTextures[0].shader->bind();
        setVidShaderVars(0, false);

        currentShader = this->m_vidTextures[0].shader;
        break;
    }

    m_model->Draw(m_modelViewMatrix, m_projectionMatrix, currentShader, false);

    switch(enabledModelEffect)
    {
    case ModelEffectBrick:
        currentShader->release();
        break;
    case ModelEffectVideo:
    case ModelEffectVideoLit:
        this->m_vidTextures[0].effect = VidShaderNoEffect;
        setAppropriateVidShader(0);
        this->m_vidTextures[0].shader->bind();
        setVidShaderVars(0, false);

        printOpenGLError(__FILE__, __LINE__);
        break;
    }

    // Draw videos around the object
    for(int vidIx = 0; vidIx < this->m_vidTextures.size(); vidIx++)
    {
        if(this->m_vidTextures[vidIx].texInfoValid)
        {
            // Render a quad with the video on it:
            glActiveTexture(GL_RECT_VID_TEXTURE0);
            glBindTexture(GL_RECT_VID_TEXTURE_2D, this->m_vidTextures[vidIx].texId);
            printOpenGLError(__FILE__, __LINE__);

            if((this->m_vidTextures[vidIx].effect == VidShaderAlphaMask) && this->m_alphaTextureLoaded)
            {
                glEnable (GL_BLEND);
                glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glActiveTexture(GL_RECT_TEXTURE1);
                glBindTexture(GL_RECT_TEXTURE_2D, this->m_alphaTextureId);
            }

            this->m_vidTextures[vidIx].shader->bind();
            setVidShaderVars(vidIx, false);
            printOpenGLError(__FILE__, __LINE__);

            if(this->m_vidTextures[vidIx].effect == VidShaderColourHilightSwap)
            {
                this->m_vidTextures[vidIx].shader->setUniformValue("u_componentSwapR", m_colourComponentSwapR);
                this->m_vidTextures[vidIx].shader->setUniformValue("u_componentSwapG", m_colourComponentSwapG);
                this->m_vidTextures[vidIx].shader->setUniformValue("u_componentSwapB", m_colourComponentSwapB);
            }

            QGLShaderProgram *vidShader = this->m_vidTextures[vidIx].shader;

            QMatrix4x4 vidQuadMatrix = this->m_modelViewMatrix;

            if(m_stackVidQuads)
            {
                vidQuadMatrix.translate(0.0, 0.0, 2.0);
                vidQuadMatrix.translate(0.0, 0.0, 0.2*vidIx);
            }
            else
            {
                vidQuadMatrix.rotate((360/this->m_vidTextures.size())*vidIx, 0.0, 1.0, 0.0);
                vidQuadMatrix.translate(0.0, 0.0, 2.0);
            }


            vidShader->setUniformValue("u_mvp_matrix", m_projectionMatrix * vidQuadMatrix);
            vidShader->setUniformValue("u_mv_matrix", vidQuadMatrix);

            // Need to set these arrays up here as shader instances are shared between
            // all the videos:
            vidShader->enableAttributeArray("a_texCoord");
            vidShader->setAttributeArray("a_texCoord", this->m_vidTextures[vidIx].triStripTexCoords);

            if(this->m_vidTextures[vidIx].effect == VidShaderAlphaMask)
            {
                vidShader->enableAttributeArray("a_alphaTexCoord");
                vidShader->setAttributeArray("a_alphaTexCoord", this->m_vidTextures[vidIx].triStripAlphaTexCoords);
            }

            vidShader->enableAttributeArray("a_vertex");
            vidShader->setAttributeArray("a_vertex", this->m_vidTextures[vidIx].triStripVertices);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            vidShader->disableAttributeArray("a_vertex");
            if(this->m_vidTextures[vidIx].effect == VidShaderAlphaMask)
            {
                vidShader->disableAttributeArray("a_alphaTexCoord");
            }
            vidShader->disableAttributeArray("a_texCoord");
        }
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    painter.endNativePainting();
    QString framesPerSecond;
    framesPerSecond.setNum(m_frames /(m_frameTime.elapsed() / 1000.0), 'f', 2);
    painter.setPen(Qt::white);
    painter.drawText(20, 40, framesPerSecond + " fps");
    painter.end();
    swapBuffers();

    if (!(m_frames % 100))
    {
        m_frameTime.start();
        m_frames = 0;
    }
    ++m_frames;
}

void GLWidget::resizeGL(int wid, int ht)
{
    float vp = 0.8f;
    float aspect = (float) wid / (float) ht;

    glViewport(0, 0, wid, ht);

    this->m_projectionMatrix = QMatrix4x4();
    this->m_projectionMatrix.frustum(-vp, vp, -vp / aspect, vp / aspect, 1.0, 50.0);
}

void GLWidget::newFrame(int vidIx)
{
    if(this->m_vidPipelines[vidIx])
    {
#ifdef ENABLE_FRAME_COUNT_DEBUG
        qDebug("GLWidget: vid %d frame %d", vidIx, this->m_vidTextures[vidIx].frameCount++);
#endif

        Pipeline *pipeline = this->m_vidPipelines[vidIx];


        /* If we have a vid frame currently, return it back to the video
           system */
        if(this->m_vidTextures[vidIx].buffer)
        {
            pipeline->m_outgoingBufQueue.put(this->m_vidTextures[vidIx].buffer);
            PIPELINE_DEBUG("GLWidget: vid %d pushed buffer %p to outgoing queue", vidIx, this->m_vidTextures[vidIx].buffer);
        }

        void *newBuf = NULL;
        if(pipeline->m_incomingBufQueue.get(&newBuf) == true)
        {
            this->m_vidTextures[vidIx].buffer = newBuf;
        }
        else
        {
            this->m_vidTextures[vidIx].buffer = NULL;
            return;
        }

        PIPELINE_DEBUG("GLWidget: vid %d popped buffer %p from incoming queue", vidIx, this->m_vidTextures[vidIx].buffer);

        this->makeCurrent();

        // Load the gst buf into a texture
        if(this->m_vidTextures[vidIx].texInfoValid == false)
        {
            PIPELINE_DEBUG("GLWidget: Received first frame of vid %d", vidIx);

            // Try and keep this fairly portable to other media frameworks by
            // leaving info extraction within pipeline class
            this->m_vidTextures[vidIx].width = pipeline->getWidth();
            this->m_vidTextures[vidIx].height = pipeline->getHeight();
            this->m_vidTextures[vidIx].colourFormat = pipeline->getColourFormat();
            this->m_vidTextures[vidIx].texInfoValid = true;

            setAppropriateVidShader(vidIx);

            this->m_vidTextures[vidIx].shader->bind();
            printOpenGLError(__FILE__, __LINE__);
            // Setting shader variables here will have no effect as they are set on every render,
            // but do it to check for errors, so we don't need to check on every render
            // and program output doesn't go mad
            setVidShaderVars(vidIx, true);

            GLfloat vidWidth = this->m_vidTextures[vidIx].width;
            GLfloat vidHeight = this->m_vidTextures[vidIx].height;

            this->m_vidTextures[vidIx].triStripTexCoords[0]      = QVector2D(vidWidth, 0.0f);
            this->m_vidTextures[vidIx].triStripVertices[0]       = QVector2D(VIDTEXTURE_RIGHT_X, VIDTEXTURE_TOP_Y);

            this->m_vidTextures[vidIx].triStripTexCoords[1]      = QVector2D(0.0f, 0.0f);
            this->m_vidTextures[vidIx].triStripVertices[1]       = QVector2D(VIDTEXTURE_LEFT_X, VIDTEXTURE_TOP_Y);

            this->m_vidTextures[vidIx].triStripTexCoords[2]      = QVector2D(vidWidth, vidHeight);
            this->m_vidTextures[vidIx].triStripVertices[2]       = QVector2D(VIDTEXTURE_RIGHT_X, VIDTEXTURE_BOT_Y);

            this->m_vidTextures[vidIx].triStripTexCoords[3]      = QVector2D(0.0f, vidHeight);
            this->m_vidTextures[vidIx].triStripVertices[3]       = QVector2D(VIDTEXTURE_LEFT_X, VIDTEXTURE_BOT_Y);
        }

        loadNewTexture(vidIx);

#ifdef ENABLE_YUV_WINDOW
        if((vidIx == 0) && (m_yuvWindow->isVisible()))
        {
            QImage yuvImage;
            switch(this->m_vidTextures[vidIx].colourFormat)
            {
            case ColFmt_I420:
            default:
                yuvImage = QImage(this->m_vidPipelines[vidIx]->bufToVidDataStart(this->m_vidTextures[vidIx].buffer),
                                this->m_vidTextures[vidIx].width,
                                this->m_vidTextures[vidIx].height*1.5f,
                                QImage::Format_Indexed8);
                break;
            case ColFmt_UYVY:
                yuvImage = QImage(this->m_vidPipelines[vidIx]->bufToVidDataStart(this->m_vidTextures[vidIx].buffer),
                                this->m_vidTextures[vidIx].width*2,
                                this->m_vidTextures[vidIx].height,
                                QImage::Format_Indexed8);
                break;
            }
            yuvImage.setColorTable(m_colourMap);
            m_yuvWindow->m_imageLabel->setPixmap(QPixmap::fromImage(yuvImage));
        }
#endif

        printOpenGLError(__FILE__, __LINE__);

        this->update();
    }
}

void GLWidget::loadNewTexture(int vidIx)
{
    glBindTexture (GL_RECT_VID_TEXTURE_2D, this->m_vidTextures[vidIx].texId);

    switch(this->m_vidTextures[vidIx].colourFormat)
    {
    case ColFmt_I420:
        glTexImage2D  (GL_RECT_VID_TEXTURE_2D, 0, GL_LUMINANCE,
                       this->m_vidTextures[vidIx].width,
                       this->m_vidTextures[vidIx].height*1.5f,
                       0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                       this->m_vidPipelines[vidIx]->bufToVidDataStart(this->m_vidTextures[vidIx].buffer));
        break;
    case ColFmt_UYVY:
        glTexImage2D  (GL_RECT_VID_TEXTURE_2D, 0, GL_LUMINANCE,
                       this->m_vidTextures[vidIx].width*2,
                       this->m_vidTextures[vidIx].height,
                       0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                       this->m_vidPipelines[vidIx]->bufToVidDataStart(this->m_vidTextures[vidIx].buffer));
        break;
    default:
        qCritical("Decide how to load texture for colour format %d", this->m_vidTextures[vidIx].colourFormat);
        break;
    }
}

void GLWidget::pipelineFinished(int vidIx)
{
#ifdef ENABLE_FRAME_COUNT_DEBUG
    this->m_vidTextures[vidIx].frameCount = 0;
#endif

    if(this->m_closing)
    {
        delete(this->m_vidPipelines[vidIx]);
        this->m_vidPipelines.replace(vidIx, NULL);
        this->m_vidTextures[vidIx].texInfoValid = false;

        // Check if any gst threads left, if not close
        bool allFinished = true;
        for(int i = 0; i < this->m_vidPipelines.size(); i++)
        {
            if(this->m_vidPipelines[i] != NULL)
            {
                // Catch any threads which were already finished at quitting time
                if(this->m_vidPipelines[i]->isFinished())
                {
                    delete(this->m_vidPipelines[vidIx]);
                    this->m_vidPipelines.replace(vidIx, NULL);
                    this->m_vidTextures[vidIx].texInfoValid = false;
                }
                else
                {
                    allFinished = false;
                    break;
                }
            }
        }
        if(allFinished)
        {
            close();
        }
    }
    else
    {
        delete(this->m_vidPipelines[vidIx]);
        this->m_vidTextures[vidIx].texInfoValid = false;

        this->m_vidPipelines[vidIx] = createPipeline(vidIx);

        if(this->m_vidPipelines[vidIx] == NULL)
        {
            qCritical("Error creating pipeline for vid %d", vidIx);
            return;
        }

        QObject::connect(this->m_vidPipelines[vidIx], SIGNAL(finished(int)),
                         this, SLOT(pipelineFinished(int)));
        QObject::connect(this, SIGNAL(closeRequested()),
                         this->m_vidPipelines[vidIx], SLOT(Stop()), Qt::QueuedConnection);

        this->m_vidPipelines[vidIx]->Configure();
        this->m_vidPipelines[vidIx]->Start();
    }
}

// Layout size
QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}

// Animation
static int qNormalizeAngle(int angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;

    return angle;
}

void GLWidget::animate()
{
    /* Increment wrt inertia */
    if (m_rotateOn)
    {
        m_xRot = qNormalizeAngle(m_xRot + (8 * m_yInertia));
        m_yRot = qNormalizeAngle(m_yRot + (8 * m_xInertia));
    }

    /* Colour swapping effect shader */
    if(m_colourSwapDirUpwards)
    {
        if((m_colourComponentSwapB.z() < 0.1) || (m_colourComponentSwapG.z() > 0.9))
        {
            m_colourSwapDirUpwards = false;
        }
        else
        {
            m_colourComponentSwapB.setZ(m_colourComponentSwapB.z() - 0.01);
            m_colourComponentSwapG.setZ(m_colourComponentSwapG.z() + 0.01);
        }
    }
    else
    {
        if((m_colourComponentSwapB.z() > 0.9) || (m_colourComponentSwapG.z() < 0.1))
        {
            m_colourSwapDirUpwards = true;
        }
        else
        {
            m_colourComponentSwapB.setZ(m_colourComponentSwapB.z() + 0.01);
            m_colourComponentSwapG.setZ(m_colourComponentSwapG.z() - 0.01);
        }
    }

    update();
}

// Input events
void GLWidget::cycleVidShaderSlot()
{
    int lastVidDrawn = this->m_vidTextures.size() - 1;
    if (this->m_vidTextures[lastVidDrawn].effect >= VidShaderLast)
        this->m_vidTextures[lastVidDrawn].effect = VidShaderFirst;
    else
        this->m_vidTextures[lastVidDrawn].effect = (VidShaderEffectType) ((int) this->m_vidTextures[lastVidDrawn].effect + 1);

    setAppropriateVidShader(lastVidDrawn);
    this->m_vidTextures[lastVidDrawn].shader->bind();
    printOpenGLError(__FILE__, __LINE__);
    // Setting shader variables here will have no effect as they are set on every render,
    // but do it to check for errors, so we don't need to check on every render
    // and program output doesn't go mad
    setVidShaderVars(lastVidDrawn, true);
}

void GLWidget::cycleModelShaderSlot()
{
    if (m_currentModelEffectIndex >= ModelEffectLast)
        m_currentModelEffectIndex = ModelEffectFirst;
    else
        m_currentModelEffectIndex = (ModelEffectType) ((int) m_currentModelEffectIndex + 1);
}

void GLWidget::showYUVWindowSlot()
{
    m_yuvWindow->show();
}

void GLWidget::loadVideoSlot()
{
    int lastVidDrawn = this->m_vidTextures.size() - 1;

    QString newFileName = QFileDialog::getOpenFileName(0, "Select a video file",
                                                         m_dataFilesDir + "videos/", "Videos (*.avi *.mkv *.ogg *.asf *.mov);;All (*.*)");
    if(newFileName.isNull() == false)
    {
        this->m_videoLoc[lastVidDrawn] = newFileName;

        //this->m_vidPipelines[lastVidDrawn]->setChooseNewOnFinished();
        this->m_vidPipelines[lastVidDrawn]->Stop();
    }
}

void GLWidget::loadModelSlot()
{
    // Load a Wavefront OBJ model file. Get the filename before doing anything else
    QString objFileName = QFileDialog::getOpenFileName(0, "Select a Wavefront OBJ file",
                                                          m_dataFilesDir + "models/", "Wavefront OBJ (*.obj)");
    if(objFileName.isNull() == false)
    {
        if(m_model->Load(objFileName) != 0)
        {
            qCritical() << "Couldn't load obj model file " << objFileName;
        }
        m_model->SetScale(MODEL_BOUNDARY_SIZE);
    }
}

void GLWidget::loadAlphaSlot()
{
    // Load an alpha mask texture. Get the filename before doing anything else
    QString alphaTexFileName = QFileDialog::getOpenFileName(0, "Select an image file",
                                                            m_dataFilesDir + "alphamasks/", "Pictures (*.bmp *.jpg *.jpeg *.gif);;All (*.*)");
    if(alphaTexFileName.isNull() == false)
    {
        QImage alphaTexImage(alphaTexFileName);
        if(alphaTexImage.isNull() == false)
        {
            // Ok, a new image is loaded
            if(m_alphaTextureLoaded)
            {
                // Delete the old texture
                m_alphaTextureLoaded = false;
                deleteTexture(m_alphaTextureId);
            }

            // Bind new image to texture
            m_alphaTextureId = bindTexture(alphaTexImage.mirrored(true, true), GL_RECT_TEXTURE_2D);
            m_alphaTexWidth = alphaTexImage.width();
            m_alphaTexHeight = alphaTexImage.height();
            // Update alpha tex co-ords in shader in case it is active:
            setVidShaderVars((this->m_vidTextures.size() - 1), true);
            m_alphaTextureLoaded = true;
        }
    }
}

void GLWidget::rotateToggleSlot(bool toggleState)
{
    m_rotateOn = toggleState;

    if (!m_rotateOn)
    {
        m_xInertiaOld = m_xInertia;
        m_yInertiaOld = m_yInertia;
    }
    else
    {
        m_xInertia = m_xInertiaOld;
        m_yInertia = m_yInertiaOld;

        // To prevent confusion, force some rotation
        if ((m_xInertia == 0.0) && (m_yInertia == 0.0))
            m_xInertia = -0.5;
    }
}

void GLWidget::stackVidsToggleSlot(int toggleState)
{
    if(toggleState == Qt::Checked)
        m_stackVidQuads = true;
    else
        m_stackVidQuads = false;
}

void GLWidget::cycleBackgroundSlot()
{
    switch( m_clearColorIndex++ )
    {
        case 0:  qglClearColor(QColor(Qt::black));
             break;
        case 1:  qglClearColor(QColor::fromRgbF(0.2f, 0.2f, 0.3f, 1.0f));
             break;
        default: qglClearColor(QColor::fromRgbF(0.7f, 0.7f, 0.7f, 1.0f));
             m_clearColorIndex = 0;
             break;
    }
}

void GLWidget::resetPosSlot()
{
    m_xRot = 0;
    m_yRot = 35;
    m_zRot = 0;
    m_xLastIncr = 0;
    m_yLastIncr = 0;
    m_xInertia = -0.5;
    m_yInertia = 0;
    m_scaleValue    = 1.0;
}

void GLWidget::exitSlot()
{
    close();
}



void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();

    if (event->button() == Qt::LeftButton)
    {
        m_xInertia = 0;
        m_yInertia = 0;

        m_xLastIncr = 0;
        m_yLastIncr = 0;
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Left button released
        m_lastPos.setX(-1);
        m_lastPos.setY(-1);

        if (m_xLastIncr > INERTIA_THRESHOLD)
          m_xInertia = (m_xLastIncr - INERTIA_THRESHOLD)*INERTIA_FACTOR;

        if (-m_xLastIncr > INERTIA_THRESHOLD)
          m_xInertia = (m_xLastIncr + INERTIA_THRESHOLD)*INERTIA_FACTOR;

        if (m_yLastIncr > INERTIA_THRESHOLD)
          m_yInertia = (m_yLastIncr - INERTIA_THRESHOLD)*INERTIA_FACTOR;

        if (-m_yLastIncr > INERTIA_THRESHOLD)
          m_yInertia = (m_yLastIncr + INERTIA_THRESHOLD)*INERTIA_FACTOR;

    }
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if((m_lastPos.x() != -1) && (m_lastPos.y() != -1))
    {
        m_xLastIncr = event->x() - m_lastPos.x();
        m_yLastIncr = event->y() - m_lastPos.y();

        if ((event->modifiers() & Qt::ControlModifier)
            || (event->buttons() & Qt::RightButton))
        {
           if (m_lastPos.x() != -1)
           {
               m_zRot = qNormalizeAngle(m_zRot + (8 * m_xLastIncr));
               m_scaleValue += (m_yLastIncr)*SCALE_FACTOR;
               update();
           }
        }
        else
        {
           if (m_lastPos.x() != -1)
           {
               m_xRot = qNormalizeAngle(m_xRot + (8 * m_yLastIncr));
               m_yRot = qNormalizeAngle(m_yRot + (8 * m_xLastIncr));
               update();
           }
        }
    }

    m_lastPos = event->pos();
}

void GLWidget::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
        case Qt::Key_Question:
        case Qt::Key_H:
            std::cout <<  "\nKeyboard commands:\n\n"
                          "? - Help\n"
                          "q, <esc> - Quit\n"
                          "b - Toggle among background clear colors\n"
                          "m - Load a different model to render\n"
                          "s - "
                          "a - "
                          "v - "
                          "o - "
                          "p - "
                          "<home>     - reset zoom and rotation\n"
                          "<space> or <click>        - stop rotation\n"
                          "<+>, <-> or <ctrl + drag> - zoom model\n"
                          "<arrow keys> or <drag>    - rotate model\n"
#ifdef ENABLE_YUV_WINDOW
                          "y - View yuv data of vid 0 in modeless window"
#endif
                          "\n";
            break;
        case Qt::Key_Escape:
        case Qt::Key_Q:
            exitSlot();
            break;

        case Qt::Key_B:
            cycleBackgroundSlot();
            break;

        case Qt::Key_S:
            cycleVidShaderSlot();
            break;
        case Qt::Key_A:
            loadAlphaSlot();
            break;
        case Qt::Key_M:
            loadModelSlot();
            break;
        case Qt::Key_V:
            loadVideoSlot();
            break;
        case Qt::Key_O:
            cycleModelShaderSlot();
            break;
        case Qt::Key_P:
            // Decouple bool used within class from Qt check box state enum values
            stackVidsToggleSlot(m_stackVidQuads ? Qt::Unchecked : Qt::Checked);
            emit stackVidsStateChanged(m_stackVidQuads ? Qt::Checked : Qt::Unchecked);
            break;

        case Qt::Key_Space:
            rotateToggleSlot(m_rotateOn ? false : true); //Qt::Unchecked : Qt::Checked)
            emit rotateStateChanged(m_rotateOn);// ? Qt::Checked : Qt::Unchecked);
            break;
        case Qt::Key_Plus:
            m_scaleValue += SCALE_INCREMENT;
            break;
        case Qt::Key_Minus:
            m_scaleValue -= SCALE_INCREMENT;
            break;
        case Qt::Key_Home:
            resetPosSlot();
            break;
        case Qt::Key_Left:
           m_yRot -= 8;
            break;
        case Qt::Key_Right:
           m_yRot += 8;
            break;
        case Qt::Key_Up:
           m_xRot -= 8;
            break;
        case Qt::Key_Down:
           m_xRot += 8;
            break;

#ifdef ENABLE_YUV_WINDOW
        case Qt::Key_Y:
            showYUVWindowSlot();
            break;
#endif

        default:
            QGLWidget::keyPressEvent(e);
            break;
    }
}

void GLWidget::closeEvent(QCloseEvent* event)
{
    if(this->m_closing == false)
    {
        this->m_closing = true;
        emit closeRequested();

        // Just in case, check now if any gst threads still exist, if not, close application now
        bool allFinished = true;
        for(int i = 0; i < this->m_vidPipelines.size(); i++)
        {
            if(this->m_vidPipelines[i] != NULL)
            {
                allFinished = false;
                break;
            }
        }
        if(allFinished)
        {
            close();
        }
        event->ignore();
    }
    else
    {
        // This is where we're all finished and really are m_closing now.
        // At the mo, tell parent to close too.
        QWidget* _parent = dynamic_cast<QWidget*>(parent());
        if(_parent)
            _parent->close();
    }
}

// Shader management
void GLWidget::setAppropriateVidShader(int vidIx)
{
    switch(this->m_vidTextures[vidIx].colourFormat)
    {
#ifdef VIDI420_SHADERS_NEEDED
    case ColFmt_I420:
        switch(this->m_vidTextures[vidIx].effect)
        {
        case VidShaderNoEffect:
            this->m_vidTextures[vidIx].shader = &m_I420NoEffect;
            break;
        case VidShaderNoEffectNormalisedTexCoords:
            this->m_vidTextures[vidIx].shader = &m_I420NoEffectNormalised;
            break;
        case VidShaderLitNormalisedTexCoords:
            this->m_vidTextures[vidIx].shader = &m_I420LitNormalised;
            break;
        case VidShaderColourHilight:
            this->m_vidTextures[vidIx].shader = &m_I420ColourHilight;
            break;
        case VidShaderColourHilightSwap:
            this->m_vidTextures[vidIx].shader = &m_I420ColourHilightSwap;
            break;
        case VidShaderAlphaMask:
            this->m_vidTextures[vidIx].shader = &m_I420AlphaMask;
            break;
        }
        break;
#endif
#ifdef VIDUYVY_SHADERS_NEEDED
    case ColFmt_UYVY:
        switch(this->m_vidTextures[vidIx].effect)
        {
        case VidShaderNoEffect:
            this->m_vidTextures[vidIx].shader = &m_UYVYNoEffect;
            break;
        case VidShaderNoEffectNormalisedTexCoords:
            this->m_vidTextures[vidIx].shader = &m_UYVYNoEffectNormalised;
            break;
        case VidShaderLitNormalisedTexCoords:
            this->m_vidTextures[vidIx].shader = &m_UYVYLitNormalised;
            break;
        case VidShaderColourHilight:
            this->m_vidTextures[vidIx].shader = &m_UYVYColourHilight;
            break;
        case VidShaderColourHilightSwap:
            this->m_vidTextures[vidIx].shader = &m_UYVYColourHilightSwap;
            break;
        case VidShaderAlphaMask:
            this->m_vidTextures[vidIx].shader = &m_UYVYAlphaMask;
            break;
        }
        break;
#endif
    default:
        qCritical ("Haven't implemented a shader for colour format %d yet, or its not enabled in the build",
                   this->m_vidTextures[vidIx].colourFormat);
        break;
    }
}

// Shader WILL be all set up for the specified video texture when this is called,
// or else!
void GLWidget::setVidShaderVars(int vidIx, bool printErrors)
{
    // TODO: move common vars out of switch

    switch(this->m_vidTextures[vidIx].effect)
    {
    case VidShaderNoEffect:
    case VidShaderNoEffectNormalisedTexCoords:
        // Temp:
        printOpenGLError(__FILE__, __LINE__);

        this->m_vidTextures[vidIx].shader->setUniformValue("u_vidTexture", 0); // texture unit index
        // Temp:
        printOpenGLError(__FILE__, __LINE__);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_yHeight", (GLfloat)this->m_vidTextures[vidIx].height);
        // Temp:
        printOpenGLError(__FILE__, __LINE__);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_yWidth", (GLfloat)this->m_vidTextures[vidIx].width);

        if(printErrors) printOpenGLError(__FILE__, __LINE__);
        break;

    case VidShaderLitNormalisedTexCoords:
        this->m_vidTextures[vidIx].shader->setUniformValue("u_vidTexture", 0); // texture unit index
        this->m_vidTextures[vidIx].shader->setUniformValue("u_yHeight", (GLfloat)this->m_vidTextures[vidIx].height);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_yWidth", (GLfloat)this->m_vidTextures[vidIx].width);

        this->m_vidTextures[vidIx].shader->setUniformValue("u_lightPosition", QVector3D(0.0, 0.0, 4.0));

        if(printErrors) printOpenGLError(__FILE__, __LINE__);
        break;

    case VidShaderColourHilight:
        this->m_vidTextures[vidIx].shader->setUniformValue("u_vidTexture", 0); // texture unit index
        this->m_vidTextures[vidIx].shader->setUniformValue("u_yHeight", (GLfloat)this->m_vidTextures[vidIx].height);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_yWidth", (GLfloat)this->m_vidTextures[vidIx].width);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_colrToDisplayMin", m_colourHilightRangeMin);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_colrToDisplayMax", m_colourHilightRangeMax);
        if(printErrors) printOpenGLError(__FILE__, __LINE__);
        break;

    case VidShaderColourHilightSwap:
        this->m_vidTextures[vidIx].shader->setUniformValue("u_vidTexture", 0); // texture unit index
        this->m_vidTextures[vidIx].shader->setUniformValue("u_yHeight", (GLfloat)this->m_vidTextures[vidIx].height);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_yWidth", (GLfloat)this->m_vidTextures[vidIx].width);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_colrToDisplayMin", m_colourHilightRangeMin);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_colrToDisplayMax", m_colourHilightRangeMax);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_componentSwapR", m_colourComponentSwapR);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_componentSwapG", m_colourComponentSwapG);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_componentSwapB", m_colourComponentSwapB);
        if(printErrors) printOpenGLError(__FILE__, __LINE__);
        break;

    case VidShaderAlphaMask:
        this->m_vidTextures[vidIx].shader->setUniformValue("u_vidTexture", 0); // texture unit index
        this->m_vidTextures[vidIx].shader->setUniformValue("u_yHeight", (GLfloat)this->m_vidTextures[vidIx].height);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_yWidth", (GLfloat)this->m_vidTextures[vidIx].width);
        this->m_vidTextures[vidIx].shader->setUniformValue("u_alphaTexture", 1); // texture unit index
        if(printErrors) printOpenGLError(__FILE__, __LINE__);

        this->m_vidTextures[vidIx].triStripAlphaTexCoords[0] = QVector2D(m_alphaTexWidth, 0.0f);
        this->m_vidTextures[vidIx].triStripAlphaTexCoords[1] = QVector2D(0.0f, 0.0f);
        this->m_vidTextures[vidIx].triStripAlphaTexCoords[2] = QVector2D(m_alphaTexWidth, m_alphaTexHeight);
        this->m_vidTextures[vidIx].triStripAlphaTexCoords[3] = QVector2D(0.0f, m_alphaTexHeight);
        break;

    default:
        qDebug ("Invalid effect set on vidIx %d", vidIx);
        break;
    }
}

int GLWidget::loadShaderFile(QString fileName, QString &shaderSource)
{
    fileName = m_dataFilesDir + fileName;

    shaderSource.clear();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical("File '%s' does not exist!", qPrintable(fileName));
        return -1;
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        shaderSource += in.readLine();
        shaderSource += "\n";
    }

    return 0;
}

int GLWidget::setupShader(QGLShaderProgram *prog, GLShaderModule shaderList[], int listLen)
{
    bool ret;

    qDebug ("Setting up a shader:");

    QString shaderSource;
    for(int listIx = 0; listIx < listLen; listIx++)
    {
        if(shaderList[listIx].type == QGLShader::Vertex)
        {
            QString nextShaderSource;

            qDebug ("concatenating %s", shaderList[listIx].sourceFileName);

            ret = loadShaderFile(shaderList[listIx].sourceFileName, nextShaderSource);
            if(ret != 0)
            {
                return ret;
            }

            shaderSource += nextShaderSource;
        }
    }

    if(!shaderSource.isEmpty())
    {
        qDebug ("compiling vertex shader");

        ret = prog->addShaderFromSourceCode(QGLShader::Vertex, shaderSource);
#if 0
        if(ret == false)
        {
            qCritical() << "Compile log for shader " << shaderList[listIx].sourceFileName
                        << ":\n" << prog->log();
            return -1;
        }
#endif
    }

    shaderSource.clear();

    for(int listIx = 0; listIx < listLen; listIx++)
    {
        if(shaderList[listIx].type == QGLShader::Fragment)
        {
            QString nextShaderSource;

            qDebug ("concatenating %s", shaderList[listIx].sourceFileName);

            ret = loadShaderFile(shaderList[listIx].sourceFileName, nextShaderSource);
            if(ret != 0)
            {
                return ret;
            }

            shaderSource += nextShaderSource;
        }
    }

    if(!shaderSource.isEmpty())
    {
        qDebug ("compiling fragment shader");

        ret = prog->addShaderFromSourceCode(QGLShader::Fragment, shaderSource);
#if 0
        if(ret == false)
        {
            qCritical() << "Compile log for shader " << shaderList[listIx].sourceFileName
                        << ":\n" << prog->log();
            return -1;
        }
#endif
    }

    ret = prog->link();
    if(ret == false)
    {
        qCritical() << "Link log for shaders ";
        for(int listIx = 0; listIx < listLen; listIx++)
        {
            qCritical() << shaderList[listIx].sourceFileName << " ";
        }
        qCritical() << "\n" << prog->log();
        return -1;
    }

    ret = prog->bind();
    if(ret == false)
    {
        return -1;
    } 

    printOpenGLError(__FILE__, __LINE__);

    return 0;
}


int GLWidget::printOpenGLError(const char *file, int line)
{
    //
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    //
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
#ifdef GLU_NEEDED
        qCritical() << "glError in file " << file << " @ line " << line << ": " << (const char *)gluErrorString(glErr);
#else
        qCritical() << "glError in file " << file << " @ line " << line << ": " << glErr;
#endif
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}
