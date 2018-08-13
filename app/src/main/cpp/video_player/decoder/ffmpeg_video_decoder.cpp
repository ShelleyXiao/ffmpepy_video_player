#include "ffmpeg_video_decoder.h"

#define LOG_TAG "FFMPEGVideoDecoder"

FFMPEGVideoDecoder::FFMPEGVideoDecoder() {
}

FFMPEGVideoDecoder::FFMPEGVideoDecoder(JavaVM *g_jvm, jobject obj)
        : VideoDecoder(g_jvm, obj) {
}

FFMPEGVideoDecoder::~FFMPEGVideoDecoder() {
}

TextureFrameUploader *FFMPEGVideoDecoder::createTextureFrameUploader() {
    TextureFrameUploader *textureFrameUploader = new YUVTextureFrameUploader();
    return textureFrameUploader;
}

float FFMPEGVideoDecoder::updateTexImage(TextureFrame *textureFrame) {
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

bool FFMPEGVideoDecoder::decodeVideoFrame(AVPacket packet, int *decodeVideoErrorState) {
//    int pktSize = packet.size;

//    int gotframe = 0;

//	while (pktSize > 0) {
//		int len = avcodec_decode_video2(videoCodecCtx, videoFrame, &gotframe, &packet);
//		if (len < 0) {
//			LOGI("decode video error, skip packet");
//			*decodeVideoErrorState = 1;
//			break;
//		}
//		if (gotframe) {
//			if (videoFrame->interlaced_frame) {
////				avpicture_deinterlace((AVPicture*) videoFrame, (AVPicture*) videoFrame, videoCodecCtx->pix_fmt, videoCodecCtx->width, videoCodecCtx->height);
//			}
//			this->uploadTexture();
//		}
//		if (0 == len) {
//			break;
//		}
//		pktSize -= len;
//	}

    int gotframe = avcodec_send_packet(videoCodecCtx, &packet);

    if (gotframe != 0) {
        *decodeVideoErrorState = 1;
        return gotframe;

    }

    while (avcodec_receive_frame(videoCodecCtx, videoFrame) >= 0) {

        //读取到一帧音频或者视频
        //处理解码后音视频 frame
        this->uploadTexture();

    }

    return (bool) gotframe;
}

void FFMPEGVideoDecoder::flushVideoFrames(AVPacket packet, int *decodeVideoErrorState) {
    if (videoCodecCtx->codec->capabilities & CODEC_CAP_DELAY) {
        packet.data = 0;
        packet.size = 0;
        av_init_packet(&packet);
//        int gotframe = 0;
//        int len = avcodec_decode_video2(videoCodecCtx, videoFrame, &gotframe, &packet);
//
//        LOGI("flush video %d", gotframe);
//
//        if (len < 0) {
//            LOGI("decode video error, skip packet");
//            *decodeVideoErrorState = 1;
//        }
//        if (gotframe) {
//            if (videoFrame->interlaced_frame) {
////				avpicture_deinterlace((AVPicture*) videoFrame, (AVPicture*) videoFrame, videoCodecCtx->pix_fmt, videoCodecCtx->width, videoCodecCtx->height);
//            }
//            this->uploadTexture();
//        } else {
//            LOGI("output EOF");
//            isVideoOutputEOF = true;
//        }

        int gotframe = avcodec_send_packet(videoCodecCtx, &packet);

        if (gotframe != 0) {
            *decodeVideoErrorState = 1;

        }

        gotframe = avcodec_receive_frame(videoCodecCtx, videoFrame);
        if (gotframe >= 0) {
            //读取到一帧音频或者视频
            //处理解码后音视频 frame
            this->uploadTexture();
        } else {
            LOGI("output EOF");
            isVideoOutputEOF = true;
        }
    }
}
