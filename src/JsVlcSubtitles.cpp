#include "JsVlcSubtitles.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"

napi_ref JsVlcSubtitles::_jsConstructor = nullptr;

void JsVlcSubtitles::initJsApi(napi_env env)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_GETTER("count", get_count),
        DECLARE_NAPI_PROPERTY("track", get_track, set_track),
        DECLARE_NAPI_PROPERTY("delay", get_delay, set_delay),
    };

    napi_value constructor;
    napi_define_class(env, "VlcSubtitles", NAPI_AUTO_LENGTH, jsCreate, nullptr, sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_create_reference(env, constructor, 1, &_jsConstructor);
}

napi_ref JsVlcSubtitles::create(JsVlcPlayer& player, napi_env env)
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

napi_value JsVlcSubtitles::jsCreate(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);

    JsVlcPlayer* jsPlayer;
    napi_get_value_external(env, args[0], (void**)&jsPlayer);

    JsVlcSubtitles* obj = new JsVlcSubtitles(jsPlayer);
    napi_wrap(env, this_arg, obj, jsFinalize, nullptr, &obj->_wrapper);

    return this_arg;
}

void JsVlcSubtitles::jsFinalize(napi_env env, void* data, void* hint) {
    delete static_cast<JsVlcSubtitles*>(data);
}

JsVlcSubtitles::JsVlcSubtitles(JsVlcPlayer* jsPlayer) :
    _jsPlayer(jsPlayer), _wrapper(nullptr)
{
}

unsigned JsVlcSubtitles::count() { return _jsPlayer->player().subtitles().track_count(); }
int JsVlcSubtitles::track() { return _jsPlayer->player().subtitles().get_track(); }
void JsVlcSubtitles::setTrack(int track) { _jsPlayer->player().subtitles().set_track(track); }
int JsVlcSubtitles::delay() { return static_cast<int>(_jsPlayer->player().subtitles().get_delay()); }
void JsVlcSubtitles::setDelay(int delay) { _jsPlayer->player().subtitles().set_delay(delay); }

napi_value JsVlcSubtitles::get_count(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcSubtitles* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->count());
}

napi_value JsVlcSubtitles::get_track(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcSubtitles* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->track());
}

napi_value JsVlcSubtitles::set_track(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcSubtitles* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setTrack(FromNapiValue<int32_t>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcSubtitles::get_delay(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcSubtitles* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->delay());
}

napi_value JsVlcSubtitles::set_delay(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcSubtitles* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setDelay(FromNapiValue<int32_t>(env, args[0]));
    }
    return nullptr;
}
