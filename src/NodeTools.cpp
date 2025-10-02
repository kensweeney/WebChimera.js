#include "NodeTools.h"


template<>
std::vector<std::string> FromJsValue<std::vector<std::string> >(
    const v8::Local<v8::Value>& value)
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();

    std::vector<std::string> result;

    if(value->IsArray()) {
        Local<Array> jsArray = Local<Array>::Cast(value);

        for(unsigned i = 0 ; i < jsArray->Length(); ++i) {
            v8::Local<v8::Value> element;
            if (jsArray->Get(context, i).ToLocal(&element)) {
                v8::String::Utf8Value item(isolate, element);
                if (*item) {
                    result.emplace(result.end(), *item);
                }
            }
        }
    }

    return std::move(result);
}

static v8::Local<v8::Function> RequireFunc(const v8::Local<v8::Object>& thisModule)
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();

    Local<String> name =
        String::NewFromUtf8(
            isolate,
            "require", v8::NewStringType::kInternalized).ToLocalChecked();

    v8::Local<v8::Value> require_val;
    if (!thisModule->Get(context, name).ToLocal(&require_val)) {
        // Return empty handle if not found
        return v8::Local<Function>();
    }
    return v8::Local<Function>::Cast(require_val);
}

v8::Local<v8::Object> Require(
    const v8::Local<v8::Object>& thisModule,
    const char* module)
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> global = context->Global();

    Local<Value> argv[] =
        { String::NewFromUtf8(isolate, module, v8::NewStringType::kInternalized).ToLocalChecked() };

    v8::Local<v8::Value> require_result;
    if (!RequireFunc(thisModule)->Call(context, global, 1, argv).ToLocal(&require_result)) {
        // Return empty handle if call fails
        return v8::Local<Object>();
    }
    return v8::Local<Object>::Cast(require_result);
}
