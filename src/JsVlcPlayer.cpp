#include "JsVlcPlayer.h"

#include <string.h>
#include <vector>
#include <windows.h>

#include "NodeTools.h"
#include "JsVlcInput.h"
#include "JsVlcAudio.h"
#include "JsVlcVideo.h"
#include "JsVlcSubtitles.h"
#include "JsVlcPlaylist.h"

#undef min
#undef max

const char* JsVlcPlayer::callbackNames[] =
{
    "FrameSetup", "FrameReady", "FrameCleanup",
    "MediaChanged", "NothingSpecial", "Opening", "Buffering", "Playing", "Paused", "Stopped", "Forward", "Backward", "EndReached", "EncounteredError",
    "TimeChanged", "PositionChanged", "SeekableChanged", "PausableChanged", "LengthChanged",
    "LogMessage"
};

napi_ref JsVlcPlayer::_jsConstructor = nullptr;

struct JsVlcPlayer::ContextData
{
    ContextData(napi_env env, napi_value thisModule) {
        napi_create_reference(env, thisModule, 1, &thisModuleRef);
    }
    ~ContextData();

    napi_ref thisModuleRef;
    std::set<JsVlcPlayer*> instances;
};

JsVlcPlayer::ContextData::~ContextData()
{
    for(JsVlcPlayer* p : instances) {
        p->close();
    }
}

struct JsVlcPlayer::AsyncData
{
    virtual void process(JsVlcPlayer*) = 0;
    virtual ~AsyncData() = default;
};

struct JsVlcPlayer::CallbackData : public JsVlcPlayer::AsyncData
{
    CallbackData(JsVlcPlayer::Callbacks_e callback) : callback(callback) {}
    void process(JsVlcPlayer* jsPlayer) override {
        jsPlayer->callCallback(callback);
    }
    const JsVlcPlayer::Callbacks_e callback;
};

struct JsVlcPlayer::LibvlcEvent : public JsVlcPlayer::AsyncData
{
    LibvlcEvent(const libvlc_event_t& libvlcEvent) : libvlcEvent(libvlcEvent) {}
    void process(JsVlcPlayer* jsPlayer) override {
        jsPlayer->handleLibvlcEvent(libvlcEvent);
    }
    const libvlc_event_t libvlcEvent;
};

struct JsVlcPlayer::LibvlcLogEvent : public JsVlcPlayer::AsyncData
{
    LibvlcLogEvent(int level, const std::string& message, const std::string& format) :
        level(level), message(message), format(format) {}

    void process(JsVlcPlayer* jsPlayer) override {
        napi_env env = jsPlayer->_env;
        napi_value jsLevel, jsMessage, jsFormat;
        napi_create_int32(env, level, &jsLevel);
        napi_create_string_utf8(env, message.c_str(), NAPI_AUTO_LENGTH, &jsMessage);
        napi_create_string_utf8(env, format.c_str(), NAPI_AUTO_LENGTH, &jsFormat);
        jsPlayer->callCallback(CB_LogMessage, { jsLevel, jsMessage, jsFormat });
    }

    const int level;
    std::string message;
    std::string format;
};

#define NAPI_METHOD(name, func) \
    { name, nullptr, func, nullptr, nullptr, nullptr, napi_default, nullptr }

#define NAPI_GETTER(name, getter) \
    { name, nullptr, nullptr, getter, nullptr, nullptr, napi_default, nullptr }

#define NAPI_PROPERTY(name, getter, setter) \
    { name, nullptr, nullptr, getter, setter, nullptr, napi_default, nullptr }

napi_value JsVlcPlayer::initJsApi(napi_env env, napi_value exports)
{
    ContextData* contextData = new ContextData(env, exports);
    napi_add_env_cleanup_hook(env, [](void* arg) { delete static_cast<ContextData*>(arg); }, contextData);

    JsVlcInput::initJsApi(env);
    JsVlcAudio::initJsApi(env);
    JsVlcVideo::initJsApi(env);
    JsVlcSubtitles::initJsApi(env);
    JsVlcPlaylist::initJsApi(env);

    napi_property_descriptor properties[] = {
        NAPI_METHOD("play", jsPlay),
        NAPI_METHOD("pause", [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value this_arg;
            napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
            JsVlcPlayer* p;
            napi_unwrap(env, this_arg, (void**)&p);
            p->pause();
            return nullptr;
        }),
        NAPI_METHOD("togglePause", [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value this_arg;
            napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
            JsVlcPlayer* p;
            napi_unwrap(env, this_arg, (void**)&p);
            p->togglePause();
            return nullptr;
        }),
        NAPI_METHOD("stop", [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value this_arg;
            napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
            JsVlcPlayer* p;
            napi_unwrap(env, this_arg, (void**)&p);
            p->stop();
            return nullptr;
        }),
        NAPI_METHOD("toggleMute", [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value this_arg;
            napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
            JsVlcPlayer* p;
            napi_unwrap(env, this_arg, (void**)&p);
            p->toggleMute();
            return nullptr;
        }),
        NAPI_METHOD("close", [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value this_arg;
            napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
            JsVlcPlayer* p;
            napi_unwrap(env, this_arg, (void**)&p);
            p->close();
            return nullptr;
        }),
        NAPI_PROPERTY("onFrameSetup", JsVlcPlayer::getJsCallback<CB_FrameSetup>, JsVlcPlayer::setJsCallback<CB_FrameSetup>),
        NAPI_PROPERTY("onFrameReady", JsVlcPlayer::getJsCallback<CB_FrameReady>, JsVlcPlayer::setJsCallback<CB_FrameReady>),
        NAPI_PROPERTY("onFrameCleanup", JsVlcPlayer::getJsCallback<CB_FrameCleanup>, JsVlcPlayer::setJsCallback<CB_FrameCleanup>),
        NAPI_PROPERTY("onMediaChanged", JsVlcPlayer::getJsCallback<CB_MediaPlayerMediaChanged>, JsVlcPlayer::setJsCallback<CB_MediaPlayerMediaChanged>),
        NAPI_PROPERTY("onNothingSpecial", JsVlcPlayer::getJsCallback<CB_MediaPlayerNothingSpecial>, JsVlcPlayer::setJsCallback<CB_MediaPlayerNothingSpecial>),
        NAPI_PROPERTY("onOpening", JsVlcPlayer::getJsCallback<CB_MediaPlayerOpening>, JsVlcPlayer::setJsCallback<CB_MediaPlayerOpening>),
        NAPI_PROPERTY("onBuffering", JsVlcPlayer::getJsCallback<CB_MediaPlayerBuffering>, JsVlcPlayer::setJsCallback<CB_MediaPlayerBuffering>),
        NAPI_PROPERTY("onPlaying", JsVlcPlayer::getJsCallback<CB_MediaPlayerPlaying>, JsVlcPlayer::setJsCallback<CB_MediaPlayerPlaying>),
        NAPI_PROPERTY("onPaused", JsVlcPlayer::getJsCallback<CB_MediaPlayerPaused>, JsVlcPlayer::setJsCallback<CB_MediaPlayerPaused>),
        NAPI_PROPERTY("onForward", JsVlcPlayer::getJsCallback<CB_MediaPlayerForward>, JsVlcPlayer::setJsCallback<CB_MediaPlayerForward>),
        NAPI_PROPERTY("onBackward", JsVlcPlayer::getJsCallback<CB_MediaPlayerBackward>, JsVlcPlayer::setJsCallback<CB_MediaPlayerBackward>),
        NAPI_PROPERTY("onEncounteredError", JsVlcPlayer::getJsCallback<CB_MediaPlayerEncounteredError>, JsVlcPlayer::setJsCallback<CB_MediaPlayerEncounteredError>),
        NAPI_PROPERTY("onEndReached", JsVlcPlayer::getJsCallback<CB_MediaPlayerEndReached>, JsVlcPlayer::setJsCallback<CB_MediaPlayerEndReached>),
        NAPI_PROPERTY("onStopped", JsVlcPlayer::getJsCallback<CB_MediaPlayerStopped>, JsVlcPlayer::setJsCallback<CB_MediaPlayerStopped>),
        NAPI_PROPERTY("onTimeChanged", JsVlcPlayer::getJsCallback<CB_MediaPlayerTimeChanged>, JsVlcPlayer::setJsCallback<CB_MediaPlayerTimeChanged>),
        NAPI_PROPERTY("onPositionChanged", JsVlcPlayer::getJsCallback<CB_MediaPlayerPositionChanged>, JsVlcPlayer::setJsCallback<CB_MediaPlayerPositionChanged>),
        NAPI_PROPERTY("onSeekableChanged", JsVlcPlayer::getJsCallback<CB_MediaPlayerSeekableChanged>, JsVlcPlayer::setJsCallback<CB_MediaPlayerSeekableChanged>),
        NAPI_PROPERTY("onPausableChanged", JsVlcPlayer::getJsCallback<CB_MediaPlayerPausableChanged>, JsVlcPlayer::setJsCallback<CB_MediaPlayerPausableChanged>),
        NAPI_PROPERTY("onLengthChanged", JsVlcPlayer::getJsCallback<CB_MediaPlayerLengthChanged>, JsVlcPlayer::setJsCallback<CB_MediaPlayerLengthChanged>),
        NAPI_PROPERTY("onLogMessage", JsVlcPlayer::getJsCallback<CB_LogMessage>, JsVlcPlayer::setJsCallback<CB_LogMessage>)
    };

    napi_value constructor;
    napi_define_class(env, "VlcPlayer", NAPI_AUTO_LENGTH, jsCreate, contextData, sizeof(properties)/sizeof(properties[0]), properties, &constructor);
    napi_create_reference(env, constructor, 1, &_jsConstructor);
    napi_set_named_property(env, exports, "VlcPlayer", constructor);
    napi_set_named_property(env, exports, "createPlayer", constructor);

    // ... set other exports like vlcVersion
    return exports;
}

napi_value JsVlcPlayer::jsCreate(napi_env env, napi_callback_info info) {
    napi_value target;
    napi_get_new_target(env, info, &target);
    if (target) {
        napi_value this_arg;
        napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
        JsVlcPlayer* player = new JsVlcPlayer(env, info);
        napi_wrap(env, this_arg, player, jsFinalize, nullptr, &player->_wrapper);
        return this_arg;
    } else {
        // Invoked as plain function `VlcPlayer(...)`, turn into construct call.
        size_t argc = 1;
        napi_value args[1];
        napi_value this_arg;
        napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);

        napi_value cons;
        napi_get_reference_value(env, _jsConstructor, &cons);
        
        napi_value instance;
        napi_new_instance(env, cons, argc, args, &instance);
        return instance;
    }
}

void JsVlcPlayer::jsFinalize(napi_env env, void* data, void* hint) {
    delete static_cast<JsVlcPlayer*>(data);
}

JsVlcPlayer::JsVlcPlayer(napi_env env, napi_callback_info info) : _env(env), _libvlc(nullptr) {
    _wrapper = nullptr;
    for(int i=0; i<CB_Max; ++i) _jsCallbacks[i] = nullptr;
    _jsEventEmitterRef = nullptr;
    _jsFrameBufferRef = nullptr;
    _jsInputRef = nullptr;
    _jsAudioRef = nullptr;
    _jsVideoRef = nullptr;
    _jsSubtitlesRef = nullptr;
    _jsPlaylistRef = nullptr;

    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    void* data;
    napi_get_cb_info(env, info, &argc, args, &this_arg, &data);
    _contextData = static_cast<ContextData*>(data);
    _contextData->instances.insert(this);

    uv_loop_t* loop;
    napi_get_uv_event_loop(env, &loop);

    uv_async_init(loop, &_async, [](uv_async_t* handle) {
        if(handle->data) reinterpret_cast<JsVlcPlayer*>(handle->data)->handleAsync();
    });
    _async.data = this;

    uv_timer_init(loop, &_errorTimer);
    _errorTimer.data = this;

    napi_value eventEmitter = Require(env, "events");
    napi_value eventEmitterConstructor;
    napi_get_named_property(env, eventEmitter, "EventEmitter", &eventEmitterConstructor);
    napi_value eventEmitterInstance;
    napi_new_instance(env, eventEmitterConstructor, 0, nullptr, &eventEmitterInstance);
    napi_create_reference(env, eventEmitterInstance, 1, &_jsEventEmitterRef);

    initLibvlc(env, argc > 0 ? args[0] : nullptr);

    _player.set_playback_mode(vlc::mode_normal);

    if(_libvlc && _player.open(_libvlc)) {
        _player.register_callback(this);
        VlcVideoOutput::open(&_player.basic_player());
    }

    _jsInputRef = JsVlcInput::create(*this, env);
    _jsAudioRef = JsVlcAudio::create(env, *this);
    _jsVideoRef = JsVlcVideo::create(*this, env);
    _jsSubtitlesRef = JsVlcSubtitles::create(*this, env);
    _jsPlaylistRef = JsVlcPlaylist::create(env, *this);
}

void JsVlcPlayer::initLibvlc(napi_env env, napi_value vlcOpts) {
    if(_libvlc) {
        libvlc_release(_libvlc);
        _libvlc = nullptr;
    }

    bool is_array = false;
    if (vlcOpts) {
        napi_is_array(env, vlcOpts, &is_array);
    }

    if(vlcOpts && is_array) {
        uint32_t length;
        napi_get_array_length(env, vlcOpts, &length);
        std::vector<std::string> opts_storage;
        std::vector<const char*> libvlcOpts;
        opts_storage.reserve(length);
        libvlcOpts.reserve(length);

        for(uint32_t i = 0; i < length; ++i) {
            napi_value opt_val;
            napi_get_element(env, vlcOpts, i, &opt_val);
            opts_storage.push_back(FromNapiValue<std::string>(env, opt_val));
            libvlcOpts.push_back(opts_storage.back().c_str());
        }
        _libvlc = libvlc_new(libvlcOpts.size(), libvlcOpts.data());
    } else {
        _libvlc = libvlc_new(0, nullptr);
    }

    if(_libvlc) {
        libvlc_log_set(_libvlc, JsVlcPlayer::log_event_wrapper, this);
    }
}

JsVlcPlayer::~JsVlcPlayer() {
    close();
    _contextData->instances.erase(this);
    for(int i=0; i<CB_Max; ++i) {
        if (_jsCallbacks[i]) napi_delete_reference(_env, _jsCallbacks[i]);
    }
    if (_jsEventEmitterRef) napi_delete_reference(_env, _jsEventEmitterRef);
    if (_jsFrameBufferRef) napi_delete_reference(_env, _jsFrameBufferRef);
    if (_jsInputRef) napi_delete_reference(_env, _jsInputRef);
    if (_jsAudioRef) napi_delete_reference(_env, _jsAudioRef);
    if (_jsVideoRef) napi_delete_reference(_env, _jsVideoRef);
    if (_jsSubtitlesRef) napi_delete_reference(_env, _jsSubtitlesRef);
    if (_jsPlaylistRef) napi_delete_reference(_env, _jsPlaylistRef);
}

void JsVlcPlayer::close() {
    _player.unregister_callback(this);
    VlcVideoOutput::close();
    _player.close();

    _async.data = nullptr;
    uv_close(reinterpret_cast<uv_handle_t*>(&_async), 0);

    _errorTimer.data = nullptr;
    uv_timer_stop(&_errorTimer);

    if(_libvlc) {
        libvlc_release(_libvlc);
        _libvlc = nullptr;
    }
}

void JsVlcPlayer::media_player_event(const libvlc_event_t* e) {
    std::lock_guard<std::mutex> lock(_asyncDataGuard);
    _asyncData.emplace_back(new LibvlcEvent(*e));
    uv_async_send(&_async);
}

void JsVlcPlayer::log_event_wrapper(void *data, int level, const libvlc_log_t* ctx, const char* fmt, va_list args) {
    ((JsVlcPlayer *)data)->log_event(level, ctx, fmt, args);
}

#ifndef _MSC_VER
inline int _vscprintf(const char* format, va_list argptr) {
    return vsnprintf(nullptr, 0, format, argptr);
}
#endif

void JsVlcPlayer::log_event(int level, const libvlc_log_t* ctx, const char* fmt, va_list args) {
    va_list argsCopy;
    va_copy(argsCopy, args);
    int messageSize = _vscprintf(fmt, argsCopy);
    va_end(argsCopy);

    if(messageSize <= 0) return;

    std::string message(messageSize + 1, '\0');
    vsnprintf(&message[0], message.size(), fmt, args);
    message.resize(strlen(message.c_str()));

    std::lock_guard<std::mutex> lock(_asyncDataGuard);
    _asyncData.emplace_back(new LibvlcLogEvent(level, message, fmt));
    uv_async_send(&_async);
}

void JsVlcPlayer::handleAsync() {
    std::deque<std::unique_ptr<AsyncData>> tmpData;
    {
        std::lock_guard<std::mutex> lock(_asyncDataGuard);
        _asyncData.swap(tmpData);
    }
    for(const auto& i: tmpData) {
        i->process(this);
        if(VlcVideoOutput::isFrameReady()) {
            onFrameReady();
        }
    }
}

void* JsVlcPlayer::onFrameSetup(const RV32VideoFrame& videoFrame) {
    if(0 == videoFrame.width() || 0 == videoFrame.height() || 0 == videoFrame.size()) return nullptr;

    void* buffer_data;
    napi_value array_buffer;
    napi_create_arraybuffer(_env, videoFrame.size(), &buffer_data, &array_buffer);

    napi_value typed_array;
    napi_create_typedarray(_env, napi_uint8_array, videoFrame.size(), array_buffer, 0, &typed_array);

    napi_set_named_property(_env, typed_array, "width", ToNapiValue(_env, (int32_t)videoFrame.width()));
    napi_set_named_property(_env, typed_array, "height", ToNapiValue(_env, (int32_t)videoFrame.height()));
    napi_set_named_property(_env, typed_array, "pixelFormat", ToNapiValue(_env, (int32_t)PixelFormat::RV32));

    if (_jsFrameBufferRef) napi_delete_reference(_env, _jsFrameBufferRef);
    napi_create_reference(_env, typed_array, 1, &_jsFrameBufferRef);

    callCallback(CB_FrameSetup, {
        ToNapiValue(_env, (int32_t)videoFrame.width()),
        ToNapiValue(_env, (int32_t)videoFrame.height()),
        ToNapiValue(_env, (int32_t)PixelFormat::RV32),
        typed_array
    });

    return buffer_data;
}

void* JsVlcPlayer::onFrameSetup(const I420VideoFrame& videoFrame) {
    if(0 == videoFrame.width() || 0 == videoFrame.height() || 0 == videoFrame.size()) return nullptr;

    void* buffer_data;
    napi_value array_buffer;
    napi_create_arraybuffer(_env, videoFrame.size(), &buffer_data, &array_buffer);

    napi_value typed_array;
    napi_create_typedarray(_env, napi_uint8_array, videoFrame.size(), array_buffer, 0, &typed_array);

    napi_set_named_property(_env, typed_array, "width", ToNapiValue(_env, (int32_t)videoFrame.width()));
    napi_set_named_property(_env, typed_array, "height", ToNapiValue(_env, (int32_t)videoFrame.height()));
    napi_set_named_property(_env, typed_array, "pixelFormat", ToNapiValue(_env, (int32_t)PixelFormat::I420));
    napi_set_named_property(_env, typed_array, "uOffset", ToNapiValue(_env, (uint32_t)videoFrame.uPlaneOffset()));
    napi_set_named_property(_env, typed_array, "vOffset", ToNapiValue(_env, (uint32_t)videoFrame.vPlaneOffset()));

    if (_jsFrameBufferRef) napi_delete_reference(_env, _jsFrameBufferRef);
    napi_create_reference(_env, typed_array, 1, &_jsFrameBufferRef);

    callCallback(CB_FrameSetup, {
        ToNapiValue(_env, (int32_t)videoFrame.width()),
        ToNapiValue(_env, (int32_t)videoFrame.height()),
        ToNapiValue(_env, (int32_t)PixelFormat::I420),
        typed_array
    });

    return buffer_data;
}

void JsVlcPlayer::onFrameReady() {
    if (!_jsFrameBufferRef) return;
    napi_value frame;
    napi_get_reference_value(_env, _jsFrameBufferRef, &frame);
    callCallback(CB_FrameReady, { frame });
}

void JsVlcPlayer::onFrameCleanup() {
    callCallback(CB_FrameCleanup);
}

void JsVlcPlayer::handleLibvlcEvent(const libvlc_event_t& libvlcEvent) {
    Callbacks_e callback = CB_Max;
    std::vector<napi_value> args;

    switch(libvlcEvent.type) {
        case libvlc_MediaPlayerMediaChanged: callback = CB_MediaPlayerMediaChanged; break;
        case libvlc_MediaPlayerNothingSpecial: callback = CB_MediaPlayerNothingSpecial; break;
        case libvlc_MediaPlayerOpening: callback = CB_MediaPlayerOpening; break;
        case libvlc_MediaPlayerBuffering:
            callback = CB_MediaPlayerBuffering;
            args.push_back(ToNapiValue(_env, (double)libvlcEvent.u.media_player_buffering.new_cache));
            break;
        case libvlc_MediaPlayerPlaying: callback = CB_MediaPlayerPlaying; break;
        case libvlc_MediaPlayerPaused: callback = CB_MediaPlayerPaused; break;
        case libvlc_MediaPlayerStopped: callback = CB_MediaPlayerStopped; break;
        case libvlc_MediaPlayerForward: callback = CB_MediaPlayerForward; break;
        case libvlc_MediaPlayerBackward: callback = CB_MediaPlayerBackward; break;
        case libvlc_MediaPlayerEndReached:
            callback = CB_MediaPlayerEndReached;
            uv_timer_stop(&_errorTimer);
            currentItemEndReached();
            break;
        case libvlc_MediaPlayerEncounteredError:
            callback = CB_MediaPlayerEncounteredError;
            uv_timer_start(&_errorTimer, [](uv_timer_t* handle) {
                if(handle->data) static_cast<JsVlcPlayer*>(handle->data)->currentItemEndReached();
            }, 1000, 0);
            break;
        case libvlc_MediaPlayerTimeChanged:
            callback = CB_MediaPlayerTimeChanged;
            args.push_back(ToNapiValue(_env, (double)libvlcEvent.u.media_player_time_changed.new_time));
            break;
        case libvlc_MediaPlayerPositionChanged:
            callback = CB_MediaPlayerPositionChanged;
            args.push_back(ToNapiValue(_env, (double)libvlcEvent.u.media_player_position_changed.new_position));
            break;
        case libvlc_MediaPlayerSeekableChanged:
            callback = CB_MediaPlayerSeekableChanged;
            args.push_back(ToNapiValue(_env, (bool)libvlcEvent.u.media_player_seekable_changed.new_seekable));
            break;
        case libvlc_MediaPlayerPausableChanged:
            callback = CB_MediaPlayerPausableChanged;
            args.push_back(ToNapiValue(_env, (bool)libvlcEvent.u.media_player_pausable_changed.new_pausable));
            break;
        case libvlc_MediaPlayerLengthChanged:
            callback = CB_MediaPlayerLengthChanged;
            args.push_back(ToNapiValue(_env, (double)libvlcEvent.u.media_player_length_changed.new_length));
            break;
    }

    if(callback != CB_Max) {
        callCallback(callback, std::initializer_list<napi_value>(args.data(), args.data() + args.size()));
    }
}

void JsVlcPlayer::currentItemEndReached() {
    if(vlc::mode_single != player().get_playback_mode())
        player().next();
}

napi_value JsVlcPlayer::jsPlay(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);

    JsVlcPlayer* jsPlayer;
    napi_unwrap(env, this_arg, (void**)&jsPlayer);

    if(argc == 0) {
        jsPlayer->play();
    } else if(argc == 1) {
        jsPlayer->play(FromNapiValue<std::string>(env, args[0]));
    }
    return nullptr;
}

template<JsVlcPlayer::Callbacks_e C>
napi_value JsVlcPlayer::getJsCallback(napi_env env, napi_callback_info info) {
    napi_value this_arg;
    napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
    JsVlcPlayer* p;
    napi_unwrap(env, this_arg, (void**)&p);
    return p->getJsCallback(env, C);
}

template<JsVlcPlayer::Callbacks_e C>
napi_value JsVlcPlayer::setJsCallback(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_value this_arg;
    napi_get_cb_info(env, info, &argc, args, &this_arg, nullptr);
    JsVlcPlayer* p;
    napi_unwrap(env, this_arg, (void**)&p);
    if (argc > 0) {
        p->setJsCallback(env, args[0], C);
    }
    return nullptr;
}

napi_value JsVlcPlayer::getJsCallback(napi_env env, Callbacks_e callback) {
    if (_jsCallbacks[callback]) {
        napi_value cb_func;
        napi_get_reference_value(env, _jsCallbacks[callback], &cb_func);
        return cb_func;
    }
    return nullptr;
}

void JsVlcPlayer::setJsCallback(napi_env env, napi_value value, Callbacks_e callback) {
    if (_jsCallbacks[callback]) {
        napi_delete_reference(env, _jsCallbacks[callback]);
        _jsCallbacks[callback] = nullptr;
    }
    napi_valuetype valuetype;
    napi_typeof(env, value, &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, value, 1, &_jsCallbacks[callback]);
    }
}

void JsVlcPlayer::callCallback(Callbacks_e callback, std::initializer_list<napi_value> list) {
    napi_value global;
    napi_get_global(_env, &global);

    std::vector<napi_value> argList;
    argList.reserve(list.size() + 1);
    argList.push_back(ToNapiValue(_env, callbackNames[callback]));
    argList.insert(argList.end(), list);

    if(_jsCallbacks[callback]) {
        napi_value cb_func;
        napi_get_reference_value(_env, _jsCallbacks[callback], &cb_func);
        napi_value this_arg;
        napi_get_reference_value(_env, _wrapper, &this_arg);
        napi_call_function(_env, this_arg, cb_func, argList.size() - 1, argList.data() + 1, nullptr);
    }

    if (_jsEventEmitterRef) {
        napi_value eventEmitter;
        napi_get_reference_value(_env, _jsEventEmitterRef, &eventEmitter);
        napi_value emit_func;
        napi_get_named_property(_env, eventEmitter, "emit", &emit_func);
        napi_call_function(_env, eventEmitter, emit_func, argList.size(), argList.data(), nullptr);
    }
}

void JsVlcPlayer::play() { player().play(); }
void JsVlcPlayer::play(const std::string& mrl) {
    vlc::player& p = player();
    p.clear_items();
    const int idx = p.add_media(mrl.c_str());
    if(idx >= 0) p.play(idx);
}
void JsVlcPlayer::pause() { player().pause(); }
void JsVlcPlayer::togglePause() { player().togglePause(); }
void JsVlcPlayer::stop() { player().stop(); }
void JsVlcPlayer::toggleMute() { player().audio().toggle_mute(); }

napi_value JsVlcPlayer::input(napi_env env) {
    napi_value obj;
    napi_get_reference_value(env, _jsInputRef, &obj);
    return obj;
}
napi_value JsVlcPlayer::audio(napi_env env) {
    napi_value obj;
    napi_get_reference_value(env, _jsAudioRef, &obj);
    return obj;
}
napi_value JsVlcPlayer::video(napi_env env) {
    napi_value obj;
    napi_get_reference_value(env, _jsVideoRef, &obj);
    return obj;
}
napi_value JsVlcPlayer::subtitles(napi_env env) {
    napi_value obj;
    napi_get_reference_value(env, _jsSubtitlesRef, &obj);
    return obj;
}
napi_value JsVlcPlayer::playlist(napi_env env) {
    napi_value obj;
    napi_get_reference_value(env, _jsPlaylistRef, &obj);
    return obj;
}
