//
// Created by ShaudXiao on 2018/7/26.
//

#ifndef ANDROIDFFMPEGPLAYER_FFMPEG_VIDEO_DECODER_H
#define ANDROIDFFMPEGPLAYER_FFMPEG_VIDEO_DECODER_H

#include "video_decoder.h"

#include "../texture_uploader/texture_frame_uploader.h"
#include "../texture_uploader/yuv_texture_frame_uploader.h"

class FFmpegVideoDecoder : public VideoDecoder {
public:
    FFmpegVideoDecoder();

    FFmpegVideoDecoder(JavaVM *g_jvm,
                       jobject obj
    );

    virtual ~FFmpegVideoDecoder();

    virtual float updateTexImage(TextureFrame *textureFrame);

protected:
    virtual TextureFrameUploader *createTextureFrameUploader();

    virtual bool decodeVideoFrame(AVPacket packet, int *decodeVideoErrorState);

    virtual void flushVideoFrames(AVPacket packet, int *decodeVideoErrorState);

    virtual int initAnalyzeDurationAndProbesize(int *max_analyze_durations, int analyzeDurationSize,
                                                int probesize, bool fpsProbeSizeConfigured) {
        return 1;
    };
};

#endif //ANDROIDFFMPEGPLAYER_FFMPEG_VIDEO_DECODER_H
