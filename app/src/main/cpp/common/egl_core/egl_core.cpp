//
// Created by ShaudXiao on 2018/7/24.
//

#include "./egl_core.h"
#include "./egl_shared_context.h"

#define LOG_TAG "EGLCore"

EGLCore::EGLCore() {
    pfneglPresentationTimeANDROID = 0;
    display = EGL_NO_DISPLAY;
    context = EGL_NO_DISPLAY;
}

EGLCore::~EGLCore() {

}

bool EGLCore::init() {
    return this->initContext(NULL);
}

bool EGLCore::initContext(EGLContext sharedContext) {
    EGLint numConfigs;
    EGLint width;
    EGLint height;


    const EGLint attribs[] = {EGL_BUFFER_SIZE, 32,
                              EGL_ALPHA_SIZE, 8,
                              EGL_BLUE_SIZE, 8,
                              EGL_GREEN_SIZE, 8,
                              EGL_RED_SIZE, 8,
                              EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES2_BIT,
                              EGL_SURFACE_TYPE,
                              EGL_WINDOW_BIT,
                              EGL_NONE
    };

    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay() returned error %d", eglGetError());
        return false;
    }
    if (!eglInitialize(display, 0, 0)) {
        LOGE("eglInitialize() returned error %d", eglGetError());
        return false;
    }

    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
        LOGE("eglChooseConfig() returned error %d", eglGetError());
        release();
        return false;
    }

    EGLint eglContextAttributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    if (!(context = eglCreateContext(display, config,
                                     NULL == sharedContext ? EGL_NO_CONTEXT : sharedContext,
                                     eglContextAttributes))) {
        LOGE("eglCreateContext() returned error %d", eglGetError());
        release();
        return false;
    }

    pfneglPresentationTimeANDROID = (PFNEGLPRESENTATIONTIMEANDROIDPROC) eglGetProcAddress(
            "eglPresentationTimeANDROID");
    if (!pfneglPresentationTimeANDROID) {
        LOGE("eglPresentationTimeANDROID is not available!");
    }

    return true;
}

bool EGLCore::initWithSharedContext() {
    EGLContext context = EglShareContext::getSharedContext();
    if (context == EGL_NO_CONTEXT) {
        return false;
    }

    return initContext(context);
}

void EGLCore::releaseSurface(EGLSurface eglSurface) {
    eglDestroySurface(display, eglSurface);
    eglSurface = EGL_NO_SURFACE;
}

void EGLCore::release() {
    if (EGL_NO_DISPLAY != display && EGL_NO_CONTEXT != context) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        LOGI("after eglMakecurrent...");
        eglDestroyContext(display, context);
        LOGI("after  eglDestroyContext...");
    }

    display = EGL_NO_DISPLAY;
    context = EGL_NO_DISPLAY;
}

EGLSurface EGLCore::createWindowSurface(ANativeWindow *_window) {
    EGLSurface eglSurface;
    EGLint format;

    if (_window == NULL) {
        LOGE("EGLCore:: createWindowSurface _windows is NULL!");
        return NULL;
    }

    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
        LOGE("EGLCore:: eglGetConfigAttrib returned error %d", eglGetError());
        release();
        return NULL;
    }

    ANativeWindow_setBuffersGeometry(_window, 0, 0, format);
    if (!(eglSurface = eglCreateWindowSurface(display, config, _window, 0))) {
        LOGE("EGLCore:: eglCreateWindowSurface returned error %d", eglGetError());
    }

    return eglSurface;
}

EGLSurface EGLCore::createOffscreenSurface(int width, int height) {
    EGLSurface eglSurface;
    EGLint PbufferAttributes[] = {EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE, EGL_NONE};
    if (!(eglSurface = eglCreatePbufferSurface(display, config, PbufferAttributes))) {
        LOGE("eglCreatePbufferSurface() returned error %d", eglGetError());
    }
    return eglSurface;
}




bool EGLCore::makeCurrent(EGLSurface eglSurface) {
    return eglMakeCurrent(display, eglSurface, eglSurface, context);
}

void EGLCore::doneCurrent() {
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

bool EGLCore::swapBuffers(EGLSurface eglSurface) {
    return eglSwapBuffers(display, eglSurface);
}

int EGLCore::querySurface(EGLSurface surface, int what) {
    int value = -1;
    eglQuerySurface(display, surface, what, &value);
    return value;
}

int EGLCore::setPresentationTime(EGLSurface surface, khronos_stime_nanoseconds_t nsecs) {
    pfneglPresentationTimeANDROID(display, surface, nsecs);
    return 1;
}


EGLContext EGLCore::getContext() {
    return context;
}

EGLDisplay EGLCore::getDisplay() {
    return display;
}

EGLConfig EGLCore::getConfig() {
    return config;
}