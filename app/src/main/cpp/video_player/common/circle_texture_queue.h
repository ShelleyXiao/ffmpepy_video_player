//
// Created by ShaudXiao on 2018/7/26.
//

#ifndef ANDROIDFFMPEGPLAYER_CIRCLE_TEXTURE_QUEUE_H
#define ANDROIDFFMPEGPLAYER_CIRCLE_TEXTURE_QUEUE_H

#include <pthread.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "CommonTools.h"

#define DEFAULT_QUEUE_SIZE 10

#define INVALID_FRAME_POSITION -1.0

typedef void(*onSignalFrameAvailableCallback)(void *ctx);


typedef struct FrameTexture{
    GLuint texId;
    float position;
    int width;
    int height;

    FrameTexture() {
        texId = 0;
        position = INVALID_FRAME_POSITION;
        width = 0;
        height = 0;
    }

    ~FrameTexture() {
        if(texId) {
            glDeleteTextures(1, &texId);
        }
    }
} FrameTexture;


typedef struct FrameTextureNode {
    FrameTexture *texture;
    struct FrameTextureNode *next;

    FrameTextureNode() {

        texture = NULL;
        next = NULL;
    }
} FrameTextureNode;

class CircleFrameTextureQueue {
public:
    CircleFrameTextureQueue(const char *queuNameParam);
    ~CircleFrameTextureQueue();

    /** 在Uploader中的EGL Thread中调用 **/
    void init(int width, int height, int queueSize);

    /**
	 * 当视频解码器解码出一帧视频
	 * 	1、锁住pushCursor所在的FrameTexture;
	 * 	2、客户端自己向FrameTexture做拷贝或者上传纹理操作
	 * 	3、解锁pushCursor所在的FrameTexture并且移动
	 */
    FrameTexture* lockPushCursorFrameTexture();
    void unLockPushCursorFrameTexture();

    /**
	 * return < 0 if aborted,
	 * 			0 if no packet
	 *		  > 0 if packet.
	 */
    int front(FrameTexture **frameTexture);
    int pop();
    /** 清空queue **/
    void clear();
    /** 获取queue中有效的数据参数 **/
    int getValidSize();

    void abort();

    bool getAvailable() {
        return isAvailable;
    }

    FrameTexture* getFirstFrameFrameTexture();

    void setIsFirstFrame(bool value);

    bool getIsFirstFrame();

private:
    FrameTextureNode* head;
    FrameTextureNode* tail;
    FrameTextureNode* pullCursor;
    FrameTextureNode* pushCursor;
    FrameTexture* firstFrame;

    int queueSize;
    bool isAvailable;
    bool mAbortRequest;
    bool isFirstFrame;
    pthread_mutex_t mLock;
    pthread_cond_t mCondition;
    const char* queueName;

    void flush();
    FrameTexture* buildFrameTexture(int width, int height, float position);

    bool checkGlError(const char* op);
    void buildGPUFrame(FrameTexture* frameTexture, int width, int height);
};

#endif //ANDROIDFFMPEGPLAYER_CIRCLE_TEXTURE_QUEUE_H
