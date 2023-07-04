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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_EXTERNAL_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_EXTERNAL_H

#include "ark_native_value.h"

class ArkNativeExternal : public ArkNativeValue, public NativeExternal {
public:
    ArkNativeExternal(ArkNativeEngine* engine, void* value, NativeFinalize callback, void* hint,
        size_t nativeBindingSize = 0);
    ArkNativeExternal(ArkNativeEngine* engine, Local<JSValueRef> value);
    ~ArkNativeExternal() override;

    void* GetInterface(int interfaceId) override;

    operator void*() override;

private:
    static void ArkExternalDeleterCallback(void* data, void* info);
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_EXTERNAL_H */
