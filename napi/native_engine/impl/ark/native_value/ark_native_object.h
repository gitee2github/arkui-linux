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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_OBJECT_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_OBJECT_H

#include <mutex>

#include "ark_native_value.h"

class ArkNativeObject : public ArkNativeValue, public NativeObject {
public:
    explicit ArkNativeObject(ArkNativeEngine* engine);
    ArkNativeObject(ArkNativeEngine* engine, Local<JSValueRef> value);
    ArkNativeObject(ArkNativeEngine* engine, void* detach, void* attach);
    ArkNativeObject(ArkNativeEngine* engine, DetachCallback detach, AttachCallback attach);
    ~ArkNativeObject() override;

    bool ConvertToNativeBindingObject(
        void* engine, DetachCallback detach, AttachCallback attach, void *object, void *hint) override;
    void* GetInterface(int interfaceId) override;
    static void* DetachFuncCallback(void* engine, void* object, void* hint, void* detachData);
    static Local<JSValueRef> AttachFuncCallback(void* engine, void* object, void* hint, void* attachData);
    void SetNativePointer(void* pointer, NativeFinalize cb, void* hint, NativeReference** reference = nullptr,
        size_t nativeBindingSize = 0) override;
    void* GetNativePointer() override;
    void SetNativeBindingPointer(void* enginePointer, void* objPointer,
        void* hint, void* detachData, void* attachData) override;
    void* GetNativeBindingPointer(uint32_t index) override;
    void AddFinalizer(void* pointer, NativeFinalize cb, void* hint) override;
    void Freeze() override;
    void Seal() override;
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

    NativeValue* GetAllPropertyNames(
        napi_key_collection_mode keyMode, napi_key_filter keyFilter, napi_key_conversion keyConversion) override;
    bool AssociateTypeTag(NapiTypeTag* typeTag) override;
    bool CheckTypeTag(NapiTypeTag* typeTag) override;
    void SetModuleName(std::string moduleName);
    std::string GetModuleName();

private:
    static AttachCallback attach_;
    static DetachCallback detach_;
    std::mutex funcMutex_;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_OBJECT_H */
