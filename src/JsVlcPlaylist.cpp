#include "JsVlcPlaylist.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"
#include "JsVlcMedia.h"
#include "JsVlcPlaylistItems.h"

napi_ref JsVlcPlaylist::_jsConstructor = nullptr;

void JsVlcPlaylist::initJsApi(napi_env env)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("add", add_item),
        DECLARE_NAPI_METHOD("play", play_item),
        DECLARE_NAPI_METHOD("next", next),
        DECLARE_NAPI_METHOD("prev", prev),
        DECLARE_NAPI_GETTER("items", get_items),
        DECLARE_NAPI_GETTER("currentItem", get_currentItem),
        DECLARE_NAPI_PROPERTY("playbackMode", get_playbackMode, set_playbackMode),
    };

    napi_value constructor;
    napi_define_class(env, "VlcPlaylist", NAPI_AUTO_LENGTH, jsCreate, nullptr, sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_create_reference(env, constructor, 1, &_jsConstructor);
}

napi_ref JsVlcPlaylist::create(napi_env env, JsVlcPlayer& player)
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

napi_value JsVlcPlaylist::jsCreate(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);

    JsVlcPlayer* jsPlayer;
    napi_get_value_external(env, args[0], (void**)&jsPlayer);

    JsVlcPlaylist* obj = new JsVlcPlaylist(jsPlayer);
    napi_wrap(env, this_arg, obj, jsFinalize, nullptr, &obj->_wrapper);

    return this_arg;
}

void JsVlcPlaylist::jsFinalize(napi_env env, void* data, void* hint) {
    delete static_cast<JsVlcPlaylist*>(data);
}

JsVlcPlaylist::JsVlcPlaylist(JsVlcPlayer* jsPlayer) :
    _jsPlayer(jsPlayer), _wrapper(nullptr)
{
    _jsPlaylistItemsRef = JsVlcPlaylistItems::create(_jsPlayer->getEnv(), *_jsPlayer);
}

JsVlcPlaylist::~JsVlcPlaylist() {
    if (_wrapper) {
        napi_delete_reference(_jsPlayer->getEnv(), _wrapper);
    }
    if (_jsPlaylistItemsRef) {
        napi_delete_reference(_jsPlayer->getEnv(), _jsPlaylistItemsRef);
    }
}

int JsVlcPlaylist::add(const std::string& mrl, const std::vector<std::string>& options)
{
    libvlc_media_t* libvlc_media = libvlc_media_new_location(_jsPlayer->get_instance(), mrl.c_str());
    for (const auto& opt : options) {
        libvlc_media_add_option(libvlc_media, opt.c_str());
    }
    vlc::media media(libvlc_media, true);
    return _jsPlayer->player().add_media(media);
}

bool JsVlcPlaylist::play(unsigned index)
{
    return _jsPlayer->player().play(index);
}

void JsVlcPlaylist::next()
{
    _jsPlayer->player().next();
}

void JsVlcPlaylist::prev()
{
    _jsPlayer->player().prev();
}

napi_value JsVlcPlaylist::items(napi_env env)
{
    napi_value items_obj;
    napi_get_reference_value(env, _jsPlaylistItemsRef, &items_obj);
    return items_obj;
}

int JsVlcPlaylist::currentItem()
{
    return _jsPlayer->player().current_item();
}

std::string JsVlcPlaylist::playbackMode()
{
    switch (_jsPlayer->player().get_playback_mode())
    {
    case vlc::mode_loop: return "loop";
    case vlc::mode_single: return "repeat";
    default: return "default";
    }
}

void JsVlcPlaylist::setPlaybackMode(const std::string& mode)
{
    if (mode == "loop") {
        _jsPlayer->player().set_playback_mode(vlc::mode_loop);
    } else if (mode == "repeat") {
        _jsPlayer->player().set_playback_mode(vlc::mode_single);
    } else {
        _jsPlayer->player().set_playback_mode(vlc::mode_normal);
    }
}

napi_value JsVlcPlaylist::add_item(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcPlaylist* obj;
    napi_unwrap(env, this_arg, (void**)&obj);

    if (argc > 0) {
        std::string mrl = FromNapiValue<std::string>(env, args[0]);
        std::vector<std::string> options;
        if (argc > 1) {
            options = FromNapiValue<std::vector<std::string>>(env, args[1]);
        }
        return ToNapiValue(env, obj->add(mrl, options));
    }
    return nullptr;
}

napi_value JsVlcPlaylist::play_item(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcPlaylist* obj;
    napi_unwrap(env, this_arg, (void**)&obj);

    if (argc > 0) {
        return ToNapiValue(env, obj->play(FromNapiValue<uint32_t>(env, args[0])));
    }
    return nullptr;
}

napi_value JsVlcPlaylist::next(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcPlaylist* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    obj->next();
    return nullptr;
}

napi_value JsVlcPlaylist::prev(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcPlaylist* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    obj->prev();
    return nullptr;
}

napi_value JsVlcPlaylist::get_items(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcPlaylist* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return obj->items(env);
}

napi_value JsVlcPlaylist::get_currentItem(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcPlaylist* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->currentItem());
}

napi_value JsVlcPlaylist::get_playbackMode(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcPlaylist* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->playbackMode());
}

napi_value JsVlcPlaylist::set_playbackMode(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcPlaylist* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setPlaybackMode(FromNapiValue<std::string>(env, args[0]));
    }
    return nullptr;
}
