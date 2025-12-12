    #include <node_api.h>

#include "JsVlcPlayer.h"
#include "NodeTools.h"

static napi_value Init(napi_env env, napi_value exports) {
    return JsVlcPlayer::initJsApi(env, exports);
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
