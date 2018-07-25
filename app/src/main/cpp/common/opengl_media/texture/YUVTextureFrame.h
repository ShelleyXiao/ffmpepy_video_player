//
// Created by ShaudXiao on 2018/7/25.
//

#ifndef ANDROIDFFMPEGPLAYER_YUVTEXTUREFRAME_H
#define ANDROIDFFMPEGPLAYER_YUVTEXTUREFRAME_H

#include <GLES2/gl2.h>
#include <common/opengl_media/movie_frame.h>
#include "texture_frame.h"

class YUVTextureFrame: public TextureFrame {
private:
    bool writeFlag;
    GLuint textures[3];

    int initTexture();

    VideoFrame *frame;

public:
    YUVTextureFrame();

    virtual ~YUVTextureFrame();

    void setVideoFrame(VideoFrame *yuvFrame);

    bool createTextrue();

    void updateTexImage();

    bool bindTextrue(GLint *uniformSamplers);

    void dealloc();

};

#endif //ANDROIDFFMPEGPLAYER_YUVTEXTUREFRAME_H
