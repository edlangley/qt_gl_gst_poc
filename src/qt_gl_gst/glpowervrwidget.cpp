
#if 1 // temp whilst reviving my build env

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "applogger.h"
#include "glpowervrwidget.h"
#include "cmem.h"

#define COLFMT_TO_BC_FOURCC(fourCC) fourCC

GLPowerVRWidget::GLPowerVRWidget(int argc, char *argv[], QWidget *parent) :
    GLWidget(argc, argv, parent)
{
    if(CMEM_init() == -1)
    {
        LOG(LOG_GL, Logger::Error, "Error calling CMEM_init");
        close();
    }
}

GLPowerVRWidget::~GLPowerVRWidget()
{
}

void GLPowerVRWidget::initializeGL()
{
    GLWidget::initializeGL();

    const GLubyte *glExtCStr;

    if (!(glExtCStr = glGetString(GL_EXTENSIONS)))
    {
        LOG(LOG_GL, Logger::Error, "Can't get GLES 2.0 extensions");
        close();
    }

    if (!strstr((char *)glExtCStr, "GL_IMG_texture_stream2"))
    {
        LOG(LOG_GL, Logger::Error, "GL_IMG_texture_stream2 extension not present");
        close();
    }

    glTexBindStreamIMG = (PFNGLTEXBINDSTREAMIMGPROC)this->context()->getProcAddress("glTexBindStreamIMG");
    glGetTexAttrIMG = (PFNGLGETTEXSTREAMDEVICEATTRIBUTEIVIMGPROC)this->context()->getProcAddress("glGetTexStreamDeviceAttributeivIMG");
    glGetTexDeviceIMG = (PFNGLGETTEXSTREAMDEVICENAMEIMGPROC)this->context()->getProcAddress("glGetTexStreamDeviceNameIMG");

    if (!glTexBindStreamIMG || !glGetTexAttrIMG || !glGetTexDeviceIMG)
    {
        LOG(LOG_GL, Logger::Error, "Couldn't get pointers to IMG extension functions");
        close();
    }
}

Pipeline *GLPowerVRWidget::createPipeline(int vidIx)
{
    if(vidIx > MAX_BCDEV)
    {
        LOG(LOG_GL, Logger::Error, "ERROR: vidIx=%d which is greater than bccat devs available", vidIx);
        return NULL;
    }

    // Only need to init Bufferclass dev if haven't already
    if(vidIx >= m_bcFds.size())
    {
        m_bcFds.resize(vidIx+1);

        int bcFd;
        QString bcDevName = QString("/dev/bccat%1").arg(vidIx);
        if((bcFd = open(bcDevName.toUtf8().constData(), O_RDWR|O_NDELAY)) == -1)
        {
            LOG(LOG_GL, Logger::Error, "ERROR: open %s failed", bcDevName.toUtf8().constData());
            return NULL;
        }
        LOG(LOG_GL, Logger::Debug1, "opened %s fd=%d", bcDevName.toUtf8().constData(), bcFd);

        m_bcFds.replace(vidIx, bcFd);
    }

    // Size all the other containers for buffer handling appropriately
    if(vidIx >= m_vidBufferAddressesSet.size())
    {
        m_vidBufferAddressesSet.resize(vidIx+1);
    }
    m_vidBufferAddressesSet.replace(vidIx, false);

    if(vidIx >= m_vidBufferAddresses.size())
    {
        m_vidBufferAddresses.resize(vidIx+1);
    }
    m_vidBufferAddresses.replace(vidIx, QVector<bc_buf_ptr_t>());

    return new TIGStreamerPipeline(vidIx, this->m_videoLoc[vidIx], SLOT(newFrame(int)), this);
}

int GLPowerVRWidget::totalVidBuffers()
{
    int total = 0;

    QVector< QVector<bc_buf_ptr_t> >::iterator bufVecPtr;
    for(bufVecPtr = m_vidBufferAddresses.begin(); bufVecPtr != m_vidBufferAddresses.end(); ++bufVecPtr)
    {
        total += bufVecPtr->size();
    }

    return total;
}

bool GLPowerVRWidget::loadNewTexture(int vidIx)
{
    bool texLoaded = false;

    unsigned long currentVidBufferAddress = (unsigned long)CMEM_getPhys(this->m_vidPipelines[vidIx]->bufToVidDataStart(this->m_vidTextures[vidIx].buffer));

    LOG(LOG_GL, Logger::Debug2, "vid %d, CMEM phys=%lx", vidIx, currentVidBufferAddress);

    if(m_vidBufferAddressesSet.contains(false))
    {
        bool gotThisBuffer = false;
        QVector<bc_buf_ptr_t>::iterator bufPtr;
        for(bufPtr = m_vidBufferAddresses[vidIx].begin(); bufPtr != m_vidBufferAddresses[vidIx].end(); ++bufPtr)
        {
            if(bufPtr->pa == currentVidBufferAddress)
            {
                // Already recorded this buffer address
                LOG(LOG_GL, Logger::Debug2, "vid %d, already saved phys addr %lx", vidIx, currentVidBufferAddress);
                gotThisBuffer = true;

                // If we've got the same buffer a second time,
                // assume that means we have all the buffer addresses
                m_vidBufferAddressesSet[vidIx] = true;
            }
        }

        if(!gotThisBuffer)
        {
            // A new buffer has come through, record the details:
            bc_buf_ptr_t bc_buf;
            bc_buf.index = totalVidBuffers(); // Multiple buffers per possibly multiple videos
            // Size parameter isn't actually used in the driver just yet but fill in anyway for
            // future proofing:
            switch(this->m_vidTextures[vidIx].colourFormat)
            {
            case ColFmt_I420:
                bc_buf.size = this->m_vidTextures[vidIx].width * this->m_vidTextures[vidIx].height * 1.5f;
                break;
            case ColFmt_UYVY:
            default:
                bc_buf.size = this->m_vidTextures[vidIx].width * this->m_vidTextures[vidIx].height * 2;
                break;
            }
            bc_buf.pa = currentVidBufferAddress;
            LOG(LOG_GL, Logger::Debug1, "vid %d, saving bc_buf_ptr_t: index=%d, size=%d, pa=%lx",
                vidIx, bc_buf.index, bc_buf.size, bc_buf.pa);
            m_vidBufferAddresses[vidIx].push_back(bc_buf);
        }

        // Have we got all the buffer addresses we are waiting for, for all videos?
        if(!m_vidBufferAddressesSet.contains(false))
        {
            LOG(LOG_GL, Logger::Debug1, "got all the bc_buf_ptr_t entries for all vids");

            for(int currentVidIx = 0; currentVidIx < m_vidPipelines.size(); currentVidIx++)
            {
                // We now definitely have the size information needed to prep the driver:
                bc_buf_params_t bufParams;
                bufParams.count = m_vidBufferAddresses[currentVidIx].size();
                bufParams.width = this->m_vidTextures[currentVidIx].width;
                bufParams.height = this->m_vidTextures[currentVidIx].height;

                bufParams.fourcc = COLFMT_TO_BC_FOURCC(this->m_vidPipelines[currentVidIx]->getColourFormat());
                bufParams.type = BC_MEMORY_USERPTR;

                LOG(LOG_GL, Logger::Debug1, "vid %d, calling BCIOREQ_BUFFERS, count=%d, fourCC=0x%08X, width=%d, height=%d, type=%d",
                    currentVidIx, bufParams.count, bufParams.fourcc, bufParams.width, bufParams.height, bufParams.type);
                int retVal;
                if((retVal = ioctl(m_bcFds[currentVidIx], BCIOREQ_BUFFERS, &bufParams)) != 0)
                {
                    LOG(LOG_GL, Logger::Error, "ERROR: BCIOREQ_BUFFERS on fd %d failed, retVal=%d, errno=%d:%s",
                        m_bcFds[currentVidIx], retVal, errno, strerror(errno));
                    return false;
                }

                LOG(LOG_GL, Logger::Debug1, "vid %d, calling BCIOGET_BUFFERCOUNT", currentVidIx);
                BCIO_package ioctlVar;
                if (ioctl(m_bcFds[currentVidIx], BCIOGET_BUFFERCOUNT, &ioctlVar) != 0)
                {
                    LOG(LOG_GL, Logger::Error, "ERROR: BCIOGET_BUFFERCOUNT failed");
                    return false;
                }

                if (ioctlVar.output == 0)
                {
                    LOG(LOG_GL, Logger::Error, "ERROR: no texture buffers available");
                    return false;
                }

                for(bufPtr = m_vidBufferAddresses[currentVidIx].begin(); bufPtr != m_vidBufferAddresses[currentVidIx].end(); ++bufPtr)
                {
                    if (ioctl(m_bcFds[currentVidIx], BCIOSET_BUFFERPHYADDR, bufPtr) != 0)
                    {
                        LOG(LOG_GL, Logger::Error, "ERROR: BCIOSET_BUFFERADDR[%d]: failed (0x%lx)",
                            bufPtr->index, bufPtr->pa);
                    }
                }
            }

            printOpenGLError(__FILE__, __LINE__);

            // Should be able to set up the GLES side now:
            const GLubyte *imgDevName;
            imgDevName = this->glGetTexDeviceIMG(vidIx);

            printOpenGLError(__FILE__, __LINE__);

            GLint numImgBufs, imgBufWidth, imgBufHeight, imgBufFmt;
            this->glGetTexAttrIMG(vidIx, GL_TEXTURE_STREAM_DEVICE_NUM_BUFFERS_IMG, &numImgBufs);
            this->glGetTexAttrIMG(vidIx, GL_TEXTURE_STREAM_DEVICE_WIDTH_IMG, &imgBufWidth);
            this->glGetTexAttrIMG(vidIx, GL_TEXTURE_STREAM_DEVICE_HEIGHT_IMG, &imgBufHeight);
            this->glGetTexAttrIMG(vidIx, GL_TEXTURE_STREAM_DEVICE_FORMAT_IMG, &imgBufFmt);

            LOG(LOG_GL, Logger::Debug1, "GLES IMG attrs: dev name: %s, numbufs=%d, width=%d, height=%d, format=%d",
                   imgDevName, numImgBufs, imgBufWidth, imgBufHeight, imgBufFmt);

            printOpenGLError(__FILE__, __LINE__);

            /* do stuff from setup_shaders() in bc_cat example common.c code here */
            glActiveTexture(GL_TEXTURE0);


            for(int currentVidIx = 0; currentVidIx < m_vidPipelines.size(); currentVidIx++)
            {
                // Delete the single texture ID created in superclass,
                // ready to create the pool of texture IDs for each video instead
                glDeleteTextures (1, &m_vidTextures[currentVidIx].texId);

                // Loop through all the buffers, link buf index to tex IDs:
                QVector<bc_buf_ptr_t>::iterator bufPtr;
                for(bufPtr = m_vidBufferAddresses[currentVidIx].begin(); bufPtr != m_vidBufferAddresses[currentVidIx].end(); ++bufPtr)
                {
                    GLuint newTexId;
                    glGenTextures(1, &newTexId);

                    LOG(LOG_GL, Logger::Debug1, "calling glBindTexture texId=%d",
                        newTexId);
                    glBindTexture(GL_TEXTURE_STREAM_IMG, newTexId);
                    printOpenGLError(__FILE__, __LINE__);

                    LOG(LOG_GL, Logger::Debug1, "setting texture filters");
                    // specify filters
                    //glTexParameterf(GL_TEXTURE_STREAM_IMG, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    //glTexParameterf(GL_TEXTURE_STREAM_IMG, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    // cmem examples use:
                    glTexParameterf(GL_TEXTURE_STREAM_IMG, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameterf(GL_TEXTURE_STREAM_IMG, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    printOpenGLError(__FILE__, __LINE__);

                    // assign the buffer
                    LOG(LOG_GL, Logger::Debug1, "calling glTexBindStreamIMG buf index=%d", bufPtr->index);
                    glTexBindStreamIMG(vidIx, bufPtr->index);
                    printOpenGLError(__FILE__, __LINE__);

                    m_bcBufIxToTexId[bufPtr->index] = newTexId;

                    // When the loop exits, take last buffer captured for vid as active texture
                    this->m_vidTextures[currentVidIx].texId = newTexId;
                }
            }
            texLoaded = true;
        }

    }
    else
    {
        LOG(LOG_GL, Logger::Debug2, "vid %d, looking up bc_buf_ptr_t index for buf pa=%lx",
            vidIx, currentVidBufferAddress);

        bool bufAdrFound = false;
        QVector<bc_buf_ptr_t>::iterator bufPtr;
        for(bufPtr = m_vidBufferAddresses[vidIx].begin(); bufPtr != m_vidBufferAddresses[vidIx].end(); ++bufPtr)
        {
            if(bufPtr->pa == currentVidBufferAddress)
            {
                LOG(LOG_GL, Logger::Debug2, "vid %d, setting texture to bc_buf_ptr_t index %d, texId %d",
                    vidIx, bufPtr->index, m_bcBufIxToTexId[bufPtr->index]);
                this->m_vidTextures[vidIx].texId = m_bcBufIxToTexId[bufPtr->index];
                bufAdrFound = true;
                break;
            }
        }

        if(bufAdrFound)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_STREAM_IMG, this->m_vidTextures[vidIx].texId);

            texLoaded = true;
        }
        else
        {
            LOG(LOG_GL, Logger::Error, "new vid buffer arrived after all expected buffers have been setup, pa=%lx",
                currentVidBufferAddress);
        }
    }

    return texLoaded;
}

#endif
