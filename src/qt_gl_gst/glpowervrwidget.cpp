
#if 1 // temp whilst reviving my build env

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "glpowervrwidget.h"
#include "cmem.h"

GLPowerVRWidget::GLPowerVRWidget(int argc, char *argv[], QWidget *parent) :
    GLWidget(argc, argv, parent)
{

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
        qCritical("Can't get GLES 2.0 extensions");
        close();
    }

    if (!strstr((char *)glExtCStr, "GL_IMG_texture_stream2"))
    {
        qCritical("GL_IMG_texture_stream2 extension not present");
        close();
    }

    glTexBindStreamIMG = (PFNGLTEXBINDSTREAMIMGPROC)this->context()->getProcAddress("glTexBindStreamIMG");
    glGetTexAttrIMG = (PFNGLGETTEXSTREAMDEVICEATTRIBUTEIVIMGPROC)this->context()->getProcAddress("glGetTexStreamDeviceAttributeivIMG");
    glGetTexDeviceIMG = (PFNGLGETTEXSTREAMDEVICENAMEIMGPROC)this->context()->getProcAddress("glGetTexStreamDeviceNameIMG");

    if (!glTexBindStreamIMG || !glGetTexAttrIMG || !glGetTexDeviceIMG)
    {
        qCritical("Couldn't get pointers to IMG extension functions");
        close();
    }
}

Pipeline *GLPowerVRWidget::createPipeline(int vidIx)
{
    if(vidIx > MAX_BCDEV)
    {
        qCritical("ERROR: vidIx=%d which is greater than bccat devs available", vidIx);
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
            qCritical("ERROR: open %s failed", qPrintable(bcDevName));
            return NULL;
        }

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

    if(vidIx >= m_currentTexIx.size())
    {
        m_currentTexIx.resize(vidIx+1);
    }
    m_currentTexIx.replace(vidIx, 0);


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

void GLPowerVRWidget::loadNewTexture(int vidIx)
{
    unsigned long currentVidBufferAddress = (unsigned long)CMEM_getPhys(this->m_vidPipelines[vidIx]->bufToVidDataStart(this->m_vidTextures[vidIx].buffer));

    if(!m_vidBufferAddressesSet[vidIx])
    {
        QVector<bc_buf_ptr_t>::iterator bufPtr;
        for(bufPtr = m_vidBufferAddresses[vidIx].begin(); bufPtr != m_vidBufferAddresses[vidIx].end(); ++bufPtr)
        {
            if(bufPtr->pa == currentVidBufferAddress)
            {
                // Already recorded this buffer address
                return;
            }
        }

        // A new buffer has come through, record the details:
        bc_buf_ptr_t bc_buf;
        bc_buf.index = totalVidBuffers();
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
        m_vidBufferAddresses[vidIx].push_back(bc_buf);

        // Have we got all the buffer addresses we are waiting for?
        if(m_vidBufferAddresses[vidIx].size() >= NUM_OUTPUT_BUFS_PER_VID)
        {
            // We now definitely have the size information needed to prep the driver:
            bc_buf_params_t bufParams;
            bufParams.count = NUM_OUTPUT_BUFS_PER_VID;
            switch(this->m_vidTextures[vidIx].colourFormat)
            {
            case ColFmt_I420:
                bufParams.width = this->m_vidTextures[vidIx].width;
                bufParams.height = this->m_vidTextures[vidIx].height*1.5f;
                break;
            case ColFmt_UYVY:
            default:
                bufParams.width = this->m_vidTextures[vidIx].width*2;
                bufParams.height = this->m_vidTextures[vidIx].height;
                break;
            }
            bufParams.fourcc = this->m_vidPipelines[vidIx]->getFourCC();
            bufParams.type = BC_MEMORY_USERPTR;

            if (ioctl(m_bcFds[vidIx], BCIOREQ_BUFFERS, &bufParams) != 0)
            {
                qCritical("ERROR: BCIOREQ_BUFFERS failed");
                return;
            }

            BCIO_package ioctlVar;
            if (ioctl(m_bcFds[vidIx], BCIOGET_BUFFERCOUNT, &ioctlVar) != 0)
            {
                qCritical("ERROR: BCIOGET_BUFFERCOUNT failed");
                return;
            }

            if (ioctlVar.output == 0)
            {
                qCritical("ERROR: no texture buffers available");
                return;
            }


            for(bufPtr = m_vidBufferAddresses[vidIx].begin(); bufPtr != m_vidBufferAddresses[vidIx].end(); ++bufPtr)
            {
                if (ioctl(m_bcFds[vidIx], BCIOSET_BUFFERPHYADDR, bufPtr) != 0)
                {
                    qCritical("ERROR: BCIOSET_BUFFERADDR[%d]: failed (0x%lx)\n",
                           bufPtr->index, bufPtr->pa);
                }
            }

            // Should be able to set up the GLES side now:
            const GLubyte *imgDevName;
            imgDevName = this->glGetTexDeviceIMG(vidIx);

            GLint numImgBufs, imgBufWidth, imgBufHeight, imgBufFmt;
            this->glGetTexAttrIMG(vidIx, GL_TEXTURE_STREAM_DEVICE_NUM_BUFFERS_IMG, &numImgBufs);
            this->glGetTexAttrIMG(vidIx, GL_TEXTURE_STREAM_DEVICE_WIDTH_IMG, &imgBufWidth);
            this->glGetTexAttrIMG(vidIx, GL_TEXTURE_STREAM_DEVICE_HEIGHT_IMG, &imgBufHeight);
            this->glGetTexAttrIMG(vidIx, GL_TEXTURE_STREAM_DEVICE_FORMAT_IMG, &imgBufFmt);

            qDebug("GLES IMG attrs: dev name: %d, numbufs=%d, width=%d, height=%d, format=%d",
                   imgDevName, numImgBufs, imgBufWidth, imgBufHeight, imgBufFmt);


            /* do stuff from setup_shaders() in bc_cat example common.c code here */
            glBindTexture(GL_TEXTURE_STREAM_IMG, this->m_vidTextures[vidIx].texId);

            /* specify filters */
            glTexParameterf(GL_TEXTURE_STREAM_IMG, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_STREAM_IMG, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // cmem examples use:
            //glTexParameterf(GL_TEXTURE_STREAM_IMG, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            //glTexParameterf(GL_TEXTURE_STREAM_IMG, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            m_vidBufferAddressesSet[vidIx] = true;
        }

    }
    else
    {
        QVector<bc_buf_ptr_t>::iterator bufPtr;
        for(bufPtr = m_vidBufferAddresses[vidIx].begin(); bufPtr != m_vidBufferAddresses[vidIx].end(); ++bufPtr)
        {
            if(bufPtr->pa == currentVidBufferAddress)
            {
                m_currentTexIx[vidIx] = bufPtr->index;
                break;
            }
        }


        glBindTexture(GL_TEXTURE_STREAM_IMG, this->m_vidTextures[vidIx].texId);
        glTexBindStreamIMG(GL_TEXTURE_STREAM_IMG, bufPtr->index);
    }

}

#endif
