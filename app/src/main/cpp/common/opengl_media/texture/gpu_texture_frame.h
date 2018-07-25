//
// Created by ShaudXiao on 2018/7/25.
//

#ifndef ANDROIDFFMPEGPLAYER_GPUTEXTUREFRAME_H
#define ANDROIDFFMPEGPLAYER_GPUTEXTUREFRAME_H

#include <GLES2/gl2.h>
#include <common/opengl_media/movie_frame.h>
#include "texture_frame.h"

class GPUTextureFrame : public TextureFrame {
private:

    int initTexture();

    GLuint decodeTexId;

public:
    GPUTextureFrame();

    virtual ~GPUTextureFrame();

    bool createTextrue();

    void updateTexImage();

    bool bindTextrue(GLint *uniformSamplers);

    void dealloc();

    GLuint getDecodeTextId() {
        return decodeTexId;
    }

};

#endif //ANDROIDFFMPEGPLAYER_YUVTEXTUREFRAME_H
