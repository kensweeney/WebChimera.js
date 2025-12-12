#include "JsVlcVideo.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"
#include "JsVlcDeinterlace.h"

napi_ref JsVlcVideo::_jsConstructor = nullptr;

void JsVlcVideo::initJsApi(napi_env env)
{
    JsVlcDeinterlace::initJsApi(env);

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_GETTER("count", get_count),
        DECLARE_NAPI_PROPERTY("track", get_track, set_track),
        DECLARE_NAPI_PROPERTY("contrast", get_contrast, set_contrast),
        DECLARE_NAPI_PROPERTY("brightness", get_brightness, set_brightness),
        DECLARE_NAPI_PROPERTY("hue", get_hue, set_hue),
        DECLARE_NAPI_PROPERTY("saturation", get_saturation, set_saturation),
        DECLARE_NAPI_PROPERTY("gamma", get_gamma, set_gamma),
        DECLARE_NAPI_GETTER("deinterlace", get_deinterlace),
    };

    napi_value constructor;
    napi_define_class(env, "VlcVideo", NAPI_AUTO_LENGTH, jsCreate, nullptr, sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_create_reference(env, constructor, 1, &_jsConstructor);
}

napi_ref JsVlcVideo::create(JsVlcPlayer& player, napi_env env)
{
    napi_value cons;
    napi_get_reference_value(env, _jsConstructor, &cons);

    napi_value external_player;
    napi_create_external(env, &player, nullptr, nullptr, &external_player);

    napi_value instance;
    napi_new_instance(env, cons, 1, &external_player, &instance);

    napi_ref instance_ref;
    napi_create_reference(env, instance, 1, &instance_ref);
    return instance_ref;
}

napi_value JsVlcVideo::jsCreate(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);

    JsVlcPlayer* jsPlayer;
    napi_get_value_external(env, args[0], (void**)&jsPlayer);

    JsVlcVideo* obj = new JsVlcVideo(jsPlayer, env);
    napi_wrap(env, this_arg, obj, jsFinalize, nullptr, &obj->_wrapper);

    return this_arg;
}

void JsVlcVideo::jsFinalize(napi_env env, void* data, void* hint) {
    delete static_cast<JsVlcVideo*>(data);
}

JsVlcVideo::JsVlcVideo(JsVlcPlayer* jsPlayer, napi_env env) :
    _jsPlayer(jsPlayer), _wrapper(nullptr), _env(env)
{
    _jsDeinterlaceRef = JsVlcDeinterlace::create(*jsPlayer, env);
}

JsVlcVideo::~JsVlcVideo() {
    napi_delete_reference(_env, _jsDeinterlaceRef);
}

unsigned JsVlcVideo::count() { return _jsPlayer->player().video().track_count(); }
int JsVlcVideo::track() { return _jsPlayer->player().video().get_track(); }
void JsVlcVideo::setTrack(unsigned track) { _jsPlayer->player().video().set_track(track); }
double JsVlcVideo::contrast() { return _jsPlayer->player().video().get_contrast(); }
void JsVlcVideo::setContrast(double contrast) { _jsPlayer->player().video().set_contrast(static_cast<float>(contrast)); }
double JsVlcVideo::brightness() { return _jsPlayer->player().video().get_brightness(); }
void JsVlcVideo::setBrightness(double brightness) { _jsPlayer->player().video().set_brightness(static_cast<float>(brightness)); }
int JsVlcVideo::hue() { return _jsPlayer->player().video().get_hue(); }
void JsVlcVideo::setHue(int hue) { _jsPlayer->player().video().set_hue(hue); }
double JsVlcVideo::saturation() { return _jsPlayer->player().video().get_saturation(); }
void JsVlcVideo::setSaturation(double saturation) { _jsPlayer->player().video().set_saturation(static_cast<float>(saturation)); }
double JsVlcVideo::gamma() { return _jsPlayer->player().video().get_gamma(); }
void JsVlcVideo::setGamma(double gamma) { _jsPlayer->player().video().set_gamma(static_cast<float>(gamma)); }
napi_value JsVlcVideo::deinterlace(napi_env env) {
    napi_value deinterlace_obj;
    napi_get_reference_value(env, _jsDeinterlaceRef, &deinterlace_obj);
    return deinterlace_obj;
}

napi_value JsVlcVideo::get_count(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->count());
}

napi_value JsVlcVideo::get_track(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->track());
}

napi_value JsVlcVideo::set_track(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setTrack(FromNapiValue<uint32_t>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcVideo::get_contrast(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->contrast());
}

napi_value JsVlcVideo::set_contrast(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setContrast(FromNapiValue<double>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcVideo::get_brightness(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->brightness());
}

napi_value JsVlcVideo::set_brightness(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setBrightness(FromNapiValue<double>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcVideo::get_hue(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->hue());
}

napi_value JsVlcVideo::set_hue(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setHue(FromNapiValue<int32_t>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcVideo::get_saturation(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->saturation());
}

napi_value JsVlcVideo::set_saturation(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setSaturation(FromNapiValue<double>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcVideo::get_gamma(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->gamma());
}

napi_value JsVlcVideo::set_gamma(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setGamma(FromNapiValue<double>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcVideo::get_deinterlace(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcVideo* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return obj->deinterlace(env);
}
