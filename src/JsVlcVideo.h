#pragma once

#include <string>
#include <node_api.h>

class JsVlcPlayer;

class JsVlcVideo
{
public:
    static void initJsApi(napi_env env);
    static napi_ref create(JsVlcPlayer& player, napi_env env);

    unsigned count();
    int track();
    void setTrack(unsigned);
    double contrast();
    void setContrast(double);
    double brightness();
    void setBrightness(double);
    int hue();
    void setHue(int);
    double saturation();
    void setSaturation(double);
    double gamma();
    void setGamma(double);
    napi_value deinterlace(napi_env env);

private:
    JsVlcVideo(JsVlcPlayer* jsPlayer, napi_env env);
    ~JsVlcVideo();

    static napi_value jsCreate(napi_env env, napi_callback_info info);
    static void jsFinalize(napi_env env, void* data, void* hint);

    static napi_value get_count(napi_env env, napi_callback_info info);
    static napi_value get_track(napi_env env, napi_callback_info info);
    static napi_value set_track(napi_env env, napi_callback_info info);
    static napi_value get_contrast(napi_env env, napi_callback_info info);
    static napi_value set_contrast(napi_env env, napi_callback_info info);
    static napi_value get_brightness(napi_env env, napi_callback_info info);
    static napi_value set_brightness(napi_env env, napi_callback_info info);
    static napi_value get_hue(napi_env env, napi_callback_info info);
    static napi_value set_hue(napi_env env, napi_callback_info info);
    static napi_value get_saturation(napi_env env, napi_callback_info info);
    static napi_value set_saturation(napi_env env, napi_callback_info info);
    static napi_value get_gamma(napi_env env, napi_callback_info info);
    static napi_value set_gamma(napi_env env, napi_callback_info info);
    static napi_value get_deinterlace(napi_env env, napi_callback_info info);

    JsVlcPlayer* _jsPlayer;
    napi_ref _wrapper;
    napi_ref _jsDeinterlaceRef;
    napi_env _env;

    static napi_ref _jsConstructor;
};
