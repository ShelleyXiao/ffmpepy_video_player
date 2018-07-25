//
// Created by ShaudXiao on 2018/7/24.
//
#include "./opensl_es_context.h"

#define LOG_TAG "OpenSLESContext"

OpenSLESContext::OpenSLESContext(){
    isInited = false;
}

void OpenSLESContext::init() {
    LOGI("CREATE ENGINE");
    SLresult result = createEngine();
    LOGI("createEngine result is s%", ResultToString(result));
    if(SL_RESULT_SUCCESS == result) {
        result = RealizeObject(engineObject);
        if(SL_RESULT_SUCCESS == result)  {
	        result = GetEngineInterface();
        }
    }
}

 OpenSLESContext* OpenSLESContext::instance = new OpenSLESContext();

OpenSLESContext* OpenSLESContext::GetInstance() {
    if(!instance->isInited) {
        instance->init();
        instance->isInited = true;
    }

    return instance;
}