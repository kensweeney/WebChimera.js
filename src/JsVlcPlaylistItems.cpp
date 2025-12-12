#include "JsVlcPlaylistItems.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"
#include "JsVlcMedia.h"

napi_ref JsVlcPlaylistItems::_jsConstructor = nullptr;

void JsVlcPlaylistItems::initJsApi(napi_env env)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_GETTER("count", get_count),
        DECLARE_NAPI_METHOD("clear", clear_items),
        DECLARE_NAPI_METHOD("remove", remove_item),
        DECLARE_NAPI_METHOD("item", get_item),
    };

    napi_value constructor;
    napi_define_class(env, "VlcPlaylistItems", NAPI_AUTO_LENGTH, jsCreate, nullptr, sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_create_reference(env, constructor, 1, &_jsConstructor);
}

napi_ref JsVlcPlaylistItems::create(napi_env env, JsVlcPlayer& player)
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

napi_value JsVlcPlaylistItems::jsCreate(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);

    JsVlcPlayer* jsPlayer;
    napi_get_value_external(env, args[0], (void**)&jsPlayer);

    JsVlcPlaylistItems* obj = new JsVlcPlaylistItems(jsPlayer);
    napi_wrap(env, this_arg, obj, jsFinalize, nullptr, &obj->_wrapper);

    return this_arg;
}

void JsVlcPlaylistItems::jsFinalize(napi_env env, void* data, void* hint) {
    delete static_cast<JsVlcPlaylistItems*>(data);
}

JsVlcPlaylistItems::JsVlcPlaylistItems(JsVlcPlayer* jsPlayer) :
    _jsPlayer(jsPlayer), _wrapper(nullptr)
{
}

JsVlcPlaylistItems::~JsVlcPlaylistItems() {
    if (_wrapper) {
        napi_delete_reference(_jsPlayer->getEnv(), _wrapper);
    }
}

napi_value JsVlcPlaylistItems::item(napi_env env, uint32_t index)
{
    return JsVlcMedia::create(env, *_jsPlayer, _jsPlayer->player().get_media(index));
}

unsigned JsVlcPlaylistItems::count()
{
    return _jsPlayer->player().item_count();
}

void JsVlcPlaylistItems::clear()
{
    return _jsPlayer->player().clear_items();
}

bool JsVlcPlaylistItems::remove(unsigned int idx)
{
   return _jsPlayer->player().delete_item(idx);
}

napi_value JsVlcPlaylistItems::get_item(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);

    JsVlcPlaylistItems* obj;
    napi_unwrap(env, this_arg, (void**)&obj);

    if (argc > 0) {
        uint32_t index = FromNapiValue<uint32_t>(env, args[0]);
        if (index < obj->count()) {
            return obj->item(env, index);
        }
    }

    return nullptr;
}

napi_value JsVlcPlaylistItems::get_count(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcPlaylistItems* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->count());
}

napi_value JsVlcPlaylistItems::clear_items(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcPlaylistItems* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    obj->clear();
    return nullptr;
}

napi_value JsVlcPlaylistItems::remove_item(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcPlaylistItems* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        return ToNapiValue(env, obj->remove(FromNapiValue<uint32_t>(env, args[0])));
    }
    return nullptr;
}
