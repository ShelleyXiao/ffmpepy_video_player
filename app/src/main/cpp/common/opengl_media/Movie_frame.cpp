//
// Created by ShaudXiao on 2018/7/25.
//

#include "./movie_frame.h"

MovieFrame::MovieFrame() {
    position = 0.0;
    duration = 0.0;
}

MovieFrame::~MovieFrame() {}

AudioFrame::AudioFrame() {
    dataUseUp = true;
    samples = NULL;
    size = 0;
}

void AudioFrame::fillFullData() {
    dataUseUp = false;
}

void AudioFrame::useUpData() {
    dataUseUp = true;
}

bool  AudioFrame::isDataUseUp() {
    return dataUseUp;
}

AudioFrame::~AudioFrame() {
    if(samples != NULL) {
        delete samples;
        samples = NULL;
    }
}

VideoFrame::VideoFrame() {
    luma = NULL;
    chromaB = NULL;
    chromaR = NULL;
    width = 0;
    height = 0;
}

VideoFrame::~VideoFrame() {
    if(luma != NULL) {
        delete luma;
        luma = NULL;
    }

    if(chromaB != NULL) {
        delete chromaB;
        chromaB = NULL;
    }

    if(chromaR != NULL) {
        delete chromaR;
        chromaR = NULL;
    }
}
