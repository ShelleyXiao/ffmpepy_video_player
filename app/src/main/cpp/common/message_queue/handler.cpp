//
// Created by ShaudXiao on 2018/7/25.
//

#include "handle.h"

Handler::Handler(MessageQueue *mQueue) {
    this->mQueue = mQueue;
}
Handler::~Handler() {

}

int Handler::postMessage(Message *msg) {
    msg->handler = this;
    return mQueue->enqueueMessage(msg);
}

