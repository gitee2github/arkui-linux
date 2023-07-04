/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_NATIVE_REFERENCE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_NATIVE_REFERENCE_H

#include "native_engine/native_value.h"

#include "native_engine/native_reference.h"

class ArkNativeEngine;

using panda::Global;
using panda::JSValueRef;
using panda::Local;

class ArkNativeReference : public NativeReference {
public:
    ArkNativeReference(ArkNativeEngine* engine,
                       NativeValue* value,
                       uint32_t initialRefcount,
                       bool deleteSelf,
                       NativeFinalize callback = nullptr,
                       void* data = nullptr,
                       void* hint = nullptr);
    ~ArkNativeReference() override;

    uint32_t Ref() override;
    uint32_t Unref() override;
    NativeValue* Get() override;
    void* GetData() override;
    operator NativeValue*() override;

private:
    ArkNativeEngine* engine_;
    Global<JSValueRef> value_;
    uint32_t refCount_ {0};
    bool deleteSelf_ {false};
    bool hasDelete_ {false};

#ifdef ENABLE_CONTAINER_SCOPE
    int32_t scopeId_ = -1;
#endif

    NativeFinalize callback_ = nullptr;
    void* data_ = nullptr;
    void* hint_ = nullptr;

    void FinalizeCallback();

    static void FirstPassCallBack(void* ref);
    static void SecondPassCallBack(void* ref);
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_NATIVE_REFERENCE_H */
