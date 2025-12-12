#pragma once

#include <string>
#include <vector>
#include <node_api.h>
#include "vlc_player.h"

class JsVlcPlayer;

class JsVlcPlaylist
{
public:
    static void initJsApi(napi_env env);
    static napi_ref create(napi_env env, JsVlcPlayer& player);

private:
    JsVlcPlaylist(JsVlcPlayer* jsPlayer);
    ~JsVlcPlaylist();

    static napi_value jsCreate(napi_env env, napi_callback_info info);
    static void jsFinalize(napi_env env, void* data, void* hint);

    int add(const std::string& mrl, const std::vector<std::string>& options);
    bool play(unsigned index);
    void next();
    void prev();
    napi_value items(napi_env env);
    int currentItem();
    std::string playbackMode();
    void setPlaybackMode(const std::string& mode);

    static napi_value add_item(napi_env env, napi_callback_info info);
    static napi_value play_item(napi_env env, napi_callback_info info);
    static napi_value next(napi_env env, napi_callback_info info);
    static napi_value prev(napi_env env, napi_callback_info info);
    static napi_value get_items(napi_env env, napi_callback_info info);
    static napi_value get_currentItem(napi_env env, napi_callback_info info);
    static napi_value get_playbackMode(napi_env env, napi_callback_info info);
    static napi_value set_playbackMode(napi_env env, napi_callback_info info);

    JsVlcPlayer* _jsPlayer;
    napi_ref _wrapper;
    napi_ref _jsPlaylistItemsRef;

    static napi_ref _jsConstructor;
};
