#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <node_api.h>
#include <thread>
#include "Common.h"
#include "ATE/ATE.h"

#define DRAWER_COUT     20

typedef struct {
    bool        running;
    bool        reqStop;
    uint32_t    interval;
    uint32_t    duration;
    uint32_t    flags;
    std::thread thread;
    napi_threadsafe_function cb;
} TestRteType;

enum CallBackType { 
    CALLBACK_TYPE_UPDATE = 0,
    CALLBACK_TYPE_EXIT
};

typedef struct {
    CallBackType type;
    void        *param;
} CallBackContent;

typedef struct {
    uint32_t   id;
    DutInfo    dutInfos[DUT_COUNT];
} DrawerInfoType;

extern CATE         _ATE;  
extern TestRteType  _TestRte;
extern DrawerInfoType  _DrawerInfo[DRAWER_COUT];

napi_value OpenDevice(napi_env env, napi_callback_info info);
napi_value CloseDevice(napi_env env, napi_callback_info info);
napi_value SetupDutPower(napi_env env, napi_callback_info info);
napi_value SelectDrawer(napi_env env, napi_callback_info info);
napi_value StartSingleTest(napi_env env, napi_callback_info info);
napi_value StartMultiTest(napi_env env, napi_callback_info info);
napi_value StopMultiTest(napi_env env, napi_callback_info info);

void CallJs(napi_env env, napi_value js_cb, void* context, void* data);

void SingleTestThread();
void MultiTestThread();

#endif