#pragma once

#include <string>
#include <node_api.h>

class JsVlcPlayer;

class JsVlcAudio
{
public:
    static void initJsApi(napi_env env);
    static napi_ref create(napi_env env, JsVlcPlayer& player);

    std::string description(uint32_t index);
    unsigned count();
    int track();
    void setTrack(int);
    bool muted();
    void setMuted(bool muted);
    unsigned volume();
    void setVolume(unsigned);
    int channel();
    void setChannel(unsigned);
    int delay();
    void setDelay(int);
    void toggleMute();

private:
    JsVlcAudio(JsVlcPlayer* jsPlayer);
    ~JsVlcAudio();

    static napi_value jsCreate(napi_env env, napi_callback_info info);
    static void jsFinalize(napi_env env, void* data, void* hint);

    static napi_value get_description(napi_env env, napi_callback_info info);
    static napi_value get_count(napi_env env, napi_callback_info info);
    static napi_value get_track(napi_env env, napi_callback_info info);
    static napi_value set_track(napi_env env, napi_callback_info info);
    static napi_value get_muted(napi_env env, napi_callback_info info);
    static napi_value set_muted(napi_env env, napi_callback_info info);
    static napi_value get_volume(napi_env env, napi_callback_info info);
    static napi_value set_volume(napi_env env, napi_callback_info info);
    static napi_value get_channel(napi_env env, napi_callback_info info);
    static napi_value set_channel(napi_env env, napi_callback_info info);
    static napi_value get_delay(napi_env env, napi_callback_info info);
    static napi_value set_delay(napi_env env, napi_callback_info info);
    static napi_value toggle_mute(napi_env env, napi_callback_info info);

    JsVlcPlayer* _jsPlayer;
    napi_ref _wrapper;

    static napi_ref _jsConstructor;
};
