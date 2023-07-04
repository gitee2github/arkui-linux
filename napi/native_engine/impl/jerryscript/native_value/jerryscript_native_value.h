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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_VALUE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_VALUE_H

#include "jerryscript_headers.h"

#include "native_engine/native_value.h"

#include "jerryscript_native_engine.h"

class JerryScriptNativeValue : public NativeValue {
public:
    JerryScriptNativeValue(JerryScriptNativeEngine* engine, jerry_value_t value);
    ~JerryScriptNativeValue() override;

    void* GetInterface(const int interfaceId) override;

    NativeValueType TypeOf() override;
    bool InstanceOf(NativeValue* obj) override;

    bool IsArray() override;
    bool IsArrayBuffer() override;
    bool IsBuffer() override;
    bool IsDate() override;
    bool IsError() override;
    bool IsTypedArray() override;
    bool IsDataView() override;
    bool IsPromise() override;
    bool IsCallable() override { return false; }

    bool IsMapIterator() override;
    bool IsSetIterator() override;
    bool IsGeneratorObject() override;
    bool IsModuleNamespaceObject() override;
    bool IsProxy() override;
    bool IsRegExp() override;
    bool IsNumberObject() override;
    bool IsMap() override;
    bool IsSet() override;
    bool IsArgumentsObject() override;
    bool IsAsyncFunction() override;
    bool IsBooleanObject() override;
    bool IsGeneratorFunction() override;
    bool IsStringObject() override;
    bool IsSymbolObject() override;
    bool IsWeakMap() override;
    bool IsWeakSet() override;
    bool IsBigInt64Array() override;
    bool IsBigUint64Array() override;
    bool IsSharedArrayBuffer() override;

    NativeValue* ToBoolean() override;
    NativeValue* ToNumber() override;
    NativeValue* ToString() override;
    NativeValue* ToObject() override;

    bool StrictEquals(NativeValue* value) override;

protected:
    JerryScriptNativeEngine* engine_;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_VALUE_H */
