#include <node_api.h>
#include <stdio.h>
#include "global.h"
#include "PassThruMisc.h"

CATE _ATE;  
TestRteType     _TestRte = { false, 0 };
DrawerInfoType  _DrawerInfo[DRAWER_COUT];

void InitDriverList(napi_env env, napi_value exports)
{
    PassThru_DriverArray drivers = { 0, 0 };
    napi_status status;
    napi_value  list;

    PassThru_loadDriver(drivers);
    status = napi_create_array_with_length(env, drivers.count, &list);
    if (status != napi_ok) {
        tr_err("<%s> napi_create_array_with_length error.\r\n", __FUNCTION__);
        return;
    }

    for (unsigned int i = 0; i < drivers.count; i++) {
        napi_value element, name, vendor, library;
        napi_create_object(env, &element);
        napi_create_string_utf8(env, drivers.librarys[i].szName, NAPI_AUTO_LENGTH, &name);
        napi_set_named_property(env, element, "name", name);
        napi_create_string_utf8(env, drivers.librarys[i].szVendor, NAPI_AUTO_LENGTH, &vendor);
        napi_set_named_property(env, element, "vendor", vendor);
        napi_create_string_utf8(env, drivers.librarys[i].szFunctionLibrary, NAPI_AUTO_LENGTH, &library);
        napi_set_named_property(env, element, "library", library);
        napi_set_element(env, list, i, element);
    }
    napi_set_named_property(env, exports, "drivers", list);
}

void RegMethod(napi_env env, napi_value exports, const char* name, napi_callback method)
{
    napi_value m;
    napi_status status;

    status = napi_create_function(env, name, NAPI_AUTO_LENGTH, method, NULL, &m);
    if (status != napi_ok) {
        tr_err("<%s> napi_create_function %s failure.\r\n", __FUNCTION__, name);
        return;
    }

    status = napi_set_named_property(env, exports, name, m);
    if (status != napi_ok) {
        tr_err("<%s> napi_set_named_property %s failure.\r\n", __FUNCTION__, name);
        return;
    }
}

napi_value Init(napi_env env, napi_value exports) 
{
    InitDriverList(env, exports);
    RegMethod(env, exports, "OpenDevice",  OpenDevice);
    RegMethod(env, exports, "CloseDevice", CloseDevice);
    RegMethod(env, exports, "SetupDutPower", SetupDutPower);
    RegMethod(env, exports, "SelectDrawer", SelectDrawer);
    RegMethod(env, exports, "StartSingleTest", StartSingleTest);
    RegMethod(env, exports, "StartMultiTest", StartMultiTest);
    RegMethod(env, exports, "StopMultiTest", StopMultiTest);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)