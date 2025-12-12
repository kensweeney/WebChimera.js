#include "JsVlcDeinterlace.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"

napi_ref JsVlcDeinterlace::_jsConstructor = nullptr;

void JsVlcDeinterlace::initJsApi(napi_env env)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("enable", enable),
        DECLARE_NAPI_METHOD("disable", disable),
    };

    napi_value constructor;
    napi_define_class(env, "VlcDeinterlace", NAPI_AUTO_LENGTH, jsCreate, nullptr, sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_create_reference(env, constructor, 1, &_jsConstructor);
}

napi_ref JsVlcDeinterlace::create(JsVlcPlayer& player, napi_env env)
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

napi_value JsVlcDeinterlace::jsCreate(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);

    JsVlcPlayer* jsPlayer;
    napi_get_value_external(env, args[0], (void**)&jsPlayer);

    JsVlcDeinterlace* obj = new JsVlcDeinterlace(jsPlayer);
    napi_wrap(env, this_arg, obj, jsFinalize, nullptr, &obj->_wrapper);

    return this_arg;
}

void JsVlcDeinterlace::jsFinalize(napi_env env, void* data, void* hint) {
    delete static_cast<JsVlcDeinterlace*>(data);
}

JsVlcDeinterlace::JsVlcDeinterlace(JsVlcPlayer* jsPlayer) :
    _jsPlayer(jsPlayer), _wrapper(nullptr)
{
}

void JsVlcDeinterlace::enable(const std::string& mode)
{
    libvlc_video_set_deinterlace(_jsPlayer->player().get_mp(), mode.c_str());
}

void JsVlcDeinterlace::disable()
{
    libvlc_video_set_deinterlace(_jsPlayer->player().get_mp(), nullptr);
}

napi_value JsVlcDeinterlace::enable(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);

    JsVlcDeinterlace* obj;
    napi_unwrap(env, this_arg, (void**)&obj);

    if (argc > 0) {
        obj->enable(FromNapiValue<std::string>(env, args[0]));
    }
    return nullptr;
}

napi_value JsVlcDeinterlace::disable(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);

    JsVlcDeinterlace* obj;
    napi_unwrap(env, this_arg, (void**)&obj);

    obj->disable();
    return nullptr;
}
