//
// Created by ShaudXiao on 2018/7/24.
//

#ifndef ANDROIDFFMPEGPLAYER_HANDLE_H
#define ANDROIDFFMPEGPLAYER_HANDLE_H

#include "../CommonTools.h"
#include "message_queue.h"

class Handler {
private:
    MessageQueue *mQueue;

public:
    Handler(MessageQueue *mQueue);
    ~Handler();

    int postMessage(Message *msg);
    virtual void handleMessage(Message *msg);
};

#endif //ANDROIDFFMPEGPLAYER_HANDLE_H
