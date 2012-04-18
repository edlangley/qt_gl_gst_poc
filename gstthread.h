/*
 * GStreamer
 * Copyright (C) 2009 Julien Isorce <julien.isorce@gmail.com>
 * Copyright (C) 2009 Andrey Nechypurenko <andreynech@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GSTTHREAD_H
#define GSTTHREAD_H

#include <QtCore/QThread>
#include "glcontextid.h"

class Pipeline;

class GstThread : public QThread
{
    Q_OBJECT

public:
    GstThread(int vidIx,
            const GLContextID &ctxId,
            const QString &videoLocation,
            const char *renderer_slot,
            QObject *parent = 0);

    ~GstThread();

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
    const GLContextID m_contextId;
    const QString m_videoLocation;
    Pipeline* m_pipeline;
    bool m_chooseNew;
};

#endif
