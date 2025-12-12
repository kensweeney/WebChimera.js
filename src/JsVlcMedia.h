#pragma once

#include <string>
#include <node_api.h>
#include "vlc_player.h"

class JsVlcPlayer;

class JsVlcMedia {
public:
    static void initJsApi(napi_env env);
    static napi_value create(napi_env env, JsVlcPlayer& player, const vlc::media& media);

private:
    JsVlcMedia(JsVlcPlayer* jsPlayer, const vlc::media& media);
    ~JsVlcMedia();

    static napi_value jsCreate(napi_env env, napi_callback_info info);
    static void jsFinalize(napi_env env, void* data, void* hint);

    // Getter methods
    std::string meta(libvlc_meta_t e_meta);
    std::string artist();
    std::string genre();
    std::string copyright();
    std::string album();
    std::string trackNumber();
    std::string description();
    std::string rating();
    std::string date();
    std::string URL();
    std::string language();
    std::string nowPlaying();
    std::string publisher();
    std::string encodedBy();
    std::string artworkURL();
    std::string trackID();
    std::string mrl();
    bool parsed();
    double duration();
    std::string title();
    std::string setting();
    bool disabled();

    // Setter methods
    void setMeta(libvlc_meta_t e_meta, const std::string& meta);
    void setTitle(const std::string& title);
    void setSetting(const std::string& setting);
    void setDisabled(bool disabled);

    // Other methods
    void parse();
    void parseAsync();

    // N-API Method & Property Wrappers
    static napi_value get_meta_wrapper(napi_env env, napi_callback_info info, libvlc_meta_t meta_enum);
    static napi_value get_artist(napi_env env, napi_callback_info info);
    static napi_value get_genre(napi_env env, napi_callback_info info);
    static napi_value get_copyright(napi_env env, napi_callback_info info);
    static napi_value get_album(napi_env env, napi_callback_info info);
    static napi_value get_trackNumber(napi_env env, napi_callback_info info);
    static napi_value get_description(napi_env env, napi_callback_info info);
    static napi_value get_rating(napi_env env, napi_callback_info info);
    static napi_value get_date(napi_env env, napi_callback_info info);
    static napi_value get_URL(napi_env env, napi_callback_info info);
    static napi_value get_language(napi_env env, napi_callback_info info);
    static napi_value get_nowPlaying(napi_env env, napi_callback_info info);
    static napi_value get_publisher(napi_env env, napi_callback_info info);
    static napi_value get_encodedBy(napi_env env, napi_callback_info info);
    static napi_value get_artworkURL(napi_env env, napi_callback_info info);
    static napi_value get_trackID(napi_env env, napi_callback_info info);
    static napi_value get_mrl(napi_env env, napi_callback_info info);
    static napi_value get_parsed(napi_env env, napi_callback_info info);
    static napi_value get_duration(napi_env env, napi_callback_info info);

    static napi_value get_title(napi_env env, napi_callback_info info);
    static napi_value set_title(napi_env env, napi_callback_info info);
    static napi_value get_setting(napi_env env, napi_callback_info info);
    static napi_value set_setting(napi_env env, napi_callback_info info);
    static napi_value get_disabled(napi_env env, napi_callback_info info);
    static napi_value set_disabled(napi_env env, napi_callback_info info);

    static napi_value parse(napi_env env, napi_callback_info info);
    static napi_value parseAsync(napi_env env, napi_callback_info info);

    vlc::media& get_media() { return _media; }

    JsVlcPlayer* _jsPlayer;
    vlc::media _media;
    napi_ref _wrapper;

    static napi_ref _jsConstructor;
};
