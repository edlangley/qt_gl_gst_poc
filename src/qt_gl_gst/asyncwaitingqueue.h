#ifndef ASYNCWAITINGQUEUE_H
#define ASYNCWAITINGQUEUE_H

#include <QMutex>
#include <QWaitCondition>
#include <QList>

/* Thread safe queue implementation which can block (with timeout)
   on get until an item arrives in the queue, at which point thread
   is woken up.
*/
template<class T>
class AsyncQueue
{
public:
    AsyncQueue() : m_waitingReaders(0) {}

    int size()
    {
        QMutexLocker locker(&m_mutex);
        return this->m_buffer.size();
    }

    void put(const T& item)
    {
        QMutexLocker locker(&m_mutex);
        this->m_buffer.push_back(item);
        if(this->m_waitingReaders)
            this->m_bufferIsNotEmpty.wakeOne();
    }

    bool get(T *itemDestPtr, unsigned long time_ms = 0)
    {
        QMutexLocker locker(&m_mutex);
        bool itemInQueue = false;

        itemInQueue = (this->m_buffer.size()) ? true : false;
        if(!itemInQueue && time_ms)
        {
            ++(this->m_waitingReaders);
            itemInQueue = this->m_bufferIsNotEmpty.wait(&m_mutex, time_ms);
            --(this->m_waitingReaders);
        }

        if(itemInQueue)
        {
            T item = this->m_buffer.front();
            this->m_buffer.pop_front();
            *itemDestPtr = item;
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    typedef QList<T> Container;
    QMutex m_mutex;
    QWaitCondition m_bufferIsNotEmpty;
    Container m_buffer;
    short m_waitingReaders;
};


#endif // ASYNCWAITINGQUEUE_H
