#pragma once

#include <node_api.h>

class JsVlcPlayer;

class JsVlcInput
{
public:
    static void initJsApi(napi_env env);
    static napi_ref create(JsVlcPlayer& player, napi_env env);

    double length();
    double fps();
    unsigned state();
    bool hasVout();

    double position();
    void setPosition(double);

    double time();
    void setTime(double);

    double rate();
    void setRate(double);

private:
    JsVlcInput(JsVlcPlayer* jsPlayer);

    static napi_value jsCreate(napi_env env, napi_callback_info info);
    static void jsFinalize(napi_env env, void* data, void* hint);

    static napi_value get_length(napi_env env, napi_callback_info info);
    static napi_value get_fps(napi_env env, napi_callback_info info);
    static napi_value get_state(napi_env env, napi_callback_info info);
    static napi_value get_hasVout(napi_env env, napi_callback_info info);
    static napi_value get_position(napi_env env, napi_callback_info info);
    static napi_value set_position(napi_env env, napi_callback_info info);
    static napi_value get_time(napi_env env, napi_callback_info info);
    static napi_value set_time(napi_env env, napi_callback_info info);
    static napi_value get_rate(napi_env env, napi_callback_info info);
    static napi_value set_rate(napi_env env, napi_callback_info info);

    JsVlcPlayer* _jsPlayer;
    napi_ref _wrapper;

    static napi_ref _jsConstructor;
};
