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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_OBJECT_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_OBJECT_H

#include "v8_native_value.h"

class V8NativeObject : public V8NativeValue, public NativeObject {
public:
    explicit V8NativeObject(V8NativeEngine* engine);
    V8NativeObject(V8NativeEngine* engine, v8::Local<v8::Value> value);
    ~V8NativeObject() override;

    void* GetInterface(int interfaceId) override;
    bool ConvertToNativeBindingObject(
        void* engine, DetachCallback detach, AttachCallback attach, void *object, void *hint) override;
    void SetNativePointer(void* pointer, NativeFinalize cb, void* hint, NativeReference** reference = nullptr,
        size_t nativeBindingSize) override;
    void* GetNativePointer() override;
    void SetNativeBindingPointer(
        void* enginePointer, void* objPointer, void* hint, void* detachData, void* attachData) override;
    void* GetNativeBindingPointer(uint32_t index) override;

    NativeValue* GetPropertyNames() override;
    NativeValue* GetEnumerablePropertyNames() override;

    NativeValue* GetPrototype() override;

    bool DefineProperty(NativePropertyDescriptor propertyDescriptor) override;

    bool SetProperty(NativeValue* key, NativeValue* value) override;
    NativeValue* GetProperty(NativeValue* key) override;
    bool HasProperty(NativeValue* key) override;
    bool DeleteProperty(NativeValue* key) override;

    bool SetProperty(const char* name, NativeValue* value) override;
    NativeValue* GetProperty(const char* name) override;
    bool HasProperty(const char* name) override;
    bool DeleteProperty(const char* name) override;

    bool SetPrivateProperty(const char* name, NativeValue* value) override;
    NativeValue* GetPrivateProperty(const char* name) override;
    bool HasPrivateProperty(const char* name) override;
    bool DeletePrivateProperty(const char* name) override;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_OBJECT_H */
