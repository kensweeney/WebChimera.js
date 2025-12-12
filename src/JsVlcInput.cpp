#include "JsVlcInput.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"
#include "JsVlcDeinterlace.h"

napi_ref JsVlcInput::_jsConstructor = nullptr;

void JsVlcInput::initJsApi(napi_env env)
{
    JsVlcDeinterlace::initJsApi(env);

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_GETTER("length", get_length),
        DECLARE_NAPI_GETTER("fps", get_fps),
        DECLARE_NAPI_GETTER("state", get_state),
        DECLARE_NAPI_GETTER("hasVout", get_hasVout),
        DECLARE_NAPI_PROPERTY("position", get_position, set_position),
        DECLARE_NAPI_PROPERTY("time", get_time, set_time),
        DECLARE_NAPI_PROPERTY("rate", get_rate, set_rate),
    };

    napi_value constructor;
    napi_define_class(env, "VlcInput", NAPI_AUTO_LENGTH, jsCreate, nullptr, sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_create_reference(env, constructor, 1, &_jsConstructor);
}

napi_ref JsVlcInput::create(JsVlcPlayer& player, napi_env env)
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

napi_value JsVlcInput::jsCreate(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);

    JsVlcPlayer* jsPlayer;
    napi_get_value_external(env, args[0], (void**)&jsPlayer);

    JsVlcInput* obj = new JsVlcInput(jsPlayer);
    napi_wrap(env, this_arg, obj, jsFinalize, nullptr, &obj->_wrapper);

    return this_arg;
}

void JsVlcInput::jsFinalize(napi_env env, void* data, void* hint) {
    delete static_cast<JsVlcInput*>(data);
}

JsVlcInput::JsVlcInput(JsVlcPlayer* jsPlayer) :
    _jsPlayer(jsPlayer), _wrapper(nullptr)
{
}

double JsVlcInput::length()
{
    return static_cast<double>(_jsPlayer->player().playback().get_length());
}

double JsVlcInput::fps()
{
    return _jsPlayer->player().playback().get_fps();
}

unsigned JsVlcInput::state()
{
    return _jsPlayer->player().get_state();
}

bool JsVlcInput::hasVout()
{
    return _jsPlayer->player().video().has_vout();
}

double JsVlcInput::position()
{
    return _jsPlayer->player().playback().get_position();
}

void JsVlcInput::setPosition(double position)
{
    _jsPlayer->player().playback().set_position(static_cast<float>(position));
}

double JsVlcInput::time()
{
    return static_cast<double>(_jsPlayer->player().playback().get_time());
}

void JsVlcInput::setTime(double time)
{
    _jsPlayer->player().playback().set_time(static_cast<libvlc_time_t>(time));
}

double JsVlcInput::rate()
{
    return _jsPlayer->player().playback().get_rate();
}

void JsVlcInput::setRate(double rate)
{
    _jsPlayer->player().playback().set_rate(static_cast<float>(rate));
}

napi_value JsVlcInput::get_length(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcInput* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->length());
}

napi_value JsVlcInput::get_fps(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcInput* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->fps());
}

napi_value JsVlcInput::get_state(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcInput* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->state());
}

napi_value JsVlcInput::get_hasVout(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcInput* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->hasVout());
}

napi_value JsVlcInput::get_position(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcInput* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->position());
}

napi_value JsVlcInput::set_position(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcInput* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setPosition(FromNapiValue<double>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcInput::get_time(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcInput* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->time());
}

napi_value JsVlcInput::set_time(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcInput* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setTime(FromNapiValue<double>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcInput::get_rate(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcInput* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->rate());
}

napi_value JsVlcInput::set_rate(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcInput* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setRate(FromNapiValue<double>(env, args[0]));
    }
    return nullptr;
}

