#include "glwidget.h"
//#include "pipeline.h"

GLWidget::GLWidget(int argc, char *argv[], QWidget *parent) :
    QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba), parent),
    closing(false),
//    frame(NULL),
    brickProg(this)
{
    xRot = 0;
    yRot = 0;
    zRot = 0;

    fScale = 1.0;
    lastPos = QPoint(0, 0);
    gleModel = EModelFirst;
    clearColor = 0;


    Rotate = 1;
    xLastIncr = 0;
    yLastIncr = 0;
    fXInertia = -0.5;
    fYInertia = 0;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
    timer->start(20);

    grabKeyboard();
    // Need this for glut models:
    int zero = 0;
    glutInit(&zero, NULL);

    // Video pipeline
    for(int vidIx = 1; vidIx < argc; vidIx++)
    {
        videoLoc.push_back(QString(argv[vidIx]));
    }

    // Instantiate video pipeline for each filename specified
    for(int vidIx = 0; vidIx < this->videoLoc.size(); vidIx++)
    {
        this->gstThreads.push_back(new GstThread(vidIx, this->videoLoc[vidIx], SLOT(newFrame(int)), this));
        QObject::connect(this->gstThreads[vidIx], SIGNAL(finished(int)),
                         this, SLOT(gstThreadFinished(int)));
        QObject::connect(this, SIGNAL(closeRequested()),
                         this->gstThreads[vidIx], SLOT(stop()), Qt::QueuedConnection);
    }
}

GLWidget::~GLWidget()
{
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}

static int qNormalizeAngle(int angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;

    return angle;
}

void GLWidget::nextClearColor(void)
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

void GLWidget::initializeGL()
{
    // gstreamer shared texture approach:
#if 0
    ctx.contextId = glXGetCurrentContext();
    const char *display_name = getenv("DISPLAY");
    if(display_name == NULL)
    {
        // actually we should look for --display command line parameter here
        display_name = ":0.0";
    }
    ctx.display = XOpenDisplay(display_name);
    ctx.wnd = this->winId();

    // We need to unset Qt context before initializing gst-gl plugin.
    // Otherwise the attempt to share gst-gl context with Qt will fail.
    //this->doneCurrent();

    for(int vidIx = 0; vidIx < this->videoLoc.size(); vidIx++)
    {
        this->doneCurrent();
        this->gstThreads.push_back(new GstThread(vidIx, ctx, this->videoLoc[vidIx], SLOT(newFrame(int)), this));
        this->makeCurrent();
        QObject::connect(this->gstThreads[vidIx], SIGNAL(finished(int)),
                         this, SLOT(gstThreadFinished(int)));
        QObject::connect(this, SIGNAL(closeRequested()),
                         this->gstThreads[vidIx], SLOT(stop()), Qt::QueuedConnection);
    }
#endif



    // rest of normal gl init:

    QString verStr((const char*)glGetString(GL_VERSION));
    QStringList verNums = verStr.split(".");
    std::cout << "GL_VERSION major=" << verNums[0].toStdString() << " minor=" << verNums[1].toStdString() << "\n";

    if (verNums[0].toInt() < 2)
    {
     printf("Support for OpenGL 2.0 is required for this demo...exiting\n");
     exit(1);
    }

    qglClearColor(QColor(Qt::black));

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
#if 0
// no shader:
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);
    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
#else
// shader:
    if(readShaderSource("brick") != 0)
    {
        qFatal("Error loading shader sources");
        QApplication::exit( 1);
    }

    installShaders();

#endif

    // Create entry in tex info vector for all pipelines
    for(int vidIx = 0; vidIx < this->gstThreads.size(); vidIx++)
    {
        VidTextureInfo newInfo;
        glGenTextures(1, &newInfo.texId);
        newInfo.texInfoValid = false;
        newInfo.buffer = NULL;

        this->vidTextures.push_back(newInfo);
    }

    //this->gst_thread->start();
    for(int vidIx = 0; vidIx < this->gstThreads.size(); vidIx++)
    {
        this->gstThreads[vidIx]->start();
    }
}

void GLWidget::paintGL()
{
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -5.0);

    glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);

    glScalef(fScale, fScale, fScale);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    brickProg.bind();

    switch( gleModel )
    {
        case EModelTeapot:
            glutSolidTeapot(0.6f);
            break;
        case EModelTorus:
            glutSolidTorus(0.2f, 0.6f, 64, 64);
            break;
        case EModelSphere:
            glutSolidSphere(0.6f, 64, 64);
            break;
        case EModelCube:
            drawCube();
            break;
        default:
            drawCube();
            break;
    }

    brickProg.release();

    for(int vidIx = 0; vidIx < this->vidTextures.size(); vidIx++)
    {
        if(this->vidTextures[vidIx].texInfoValid)
        {

            // render a quad with the video on it:
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, this->vidTextures[vidIx].texId);
            if(glGetError () != GL_NO_ERROR)
            {
              qDebug ("failed to bind texture that comes from gst-gl");
              emit closeRequested();
              return;
            }

            GLfloat width = this->vidTextures[vidIx].width;
            GLfloat height = this->vidTextures[vidIx].height;

            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S,
                          GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T,
                          GL_CLAMP_TO_EDGE);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glPushMatrix();
            glRotatef((360/this->vidTextures.size())*vidIx, 0.0, 1.0, 0.0);

            glBegin(GL_QUADS);
                glTexCoord2f(width, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
                glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
                glTexCoord2f(0.0f, height); glVertex3f( 1.0f,  1.0f,  1.0f);
                glTexCoord2f(width, height); glVertex3f(-1.0f,  1.0f,  1.0f);
            glEnd();
            glPopMatrix();
        }

    }
}

void GLWidget::resizeGL(int wid, int ht)
{
    float vp = 0.8f;
    float aspect = (float) wid / (float) ht;

    glViewport(0, 0, wid, ht);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, wid, ht);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0);
    glFrustum(-vp, vp, -vp / aspect, vp / aspect, 3, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -5.0);
}

void GLWidget::newFrame(int vidIx)
{
    // use sender() to get correct thread instance
//    GstThread *callingGstThread = qobject_cast<GstThread*>(sender());
//    Pipeline *pipeline = callingGstThread->getPipeline();

    if(this->gstThreads[vidIx])
    {

        Pipeline *pipeline = this->gstThreads[vidIx]->getPipeline();
        if(!pipeline)
          return;

        /* frame is initialized as null */
        //if (this->frame)
        if (this->vidTextures[vidIx].buffer)
            pipeline->queue_output_buf.put(this->vidTextures[vidIx].buffer);

        this->vidTextures[vidIx].buffer = pipeline->queue_input_buf.get();

        // load the gst buf into a texture
        if(this->vidTextures[vidIx].texInfoValid == false)
        {
            // try and keep this fairly portable to other media frameworks by
            // leaving info extraction within pipeline class
            this->vidTextures[vidIx].width = pipeline->getWidth();
            this->vidTextures[vidIx].height = pipeline->getHeight();
            this->vidTextures[vidIx].colourFormat = pipeline->getColourFormat();
            this->vidTextures[vidIx].texInfoValid = true;
        }

        // TODO: move gst macro into pipeline class, have queue contain just pointer
        // to actual frame data
        glBindTexture (GL_TEXTURE_RECTANGLE_ARB, this->vidTextures[vidIx].texId);
        glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D  (GL_TEXTURE_RECTANGLE_ARB, 0, 1,
                       this->vidTextures[vidIx].width,
                       this->vidTextures[vidIx].height,
                       0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                       GST_BUFFER_DATA(this->vidTextures[vidIx].buffer));

        /* direct call to paintGL (no queued) */
        this->updateGL();
    }
}

void GLWidget::gstThreadFinished(int vidIx)
{
/*
    GstThread *callingGstThread = qobject_cast<GstThread*>(sender());
    bool threadIxFound = false;
    int vidIx;

    for(vidIx = 0; vidIx < this->gstThreads.size(); vidIx++)
    {
        if(callingGstThread == this->gstThreads[vidIx])
        {
            threadIxFound = true;
            break;
        }
    }
*/


    if(this->closing)
    {
        delete(this->gstThreads[vidIx]);
        this->gstThreads.replace(vidIx, NULL);
        this->vidTextures[vidIx].texInfoValid = false;

        // check if any gst threads left, if not close
        bool allFinished = true;
        for(int i = 0; i < this->gstThreads.size(); i++)
        {
            if(this->gstThreads[i] != NULL)
            {
                // catch any threads which were already finished at quitting time
                if(this->gstThreads[i]->isFinished())
                {
                    delete(this->gstThreads[vidIx]);
                    this->gstThreads.replace(vidIx, NULL);
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
    else if(this->gstThreads[vidIx]->chooseNew())
    {
        delete(this->gstThreads[vidIx]);
        this->vidTextures[vidIx].texInfoValid = false;

        this->videoLoc[vidIx] = QFileDialog::getOpenFileName(0, "Select a video file",
                                                ".", "Format (*.avi *.mkv *.ogg *.asf *.mov)");
        this->makeCurrent();
        this->gstThreads[vidIx] =
          new GstThread(vidIx, /*ctx,*/ this->videoLoc[vidIx], SLOT(newFrame()), this);
        this->makeCurrent();
        this->gstThreads[vidIx]->start();
    }
    else
    {
        // for now should just stop at last frame?
    }
}

void GLWidget::animate()
{
    /* Increment wrt inertia */
    if (Rotate)
    {
        xRot = qNormalizeAngle(xRot + (8 * fYInertia));
        yRot = qNormalizeAngle(yRot + (8 * fXInertia));
    }

    updateGL();
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
               updateGL();
           }
        }
        else
        {
           if (lastPos.x() != -1)
           {
               xRot = qNormalizeAngle(xRot + (8 * yLastIncr));
               yRot = qNormalizeAngle(yRot + (8 * xLastIncr));
               updateGL();
           }
        }
    }

    lastPos = event->pos();
}

void GLWidget::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
        case Qt::Key_B:
            nextClearColor();
            break;
        case Qt::Key_T:
            if (gleModel >= EModelLast)
                gleModel = EModelFirst;
            else
                gleModel = (EModelType) ((int) gleModel + 1);
            break;
        case Qt::Key_Space:
            Rotate = !Rotate;

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
            break;
        case Qt::Key_Plus:
            fScale += SCALE_INCREMENT;
            break;
        case Qt::Key_Minus:
            fScale -= SCALE_INCREMENT;
            break;
        case Qt::Key_Question:
        case Qt::Key_H:
            std::cout <<  "\nKeyboard commands:\n\n"
            "b - Toggle among background clear colors\n"
            "q, <esc> - Quit\n"
            "t - Toggle among models to render\n"
            "? - Help\n"
            "<home>     - reset zoom and rotation\n"
            "<space> or <click>        - stop rotation\n"
            "<+>, <-> or <ctrl + drag> - zoom model\n"
            "<arrow keys> or <drag>    - rotate model\n\n";
            break;
        case Qt::Key_Home:
            xRot = 0;
            yRot = 35;
            zRot = 0;
            xLastIncr = 0;
            yLastIncr = 0;
            fXInertia = -0.5;
            fYInertia = 0;
            fScale    = 1.0;
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

        case Qt::Key_V:
            this->gstThreads[0]->stop();
            this->gstThreads[0]->setChooseNewOnFinished();

        break;

        case Qt::Key_Escape:
        case Qt::Key_Q:
            close();
            break;

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

        // just in case, check now if any gst threads still exist, if not, close now
        bool allFinished = true;
        for(int i = 0; i < this->gstThreads.size(); i++)
        {
            if(this->gstThreads[i] != NULL)
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
}

void GLWidget::drawCube(void)
{
    float size = 1.0f;
    float scale = 0.2f;
    float delta = 0.1f;

    float A[3] = { size,  size,  size * scale + delta };
    float B[3] = { size,  size, -size * scale + delta };
    float C[3] = { size, -size, -size * scale };
    float D[3] = { size, -size,  size * scale };
    float E[3] = {-size,  size,  size * scale + delta };
    float F[3] = {-size,  size, -size * scale + delta };
    float G[3] = {-size, -size, -size * scale };
    float H[3] = {-size, -size,  size * scale };

    float I[3] = { 1.0f,  0.0f,  0.0f};
    float K[3] = {-1.0f,  0.0f,  0.0f};
    float L[3] = { 0.0f,  0.0f, -1.0f};
    float M[3] = { 0.0f,  0.0f,  1.0f};
    float N[3] = { 0.0f,  1.0f,  0.0f};
    float O[3] = { 0.0f, -1.0f,  0.0f};


    glBegin(GL_QUADS);

    glNormal3fv(I);

    glTexCoord2f(1,1);
    glVertex3fv(D);
    glTexCoord2f(0,1);
    glVertex3fv(C);
    glTexCoord2f(0,0);
    glVertex3fv(B);
    glTexCoord2f(1,0);
    glVertex3fv(A);

    glNormal3fv(K);

    glTexCoord2f(1,1);
    glVertex3fv(G);
    glTexCoord2f(0,1);
    glVertex3fv(H);
    glTexCoord2f(0,0);
    glVertex3fv(E);
    glTexCoord2f(1,0);
    glVertex3fv(F);

    glNormal3fv(L);

    glTexCoord2f(1,1);
    glVertex3fv(C);
    glTexCoord2f(0,1);
    glVertex3fv(G);
    glTexCoord2f(0,0);
    glVertex3fv(F);
    glTexCoord2f(1,0);
    glVertex3fv(B);

    glNormal3fv(M);

    glTexCoord2f(1,1);
    glVertex3fv(H);
    glTexCoord2f(0,1);
    glVertex3fv(D);
    glTexCoord2f(0,0);
    glVertex3fv(A);
    glTexCoord2f(1,0);
    glVertex3fv(E);

    glNormal3fv(N);

    glTexCoord2f(1,1);
    glVertex3fv(E);
    glTexCoord2f(0,1);
    glVertex3fv(A);
    glTexCoord2f(0,0);
    glVertex3fv(B);
    glTexCoord2f(1,0);
    glVertex3fv(F);

    glNormal3fv(O);

    glTexCoord2f(1,1);
    glVertex3fv(G);
    glTexCoord2f(0,1);
    glVertex3fv(C);
    glTexCoord2f(0,0);
    glVertex3fv(D);
    glTexCoord2f(1,0);
    glVertex3fv(H);

    glEnd();
}


int GLWidget::readShaderSource(QString baseFileName)
{
    int ret;

    QString vertFileName = baseFileName + ".vert";
    ret = readShaderFile(vertFileName, vertexShaderSource);
    if(ret != 0)
    {
        return ret;
    }

    QString fragFileName = baseFileName + ".frag";
    ret = readShaderFile(fragFileName, fragmentShaderSource);

    return ret;
}

int GLWidget::readShaderFile(QString fileName, QString &shaderSource)
{
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

int GLWidget::installShaders()
{
    bool ret;
    //QGLShaderProgram brickProg(this);

    ret = brickProg.addShaderFromSourceCode(QGLShader::Vertex,
                                          vertexShaderSource);
    printOpenGLError(__FILE__, __LINE__);
    if(ret == false)
    {
        qCritical() << "vertex shader log: " << brickProg.log();
        return -1;
    }

    ret = brickProg.addShaderFromSourceCode(QGLShader::Fragment,
                                          fragmentShaderSource);
    printOpenGLError(__FILE__, __LINE__);
    if(ret == false)
    {
        qCritical() << "fragment shader log: " << brickProg.log();
        return -1;
    }

    ret = brickProg.link();
    printOpenGLError(__FILE__, __LINE__);
    if(ret == false)
    {
        qCritical() << "shader program link log: " << brickProg.log();
        return -1;
    }

    ret = brickProg.bind();
    printOpenGLError(__FILE__, __LINE__);
    if(ret == false)
    {
        return -1;
    }

    // Set up initial uniform values

    brickProg.setUniformValue("BrickColor", QVector3D(1.0, 0.3, 0.2));
    brickProg.setUniformValue("MortarColor", QVector3D(0.85, 0.86, 0.84));
    brickProg.setUniformValue("BrickSize", QVector3D(0.30, 0.15, 0.30));
    brickProg.setUniformValue("BrickPct", QVector3D(0.90, 0.85, 0.90));
    brickProg.setUniformValue("LightPosition", QVector3D(0.0, 0.0, 4.0));

    brickProg.release();

    return 0;
}

int GLWidget::printOpenGLError(char *file, int line)
{
    //
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    //
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        qCritical() << "glError in file " << file << " @ line " << line << ": " << (const char *)gluErrorString(glErr);
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}
