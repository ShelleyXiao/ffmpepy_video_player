//
// Created by ShaudXiao on 2018/7/25.
//

#include "texture_frame.h"

#define LOG_TAG "TextureFrame"

bool TextureFrame::checkGlError(const char *op) {
    GLint error;
    for(error = glGetError(); error; error = glGetError()) {
        LOGI("ERROR:after%s() glError (0x%x)\n", op, error);
        return true;
    }

    return false;
}


TextureFrame::TextureFrame() {

}

TextureFrame::~TextureFrame() {

}
