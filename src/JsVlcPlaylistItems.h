#pragma once

#include <node_api.h>

class JsVlcPlayer;

class JsVlcPlaylistItems
{
public:
    static void initJsApi(napi_env env);
    static napi_ref create(napi_env env, JsVlcPlayer& player);

private:
    JsVlcPlaylistItems(JsVlcPlayer* jsPlayer);
    ~JsVlcPlaylistItems();

    static napi_value jsCreate(napi_env env, napi_callback_info info);
    static void jsFinalize(napi_env env, void* data, void* hint);

    napi_value item(napi_env env, uint32_t index);
    unsigned count();
    void clear();
    bool remove(unsigned idx);

    static napi_value get_item(napi_env env, napi_callback_info info);
    static napi_value get_count(napi_env env, napi_callback_info info);
    static napi_value clear_items(napi_env env, napi_callback_info info);
    static napi_value remove_item(napi_env env, napi_callback_info info);

    JsVlcPlayer* _jsPlayer;
    napi_ref _wrapper;

    static napi_ref _jsConstructor;
};
