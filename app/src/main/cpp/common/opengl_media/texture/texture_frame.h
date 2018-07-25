//
// Created by ShaudXiao on 2018/7/25.
//

#ifndef ANDROIDFFMPEGPLAYER_TEXTURE_FRAME_H
#define ANDROIDFFMPEGPLAYER_TEXTURE_FRAME_H

#include <GLES2/gl2.h>
#include "CommonTools.h"

class TextureFrame {
protected:
    bool checkGlError(const char *op);

public:
    TextureFrame();
    virtual ~TextureFrame();

    virtual bool createTextrue() = 0;
    virtual void updateTexImage() = 0;
    virtual bool bindTextrue(GLint *uniformSamplers) = 0;
    virtual void dealloc() = 0;

};

#endif //ANDROIDFFMPEGPLAYER_TEXTURE_FRAME_H
