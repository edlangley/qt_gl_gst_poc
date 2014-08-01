#ifndef GLPOWERVRWIDGET_H
#define GLPOWERVRWIDGET_H

#include "glwidget.h"
#include "bc_cat.h"

#define MAX_BCDEV                9

class GLPowerVRWidget : public GLWidget
{
    Q_OBJECT
public:
    explicit GLPowerVRWidget(int argc, char *argv[], QWidget *parent = 0);
    ~GLPowerVRWidget();

protected:
    void initializeGL();
    virtual Pipeline* createPipeline(int vidIx);
    bool loadNewTexture(int vidIx);

signals:
    
public slots:
    
private:
    int totalVidBuffers();

    PFNGLTEXBINDSTREAMIMGPROC glTexBindStreamIMG;
    PFNGLGETTEXSTREAMDEVICEATTRIBUTEIVIMGPROC glGetTexAttrIMG;
    PFNGLGETTEXSTREAMDEVICENAMEIMGPROC glGetTexDeviceIMG;

    QVector< QVector<bc_buf_ptr_t> > m_vidBufferAddresses;
    QVector<bool> m_vidBufferAddressesSet;
    QVector<int> m_bcFds;
    QMap<unsigned int, GLuint> m_bcBufIxToTexId;

};

#endif // GLPOWERVRWIDGET_H
