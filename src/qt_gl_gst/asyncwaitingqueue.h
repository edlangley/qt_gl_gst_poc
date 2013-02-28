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
    AsyncQueue() : waitingReaders(0) {}

    int size()
    {
        QMutexLocker locker(&mutex);
        return this->buffer.size();
    }

    void put(const T& item)
    {
        QMutexLocker locker(&mutex);
        this->buffer.push_back(item);
        if(this->waitingReaders)
            this->bufferIsNotEmpty.wakeOne();
    }

    bool get(T *itemDestPtr, unsigned long time_ms = 0)
    {
        QMutexLocker locker(&mutex);
        bool itemInQueue = false;

        itemInQueue = (this->buffer.size()) ? true : false;
        if(!itemInQueue && time_ms)
        {
            ++(this->waitingReaders);
            itemInQueue = this->bufferIsNotEmpty.wait(&mutex, time_ms);
            --(this->waitingReaders);
        }

        if(itemInQueue)
        {
            T item = this->buffer.front();
            this->buffer.pop_front();
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
    QMutex mutex;
    QWaitCondition bufferIsNotEmpty;
    Container buffer;
    short waitingReaders;
};


#endif // ASYNCWAITINGQUEUE_H
