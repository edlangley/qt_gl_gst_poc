
#ifndef VIDTHREAD_H
#define VIDTHREAD_H

#include <QtCore/QThread>

class Pipeline;

class VidThread : public QThread
{
    Q_OBJECT

public:
    VidThread(int vidIx,
            const QString &videoLocation,
            const char *renderer_slot,
            QObject *parent = 0);

    ~VidThread();

    Pipeline *getPipeline() { return this->m_pipeline; }
    int getVidIx() { return m_vidIx; }
    void setChooseNewOnFinished() {this->m_chooseNew = true; }
    bool chooseNew() { return this->m_chooseNew; }

signals:
    void finished(int vidIx);

public Q_SLOTS:
    void stop();
    void reemitFinished() { emit finished(m_vidIx); }

protected:
    void run();

private:
    int m_vidIx;
    const QString m_videoLocation;
    Pipeline* m_pipeline;
    bool m_chooseNew;
};

#endif
