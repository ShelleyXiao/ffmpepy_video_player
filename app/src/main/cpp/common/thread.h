//
// Created by ShaudXiao on 2018/7/24.
//

#ifndef ANDROIDFFMPEGPLAYER_THREAD_H
#define ANDROIDFFMPEGPLAYER_THREAD_H

#include <pthread.h>
#include "CommonTools.h"

class Thread {
public:
    Thread();
    ~Thread();

    void start();
    void startAsync();
    int wait();

    void waitOnNotify();
    void notify();
    virtual void stop();

protected:
    bool mRunning;
    virtual void handleRun(void *ptr);

private:
    pthread_t mThread;
    pthread_mutex_t mLock;
    pthread_cond_t mCondition;

    static void *startThread(void *ptr);
};

#endif //ANDROIDFFMPEGPLAYER_THREAD_H
