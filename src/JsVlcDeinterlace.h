#pragma once

#include <node_api.h>
#include <string>

class JsVlcPlayer;

class JsVlcDeinterlace
{
public:
    static void initJsApi(napi_env env);
    static napi_ref create(JsVlcPlayer& player, napi_env env);

private:
    JsVlcDeinterlace(JsVlcPlayer* jsPlayer);

    static napi_value jsCreate(napi_env env, napi_callback_info info);
    static void jsFinalize(napi_env env, void* data, void* hint);

    void enable(const std::string& mode);
    void disable();

    static napi_value enable(napi_env env, napi_callback_info info);
    static napi_value disable(napi_env env, napi_callback_info info);

    JsVlcPlayer* _jsPlayer;
    napi_ref _wrapper;

    static napi_ref _jsConstructor;
};
