#include "NodeTools.h"
#include <vector>
#include <string>

template<>
bool FromNapiValue<bool>(napi_env env, napi_value value) {
    bool result;
    napi_get_value_bool(env, value, &result);
    return result;
}

template<>
uint32_t FromNapiValue<uint32_t>(napi_env env, napi_value value) {
    uint32_t result;
    napi_get_value_uint32(env, value, &result);
    return result;
}

template<>
int32_t FromNapiValue<int32_t>(napi_env env, napi_value value) {
    int32_t result;
    napi_get_value_int32(env, value, &result);
    return result;
}

template<>
double FromNapiValue<double>(napi_env env, napi_value value) {
    double result;
    napi_get_value_double(env, value, &result);
    return result;
}

template<>
std::string FromNapiValue<std::string>(napi_env env, napi_value value) {
    size_t str_size;
    napi_get_value_string_utf8(env, value, nullptr, 0, &str_size);
    std::string result(str_size, '\0');
    napi_get_value_string_utf8(env, value, &result[0], str_size + 1, nullptr);
    return result;
}

template<>
std::vector<std::string> FromNapiValue<std::vector<std::string>>(napi_env env, napi_value value) {
    uint32_t array_length = 0;
    napi_get_array_length(env, value, &array_length);
    std::vector<std::string> result;
    result.reserve(array_length);

    for (uint32_t i = 0; i < array_length; ++i) {
        napi_value element;
        napi_get_element(env, value, i, &element);
        result.push_back(FromNapiValue<std::string>(env, element));
    }
    return result;
}

napi_value ToNapiValue(napi_env env, bool value) {
    napi_value result;
    napi_get_boolean(env, value, &result);
    return result;
}

napi_value ToNapiValue(napi_env env, int32_t value) {
    napi_value result;
    napi_create_int32(env, value, &result);
    return result;
}

napi_value ToNapiValue(napi_env env, uint32_t value) {
    napi_value result;
    napi_create_uint32(env, value, &result);
    return result;
}

napi_value ToNapiValue(napi_env env, double value) {
    napi_value result;
    napi_create_double(env, value, &result);
    return result;
}

napi_value ToNapiValue(napi_env env, const std::string& value) {
    napi_value result;
    napi_create_string_utf8(env, value.c_str(), value.length(), &result);
    return result;
}

napi_value ToNapiValue(napi_env env, const char* value) {
    napi_value result;
    napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value Require(napi_env env, const char* module_name) {
    napi_value global, require_func, module_str, result;
    napi_get_global(env, &global);

    napi_create_string_utf8(env, "require", NAPI_AUTO_LENGTH, &require_func);
    napi_get_property(env, global, require_func, &require_func);

    napi_create_string_utf8(env, module_name, NAPI_AUTO_LENGTH, &module_str);
    napi_call_function(env, global, require_func, 1, &module_str, &result);

    return result;
}
