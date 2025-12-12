#include "JsVlcMedia.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"

napi_ref JsVlcMedia::_jsConstructor = nullptr;

void JsVlcMedia::initJsApi(napi_env env)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_GETTER("artist", get_artist),
        DECLARE_NAPI_GETTER("genre", get_genre),
        DECLARE_NAPI_GETTER("copyright", get_copyright),
        DECLARE_NAPI_GETTER("album", get_album),
        DECLARE_NAPI_GETTER("trackNumber", get_trackNumber),
        DECLARE_NAPI_GETTER("description", get_description),
        DECLARE_NAPI_GETTER("rating", get_rating),
        DECLARE_NAPI_GETTER("date", get_date),
        DECLARE_NAPI_GETTER("URL", get_URL),
        DECLARE_NAPI_GETTER("language", get_language),
        DECLARE_NAPI_GETTER("nowPlaying", get_nowPlaying),
        DECLARE_NAPI_GETTER("publisher", get_publisher),
        DECLARE_NAPI_GETTER("encodedBy", get_encodedBy),
        DECLARE_NAPI_GETTER("artworkURL", get_artworkURL),
        DECLARE_NAPI_GETTER("trackID", get_trackID),
        DECLARE_NAPI_GETTER("mrl", get_mrl),
        DECLARE_NAPI_GETTER("parsed", get_parsed),
        DECLARE_NAPI_GETTER("duration", get_duration),
        DECLARE_NAPI_PROPERTY("title", get_title, set_title),
        DECLARE_NAPI_PROPERTY("setting", get_setting, set_setting),
        DECLARE_NAPI_PROPERTY("disabled", get_disabled, set_disabled),
        DECLARE_NAPI_METHOD("parse", parse),
        DECLARE_NAPI_METHOD("parseAsync", parseAsync),
    };

    napi_value constructor;
    napi_define_class(env, "JsVlcMedia", NAPI_AUTO_LENGTH, jsCreate, nullptr, sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_create_reference(env, constructor, 1, &_jsConstructor);
}

napi_value JsVlcMedia::create(napi_env env, JsVlcPlayer& player, const vlc::media& media)
{
    napi_value cons;
    napi_get_reference_value(env, _jsConstructor, &cons);

    napi_value argv[2];
    napi_create_external(env, &player, nullptr, nullptr, &argv[0]);
    napi_create_external(env, (void*)&media, nullptr, nullptr, &argv[1]);

    napi_value instance;
    napi_new_instance(env, cons, 2, argv, &instance);
    return instance;
}

napi_value JsVlcMedia::jsCreate(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);

    JsVlcPlayer* jsPlayer;
    napi_get_value_external(env, args[0], (void**)&jsPlayer);

    vlc::media* media;
    napi_get_value_external(env, args[1], (void**)&media);

    if (jsPlayer && media) {
        JsVlcMedia* obj = new JsVlcMedia(jsPlayer, *media);
        napi_wrap(env, this_arg, obj, jsFinalize, nullptr, &obj->_wrapper);
    }

    return this_arg;
}

void JsVlcMedia::jsFinalize(napi_env env, void* data, void* hint) {
    delete static_cast<JsVlcMedia*>(data);
}

JsVlcMedia::JsVlcMedia(JsVlcPlayer* jsPlayer, const vlc::media& media) :
    _jsPlayer(jsPlayer), _media(media), _wrapper(nullptr)
{
}

JsVlcMedia::~JsVlcMedia() {
    if (_wrapper) {
        napi_env env = _jsPlayer->getEnv();
        napi_delete_reference(env, _wrapper);
    }
}

// Implementation of all getters, setters, and methods using N-API wrappers

std::string JsVlcMedia::meta(libvlc_meta_t e_meta) { return get_media().meta(e_meta); }
void JsVlcMedia::setMeta(libvlc_meta_t e_meta, const std::string& meta) { get_media().set_meta(e_meta, meta); }

#define MEDIA_META_GETTER(name, meta_enum) \
    std::string JsVlcMedia::name() { return meta(meta_enum); } \
    napi_value JsVlcMedia::get_##name(napi_env env, napi_callback_info info) { \
        napi_value this_arg; \
        napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr); \
        JsVlcMedia* obj; \
        napi_unwrap(env, this_arg, (void**)&obj); \
        return ToNapiValue(env, obj->name()); \
    }

MEDIA_META_GETTER(artist, libvlc_meta_Artist)
MEDIA_META_GETTER(genre, libvlc_meta_Genre)
MEDIA_META_GETTER(copyright, libvlc_meta_Copyright)
MEDIA_META_GETTER(album, libvlc_meta_Album)
MEDIA_META_GETTER(trackNumber, libvlc_meta_TrackNumber)
MEDIA_META_GETTER(description, libvlc_meta_Description)
MEDIA_META_GETTER(rating, libvlc_meta_Rating)
MEDIA_META_GETTER(date, libvlc_meta_Date)
MEDIA_META_GETTER(URL, libvlc_meta_URL)
MEDIA_META_GETTER(language, libvlc_meta_Language)
MEDIA_META_GETTER(nowPlaying, libvlc_meta_NowPlaying)
MEDIA_META_GETTER(publisher, libvlc_meta_Publisher)
MEDIA_META_GETTER(encodedBy, libvlc_meta_EncodedBy)
MEDIA_META_GETTER(artworkURL, libvlc_meta_ArtworkURL)
MEDIA_META_GETTER(trackID, libvlc_meta_TrackID)

std::string JsVlcMedia::mrl() { return get_media().mrl(); }
napi_value JsVlcMedia::get_mrl(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcMedia* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->mrl());
}

bool JsVlcMedia::parsed() { return get_media().is_parsed(); }
napi_value JsVlcMedia::get_parsed(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcMedia* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->parsed());
}

double JsVlcMedia::duration() { return static_cast<double>(_media.duration()); }
napi_value JsVlcMedia::get_duration(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcMedia* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->duration());
}

void JsVlcMedia::parse() { get_media().parse(); }
napi_value JsVlcMedia::parse(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcMedia* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    obj->parse();
    return nullptr;
}

void JsVlcMedia::parseAsync() { get_media().parse(true); }
napi_value JsVlcMedia::parseAsync(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcMedia* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    obj->parseAsync();
    return nullptr;
}

std::string JsVlcMedia::title() { return meta(libvlc_meta_Title); }
void JsVlcMedia::setTitle(const std::string& title) { setMeta(libvlc_meta_Title, title); }
napi_value JsVlcMedia::get_title(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcMedia* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->title());
}
napi_value JsVlcMedia::set_title(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcMedia* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setTitle(FromNapiValue<std::string>(env, args[0]));
    }
    return nullptr;
}

std::string JsVlcMedia::setting() {
    vlc::player& p = _jsPlayer->player();
    int idx = p.find_media_index(get_media());
    return (idx >= 0) ? p.get_item_data(idx) : std::string();
}
void JsVlcMedia::setSetting(const std::string& setting) {
    vlc::player& p = _jsPlayer->player();
    int idx = p.find_media_index(get_media());
    if (idx >= 0) {
        p.set_item_data(idx, setting);
    }
}
napi_value JsVlcMedia::get_setting(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcMedia* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->setting());
}
napi_value JsVlcMedia::set_setting(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcMedia* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setSetting(FromNapiValue<std::string>(env, args[0]));
    }
    return nullptr;
}

bool JsVlcMedia::disabled() {
    vlc::player& p = _jsPlayer->player();
    int idx = p.find_media_index(get_media());
    return (idx < 0) ? false : p.is_item_disabled(idx);
}
void JsVlcMedia::setDisabled(bool disabled) {
    vlc::player& p = _jsPlayer->player();
    int idx = p.find_media_index(get_media());
    if (idx >= 0) {
        p.disable_item(idx, disabled);
    }
}
napi_value JsVlcMedia::get_disabled(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcMedia* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    return ToNapiValue(env, obj->disabled());
}
napi_value JsVlcMedia::set_disabled(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcMedia* obj;
    napi_unwrap(env, this_arg, (void**)&obj);
    if (argc > 0) {
        obj->setDisabled(FromNapiValue<bool>(env, args[0]));
    }
    return nullptr;
}