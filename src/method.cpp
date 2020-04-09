#include "global.h"

napi_value OpenDevice(napi_env env, napi_callback_info info)
{
    int32_t     errCode = -1;
    napi_value  ret;
    napi_status status;
    size_t      argc = 1;
    napi_value  args[1];
    char        path[512];
    size_t      size;

    status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if (status != napi_ok) {
        tr_err("<%s> napi_get_cb_info error.\r\n", __FUNCTION__);
        goto exit;
    }

    status = napi_get_value_string_utf8(env, args[0], path, 512, &size);
    if (status != napi_ok || size <= 0) {
        tr_err("<%s> Invaild param library path.\r\n", __FUNCTION__);
        goto exit;
    }

    if (_ATE.OpenDev(path)) {
        errCode = 0;
    }
exit:
    napi_create_int32(env, errCode, &ret);
    return ret;
}

napi_value CloseDevice(napi_env env, napi_callback_info info)
{
    napi_value  ret;
    _ATE.CloseDev();
    napi_create_int32(env, 0, &ret);
    return ret;
}

napi_value SetupDutPower(napi_env env, napi_callback_info info)
{
    int32_t     errCode = -1;
    napi_value  ret;
    napi_status status;
    size_t      argc = 3;
    napi_value  args[3];
    uint32_t    flags, min, max;

    status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if (status != napi_ok) {
        tr_err("<%s> napi_get_cb_info error.\r\n", __FUNCTION__);
        goto exit;
    }

    status = napi_get_value_uint32(env, args[0], &flags);
    if (status != napi_ok) {
        tr_err("<%s> Invaild param flags.\r\n", __FUNCTION__);
        goto exit;
    }

    status = napi_get_value_uint32(env, args[2], &max);
    if (status != napi_ok) {
        tr_err("<%s> Invaild param max.\r\n", __FUNCTION__);
        goto exit;
    }

    status = napi_get_value_uint32(env, args[1], &min);
    if (status != napi_ok) {
        tr_err("<%s> Invaild param min.\r\n", __FUNCTION__);
        goto exit;
    }

    if (_ATE.SetupDutPower(flags, max, min)) {
        errCode = 0;
    }
exit:
    napi_create_int32(env, errCode, &ret);
    return ret;
}

napi_value SelectDrawer(napi_env env, napi_callback_info info)
{
    int32_t     errCode = -1;
    napi_value  ret;
    napi_status status;
    size_t      argc = 1;
    napi_value  args[1];
    uint32_t    index;

    status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if (status != napi_ok) {
        tr_err("<%s> napi_get_cb_info error.\r\n", __FUNCTION__);
        goto exit;
    }

    status = napi_get_value_uint32(env, args[0], &index);
    if (status != napi_ok) {
        tr_err("<%s> Invaild param index.\r\n", __FUNCTION__);
        goto exit;
    }

    if (_ATE.SelectDrawer(index)) {
        errCode = 0;
    }
exit:
    napi_create_int32(env, errCode, &ret);
    return ret;
}



napi_value StartSingleTest(napi_env env, napi_callback_info info)
{
    int32_t     errCode = -1;
    napi_value  ret;
    napi_status status;
    size_t      argc = 3;
    napi_value  args[3], updateWorkName, doneWorkName;

    if (_TestRte.running) {
        tr_err("<%s> test is running.\r\n", __FUNCTION__);
        goto exit;   
    }

    status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if (status != napi_ok) {
        tr_err("<%s> napi_get_cb_info error.\r\n", __FUNCTION__);
        goto exit;
    }

    status = napi_get_value_uint32(env, args[0], &_TestRte.interval);
    if (status != napi_ok) {
        tr_err("<%s> Invaild param interval.\r\n", __FUNCTION__);
        goto exit;
    }
    napi_create_string_utf8(env, "ate test update", NAPI_AUTO_LENGTH, &updateWorkName);
    status = napi_create_threadsafe_function(env, args[1], NULL, updateWorkName, 0, 1,
                                             NULL, NULL, NULL, CallJsUpdate,
                                             &_TestRte.cb.update);
     if (status != napi_ok) {
        tr_err("<%s> napi_create_threadsafe_function update failure.\r\n", __FUNCTION__);
        goto exit;
    }

    napi_create_string_utf8(env, "ate test done", NAPI_AUTO_LENGTH, &doneWorkName);
    status = napi_create_threadsafe_function(env, args[2], NULL, doneWorkName, 0, 1,
                                             NULL, NULL, NULL, CallJsTestDone,
                                             &_TestRte.cb.done);
     if (status != napi_ok) {
        tr_err("<%s> napi_create_threadsafe_function done failure.\r\n", __FUNCTION__);
        goto exit;
    }

    _TestRte.running = true;
    _TestRte.thread = std::thread(SingleTestThread);
    errCode = 0;
exit:
    napi_create_int32(env, errCode, &ret);
    return ret;
}

napi_value StartMultiTest(napi_env env, napi_callback_info info)
{
    int32_t     errCode = -1;
    napi_value  ret;
    napi_status status;
    size_t      argc = 5;
    napi_value  args[5], updateWorkName, doneWorkName;

    if (_TestRte.running) {
        tr_err("<%s> test is running.\r\n", __FUNCTION__);
        goto exit;   
    }

    status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if (status != napi_ok) {
        tr_err("<%s> napi_get_cb_info error.\r\n", __FUNCTION__);
        goto exit;
    }

    status = napi_get_value_uint32(env, args[0], &_TestRte.interval);
    if (status != napi_ok) {
        tr_err("<%s> Invaild param interval.\r\n", __FUNCTION__);
        goto exit;
    }

    status = napi_get_value_uint32(env, args[1], &_TestRte.duration);
    if (status != napi_ok) {
        tr_err("<%s> Invaild param duration.\r\n", __FUNCTION__);
        goto exit;
    }

    status = napi_get_value_uint32(env, args[2], &_TestRte.flags);
    if (status != napi_ok) {
        tr_err("<%s> Invaild param flags.\r\n", __FUNCTION__);
        goto exit;
    }

    napi_create_string_utf8(env, "ate test update", NAPI_AUTO_LENGTH, &updateWorkName);
    status = napi_create_threadsafe_function(env, args[3], NULL, updateWorkName, 0, 1,
                                             NULL, NULL, NULL, CallJsUpdate,
                                             &_TestRte.cb.update);
     if (status != napi_ok) {
        tr_err("<%s> napi_create_threadsafe_function update failure.\r\n", __FUNCTION__);
        goto exit;
    }

    napi_create_string_utf8(env, "ate test done", NAPI_AUTO_LENGTH, &doneWorkName);
    status = napi_create_threadsafe_function(env, args[4], NULL, doneWorkName, 0, 1,
                                             NULL, NULL, NULL, CallJsTestDone,
                                             &_TestRte.cb.done);
     if (status != napi_ok) {
        tr_err("<%s> napi_create_threadsafe_function done failure.\r\n", __FUNCTION__);
        goto exit;
    }

    _TestRte.reqStop = false;
    _TestRte.running = true;
    _TestRte.thread = std::thread(MultiTestThread);
    errCode = 0;
exit:
    napi_create_int32(env, errCode, &ret);
    return ret;
}

napi_value StopMultiTest(napi_env env, napi_callback_info info)
{
    int32_t     errCode = -1;
    napi_value  ret;

    if (!_TestRte.running) {
        tr_err("<%s> test not running.\r\n", __FUNCTION__);
        goto exit;   
    }
    _TestRte.reqStop = true;
    errCode = 0;
exit:    
    napi_create_int32(env, errCode, &ret);
    return ret;
}