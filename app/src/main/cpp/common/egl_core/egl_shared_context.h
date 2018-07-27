//
// Created by ShaudXiao on 2018/7/24.
//

#ifndef ANDROIDFFMPEGPLAYER_EGL_SHARED_CONTEXT_H
#define ANDROIDFFMPEGPLAYER_EGL_SHARED_CONTEXT_H

#include "egl_core.h"

class EglShareContext {
public:
    ~EglShareContext() {}

    static EGLContext getSharedContext()
    {
        if(instance_->sharedDisplay == EGL_NO_DISPLAY) {
            instance_->init();
        }
        return instance_->sharedContext;
    }

protected:
    EglShareContext();
    void init();
private:
    static EglShareContext *instance_;
    EGLContext sharedContext;
    EGLDisplay sharedDisplay;

};

#endif //ANDROIDFFMPEGPLAYER_EGL_SHARED_CONTEXT_H
