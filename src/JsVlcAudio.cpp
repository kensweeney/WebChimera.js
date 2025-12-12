#include "JsVlcAudio.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"

napi_ref JsVlcAudio::_jsConstructor = nullptr;

void JsVlcAudio::initJsApi(napi_env env)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_GETTER("description", get_description),
        DECLARE_NAPI_GETTER("count", get_count),
        DECLARE_NAPI_PROPERTY("track", get_track, set_track),
        DECLARE_NAPI_PROPERTY("mute", get_muted, set_muted),
        DECLARE_NAPI_PROPERTY("volume", get_volume, set_volume),
        DECLARE_NAPI_PROPERTY("channel", get_channel, set_channel),
        DECLARE_NAPI_PROPERTY("delay", get_delay, set_delay),
        DECLARE_NAPI_METHOD("toggleMute", toggle_mute),
    };

    napi_value constructor;
    napi_define_class(env, "VlcAudio", NAPI_AUTO_LENGTH, jsCreate, nullptr, sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_create_reference(env, constructor, 1, &_jsConstructor);
}

napi_ref JsVlcAudio::create(napi_env env, JsVlcPlayer& player)
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

napi_value JsVlcAudio::jsCreate(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);

    JsVlcPlayer* jsPlayer;
    napi_get_value_external(env, args[0], (void**)&jsPlayer);

    JsVlcAudio* obj = new JsVlcAudio(jsPlayer);
    napi_wrap(env, this_arg, obj, jsFinalize, nullptr, &obj->_wrapper);

    return this_arg;
}

void JsVlcAudio::jsFinalize(napi_env env, void* data, void* hint) {
    delete static_cast<JsVlcAudio*>(data);
}

JsVlcAudio::JsVlcAudio(JsVlcPlayer* jsPlayer) :
    _jsPlayer(jsPlayer), _wrapper(nullptr)
{
}

JsVlcAudio::~JsVlcAudio() {
    if (_wrapper) {
        napi_delete_reference(_jsPlayer->getEnv(), _wrapper);
    }
}

std::string JsVlcAudio::description(uint32_t index)
{
    vlc::player& p = _jsPlayer->player();
    std::string name;
    libvlc_track_description_t* rootTrackDesc = libvlc_audio_get_track_description(p.get_mp());
    if(!rootTrackDesc) return name;

    unsigned count = _jsPlayer->player().audio().track_count();
    if(count && index < count) {
        libvlc_track_description_t* trackDesc = rootTrackDesc;
        for(; index && trackDesc; --index){
            trackDesc = trackDesc->p_next;
        }
        if (trackDesc && trackDesc->psz_name) {
            name = trackDesc->psz_name;
        }
    }
    libvlc_track_description_list_release(rootTrackDesc);
    return name;
}

unsigned JsVlcAudio::count() { return _jsPlayer->player().audio().track_count(); }
int JsVlcAudio::track() { return _jsPlayer->player().audio().get_track(); }
void JsVlcAudio::setTrack(int track) { _jsPlayer->player().audio().set_track(track); }
bool JsVlcAudio::muted() { return _jsPlayer->player().audio().is_muted(); }
void JsVlcAudio::setMuted(bool muted) { _jsPlayer->player().audio().set_mute(muted); }
unsigned JsVlcAudio::volume() { return _jsPlayer->player().audio().get_volume(); }
void JsVlcAudio::setVolume(unsigned volume) { _jsPlayer->player().audio().set_volume(volume); }
int JsVlcAudio::channel() { return _jsPlayer->player().audio().get_channel(); }
void JsVlcAudio::setChannel(unsigned channel) { _jsPlayer->player().audio().set_channel((libvlc_audio_output_channel_t) channel); }
int JsVlcAudio::delay() { return static_cast<int>(_jsPlayer->player().audio().get_delay()); }
void JsVlcAudio::setDelay(int delay) { _jsPlayer->player().audio().set_delay(delay); }
void JsVlcAudio::toggleMute() { _jsPlayer->player().audio().toggle_mute(); }

napi_value JsVlcAudio::get_description(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        return ToNapiValue(env, obj->description(FromNapiValue<uint32_t>(env, args[0])));
    }
    return nullptr;
}

napi_value JsVlcAudio::get_count(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->count());
}

napi_value JsVlcAudio::get_track(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->track());
}

napi_value JsVlcAudio::set_track(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setTrack(FromNapiValue<int32_t>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcAudio::get_muted(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->muted());
}

napi_value JsVlcAudio::set_muted(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setMuted(FromNapiValue<bool>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcAudio::get_volume(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->volume());
}

napi_value JsVlcAudio::set_volume(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setVolume(FromNapiValue<uint32_t>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcAudio::get_channel(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->channel());
}

napi_value JsVlcAudio::set_channel(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setChannel(FromNapiValue<uint32_t>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcAudio::get_delay(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->delay());
}

napi_value JsVlcAudio::set_delay(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setDelay(FromNapiValue<int32_t>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcAudio::toggle_mute(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcAudio* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    obj->toggleMute();
    return nullptr;
}
