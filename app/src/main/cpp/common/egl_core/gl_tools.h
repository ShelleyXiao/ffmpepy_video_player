//
// Created by ShaudXiao on 2018/7/24.
//

#ifndef CHANGBA_STUDIO_OPENGL_TOOLS_H_
#define CHANGBA_STUDIO_OPENGL_TOOLS_H_

#include "../CommonTools.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define GL_LOGI(...) __android_log_print(ANDROID_LOG_INFO, "GL_TOOLS", __VA_ARGS__)
#define GL_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "GL_TOOLS", __VA_ARGS__)

static inline void checkGLError(const char *op) {
    for(GLint error = glGetError(); error; error = glGetError()) {
        GL_LOGE("after %s() glError() (0x%x)\n", op, error);
    }
}

static inline GLuint loadShader(GLenum shaderType, const char *pSource) {
    GLuint shader = glCreateShader(shaderType);
    if(shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);

        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if(!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if(infoLen) {
                char *buf = (char *)malloc(infoLen);
                if(buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    GL_LOGI("Could not compile shader %d: \n %s\n", shaderType, buf);
                    free(buf);
                }
            } else {
                GL_LOGI( "Guessing at GL_INFO_LOG_LENGTH size\n");
                char* buf = (char*) malloc(0x1000);
                if (buf) {
                    glGetShaderInfoLog(shader, 0x1000, NULL, buf);
                    GL_LOGI("Could not compile shader %d:\n%s\n", shaderType, buf);
                    free(buf);
                }
            }
            glDeleteShader(shader);
            shader = 0;
        }
    }

    return shader;
}

static inline GLuint loadPrograme(char *pVertexSource, char *pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if(!vertexShader) {
        return 0;
    }

    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if(!fragmentShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if(program) {
        glAttachShader(program, vertexShader);
        checkGLError("glAttachShader");
        glAttachShader(program, fragmentShader);
        checkGLError("glAttachShader");
        glLinkProgram(program);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS ,&linkStatus);
        if(linkStatus != GL_FALSE) {
            GLint infoLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
            if(infoLen) {
                char *buf = (char *)malloc(infoLen);
                if(buf) {
                    glGetProgramInfoLog(program, infoLen, NULL, buf);
                    GL_LOGI("Could not linke progream  %s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
			program = 0;
        }
    }

    return program;
}

#endif