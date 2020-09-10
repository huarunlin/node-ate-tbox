
#include <time.h>
#include "global.h"

void CallJs(napi_env env, napi_value js_cb, void* context, void* data) 
{
    CallBackContent *content = (CallBackContent*)data;
    napi_status status;
    napi_value undefined, argv, event;

    // Retrieve the JavaScript `undefined` value. This will serve as the `this`
    // value for the function call.
    if (napi_get_undefined(env, &undefined) != napi_ok) {
        tr_err("<%s> napi_get_undefined failure.\r\n", __FUNCTION__);
        return;
    }

    status = napi_create_object(env, &argv);
    if (status != napi_ok) {
        tr_err("<%s> napi_create_object failure.\r\n", __FUNCTION__);
        return;
    }

    if (CALLBACK_TYPE_UPDATE == content->type) {
        DrawerInfoType *drawerInfo = (DrawerInfoType*)content->param;
        napi_value list, index;
        /* Event */
        napi_create_string_utf8(env, "update", NAPI_AUTO_LENGTH, &event);
        napi_set_named_property(env, argv, "event", event);
        /* Drawer Id */
        napi_create_uint32(env, drawerInfo->id + 1, &index);
        napi_set_named_property(env, argv, "index", index);
        /* Dut Infos */
        napi_create_array_with_length(env, DUT_COUNT, &list);
        for (unsigned int i = 0; i < DUT_COUNT; i++) {
            napi_value element, sw, avg, max, min;
            napi_create_object(env, &element);
            napi_create_uint32(env, drawerInfo->dutInfos[i].sw, &sw);
            napi_set_named_property(env, element, "sw", sw);
            napi_create_uint32(env, drawerInfo->dutInfos[i].avg, &avg);
            napi_set_named_property(env, element, "avg", avg);
            napi_create_uint32(env, drawerInfo->dutInfos[i].max, &max);
            napi_set_named_property(env, element, "max", max);
            napi_create_uint32(env, drawerInfo->dutInfos[i].min, &min);
            napi_set_named_property(env, element, "min", min);
            napi_set_element(env, list, i, element);
        }
        napi_set_named_property(env, argv, "duts", list);

    } else if (CALLBACK_TYPE_EXIT == content->type) {
        /* Event */
        napi_value code;
        napi_create_string_utf8(env, "exit", NAPI_AUTO_LENGTH, &event);
        napi_create_uint32(env, (uint32_t)(content->param), &code);
        napi_set_named_property(env, argv, "event", event);
        napi_set_named_property(env, argv, "code", code);
    }
    delete content;
    status = napi_call_function(env, undefined, js_cb, 1, &argv, NULL);
    if (status != napi_ok) {
        tr_err("<%s> napi_call_function failure.\r\n", __FUNCTION__);
        return;
    }
}

void SingleTestThread() 
{
    napi_status status;
    DrawerInfoType  *drawerInfo = &_DrawerInfo[0];
    CallBackContent *content;

    std::this_thread::sleep_for(std::chrono::milliseconds(_TestRte.interval * 1000));

    drawerInfo->id = 0;
    if (false == _ATE.GetAllDutInfo(drawerInfo->dutInfos)) {
        goto exit;
    }

    content = new CallBackContent();
    content->type = CALLBACK_TYPE_UPDATE;
    content->param = drawerInfo;
    status = napi_call_threadsafe_function(_TestRte.cb, content, napi_tsfn_blocking);
    if (status != napi_ok){
        tr_err("<%s> napi_call_threadsafe_function update failure.\r\n", __FUNCTION__);
        delete content;
        goto exit;
    } 
    tr_info("update drawer %d done\r\n", drawerInfo->id);
exit:
    _TestRte.running = false;
    content = new CallBackContent();
    content->type = CALLBACK_TYPE_EXIT;
    content->param = NULL;
    status = napi_call_threadsafe_function(_TestRte.cb, content, napi_tsfn_blocking);
    if (status != napi_ok){
        tr_err("<%s> napi_call_function done failure.\r\n", __FUNCTION__);
        delete content;
    } 
    napi_release_threadsafe_function(_TestRte.cb, napi_tsfn_release);
}

void MultiTestThread() 
{
    napi_status status;
    uint32_t mask, exitCode = 0;
    time_t  start = 0, pre = 0, now = 0, diff = 0;
    DrawerInfoType  *drawerInfo = NULL;
    CallBackContent *content;

    start = time(NULL);
    while (_TestRte.running & !_TestRte.reqStop) {
        if (_TestRte.reqStop) {
            exitCode = 1;
            break;
        }
        now = time(NULL);
        /* Check Test Done */
        if (difftime(time(NULL), start) > _TestRte.duration) {
            exitCode = 0;
            break;
        }
        /* Check update */
        if (pre != 0 && difftime(now, pre) < _TestRte.interval) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        pre = now;
        for(int i = 0; i < 20 && (_TestRte.running & !_TestRte.reqStop); i++) {
            mask = (_TestRte.flags >> i) & 0x1;
            if (0x0 == mask) {
                continue;
            }

            drawerInfo = &_DrawerInfo[i];
            drawerInfo->id = i;
            /* 选择通信抽屉 */
            // if (false == _ATE.SelectDrawer(i + 1)) {
            //     continue;
            // }
            /* 获取DUT信息 */
            if (false == _ATE.GetAllDutInfo(drawerInfo->dutInfos)) {
                continue;
            }

            content = new CallBackContent();
            content->type = CALLBACK_TYPE_UPDATE;
            content->param = drawerInfo;
            status = napi_call_threadsafe_function(_TestRte.cb, content, napi_tsfn_blocking);
            if (status != napi_ok){
                tr_err("<%s> napi_call_threadsafe_function update failure.\r\n", __FUNCTION__);
                delete content;
                continue;
            } 
        }
    }

exit:
    _TestRte.running = false;
    _TestRte.reqStop = false;
    content = new CallBackContent();
    content->type = CALLBACK_TYPE_EXIT;
    content->param = (void*)exitCode;
    status = napi_call_threadsafe_function(_TestRte.cb, content, napi_tsfn_blocking);
    if (status != napi_ok){
        tr_err("<%s> napi_call_function done failure.\r\n", __FUNCTION__);
        delete content;
    } 
    napi_release_threadsafe_function(_TestRte.cb, napi_tsfn_release);
}