//
// Created by ShaudXiao on 2018/7/24.
//

#ifndef ANDROIDFFMPEGPLAYER_OPENSL_ES_CONTEXT_H
#define ANDROIDFFMPEGPLAYER_OPENSL_ES_CONTEXT_H

#include "./opensl_es_util.h"
#include "../CommonTools.h"

class OpenSLESContext {
private:
    SLObjectItf engineObject;
    SLEngineItf engineEngine;
    bool isInited;

    SLresult createEngine() {
        SLEngineOption engineOptions[] = {{ (SLuint32) SL_ENGINEOPTION_THREADSAFE,
            (SLuint32) SL_BOOLEAN_TRUE } };

        return slCreateEngine(&engineObject, ARRAY_LEN(engineOptions), engineOptions, 0, // no interfaces
                              				0, // no interfaces
                              				0);
    }

    SLresult RealizeObject(SLObjectItf object) {
        return (*object)->Realize(object, SL_BOOLEAN_FALSE);
    }

    SLresult GetEngineInterface() {
        return  (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    }

    OpenSLESContext();
    void init();
    static OpenSLESContext* instance;

public:
    static OpenSLESContext* GetInstance();
    virtual ~OpenSLESContext();
    SLEngineItf getEngine() {
        return engineEngine;
    }

};

#endif //ANDROIDFFMPEGPLAYER_OPENSL_ES_CONTEXT_H
