#pragma once

#include <uv.h>
#include <vlc/vlc.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_media_player.h>

#include <deque>
#include <memory>
#include <mutex>
#include <set>

#include "vlc_player.h"
#include "VlcVideoOutput.h"
#include "NodeTools.h"

class JsVlcInput;
class JsVlcAudio;
class JsVlcVideo;
class JsVlcSubtitles;
class JsVlcPlaylist;

class JsVlcPlayer : public vlc::media_player_events_callback, public VlcVideoOutput
{
public:
    enum Callbacks_e
    {
        CB_FrameSetup,
        CB_FrameReady,
        CB_FrameCleanup,

        CB_MediaPlayerMediaChanged,
        CB_MediaPlayerNothingSpecial,
        CB_MediaPlayerOpening,
        CB_MediaPlayerBuffering,
        CB_MediaPlayerPlaying,
        CB_MediaPlayerPaused,
        CB_MediaPlayerStopped,
        CB_MediaPlayerForward,
        CB_MediaPlayerBackward,
        CB_MediaPlayerEndReached,
        CB_MediaPlayerEncounteredError,

        CB_MediaPlayerTimeChanged,
        CB_MediaPlayerPositionChanged,
        CB_MediaPlayerSeekableChanged,
        CB_MediaPlayerPausableChanged,
        CB_MediaPlayerLengthChanged,

        CB_LogMessage,

        CB_Max
    };

    static const char* callbackNames[CB_Max];

    static napi_value initJsApi(napi_env env, napi_value exports);

    // C++ methods
    void play();
    void play(const std::string& mrl);
    void pause();
    void togglePause();
    void stop();
    void toggleMute();

    inline vlc::player& player() { return _player; }
    inline napi_env getEnv() { return _env; }
    inline libvlc_instance_t* get_instance() { return _libvlc; }

private:
    struct ContextData;
    struct AsyncData;
    struct CallbackData;
    struct LibvlcEvent;
    struct LibvlcLogEvent;

    JsVlcPlayer(napi_env env, napi_callback_info info);
    ~JsVlcPlayer();

    void initLibvlc(napi_env env, napi_value vlcOpts);

    void close();

    void media_player_event(const libvlc_event_t* e) override;

    static void log_event_wrapper(void *data, int level, const libvlc_log_t *ctx, const char *fmt, va_list args);
    void log_event(int level, const libvlc_log_t *ctx, const char *fmt, va_list args);

    void handleAsync();

    void* onFrameSetup(const RV32VideoFrame& videoFrame) override;
    void* onFrameSetup(const I420VideoFrame& videoFrame) override;
    void onFrameReady() override;
    void onFrameCleanup() override;

    void handleLibvlcEvent(const libvlc_event_t& libvlcEvent);
    void currentItemEndReached();

    void callCallback(Callbacks_e callback, std::initializer_list<napi_value> list = {});

    // N-API methods
    static napi_value jsCreate(napi_env env, napi_callback_info info);
    static void jsFinalize(napi_env env, void* data, void* hint);

    static napi_value jsPlay(napi_env env, napi_callback_info info);

    // N-API properties
    napi_value playing(napi_env env);
    napi_value length(napi_env env);
    napi_value state(napi_env env);
    napi_value getVideoFrame(napi_env env);
    napi_value getEventEmitter(napi_env env);
    napi_value pixelFormat(napi_env env);
    void setPixelFormat(napi_env env, napi_value value);
    napi_value position(napi_env env);
    void setPosition(napi_env env, napi_value value);
    napi_value time(napi_env env);
    void setTime(napi_env env, napi_value value);
    napi_value volume(napi_env env);
    void setVolume(napi_env env, napi_value value);
    napi_value muted(napi_env env);
    void setMuted(napi_env env, napi_value value);

    // N-API callback properties
    napi_value getJsCallback(napi_env env, Callbacks_e callback);
    void setJsCallback(napi_env env, napi_value value, Callbacks_e callback);

    template<Callbacks_e C>
    static napi_value getJsCallback(napi_env env, napi_callback_info info);

    template<Callbacks_e C>
    static napi_value setJsCallback(napi_env env, napi_callback_info info);

    // Wrapped objects
    napi_value input(napi_env env);
    napi_value audio(napi_env env);
    napi_value video(napi_env env);
    napi_value subtitles(napi_env env);
    napi_value playlist(napi_env env);

    napi_env _env;
    napi_ref _wrapper;

    ContextData* _contextData;
    libvlc_instance_t* _libvlc;
    vlc::player _player;

    uv_async_t _async;
    std::mutex _asyncDataGuard;
    std::deque<std::unique_ptr<AsyncData>> _asyncData;

    uv_timer_t _errorTimer;

    napi_ref _jsCallbacks[CB_Max];
    napi_ref _jsEventEmitterRef;
    napi_ref _jsFrameBufferRef;

    napi_ref _jsInputRef;
    napi_ref _jsAudioRef;
    napi_ref _jsVideoRef;
    napi_ref _jsSubtitlesRef;
    napi_ref _jsPlaylistRef;

    static napi_ref _jsConstructor;
};

napi_status napi_call_function(napi_env env,
                               napi_value recv,
                               napi_value func,
                               size_t argc,
                               const napi_value* argv,
                               napi_value* result);
