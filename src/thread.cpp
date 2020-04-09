
#include <time.h>
#include "global.h"

void CallJsUpdate(napi_env env, napi_value js_cb, void* context, void* data) 
{
    DrawerInfoType *content = (DrawerInfoType*)data;
    napi_status status;
    napi_value undefined, argv, list, index;

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

    status = napi_create_array_with_length(env, DUT_COUNT, &list);
    if (status != napi_ok) {
        tr_err("<%s> napi_create_array_with_length failure.\r\n", __FUNCTION__);
        return;
    }

    for (unsigned int i = 0; i < DUT_COUNT; i++) {
        napi_value element, sw, avg, max, min;
        napi_create_object(env, &element);
        napi_create_uint32(env, content->dutInfos[i].sw, &sw);
        napi_set_named_property(env, element, "sw", sw);
        napi_create_uint32(env, content->dutInfos[i].avg, &avg);
        napi_set_named_property(env, element, "avg", avg);
        napi_create_uint32(env, content->dutInfos[i].max, &max);
        napi_set_named_property(env, element, "max", max);
        napi_create_uint32(env, content->dutInfos[i].min, &min);
        napi_set_named_property(env, element, "min", min);
        napi_set_element(env, list, i, element);
    }
    tr_info("<%s> update drawer %d.\r\n", __FUNCTION__, content->id + 1);
    napi_create_uint32(env, content->id + 1, &index);
    napi_set_named_property(env, argv, "index", index);
    napi_set_named_property(env, argv, "duts", list);
    // Call the JavaScript function with the item as wrapped into an instance of
    // the JavaScript `ThreadItem` class and the prime.
    status = napi_call_function(env, undefined, js_cb, 1, &argv, NULL);
    if (status != napi_ok) {
        tr_err("<%s> napi_call_function failure.\r\n", __FUNCTION__);
        return;
    }
}

void CallJsTestDone(napi_env env, napi_value js_cb, void* context, void* data) 
{
    DutInfo *infos = (DutInfo*)data;
    napi_status status;
    napi_value undefined;

    // Retrieve the JavaScript `undefined` value. This will serve as the `this`
    // value for the function call.
    if (napi_get_undefined(env, &undefined) != napi_ok) {
        tr_err("<%s> napi_get_undefined failure.\r\n", __FUNCTION__);
        return;
    }
    // Call the JavaScript function with the item as wrapped into an instance of
    // the JavaScript `ThreadItem` class and the prime.
    status = napi_call_function(env, undefined, js_cb, 0, NULL, NULL);
    if (status != napi_ok) {
        tr_err("<%s> napi_call_function failure.\r\n", __FUNCTION__);
        return;
    }
}

void SingleTestThread() 
{
    napi_status status;
    DrawerInfoType  *drawerInfo = &_DrawerInfo[0];

    std::this_thread::sleep_for(std::chrono::milliseconds(_TestRte.interval * 1000));

    drawerInfo->id = 0;
    if (false == _ATE.GetAllDutInfo(drawerInfo->dutInfos)) {
        goto exit;
    }

    status = napi_call_threadsafe_function(_TestRte.cb.update, drawerInfo, napi_tsfn_blocking);
    if (status != napi_ok){
        tr_err("<%s> napi_call_threadsafe_function update failure.\r\n", __FUNCTION__);
        goto exit;
    } 
    tr_info("update drawer %d done\r\n", drawerInfo->id);
exit:
    _TestRte.running = false;
    status = napi_call_threadsafe_function(_TestRte.cb.done, NULL, napi_tsfn_blocking);
    if (status != napi_ok){
        tr_err("<%s> napi_call_function exit failure.\r\n", __FUNCTION__);
    } 
    napi_release_threadsafe_function(_TestRte.cb.update, napi_tsfn_release);
    napi_release_threadsafe_function(_TestRte.cb.done, napi_tsfn_release);
}

void MultiTestThread() 
{
    napi_status status;
    uint32_t mask;
    time_t  start = 0, pre = 0, now = 0, diff = 0;
    DrawerInfoType  *drawerInfo = NULL;

    start = time(NULL);
    while (_TestRte.running & !_TestRte.reqStop) {
        now = time(NULL);
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

            status = napi_call_threadsafe_function(_TestRte.cb.update, drawerInfo, napi_tsfn_blocking);
            if (status != napi_ok){
                tr_err("<%s> napi_call_threadsafe_function update failure.\r\n", __FUNCTION__);
                continue;
            } 
        }

        if (difftime(time(NULL), start) > _TestRte.duration) {
            break;
        }
    }

exit:
    _TestRte.running = false;
    _TestRte.reqStop = false;
    status = napi_call_threadsafe_function(_TestRte.cb.done, NULL, napi_tsfn_blocking);
    if (status != napi_ok){
        tr_err("<%s> napi_call_function exit failure.\r\n", __FUNCTION__);
    } 
    napi_release_threadsafe_function(_TestRte.cb.update, napi_tsfn_release);
    napi_release_threadsafe_function(_TestRte.cb.done, napi_tsfn_release);
}