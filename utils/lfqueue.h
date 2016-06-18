#ifndef LFQUEUE_H
#define LFQUEUE_H

#include <QObject>
#include <QDebug>


/* macros */
#define IS_EMPTY()  (  mWIdx.load()             == mRIdx.load() )
#define IS_FULL()   ( (mWIdx.load()+1)%mSize    == mRIdx.load() )


template<class T>
class LFQueue
{
public:
    LFQueue() : mSize(0), mArray(NULL){}

    ~LFQueue() {setSize(0);}

    bool setSize(int size) {
        if(size<0)
            return false;

        if(mArray) {
            delete[] mArray;
            mArray = NULL;
        }

        if(size>0) {
            mArray = new T[size];
            if(mArray)
                mSize = size;
            return ( mArray!=NULL );
        }

        return true;
    }

    void flush() {
        mRIdx.store(0);
        mWIdx.store(0);
    }

    T* get() {
        if(IS_FULL())
            return NULL;

        return &(mArray[mWIdx.loadAcquire()]); /* prevent memory reordering (belt and braces) */
    }


    void queue() {
        #ifdef QT_DEBUG
        if(IS_FULL())
            qCritical() << "BUG: queueing in full queue";
        #endif

        int wIdx = mWIdx.load();
        mWIdx.storeRelease((wIdx+1)%mSize);
    }


    T* peek() {
        if(IS_EMPTY())
            return NULL;

        return &(mArray[mRIdx.loadAcquire()]); /* prevent memory reordering (belt and braces) */
    }


    void dequeue() {
        #ifdef QT_DEBUG
        if(IS_EMPTY())
            qCritical() << "BUG: dequeueing an empty queue";
        #endif

        int rIdx = mRIdx.load();
        mRIdx.storeRelease((rIdx+1)%mSize);
    }


private:
    int mSize;
    T*  mArray;

    QAtomicInt mRIdx;
    QAtomicInt mWIdx;
};

#endif // LFQUEUE_H
