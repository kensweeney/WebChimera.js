#pragma once

#include <string>
#include <vector>
#include <functional>

#include <node_api.h>

#include "Tools.h"

// Forward declaration
class NapiHelpers;

// N-API type conversion helpers
template<typename T>
T FromNapiValue(napi_env env, napi_value value);

template<>
bool FromNapiValue<bool>(napi_env env, napi_value value);

template<>
uint32_t FromNapiValue<uint32_t>(napi_env env, napi_value value);

template<>
int32_t FromNapiValue<int32_t>(napi_env env, napi_value value);

template<>
double FromNapiValue<double>(napi_env env, napi_value value);

template<>
std::string FromNapiValue<std::string>(napi_env env, napi_value value);

template<>
std::vector<std::string> FromNapiValue<std::vector<std::string>>(napi_env env, napi_value value);

napi_value ToNapiValue(napi_env env, bool value);
napi_value ToNapiValue(napi_env env, int32_t value);
napi_value ToNapiValue(napi_env env, uint32_t value);
napi_value ToNapiValue(napi_env env, double value);
napi_value ToNapiValue(napi_env env, const std::string& value);
napi_value ToNapiValue(napi_env env, const char* value);

// Helper for wrapping class methods
template<typename T>
class NapiWrappedClass {
public:
    static napi_value Constructor(napi_env env, napi_callback_info info) {
        napi_status status;
        napi_value target;
        status = napi_get_new_target(env, info, &target);
        if (status != napi_ok) return nullptr;

        if (target) {
            napi_value this_arg;
            status = napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
            if (status != napi_ok) return nullptr;

            T* obj = new T(env, info); // Assuming constructor signature
            status = napi_wrap(env, this_arg, obj,
                [](napi_env env, void* finalize_data, void* finalize_hint) {
                    T* instance = static_cast<T*>(finalize_data);
                    delete instance;
                },
                nullptr, nullptr);
            if (status != napi_ok) return nullptr;

            return this_arg;
        }

        return nullptr;
    }

    static void GetFinalize(napi_env env, void* finalize_data, void* finalize_hint) {
        delete static_cast<T*>(finalize_data);
    }
};

// Helper macros for defining N-API properties
#define DECLARE_NAPI_METHOD(name, func) \
    { name, nullptr, func, nullptr, nullptr, nullptr, napi_default, nullptr }

#define DECLARE_NAPI_GETTER(name, getter) \
    { name, nullptr, nullptr, getter, nullptr, nullptr, napi_default, nullptr }

#define DECLARE_NAPI_PROPERTY(name, getter, setter) \
    { name, nullptr, nullptr, getter, setter, nullptr, napi_default, nullptr }

napi_value Require(napi_env env, const char* module_name);
