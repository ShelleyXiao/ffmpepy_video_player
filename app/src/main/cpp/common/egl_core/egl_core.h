//
// Created by ShaudXiao on 2018/7/24.
//

#ifndef ANDROIDFFMPEGPLAYER_EGL_CORE_H
#define ANDROIDFFMPEGPLAYER_EGL_CORE_H

#include "CommonTools.h"
#include <pthread.h>
#include <android/native_window.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <KHR/khrplatform.h>



typedef EGLBoolean (EGLAPIENTRYP PFNEGLPRESENTATIONTIMEANDROIDPROC)(EGLDisplay display, EGLSurface surface, khronos_stime_nanoseconds_t time);

class EGLCore {

public:
       EGLCore();
       virtual ~EGLCore();

       bool init();
       bool initContext(EGLContext sharedContext);
       bool initWithSharedContext();

       EGLSurface createWindowSurface(ANativeWindow *_window);
       EGLSurface createWindowSurface(int with, int height);

       bool makeCurrent(EGLSurface eglSurface);

       void doneCurrent();

       bool swapBuffers(EGLSurface eglSurface);

       int querySurface(EGLSurface surface, int what);

       int setPresentationTime(EGLSurface surface,  khronos_stime_nanoseconds_t nsecs);

       void releaseSurface(EGLSurface eglSurface);

       void release();

       EGLContext getContext();
       EGLDisplay getDisplay();
       EGLConfig getConfig();

private:
        EGLDisplay display;
        EGLConfig  config;
        EGLContext context;

    PFNEGLPRESENTATIONTIMEANDROIDPROC pfneglPresentationTimeANDROID;
};


#endif //ANDROIDFFMPEGPLAYER_EGL_CORE_H
