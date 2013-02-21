#include <QMainWindow>
#include "glwidget.h"
#include "shaderlists.h"

#ifndef RECTTEX
 #define GL_TEXTURE_RECTANGLE_ARB            GL_TEXTURE_2D
 #define GL_TEXTURE0_ARB                     GL_TEXTURE0
 #define GL_TEXTURE1_ARB                     GL_TEXTURE1
#else
 #include "GL/glu.h"
#endif


GLWidget::GLWidget(int argc, char *argv[], QWidget *parent) :
    QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba), parent),
    closing(false),
    brickProg(this)
{
    xRot = 0;
    yRot = 0;
    zRot = 0;
    fScale = 1.0;
    lastPos = QPoint(0, 0);

    Rotate = 1;
    xLastIncr = 0;
    yLastIncr = 0;
    fXInertia = -0.5;
    fYInertia = 0;

    clearColor = 0;
    stackVidQuads = false;
    currentModelEffect = ModelEffectFirst;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
    timer->start(20);

    grabKeyboard();

    // Video shader effects vars
    ColourHilightRangeMin = QVector4D(0.0, 0.0, 0.0, 0.0);
    ColourHilightRangeMax = QVector4D(0.2, 0.2, 1.0, 1.0); // show shades of blue as they are
    ColourComponentSwapR = QVector4D(1.0, 1.0, 1.0, 0.0);
    ColourComponentSwapG = QVector4D(1.0, 1.0, 0.0, 0.0);
    ColourComponentSwapB = QVector4D(1.0, 1.0, 1.0, 0.0);
    ColourSwapDirUpwards = true;
    alphaTextureLoaded = false;

    // Video pipeline
    for(int vidIx = 1; vidIx < argc; vidIx++)
    {
        videoLoc.push_back(QString(argv[vidIx]));
    }

    // Instantiate video pipeline for each filename specified
    for(int vidIx = 0; vidIx < this->videoLoc.size(); vidIx++)
    {
        this->vidPipelines.push_back(this->createPipeline(vidIx));
        QObject::connect(this->vidPipelines[vidIx], SIGNAL(finished(int)),
                         this, SLOT(pipelineFinished(int)));
        QObject::connect(this, SIGNAL(closeRequested()),
                         this->vidPipelines[vidIx], SLOT(Stop()), Qt::QueuedConnection);

        this->vidPipelines[vidIx]->Configure();
    }

    model = NULL;

    frames = 0;
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoBufferSwap(false);
    setAutoFillBackground(false);

#ifdef ENABLE_YUV_WINDOW
    yuvWindow = new YuvDebugWindow(this);
    /* Build a colour map */
    for(int i = 0; i < 256; i++)
    {
        colourMap.push_back(qRgb(i, i, i));
    }
#endif

    dataFilesDir = QString(qgetenv(DATA_DIR_ENV_VAR_NAME));
    if(dataFilesDir.size() == 0)
    {
        dataFilesDir = QString("./");
    }
    else
    {
        dataFilesDir += "/";
    }
    qDebug("dataFilesDir = %s", dataFilesDir.toAscii().constData());
}

GLWidget::~GLWidget()
{
}

void GLWidget::initializeGL()
{
    QString verStr((const char*)glGetString(GL_VERSION));
    qDebug("GL_VERSION: %s", verStr.toAscii().constData());

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

    setupShader(&brickProg, BrickGLESShaderList, NUM_SHADERS_BRICKGLES);
    // Set up initial uniform values
//    brickProg.setUniformValue("BrickColor", QVector3D(1.0, 0.3, 0.2));
//    brickProg.setUniformValue("MortarColor", QVector3D(0.85, 0.86, 0.84));
    brickProg.setUniformValue("BrickColor", QVector3D(0.0, 0.5, 1.0));
    brickProg.setUniformValue("MortarColor", QVector3D(0.0, 0.5, 1.0));
    brickProg.setUniformValue("BrickSize", QVector3D(0.30, 0.15, 0.30));
    brickProg.setUniformValue("BrickPct", QVector3D(0.90, 0.85, 0.90));
    brickProg.setUniformValue("LightPosition", QVector3D(0.0, 0.0, 4.0));
    brickProg.release();
    printOpenGLError(__FILE__, __LINE__);

    setupShader(&I420NoEffectNormalised, VidI420NoEffectNormalisedShaderList, NUM_SHADERS_VIDI420_NOEFFECT_NORMALISED);
    setupShader(&I420LitNormalised, VidI420LitNormalisedShaderList, NUM_SHADERS_VIDI420_LIT_NORMALISED);
    setupShader(&I420NoEffect, VidI420NoEffectShaderList, NUM_SHADERS_VIDI420_NOEFFECT);
    setupShader(&I420ColourHilight, VidI420ColourHilightShaderList, NUM_SHADERS_VIDI420_COLOURHILIGHT);
    setupShader(&I420ColourHilightSwap, VidI420ColourHilightSwapShaderList, NUM_SHADERS_VIDI420_COLOURHILIGHTSWAP);
    setupShader(&I420AlphaMask, VidI420AlphaMaskShaderList, NUM_SHADERS_VIDI420_ALPHAMASK);

    setupShader(&UYVYNoEffectNormalised, VidUYVYNoEffectNormalisedShaderList, NUM_SHADERS_VIDUYVY_NOEFFECT_NORMALISED);
    setupShader(&UYVYLitNormalised, VidUYVYLitNormalisedShaderList, NUM_SHADERS_VIDUYVY_LIT_NORMALISED);
    setupShader(&UYVYNoEffect, VidUYVYNoEffectShaderList, NUM_SHADERS_VIDUYVY_NOEFFECT);
    setupShader(&UYVYColourHilight, VidUYVYColourHilightShaderList, NUM_SHADERS_VIDUYVY_COLOURHILIGHT);
    setupShader(&UYVYColourHilightSwap, VidUYVYColourHilightSwapShaderList, NUM_SHADERS_VIDUYVY_COLOURHILIGHTSWAP);
    setupShader(&UYVYAlphaMask, VidUYVYAlphaMaskShaderList, NUM_SHADERS_VIDUYVY_ALPHAMASK);

    // Set uniforms for vid shaders along with other stream details when first
    // frame comes through


    // Create entry in tex info vector for all pipelines
    for(int vidIx = 0; vidIx < this->vidPipelines.size(); vidIx++)
    {
        VidTextureInfo newInfo;
        glGenTextures(1, &newInfo.texId);
        newInfo.texInfoValid = false;
        newInfo.buffer = NULL;
        newInfo.effect = VidShaderNoEffect;
#ifdef ENABLE_FRAME_COUNT_DEBUG
        newInfo.frameCount = 0;
#endif

        this->vidTextures.push_back(newInfo);
    }

    model = new Model();
    if(model->Load(dataFilesDir + DFLT_OBJ_MODEL_FILE_NAME) != 0)
    {
        qCritical() << "Couldn't load obj model file " << dataFilesDir + DFLT_OBJ_MODEL_FILE_NAME;
    }
    model->SetScale(MODEL_BOUNDARY_SIZE);


    for(int vidIx = 0; vidIx < this->vidPipelines.size(); vidIx++)
    {
        this->vidPipelines[vidIx]->Start();
    }
}

Pipeline* GLWidget::createPipeline(int vidIx)
{
    Pipeline *newPipelinePtr;

    // Could derive a custom class later to clean this up a bit
#if defined OMAP3530
    newPipelinePtr = new TIGStreamerPipeline(vidIx, this->videoLoc[vidIx], SLOT(newFrame(int)), this);
#elif defined UNIX
    newPipelinePtr = new GStreamerPipeline(vidIx, this->videoLoc[vidIx], SLOT(newFrame(int)), this);
#endif

    return newPipelinePtr;
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

    this->modelViewMatrix = QMatrix4x4();
    this->modelViewMatrix.lookAt(QVector3D(0.0, 0.0, -5.0), QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 1.0, 0.0));
    this->modelViewMatrix.rotate(-zRot / 16.0, 0.0, 0.0, 1.0);
    this->modelViewMatrix.rotate(-xRot / 16.0, 1.0, 0.0, 0.0);
    this->modelViewMatrix.rotate(yRot / 16.0, 0.0, 1.0, 0.0);
    this->modelViewMatrix.scale(fScale);

    // Draw an object in the middle
    ModelEffectType enabledModelEffect = currentModelEffect;
    QGLShaderProgram *currentShader = NULL;
    switch(enabledModelEffect)
    {
    case ModelEffectBrick:
        brickProg.bind();
        currentShader = &brickProg;
        break;
    case ModelEffectVideo:
        glActiveTexture(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, this->vidTextures[0].texId);

        this->vidTextures[0].effect = VidShaderLitNormalisedTexCoords;
        setAppropriateVidShader(0);
        this->vidTextures[0].shader->bind();
        setVidShaderVars(0, false);

        currentShader = this->vidTextures[0].shader;
        break;
    }

    model->Draw(modelViewMatrix, projectionMatrix, currentShader, false);

    switch(enabledModelEffect)
    {
    case ModelEffectBrick:
        currentShader->release();
        break;
    case ModelEffectVideo:
        this->vidTextures[0].effect = VidShaderNoEffect;
        setAppropriateVidShader(0);
        this->vidTextures[0].shader->bind();
        setVidShaderVars(0, false);

        printOpenGLError(__FILE__, __LINE__);
        break;
    }

    // Draw videos around the object
    for(int vidIx = 0; vidIx < this->vidTextures.size(); vidIx++)
    {
        if(this->vidTextures[vidIx].texInfoValid)
        {
            // Render a quad with the video on it:
            glActiveTexture(GL_TEXTURE0_ARB);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, this->vidTextures[vidIx].texId);
            printOpenGLError(__FILE__, __LINE__);

            if((this->vidTextures[vidIx].effect == VidShaderAlphaMask) && this->alphaTextureLoaded)
            {
                glEnable (GL_BLEND);
                glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glActiveTexture(GL_TEXTURE1_ARB);
                glBindTexture(GL_TEXTURE_RECTANGLE_ARB, this->alphaTextureId);
            }

            this->vidTextures[vidIx].shader->bind();
            setVidShaderVars(vidIx, false);
            printOpenGLError(__FILE__, __LINE__);

            if(this->vidTextures[vidIx].effect == VidShaderColourHilightSwap)
            {
                this->vidTextures[vidIx].shader->setUniformValue("u_componentSwapR", ColourComponentSwapR);
                this->vidTextures[vidIx].shader->setUniformValue("u_componentSwapG", ColourComponentSwapG);
                this->vidTextures[vidIx].shader->setUniformValue("u_componentSwapB", ColourComponentSwapB);
            }

            QGLShaderProgram *vidShader = this->vidTextures[vidIx].shader;

            QMatrix4x4 vidQuadMatrix = this->modelViewMatrix;

            if(stackVidQuads)
            {
                vidQuadMatrix.translate(0.0, 0.0, 2.0);
                vidQuadMatrix.translate(0.0, 0.0, 0.2*vidIx);
            }
            else
            {
                vidQuadMatrix.rotate((360/this->vidTextures.size())*vidIx, 0.0, 1.0, 0.0);
                vidQuadMatrix.translate(0.0, 0.0, 2.0);
            }


            vidShader->setUniformValue("u_mvp_matrix", projectionMatrix * vidQuadMatrix);
            vidShader->setUniformValue("u_mv_matrix", vidQuadMatrix);

            // Need to set these arrays up here as shader instances are shared between
            // all the videos:
            vidShader->enableAttributeArray("a_texCoord");
            vidShader->setAttributeArray("a_texCoord", this->vidTextures[vidIx].triStripTexCoords);

            if(this->vidTextures[vidIx].effect == VidShaderAlphaMask)
            {
                vidShader->enableAttributeArray("a_alphaTexCoord");
                vidShader->setAttributeArray("a_alphaTexCoord", this->vidTextures[vidIx].triStripAlphaTexCoords);
            }

            vidShader->enableAttributeArray("a_vertex");
            vidShader->setAttributeArray("a_vertex", this->vidTextures[vidIx].triStripVertices);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            vidShader->disableAttributeArray("a_vertex");
            if(this->vidTextures[vidIx].effect == VidShaderAlphaMask)
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
    framesPerSecond.setNum(frames /(frameTime.elapsed() / 1000.0), 'f', 2);
    painter.setPen(Qt::white);
    painter.drawText(20, 40, framesPerSecond + " fps");
    painter.end();
    swapBuffers();

    if (!(frames % 100))
    {
        frameTime.start();
        frames = 0;
    }
    ++frames;
}

void GLWidget::resizeGL(int wid, int ht)
{
    float vp = 0.8f;
    float aspect = (float) wid / (float) ht;

    glViewport(0, 0, wid, ht);

    this->projectionMatrix = QMatrix4x4();
    this->projectionMatrix.frustum(-vp, vp, -vp / aspect, vp / aspect, 1.0, 50.0);
}

void GLWidget::newFrame(int vidIx)
{
    if(this->vidPipelines[vidIx])
    {
#ifdef ENABLE_FRAME_COUNT_DEBUG
        qDebug("GLWidget: vid %d frame %d", vidIx, this->vidTextures[vidIx].frameCount++);
#endif

        Pipeline *pipeline = this->vidPipelines[vidIx];


        /* Vid frame pointer is initialized as null */
        if(this->vidTextures[vidIx].buffer)
        {
            pipeline->m_outgoingBufQueue.put(this->vidTextures[vidIx].buffer);
            PIPELINE_DEBUG("GLWidget: vid %d pushed buffer %p to outgoing queue", vidIx, this->vidTextures[vidIx].buffer);
        }

        // TODO: use getWithWait here, return if NULL
        this->vidTextures[vidIx].buffer = pipeline->m_incomingBufQueue.get();
        PIPELINE_DEBUG("GLWidget: vid %d popped buffer %p from incoming queue", vidIx, this->vidTextures[vidIx].buffer);

        this->makeCurrent();

        // Load the gst buf into a texture
        if(this->vidTextures[vidIx].texInfoValid == false)
        {
            PIPELINE_DEBUG("GLWidget: Received first frame of vid %d", vidIx);

            // Try and keep this fairly portable to other media frameworks by
            // leaving info extraction within pipeline class
            this->vidTextures[vidIx].width = pipeline->getWidth();
            this->vidTextures[vidIx].height = pipeline->getHeight();
            this->vidTextures[vidIx].colourFormat = pipeline->getColourFormat();
            this->vidTextures[vidIx].texInfoValid = true;

            setAppropriateVidShader(vidIx);

            this->vidTextures[vidIx].shader->bind();
            printOpenGLError(__FILE__, __LINE__);
            // Setting shader variables here will have no effect as they are set on every render,
            // but do it to check for errors, so we don't need to check on every render
            // and program output doesn't go mad
            setVidShaderVars(vidIx, true);

            GLfloat vidWidth = this->vidTextures[vidIx].width;
            GLfloat vidHeight = this->vidTextures[vidIx].height;

            this->vidTextures[vidIx].triStripTexCoords[0]      = QVector2D(vidWidth, 0.0f);
            this->vidTextures[vidIx].triStripVertices[0]       = QVector2D(VIDTEXTURE_LEFT_X, VIDTEXTURE_TOP_Y);

            this->vidTextures[vidIx].triStripTexCoords[1]      = QVector2D(0.0f, 0.0f);
            this->vidTextures[vidIx].triStripVertices[1]       = QVector2D(VIDTEXTURE_RIGHT_X, VIDTEXTURE_TOP_Y);

            this->vidTextures[vidIx].triStripTexCoords[2]      = QVector2D(vidWidth, vidHeight);
            this->vidTextures[vidIx].triStripVertices[2]       = QVector2D(VIDTEXTURE_LEFT_X, VIDTEXTURE_BOT_Y);

            this->vidTextures[vidIx].triStripTexCoords[3]      = QVector2D(0.0f, vidHeight);
            this->vidTextures[vidIx].triStripVertices[3]       = QVector2D(VIDTEXTURE_RIGHT_X, VIDTEXTURE_BOT_Y);
        }


        glBindTexture (GL_TEXTURE_RECTANGLE_ARB, this->vidTextures[vidIx].texId);

        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        switch(this->vidTextures[vidIx].colourFormat)
        {
        case ColFmt_I420:
            glTexImage2D  (GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE,
                           this->vidTextures[vidIx].width,
                           1.5f*this->vidTextures[vidIx].height,
                           0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                           PIPELINE_BUFFER_VID_DATA_START(this->vidTextures[vidIx].buffer));

#ifdef ENABLE_YUV_WINDOW
            if((vidIx == 0) && (yuvWindow->isVisible()))
            {
                QImage yuvImage(PIPELINE_BUFFER_VID_DATA_START(this->vidTextures[vidIx].buffer),
                                this->vidTextures[vidIx].width,
                                1.5f*this->vidTextures[vidIx].height,
                                QImage::Format_Indexed8);
                yuvImage.setColorTable(colourMap);
                yuvWindow->imageLabel->setPixmap(QPixmap::fromImage(yuvImage));
            }
#endif
            break;
        case ColFmt_UYVY:
            glTexImage2D  (GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE,
                           this->vidTextures[vidIx].width*2,
                           this->vidTextures[vidIx].height,
                           0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                           PIPELINE_BUFFER_VID_DATA_START(this->vidTextures[vidIx].buffer));

#ifdef ENABLE_YUV_WINDOW
            if((vidIx == 0) && (yuvWindow->isVisible()))
            {
                QImage yuvImage(PIPELINE_BUFFER_VID_DATA_START(this->vidTextures[vidIx].buffer),
                                this->vidTextures[vidIx].width*2,
                                this->vidTextures[vidIx].height,
                                QImage::Format_Indexed8);
                yuvImage.setColorTable(colourMap);
                yuvWindow->imageLabel->setPixmap(QPixmap::fromImage(yuvImage));
            }
#endif
            break;
        default:
            qCritical("Decide how to load texture for colour format %d", this->vidTextures[vidIx].colourFormat);
            break;
        }
        printOpenGLError(__FILE__, __LINE__);

        this->update();
    }
}

void GLWidget::pipelineFinished(int vidIx)
{
#ifdef ENABLE_FRAME_COUNT_DEBUG
    this->vidTextures[vidIx].frameCount = 0;
#endif

    if(this->closing)
    {
        delete(this->vidPipelines[vidIx]);
        this->vidPipelines.replace(vidIx, NULL);
        this->vidTextures[vidIx].texInfoValid = false;

        // Check if any gst threads left, if not close
        bool allFinished = true;
        for(int i = 0; i < this->vidPipelines.size(); i++)
        {
            if(this->vidPipelines[i] != NULL)
            {
                // Catch any threads which were already finished at quitting time
                if(this->vidPipelines[i]->isFinished())
                {
                    delete(this->vidPipelines[vidIx]);
                    this->vidPipelines.replace(vidIx, NULL);
                    this->vidTextures[vidIx].texInfoValid = false;
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
#if 0
    else if(this->vidPipelines[vidIx]->chooseNew())
    {
        // Confirm that we have the new filename before we do anything else
        QString newFileName = QFileDialog::getOpenFileName(0, "Select a video file",
                                                           ".", "Videos (*.avi *.mkv *.ogg *.asf *.mov);;All (*.*)");

        if(newFileName.isNull() == false)
        {
            delete(this->vidPipelines[vidIx]);
            this->vidTextures[vidIx].texInfoValid = false;

            this->videoLoc[vidIx] = newFileName;
            this->vidPipelines[vidIx] = createPipeline(vidIx);

            QObject::connect(this->vidPipelines[vidIx], SIGNAL(finished(int)),
                             this, SLOT(pipelineFinished(int)));
            QObject::connect(this, SIGNAL(closeRequested()),
                             this->vidPipelines[vidIx], SLOT(stop()), Qt::QueuedConnection);

            this->vidPipelines[vidIx]->Configure();
            this->vidPipelines[vidIx]->Start();
        }
    }
#endif
    else
    {
        delete(this->vidPipelines[vidIx]);
        this->vidTextures[vidIx].texInfoValid = false;

        this->vidPipelines[vidIx] = createPipeline(vidIx);

        QObject::connect(this->vidPipelines[vidIx], SIGNAL(finished(int)),
                         this, SLOT(pipelineFinished(int)));
        QObject::connect(this, SIGNAL(closeRequested()),
                         this->vidPipelines[vidIx], SLOT(Stop()), Qt::QueuedConnection);

        this->vidPipelines[vidIx]->Configure();
        this->vidPipelines[vidIx]->Start();
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
    if (Rotate)
    {
        xRot = qNormalizeAngle(xRot + (8 * fYInertia));
        yRot = qNormalizeAngle(yRot + (8 * fXInertia));
    }

    /* Colour swapping effect shader */
    if(ColourSwapDirUpwards)
    {
        if((ColourComponentSwapB.z() < 0.1) || (ColourComponentSwapG.z() > 0.9))
        {
            ColourSwapDirUpwards = false;
        }
        else
        {
            ColourComponentSwapB.setZ(ColourComponentSwapB.z() - 0.01);
            ColourComponentSwapG.setZ(ColourComponentSwapG.z() + 0.01);
        }
    }
    else
    {
        if((ColourComponentSwapB.z() > 0.9) || (ColourComponentSwapG.z() < 0.1))
        {
            ColourSwapDirUpwards = true;
        }
        else
        {
            ColourComponentSwapB.setZ(ColourComponentSwapB.z() + 0.01);
            ColourComponentSwapG.setZ(ColourComponentSwapG.z() - 0.01);
        }
    }

    update();
}

// Input events
void GLWidget::cycleVidShaderSlot()
{
    int lastVidDrawn = this->vidTextures.size() - 1;
    if (this->vidTextures[lastVidDrawn].effect >= VidShaderLast)
        this->vidTextures[lastVidDrawn].effect = VidShaderFirst;
    else
        this->vidTextures[lastVidDrawn].effect = (VidShaderEffectType) ((int) this->vidTextures[lastVidDrawn].effect + 1);

    setAppropriateVidShader(lastVidDrawn);
    this->vidTextures[lastVidDrawn].shader->bind();
    printOpenGLError(__FILE__, __LINE__);
    // Setting shader variables here will have no effect as they are set on every render,
    // but do it to check for errors, so we don't need to check on every render
    // and program output doesn't go mad
    setVidShaderVars(lastVidDrawn, true);
}

void GLWidget::cycleModelShaderSlot()
{
    if (currentModelEffect >= ModelEffectLast)
        currentModelEffect = ModelEffectFirst;
    else
        currentModelEffect = (ModelEffectType) ((int) currentModelEffect + 1);
}

void GLWidget::showYUVWindowSlot()
{
    yuvWindow->show();
}

void GLWidget::loadVideoSlot()
{
    int lastVidDrawn = this->vidTextures.size() - 1;

    QString newFileName = QFileDialog::getOpenFileName(0, "Select a video file",
                                                         dataFilesDir + "videos/", "Videos (*.avi *.mkv *.ogg *.asf *.mov);;All (*.*)");
    if(newFileName.isNull() == false)
    {
        this->videoLoc[lastVidDrawn] = newFileName;

        //this->vidPipelines[lastVidDrawn]->setChooseNewOnFinished();
        this->vidPipelines[lastVidDrawn]->Stop();
    }
}

void GLWidget::loadModelSlot()
{
    // Load a Wavefront OBJ model file. Get the filename before doing anything else
    QString objFileName = QFileDialog::getOpenFileName(0, "Select a Wavefront OBJ file",
                                                          dataFilesDir + "models/", "Wavefront OBJ (*.obj)");
    if(objFileName.isNull() == false)
    {
        if(model->Load(objFileName) != 0)
        {
            qCritical() << "Couldn't load obj model file " << objFileName;
        }
        model->SetScale(MODEL_BOUNDARY_SIZE);
    }
}

void GLWidget::loadAlphaSlot()
{
    // Load an alpha mask texture. Get the filename before doing anything else
    QString alphaTexFileName = QFileDialog::getOpenFileName(0, "Select an image file",
                                                            dataFilesDir + "alphamasks/", "Pictures (*.bmp *.jpg *.jpeg *.gif);;All (*.*)");
    if(alphaTexFileName.isNull() == false)
    {
        QImage alphaTexImage(alphaTexFileName);
        if(alphaTexImage.isNull() == false)
        {
            // Ok, a new image is loaded
            if(alphaTextureLoaded)
            {
                // Delete the old texture
                alphaTextureLoaded = false;
                deleteTexture(alphaTextureId);
            }

            // Bind new image to texture
            alphaTextureId = bindTexture(alphaTexImage.mirrored(true, true), GL_TEXTURE_RECTANGLE_ARB);
            alphaTexWidth = alphaTexImage.width();
            alphaTexHeight = alphaTexImage.height();
            // Update alpha tex co-ords in shader in case it is active:
            setVidShaderVars((this->vidTextures.size() - 1), true);
            alphaTextureLoaded = true;
        }
    }
}

void GLWidget::rotateToggleSlot(bool toggleState)
{
    Rotate = toggleState;

    if (!Rotate)
    {
        fXInertiaOld = fXInertia;
        fYInertiaOld = fYInertia;
    }
    else
    {
        fXInertia = fXInertiaOld;
        fYInertia = fYInertiaOld;

        // To prevent confusion, force some rotation
        if ((fXInertia == 0.0) && (fYInertia == 0.0))
            fXInertia = -0.5;
    }
}

void GLWidget::stackVidsToggleSlot(int toggleState)
{
    if(toggleState == Qt::Checked)
        stackVidQuads = true;
    else
        stackVidQuads = false;
}

void GLWidget::cycleBackgroundSlot()
{
    switch( clearColor++ )
    {
        case 0:  qglClearColor(QColor(Qt::black));
             break;
        case 1:  qglClearColor(QColor::fromRgbF(0.2f, 0.2f, 0.3f, 1.0f));
             break;
        default: qglClearColor(QColor::fromRgbF(0.7f, 0.7f, 0.7f, 1.0f));
             clearColor = 0;
             break;
    }
}

void GLWidget::resetPosSlot()
{
    xRot = 0;
    yRot = 35;
    zRot = 0;
    xLastIncr = 0;
    yLastIncr = 0;
    fXInertia = -0.5;
    fYInertia = 0;
    fScale    = 1.0;
}

void GLWidget::exitSlot()
{
    close();
}



void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();

    if (event->button() == Qt::LeftButton)
    {
        fXInertia = 0;
        fYInertia = 0;

        xLastIncr = 0;
        yLastIncr = 0;
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Left button released
        lastPos.setX(-1);
        lastPos.setY(-1);

        if (xLastIncr > INERTIA_THRESHOLD)
          fXInertia = (xLastIncr - INERTIA_THRESHOLD)*INERTIA_FACTOR;

        if (-xLastIncr > INERTIA_THRESHOLD)
          fXInertia = (xLastIncr + INERTIA_THRESHOLD)*INERTIA_FACTOR;

        if (yLastIncr > INERTIA_THRESHOLD)
          fYInertia = (yLastIncr - INERTIA_THRESHOLD)*INERTIA_FACTOR;

        if (-yLastIncr > INERTIA_THRESHOLD)
          fYInertia = (yLastIncr + INERTIA_THRESHOLD)*INERTIA_FACTOR;

    }
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if((lastPos.x() != -1) && (lastPos.y() != -1))
    {
        xLastIncr = event->x() - lastPos.x();
        yLastIncr = event->y() - lastPos.y();

        if ((event->modifiers() & Qt::ControlModifier)
            || (event->buttons() & Qt::RightButton))
        {
           if (lastPos.x() != -1)
           {
               zRot = qNormalizeAngle(zRot + (8 * xLastIncr));
               fScale += (yLastIncr)*SCALE_FACTOR;
               update();
           }
        }
        else
        {
           if (lastPos.x() != -1)
           {
               xRot = qNormalizeAngle(xRot + (8 * yLastIncr));
               yRot = qNormalizeAngle(yRot + (8 * xLastIncr));
               update();
           }
        }
    }

    lastPos = event->pos();
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
            stackVidsToggleSlot(stackVidQuads ? Qt::Unchecked : Qt::Checked);
            emit stackVidsStateChanged(stackVidQuads ? Qt::Checked : Qt::Unchecked);
            break;

        case Qt::Key_Space:
            rotateToggleSlot(Rotate ? false : true); //Qt::Unchecked : Qt::Checked)
            emit rotateStateChanged(Rotate);// ? Qt::Checked : Qt::Unchecked);
            break;
        case Qt::Key_Plus:
            fScale += SCALE_INCREMENT;
            break;
        case Qt::Key_Minus:
            fScale -= SCALE_INCREMENT;
            break;
        case Qt::Key_Home:
            resetPosSlot();
            break;
        case Qt::Key_Left:
           yRot -= 8;
            break;
        case Qt::Key_Right:
           yRot += 8;
            break;
        case Qt::Key_Up:
           xRot -= 8;
            break;
        case Qt::Key_Down:
           xRot += 8;
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
    if(this->closing == false)
    {
        this->closing = true;
        emit closeRequested();

        // Just in case, check now if any gst threads still exist, if not, close application now
        bool allFinished = true;
        for(int i = 0; i < this->vidPipelines.size(); i++)
        {
            if(this->vidPipelines[i] != NULL)
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
        // This is where we're all finished and really are closing now.
        // At the mo, tell parent to close too.
        QWidget* _parent = dynamic_cast<QWidget*>(parent());
        if(_parent)
            _parent->close();
    }
}

// Shader management
void GLWidget::setAppropriateVidShader(int vidIx)
{
    switch(this->vidTextures[vidIx].colourFormat)
    {
    case ColFmt_I420:
        switch(this->vidTextures[vidIx].effect)
        {
        case VidShaderNoEffect:
            this->vidTextures[vidIx].shader = &I420NoEffect;
            break;
        case VidShaderNoEffectNormalisedTexCoords:
            this->vidTextures[vidIx].shader = &I420NoEffectNormalised;
            break;
        case VidShaderLitNormalisedTexCoords:
            this->vidTextures[vidIx].shader = &I420LitNormalised;
            break;
        case VidShaderColourHilight:
            this->vidTextures[vidIx].shader = &I420ColourHilight;
            break;
        case VidShaderColourHilightSwap:
            this->vidTextures[vidIx].shader = &I420ColourHilightSwap;
            break;
        case VidShaderAlphaMask:
            this->vidTextures[vidIx].shader = &I420AlphaMask;
            break;
        }
        break;
    case ColFmt_UYVY:
        switch(this->vidTextures[vidIx].effect)
        {
        case VidShaderNoEffect:
            this->vidTextures[vidIx].shader = &UYVYNoEffect;
            break;
        case VidShaderNoEffectNormalisedTexCoords:
            this->vidTextures[vidIx].shader = &UYVYNoEffectNormalised;
            break;
        case VidShaderLitNormalisedTexCoords:
            this->vidTextures[vidIx].shader = &UYVYLitNormalised;
            break;
        case VidShaderColourHilight:
            this->vidTextures[vidIx].shader = &UYVYColourHilight;
            break;
        case VidShaderColourHilightSwap:
            this->vidTextures[vidIx].shader = &UYVYColourHilightSwap;
            break;
        case VidShaderAlphaMask:
            this->vidTextures[vidIx].shader = &UYVYAlphaMask;
            break;
        }
        break;

    default:
        qCritical ("Haven't implemented a shader for colour format %d yet", this->vidTextures[vidIx].colourFormat);
        break;
    }
}

// Shader WILL be all set up for the specified video texture when this is called,
// or else!
void GLWidget::setVidShaderVars(int vidIx, bool printErrors)
{
    // TODO: move common vars out of switch

    switch(this->vidTextures[vidIx].effect)
    {
    case VidShaderNoEffect:
    case VidShaderNoEffectNormalisedTexCoords:
        // Temp:
        printOpenGLError(__FILE__, __LINE__);

        this->vidTextures[vidIx].shader->setUniformValue("u_vidTexture", 0); // texture unit index
        // Temp:
        printOpenGLError(__FILE__, __LINE__);
        this->vidTextures[vidIx].shader->setUniformValue("u_yHeight", (GLfloat)this->vidTextures[vidIx].height);
        // Temp:
        printOpenGLError(__FILE__, __LINE__);
        this->vidTextures[vidIx].shader->setUniformValue("u_yWidth", (GLfloat)this->vidTextures[vidIx].width);

        if(printErrors) printOpenGLError(__FILE__, __LINE__);
        break;

    case VidShaderLitNormalisedTexCoords:
        this->vidTextures[vidIx].shader->setUniformValue("u_vidTexture", 0); // texture unit index
        this->vidTextures[vidIx].shader->setUniformValue("u_yHeight", (GLfloat)this->vidTextures[vidIx].height);
        this->vidTextures[vidIx].shader->setUniformValue("u_yWidth", (GLfloat)this->vidTextures[vidIx].width);

        this->vidTextures[vidIx].shader->setUniformValue("u_lightPosition", QVector3D(0.0, 0.0, 4.0));

        if(printErrors) printOpenGLError(__FILE__, __LINE__);
        break;

    case VidShaderColourHilight:
        this->vidTextures[vidIx].shader->setUniformValue("u_vidTexture", 0); // texture unit index
        this->vidTextures[vidIx].shader->setUniformValue("u_yHeight", (GLfloat)this->vidTextures[vidIx].height);
        this->vidTextures[vidIx].shader->setUniformValue("u_yWidth", (GLfloat)this->vidTextures[vidIx].width);
        this->vidTextures[vidIx].shader->setUniformValue("u_colrToDisplayMin", ColourHilightRangeMin);
        this->vidTextures[vidIx].shader->setUniformValue("u_colrToDisplayMax", ColourHilightRangeMax);
        if(printErrors) printOpenGLError(__FILE__, __LINE__);
        break;

    case VidShaderColourHilightSwap:
        this->vidTextures[vidIx].shader->setUniformValue("u_vidTexture", 0); // texture unit index
        this->vidTextures[vidIx].shader->setUniformValue("u_yHeight", (GLfloat)this->vidTextures[vidIx].height);
        this->vidTextures[vidIx].shader->setUniformValue("u_yWidth", (GLfloat)this->vidTextures[vidIx].width);
        this->vidTextures[vidIx].shader->setUniformValue("u_colrToDisplayMin", ColourHilightRangeMin);
        this->vidTextures[vidIx].shader->setUniformValue("u_colrToDisplayMax", ColourHilightRangeMax);
        this->vidTextures[vidIx].shader->setUniformValue("u_componentSwapR", ColourComponentSwapR);
        this->vidTextures[vidIx].shader->setUniformValue("u_componentSwapG", ColourComponentSwapG);
        this->vidTextures[vidIx].shader->setUniformValue("u_componentSwapB", ColourComponentSwapB);
        if(printErrors) printOpenGLError(__FILE__, __LINE__);
        break;

    case VidShaderAlphaMask:
        this->vidTextures[vidIx].shader->setUniformValue("u_vidTexture", 0); // texture unit index
        this->vidTextures[vidIx].shader->setUniformValue("u_yHeight", (GLfloat)this->vidTextures[vidIx].height);
        this->vidTextures[vidIx].shader->setUniformValue("u_yWidth", (GLfloat)this->vidTextures[vidIx].width);
        this->vidTextures[vidIx].shader->setUniformValue("u_alphaTexture", 1); // texture unit index
        if(printErrors) printOpenGLError(__FILE__, __LINE__);

        this->vidTextures[vidIx].triStripAlphaTexCoords[0] = QVector2D(alphaTexWidth, 0.0f);
        this->vidTextures[vidIx].triStripAlphaTexCoords[1] = QVector2D(0.0f, 0.0f);
        this->vidTextures[vidIx].triStripAlphaTexCoords[2] = QVector2D(alphaTexWidth, alphaTexHeight);
        this->vidTextures[vidIx].triStripAlphaTexCoords[3] = QVector2D(0.0f, alphaTexHeight);
        break;

    default:
        qDebug ("Invalid effect set on vidIx %d", vidIx);
        break;
    }
}

int GLWidget::loadShaderFile(QString fileName, QString &shaderSource)
{
    fileName = dataFilesDir + fileName;

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
#ifdef RECTTEX
        qCritical() << "glError in file " << file << " @ line " << line << ": " << (const char *)gluErrorString(glErr);
#else
        qCritical() << "glError in file " << file << " @ line " << line << ": " << glErr;
#endif
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}
