//
// Created by ShaudXiao on 2018/7/27.
//

#include "com_zx_androidffmpegplayer_video_player_KTVPlayer.h"

#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "video_player/video_player_controller.h"

#define LOG_TAG "ChangbaPlayer_JNI_Layer"

VideoPlayerController *videoPlayerController = NULL;

static ANativeWindow *window = 0;

JNIEXPORT jboolean JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer_prepare(
        JNIEnv * env, jobject obj, jstring videoMergeFilePathParam, jintArray max_analyze_duration, jint size, jint probesize, jboolean fpsProbeSizeConfigured, jfloat minBufferedDuration, jfloat maxBufferedDuration,
        jint width, jint height, jobject surface) {
    JavaVM *g_jvm = NULL;
    env->GetJavaVM(&g_jvm);
    jobject  g_obj = env->NewGlobalRef(obj);
    char *videoMergeFilePaht = (char *)env->GetStringUTFChars(videoMergeFilePathParam, NULL);
    if(NULL == videoPlayerController) {
        videoPlayerController = new VideoPlayerController();
    }
    window = ANativeWindow_fromSurface(env, surface);
    jint *max_anaylze_duration_params =
            env->GetIntArrayElements(max_analyze_duration, 0);
    bool initCode = videoPlayerController->init(videoMergeFilePaht, g_jvm, g_obj, max_anaylze_duration_params,
                                                    size, probesize, fpsProbeSizeConfigured, minBufferedDuration, maxBufferedDuration);
    LOGI("############CONTRIL init = %d\n", initCode);
    videoPlayerController->onSurfaceCreated(window, width, height);

    env->ReleaseIntArrayElements(max_analyze_duration, max_anaylze_duration_params, 0);
    env->ReleaseStringUTFChars(videoMergeFilePathParam, videoMergeFilePaht);
	return initCode;
}

JNIEXPORT void JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer_onSurfaceCreated(JNIEnv * env, jobject obj, jobject surface, jint width, jint height) {
    LOGI("############CONTRIL init = %s", __func__);
    if(NULL != videoPlayerController) {
        window = ANativeWindow_fromSurface(env, surface);
        videoPlayerController->onSurfaceCreated(window, width, height);
    }
}

JNIEXPORT void JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer_onSurfaceDestroyed(JNIEnv * env, jobject obj, jobject surface) {
	if (NULL != videoPlayerController) {
		videoPlayerController->onSurfaceDestroyed();
	}
}

JNIEXPORT void JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer_pause(JNIEnv * env, jobject obj) {
	if(NULL != videoPlayerController) {
		videoPlayerController->pause();
	}
}

JNIEXPORT void JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer_play(JNIEnv * env, jobject obj) {
    LOGI("############CONTRIL init = %s", __func__);
    if(NULL != videoPlayerController) {
        videoPlayerController->play();
    }
}

JNIEXPORT void JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer_stop(JNIEnv * env, jobject obj) {
    if(NULL != videoPlayerController) {
        videoPlayerController->destroy();
        delete videoPlayerController;
        videoPlayerController = NULL;
    }
}

JNIEXPORT jfloat JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer_getBufferedProgress(JNIEnv * env, jobject obj) {
    if (NULL != videoPlayerController) {
        return videoPlayerController->getBufferedProgress();
    }
    return 0.0f;
}

JNIEXPORT jfloat JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer_getPlayProgress(JNIEnv * env, jobject obj) {
    if (NULL != videoPlayerController) {
        return videoPlayerController->getPlayProgress();
    }
    return 0.0f;
}

JNIEXPORT void JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer_resetRenderSize(JNIEnv * env, jobject obj, jint left, jint top, jint width, jint height) {
    if(NULL != videoPlayerController) {
        videoPlayerController->resetRenderSize(left, top, width, height);
    }
}

JNIEXPORT void JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer__seekToPosition(JNIEnv * env, jobject obj, jfloat position) {
    if(NULL != videoPlayerController) {
        videoPlayerController->seekToPosition(position);
    }
}

JNIEXPORT void JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer_seekCurrent(JNIEnv * env, jobject obj, jfloat position) {
    if(NULL != videoPlayerController) {
//		videoPlayerController->seekCurrent(position);
    }
}

JNIEXPORT void JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer_beforeSeekCurrent(JNIEnv * env, jobject obj) {
    if(NULL != videoPlayerController) {
//		videoPlayerController->beforeSeekCurrent();
    }
}

JNIEXPORT void JNICALL Java_com_zx_androidffmpegplayer_video_player_KTVPlayer_afterSeekCurrent(JNIEnv * env, jobject obj) {
    if(NULL != videoPlayerController) {
//		videoPlayerController->afterSeekCurrent();
    }
}

