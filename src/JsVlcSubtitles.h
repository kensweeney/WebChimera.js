#pragma once

#include <string>
#include <node_api.h>

class JsVlcPlayer;

class JsVlcSubtitles
{
public:
    static void initJsApi(napi_env env);
    static napi_ref create(JsVlcPlayer& player, napi_env env);

    unsigned count();
    int track();
    void setTrack(int);
    int delay();
    void setDelay(int);

private:
    JsVlcSubtitles(JsVlcPlayer* jsPlayer);

    static napi_value jsCreate(napi_env env, napi_callback_info info);
    static void jsFinalize(napi_env env, void* data, void* hint);

    static napi_value get_count(napi_env env, napi_callback_info info);
    static napi_value get_track(napi_env env, napi_callback_info info);
    static napi_value set_track(napi_env env, napi_callback_info info);
    static napi_value get_delay(napi_env env, napi_callback_info info);
    static napi_value set_delay(napi_env env, napi_callback_info info);

    JsVlcPlayer* _jsPlayer;
    napi_ref _wrapper;

    static napi_ref _jsConstructor;
};
