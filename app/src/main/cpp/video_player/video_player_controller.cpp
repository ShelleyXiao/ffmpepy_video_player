//
// Created by ShaudXiao on 2018/7/27.
//

#include "video_player_controller.h"

#define LOG_TAG "VideoPlayerController"

VideoPlayerController::VideoPlayerController() {
    userCancelled = false;

    videoOutput = NULL;
    audioOutput = NULL;

    synchronizer = NULL;

    screenHeight = 0;
    screenWidth = 0;
}

VideoPlayerController::~VideoPlayerController() {
    videoOutput = NULL;
    audioOutput = NULL;

    synchronizer = NULL;
}

void VideoPlayerController::signalOutputFrameAvaiable() {
    if (NULL != videoOutput) {
        videoOutput->signalFrameAvailable();
    }
}

bool VideoPlayerController::initAVSynchronizer() {
    synchronizer = new AVSynchronizer();
    return synchronizer->init(requestHeader, g_jvm, obj, minBufferedDuration, maxBufferedDuration);
}


void VideoPlayerController::initVideoOutput(ANativeWindow *window) {
    if (NULL == window || userCancelled) {
        return;
    }

    videoOutput = new VideoOutput();
    videoOutput->initOutput(window, screenWidth, screenHeight, videoCallbackGetTex, this);

}

bool VideoPlayerController::startAVSynchronizer() {
    LOGI("enter VideoPlayerController::startAVSynchronizer...");
    bool ret = false;

    if (userCancelled) {
        return ret;
    }

    if (this->initAVSynchronizer()) {
        if (synchronizer->validAudio()) {
            ret = this->initAudioOutput();
        }
    }

    if (ret) {
        if (NULL != synchronizer && !synchronizer->isValid()) {
            ret = false;
        } else {
            isPlaying = true;
            synchronizer->start();
            LOGI("call audioOutput start...");
            if (NULL != audioOutput) {
                audioOutput->start();
            }
            LOGI("After call audioOutput start...");
        }
    }
    LOGI("VideoPlayerController::startAVSynchronizer() init result:%s", (ret ? "success" : "fail"));
    this->setInitializedStatus(ret);

    return ret;
}

int VideoPlayerController::videoCallbackGetTex(FrameTexture **frameTex, void *ctx,
                                               bool forceGetFrame) {
    VideoPlayerController *controller = (VideoPlayerController *) ctx;

    return controller->getCorrectRenderTexture(frameTex, forceGetFrame);

}

int VideoPlayerController::getCorrectRenderTexture(FrameTexture **frameTex, bool forceGetFrame) {
    int ret = -1;

    if (!synchronizer->isDestroyed) {
        if (synchronizer->isPlayCompleted()) {
            (*frameTex) = synchronizer->getFirstRenderTexture();
        } else {
            (*frameTex) = synchronizer->getCorrectRenderTexture(forceGetFrame);
        }
        ret = 0;
    }

    return ret;
}

void VideoPlayerController::onSurfaceCreated(ANativeWindow *window, int width, int height) {
    if (NULL != window) {
        this->window = window;
    }

    if (userCancelled) {
        return;
    }

    if (width > 0 && height > 0) {
        this->screenWidth = width;
        this->screenHeight = height;
    }

    if (!videoOutput) {
        initVideoOutput(window);
    } else {
        videoOutput->onSurfaceCreated(window);
    }
}

void VideoPlayerController::onSurfaceDestroyed() {
    if (videoOutput) {
        videoOutput->onSurfaceDestroyed();
    }
}

int VideoPlayerController::audioCallbackFillData(byte *outData, size_t bufferSize, void *ctx) {
    VideoPlayerController *playerController = (VideoPlayerController *) ctx;
    return playerController->consumeAudioFrames(outData, bufferSize);
}

void VideoPlayerController::setInitializedStatus(bool initCode) {
    JNIEnv *env = 0;
    int status = 0;
    bool needAttch = false;
    status = g_jvm->GetEnv((void **) (&env), JNI_VERSION_1_4);
    if (status < 0) {
        if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
            LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
            return;
        }

        needAttch = true;
    }

    jclass cls = env->GetObjectClass(obj);
    jmethodID onInitializedFunc = env->GetMethodID(cls, "onInitializedFromNative", "(Z)V");
    env->CallVoidMethod(obj, onInitializedFunc, initCode);

    if (needAttch) {
        if (g_jvm->DetachCurrentThread() != JNI_OK) {
            LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
        }
    }
    LOGI("leave VideoPlayerController::setInitializedStatus...");
}

bool VideoPlayerController::init(char *videoFileName, JavaVM *g_jvm, jobject obj,
                                 int *max_analyze_duration, int analyzeCnt, int probesize,
                                 bool fpsProbeSizeConfigured, float minBufferedDuration,
                                 float maxBufferedDuration) {

    isPlaying = false;
    synchronizer = NULL;
    audioOutput = NULL;
    videoOutput = NULL;

    requestHeader = new DecoderRequestHeader();
    requestHeader->init(videoFileName, max_analyze_duration, analyzeCnt, probesize,
                        fpsProbeSizeConfigured);
    this->g_jvm = g_jvm;
    this->obj = obj;
    this->minBufferedDuration = minBufferedDuration;
    this->maxBufferedDuration = maxBufferedDuration;

    pthread_create(&initThreadThreadId, 0, initThreadCallback, this);

    userCancelled = false;
    return true;
}

int VideoPlayerController::getAudioChannels() {
    int channels = -1;
    if (NULL != synchronizer) {
        channels = synchronizer->getAudioChannels();
    }

    return channels;
}

bool VideoPlayerController::initAudioOutput() {
    int channels = this->getAudioChannels();
    if (channels < 0) {
        return false;
    }

    int sampleRate = synchronizer->getAudioSampleRate();
    if (sampleRate < 0) {
        return false;
    }

    audioOutput = new AudioOutput();
    SLresult lresult = audioOutput->initSoundTrack(channels, sampleRate,
                                                   audioCallbackFillData, this);
    if (SL_RESULT_SUCCESS != lresult) {
        delete audioOutput;
        audioOutput = NULL;
        return false;
    }

    return true;
}


void VideoPlayerController::play() {
    LOGI("VideoPlayerController::play %d ", (int) isPlaying);
    if (this->isPlaying)
        return;
    this->isPlaying = true;
    if (NULL != audioOutput) {
        audioOutput->play();
    }
}


void VideoPlayerController::pause() {
    LOGI("VideoPlayerController::pause");
    if (!this->isPlaying)
        return;
    this->isPlaying = false;
    if (NULL != audioOutput) {
        audioOutput->pause();
    }
}

void VideoPlayerController::resetRenderSize(int left, int top, int width, int height) {
    LOGI("VideoPlayerController::resetRenderSize");
    if (NULL != videoOutput) {
        LOGI("VideoPlayerController::resetRenderSize NULL != videoOutput width:%d, height:%d",
             width, height);
        videoOutput->resetRenderSize(left, top, width, height);
    } else {
        LOGI("VideoPlayerController::resetRenderSize NULL == videoOutput width:%d, height:%d",
             width, height);
        screenWidth = width;
        screenHeight = height;
    }
}

int VideoPlayerController::consumeAudioFrames(byte *outData, size_t bufferSize) {
    int ret = bufferSize;
    if (this->isPlaying &&
        synchronizer && !synchronizer->isDestroyed
        && !synchronizer->isPlayCompleted()) {
        ret = synchronizer->fillAudioData(outData, bufferSize);
        signalOutputFrameAvaiable();
    } else {
        memset(outData, 0, bufferSize);
    }

    return ret;
}


float VideoPlayerController::getDuration() {
    if (NULL != synchronizer) {
        return synchronizer->getDuration();
    }
    return 0.0f;
}

int VideoPlayerController::getVideoFrameWidth() {
    if (NULL != synchronizer) {
        return synchronizer->getVideoFrameWidth();
    }
    return 0;
}

int VideoPlayerController::getVideoFrameHeight() {
    if (NULL != synchronizer) {
        return synchronizer->getVideoFrameHeight();
    }
    return 0;
}

float VideoPlayerController::getBufferedProgress() {
    if (NULL != synchronizer) {
        return synchronizer->getBufferedProgress();
    }
    return 0.0f;
}

float VideoPlayerController::getPlayProgress() {
    if (NULL != synchronizer) {
        return synchronizer->getPlayProgress();
    }
    return 0.0f;
}

void VideoPlayerController::seekToPosition(float position) {
    LOGI("enter VideoPlayerController::seekToPosition...");
    if (NULL != synchronizer) {
        return synchronizer->seekToPosition(position);
    }
}

void VideoPlayerController::destroy() {
    userCancelled  = true;

    if(synchronizer) {
        synchronizer->interruptRequest();
    }

    pthread_join(initThreadThreadId, 0);

    if(NULL != videoOutput) {
        videoOutput->stopOutput();
        delete videoOutput;
        videoOutput = NULL;
    }

    if(NULL != audioOutput) {
        synchronizer->isDestroyed = true;
        this->pause();
        synchronizer->destroy();

        if(NULL != audioOutput) {
            audioOutput->stop();
            delete audioOutput;
            audioOutput = NULL;
        }

        synchronizer->clearFrameMeta();
        delete synchronizer;
        synchronizer = NULL;
    }

    if(NULL != requestHeader) {
        requestHeader->destroy();
        delete requestHeader;
        requestHeader = NULL;
    }

}

void* VideoPlayerController::initThreadCallback(void *myself){
    VideoPlayerController *controller = (VideoPlayerController*) myself;
    controller->startAVSynchronizer();
    pthread_exit(0);
    return 0;
}

EGLContext VideoPlayerController::getUploaderEGLContext() {
    if (NULL != synchronizer) {
        return synchronizer->getUploaderEGLContext();
    }
    return NULL;
}