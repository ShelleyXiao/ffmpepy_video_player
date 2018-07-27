//
// Created by ShaudXiao on 2018/7/26.
//

#include "ffmpeg_video_decoder.h"

#define LOG_TAG "FFmpegVideoDecoder"

FFmpegVideoDecoder::FFmpegVideoDecoder() {

}

FFmpegVideoDecoder::FFmpegVideoDecoder(JavaVM
                                       *g_jvm,
                                       jobject obj
) : VideoDecoder(g_jvm, obj) {

}

FFmpegVideoDecoder::~FFmpegVideoDecoder() {

}

float FFmpegVideoDecoder::updateTexImage(TextureFrame *textureFrame) {
    float position = -1;
    VideoFrame *yuvFrame = handleVideoFrame();
    if (yuvFrame) {
        ((YUVTextureFrame *) textureFrame)->setVideoFrame(yuvFrame);
        textureFrame->updateTexImage();
        position = yuvFrame->position;
        delete yuvFrame;
    }

    return position;
}

TextureFrameUploader *FFmpegVideoDecoder::createTextureFrameUploader() {
    TextureFrameUploader *textureFrameUploader1 = new YUVTextureFrameUploader();
    return textureFrameUploader1;
}

bool FFmpegVideoDecoder::decodeVideoFrame(AVPacket packet, int *decodeVideoErrorState) {
    int pktSize = packet.size;
    int gotframe = 0;

    while (pktSize > 0) {
        int len = avcodec_decode_video2(videoCodecCtx, videoFrame, &gotframe, &packet);
        if (len < 0) {
            LOGI("decode video error, skip packet");
            *decodeVideoErrorState = 1;
            break;
        }
        if (gotframe) {
            if (videoFrame->interlaced_frame) {
                avpicture_deinterlace((AVPicture *) videoFrame, (AVPicture *) videoFrame,
                                      videoCodecCtx->pix_fmt, videoCodecCtx->width,
                                      videoCodecCtx->height);
            }
            this->uploadTexture();
        }
        if (0 == len) {
            break;
        }
        pktSize -= len;
    }
    return (bool) gotframe;
}

void FFmpegVideoDecoder::flushVideoFrames(AVPacket packet, int *decodeVideoErrorState) {
    if (videoCodecCtx->codec->capabilities & CODEC_CAP_DELAY) {
        packet.data = 0;
        packet.size = 0;
        av_init_packet(&packet);
        int gotframe = 0;
        int len = avcodec_decode_video2(videoCodecCtx, videoFrame, &gotframe, &packet);

        LOGI("flush video %d", gotframe);

        if (len < 0) {
            LOGI("decode video error, skip packet");
            *decodeVideoErrorState = 1;
        }
        if (gotframe) {
            if (videoFrame->interlaced_frame) {
                avpicture_deinterlace((AVPicture *) videoFrame, (AVPicture *) videoFrame,
                                      videoCodecCtx->pix_fmt, videoCodecCtx->width,
                                      videoCodecCtx->height);
            }
            this->uploadTexture();
        } else {
            LOGI("output EOF");
            isVideoOutputEOF = true;
        }
    }
}
