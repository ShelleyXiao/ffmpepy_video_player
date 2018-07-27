//
// Created by ShaudXiao on 2018/7/25.
//

#ifndef ANDROIDFFMPEGPLAYER_MOVIE_FRAME_H
#define ANDROIDFFMPEGPLAYER_MOVIE_FRAME_H

#include "./CommonTools.h"

typedef enum {
    MovieFrameTypeNone,
    MovieFrameTypeAudio,
    MovieFrameTypeVideo
} MovieFrameType;

class MovieFrame {
public:
    float position;
    float duration;

    MovieFrame();
    virtual ~MovieFrame();

    virtual MovieFrameType getType() = 0;

};

class AudioFrame : public MovieFrame {
public:
    byte *samples;
    int size;

    AudioFrame();
    ~AudioFrame();
    MovieFrameType getType() {

        return MovieFrameTypeAudio;
    }

    bool  dataUseUp;

    void fillFullData();
    void useUpData();
    bool  isDataUseUp();
};

class VideoFrame : public MovieFrame {

public:
    uint8_t *luma;
    uint8_t *chromaB;
    uint8_t *chromaR;

    int width;
    int height;

    VideoFrame();
    ~VideoFrame();

    VideoFrame * clone() {
        VideoFrame *frame = new VideoFrame();
        frame->width = width;
        frame->height = height;

        int lumaLength = width * height;
        frame->luma = new uint8_t[lumaLength];
        memcpy(frame->luma, luma, lumaLength);

        int chormeBLength = width * height / 4;
        frame->chromaB = new uint8_t[chormeBLength];
        memcpy(frame->chromaB, chromaB, chormeBLength);

        int chormeRLength = width * height / 4;
        frame->chromaR = new uint8_t[chormeRLength];
        memcpy(frame->chromaR, chromaR, chormeRLength);

        return frame;
    }

    MovieFrameType getType()  {
        return MovieFrameTypeVideo;
    }
};

#endif //ANDROIDFFMPEGPLAYER_MOVIE_FRAME_H
