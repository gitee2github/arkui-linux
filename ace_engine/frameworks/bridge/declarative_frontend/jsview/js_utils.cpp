/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "frameworks/bridge/declarative_frontend/jsview/js_utils.h"

#include "scope_manager/native_scope_manager.h"

#if !defined(PREVIEW)
#include <dlfcn.h>
#endif

#include "base/image/pixel_map.h"
#include "base/log/ace_trace.h"
#include "base/want/want_wrap.h"
#include "frameworks/bridge/common/utils/engine_helper.h"
#include "frameworks/bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "frameworks/bridge/declarative_frontend/view_stack_processor.h"
#include "frameworks/bridge/js_frontend/engine/common/js_engine.h"

namespace OHOS::Ace::Framework {
#if !defined(PREVIEW)
class ScopeRAII {
public:
    explicit ScopeRAII(NativeScopeManager* manager) : manager_(manager)
    {
        scope_ = manager_->Open();
    }
    ~ScopeRAII()
    {
        manager_->Close(scope_);
    }

private:
    NativeScopeManager* manager_;
    NativeScope* scope_;
};

RefPtr<PixelMap> CreatePixelMapFromNapiValue(JSRef<JSVal> obj)
{
    if (!obj->IsObject()) {
        LOGE("info[0] is not an object when try CreatePixelMapFromNapiValue");
        return nullptr;
    }
    auto engine = EngineHelper::GetCurrentEngine();
    if (!engine) {
        LOGE("CreatePixelMapFromNapiValue engine is null");
        return nullptr;
    }
    auto* nativeEngine = engine->GetNativeEngine();
    if (nativeEngine == nullptr) {
        LOGE("nativeEngine is nullptr.");
        return nullptr;
    }
#ifdef USE_V8_ENGINE
    v8::Local<v8::Value> value = obj->operator v8::Local<v8::Value>();
#elif USE_QUICKJS_ENGINE
    JSValue value = obj.Get().GetHandle();
#elif USE_ARK_ENGINE
    panda::Local<JsiValue> value = obj.Get().GetLocalHandle();
#endif
    JSValueWrapper valueWrapper = value;

    ScopeRAII scope(nativeEngine->GetScopeManager());
    NativeValue* nativeValue = nativeEngine->ValueToNativeValue(valueWrapper);

    PixelMapNapiEntry pixelMapNapiEntry = JsEngine::GetPixelMapNapiEntry();
    if (!pixelMapNapiEntry) {
        LOGE("pixelMapNapiEntry is null");
        return nullptr;
    }

    void* pixmapPtrAddr =
        pixelMapNapiEntry(reinterpret_cast<napi_env>(nativeEngine), reinterpret_cast<napi_value>(nativeValue));
    if (pixmapPtrAddr == nullptr) {
        LOGE("Failed to get pixmap pointer");
        return nullptr;
    }
    return PixelMap::CreatePixelMap(pixmapPtrAddr);
}

const std::shared_ptr<Rosen::RSNode> CreateRSNodeFromNapiValue(JSRef<JSVal> obj)
{
#ifdef ENABLE_ROSEN_BACKEND
    if (!obj->IsObject()) {
        LOGE("info[0] is not an object when try CreateRSNodeFromNapiValue");
        return nullptr;
    }
    auto engine = EngineHelper::GetCurrentEngine();
    if (!engine) {
        LOGE("CreateRSNodeFromNapiValue engine is null");
        return nullptr;
    }
    auto nativeEngine = engine->GetNativeEngine();
    if (nativeEngine == nullptr) {
        LOGE("nativeEngine is nullptr.");
        return nullptr;
    }
#ifdef USE_V8_ENGINE
    v8::Local<v8::Value> value = obj->operator v8::Local<v8::Value>();
#elif USE_QUICKJS_ENGINE
    JSValue value = obj.Get().GetHandle();
#elif USE_ARK_ENGINE
    panda::Local<JsiValue> value = obj.Get().GetLocalHandle();
#endif
    JSValueWrapper valueWrapper = value;

    ScopeRAII scope(nativeEngine->GetScopeManager());
    NativeValue* nativeValue = nativeEngine->ValueToNativeValue(valueWrapper);
    if (nativeValue == nullptr) {
        LOGE("nativeValue is nullptr.");
        return nullptr;
    }
    NativeObject* object = static_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));
    if (object == nullptr) {
        return nullptr;
    }

    auto nodePtr = static_cast<std::shared_ptr<Rosen::RSNode>*>(object->GetNativePointer());
    if (nodePtr == nullptr) {
        return nullptr;
    }
    return *nodePtr;
#else
    return nullptr;
#endif
}

RefPtr<OHOS::Ace::WantWrap> CreateWantWrapFromNapiValue(JSRef<JSVal> obj)
{
    if (!obj->IsObject()) {
        LOGE("invalid object when try CreateWantWrapFromNapiValue");
        return nullptr;
    }
    auto engine = EngineHelper::GetCurrentEngine();
    CHECK_NULL_RETURN(engine, nullptr);

    NativeEngine* nativeEngine = engine->GetNativeEngine();
    CHECK_NULL_RETURN(nativeEngine, nullptr);

#ifdef USE_V8_ENGINE
    v8::Local<v8::Value> value = obj->operator v8::Local<v8::Value>();
#elif USE_QUICKJS_ENGINE
    JSValue value = obj.Get().GetHandle();
#elif USE_ARK_ENGINE
    panda::Local<JsiValue> value = obj.Get().GetLocalHandle();
#endif
    JSValueWrapper valueWrapper = value;

    ScopeRAII scope(nativeEngine->GetScopeManager());
    NativeValue* nativeValue = nativeEngine->ValueToNativeValue(valueWrapper);

    return WantWrap::CreateWantWrap(reinterpret_cast<void*>(nativeEngine), reinterpret_cast<void*>(nativeValue));
}

#endif
} // namespace OHOS::Ace::Framework
