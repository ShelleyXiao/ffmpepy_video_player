//
// Created by ShaudXiao on 2018/7/25.
//

#include "YUVTextureFrame.h"

#define LOG_TAG "YUVTextureFrame"

YUVTextureFrame::YUVTextureFrame() {

}

YUVTextureFrame::~YUVTextureFrame() {

}

void YUVTextureFrame::setVideoFrame(VideoFrame *yuvFrame) {
    this->frame = yuvFrame;
}

bool YUVTextureFrame::createTextrue() {
    LOGI("enter YUVTextureFrame::createTexture");
    textures[0] = 0;
    textures[1] = 0;
    textures[2] = 0;
    int ret = initTexture();
    if(ret < 0) {
        LOGI("initTexture faild....");
        return false;
    }

    return true;
}

void YUVTextureFrame::updateTexImage() {
    if (frame) {
        int frameWidth = frame->width;
        int frameHeight = frame->height;
        if (frameWidth % 16 != 0) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        uint8_t *pixels[3] = {frame->luma, frame->chromaB, frame->chromaR};
        int widths[3] = {frameWidth, frameWidth >> 1, frameWidth >> 1};
        int heights[3] = {frameHeight, frameHeight >> 1, frameHeight >> 1};
        for (int i = 0; i < 3; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            if (checkGlError("glBindTexture")) {
                return;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widths[i], heights[i],
                         0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels[i]);
        }

    }
}

bool YUVTextureFrame::bindTextrue(GLint *uniformSamplers) {
    for (int i = 0; i < 3; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        if (checkGlError("glTexParameteri")) {
            return -1;
        }
        glUniform1i(uniformSamplers[i], i);
    }

    return true;
}

void YUVTextureFrame::dealloc() {
    LOGI("enter YUVTextureFrame::dealloc");
    if (textures[0]) {
        glDeleteTextures(3, textures);
    }
}

int YUVTextureFrame::initTexture() {
    glGenTextures(3, textures);
    for (int i = 0; i < 3; i++) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        if (checkGlError("glBindTexture")) {
            return -1;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (checkGlError("glTexParameteri")) {
            return -1;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        if (checkGlError("glTexParameteri")) {
            return -1;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        if (checkGlError("glTexParameteri")) {
            return -1;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (checkGlError("glTexParameteri")) {
            return -1;
        }
    }

    return 1;
}

