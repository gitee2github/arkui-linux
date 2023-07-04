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

#ifndef ECMASCRIPT_BUILTINS_H
#define ECMASCRIPT_BUILTINS_H

#include "ecmascript/global_env.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/object_factory.h"

namespace panda::ecmascript {
struct ErrorParameter {
    EcmaEntrypoint nativeConstructor{nullptr};
    EcmaEntrypoint nativeMethod{nullptr};
    const char *nativePropertyName{nullptr};
    JSType nativeJstype{JSType::INVALID};
};

enum FunctionLength : uint8_t { ZERO = 0, ONE, TWO, THREE, FOUR };

class Builtins {
public:
    explicit Builtins() = default;
    ~Builtins() = default;
    NO_COPY_SEMANTIC(Builtins);
    NO_MOVE_SEMANTIC(Builtins);

    void Initialize(const JSHandle<GlobalEnv> &env, JSThread *thread);
    void InitializeForSnapshot(JSThread *thread);

private:
    JSThread *thread_{nullptr};
    ObjectFactory *factory_{nullptr};
    EcmaVM *vm_{nullptr};

    JSHandle<JSFunction> NewBuiltinConstructor(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &prototype,
                                               EcmaEntrypoint ctorFunc, const char *name, int length,
                                               kungfu::BuiltinsStubCSigns::ID builtinId =
                                               kungfu::BuiltinsStubCSigns::INVALID) const;

    JSHandle<JSFunction> NewBuiltinCjsCtor(const JSHandle<GlobalEnv> &env,
                                           const JSHandle<JSObject> &prototype, EcmaEntrypoint ctorFunc,
                                           const char *name, int length) const;

    JSHandle<JSFunction> NewFunction(const JSHandle<GlobalEnv> &env, const JSHandle<JSTaggedValue> &key,
                                     EcmaEntrypoint func, int length,
                                     kungfu::BuiltinsStubCSigns::ID builtinId =
                                     kungfu::BuiltinsStubCSigns::INVALID) const;

    void InitializeCtor(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &prototype,
                        const JSHandle<JSFunction> &ctor, const char *name, int length) const;

    void InitializeGlobalObject(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &globalObject);

    void InitializeFunction(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &emptyFuncClass) const;

    void InitializeObject(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &objFuncPrototype,
                          const JSHandle<JSObject> &objFunc);

    void InitializeNumber(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &globalObject,
                          const JSHandle<JSHClass> &primRefObjClass);

    void InitializeBigInt(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &primRefObjClass) const;

    void InitializeBigIntWithRealm(const JSHandle<GlobalEnv> &realm) const;

    void InitializeDate(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeBoolean(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &primRefObjClass) const;

    void InitializeSymbol(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeSymbolWithRealm(const JSHandle<GlobalEnv> &realm, const JSHandle<JSHClass> &objFuncInstanceHClass);

    void InitializeArray(const JSHandle<GlobalEnv> &env, const JSHandle<JSTaggedValue> &objFuncPrototypeVal) const;

    void InitializeTypedArray(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeInt8Array(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeUint8Array(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeUint8ClampedArray(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeInt16Array(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeUint16Array(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeInt32Array(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeUint32Array(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeFloat32Array(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeFloat64Array(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeBigInt64Array(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeBigUint64Array(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeAllTypeError(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeAllTypeErrorWithRealm(const JSHandle<GlobalEnv> &realm) const;

    void InitializeError(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass,
                         const JSType &errorTag) const;

    void SetErrorWithRealm(const JSHandle<GlobalEnv> &realm, const JSType &errorTag) const;

    void InitializeRegExp(const JSHandle<GlobalEnv> &env);

    // for Intl.
    JSHandle<JSFunction> NewIntlConstructor(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &prototype,
                                            EcmaEntrypoint ctorFunc, const char *name, int length);
    void InitializeIntlCtor(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &prototype,
                            const JSHandle<JSFunction> &ctor, const char *name, int length);
    void InitializeIntl(const JSHandle<GlobalEnv> &env, const JSHandle<JSTaggedValue> &objFuncPrototypeValue);
    void InitializeLocale(const JSHandle<GlobalEnv> &env);
    void InitializeDateTimeFormat(const JSHandle<GlobalEnv> &env);
    void InitializeRelativeTimeFormat(const JSHandle<GlobalEnv> &env);
    void InitializeNumberFormat(const JSHandle<GlobalEnv> &env);
    void InitializeCollator(const JSHandle<GlobalEnv> &env);
    void InitializePluralRules(const JSHandle<GlobalEnv> &env);
    void InitializeDisplayNames(const JSHandle<GlobalEnv> &env);
    void InitializeListFormat(const JSHandle<GlobalEnv> &env);

    void GeneralUpdateError(ErrorParameter *error, EcmaEntrypoint constructor, EcmaEntrypoint method, const char *name,
                            JSType type) const;

    void InitializeSet(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeMap(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeWeakMap(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeWeakSet(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeWeakRef(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeFinalizationRegistry(const JSHandle<GlobalEnv> &env,
                                        const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeMath(const JSHandle<GlobalEnv> &env, const JSHandle<JSTaggedValue> &objFuncPrototypeVal) const;

    void InitializeAtomics(const JSHandle<GlobalEnv> &env, const JSHandle<JSTaggedValue> &objFuncPrototypeVal) const;

    void InitializeJson(const JSHandle<GlobalEnv> &env, const JSHandle<JSTaggedValue> &objFuncPrototypeVal) const;

    void InitializeString(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &primRefObjHClass) const;

    void InitializeIterator(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeRegexpIterator(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &iteratorFuncClass) const;

    void InitializeStringIterator(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &iteratorFuncClass) const;

    void InitializeAsyncFromSyncIterator(const JSHandle<GlobalEnv> &env,
                                         const JSHandle<JSHClass> &iteratorFuncClass) const;

    void InitializeForinIterator(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &iteratorFuncClass) const;

    void InitializeMapIterator(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &iteratorFuncClass) const;

    void InitializeSetIterator(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &iteratorFuncClass) const;

    void InitializeArrayIterator(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &iteratorFuncClass) const;

    void InitializeArrayBuffer(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeSharedArrayBuffer(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeDataView(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeForPromiseFuncClass(const JSHandle<GlobalEnv> &env);

    void InitializeProxy(const JSHandle<GlobalEnv> &env);

    void InitializeReflect(const JSHandle<GlobalEnv> &env, const JSHandle<JSTaggedValue> &objFuncPrototypeVal) const;

    void InitializeAsyncFunction(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeGeneratorFunction(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;
    
    void InitializeAsyncGeneratorFunction(const JSHandle<GlobalEnv> &env,
                                          const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeAsyncGenerator(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeAsyncIterator(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeGenerator(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    JSHandle<JSFunction> InitializeExoticConstructor(const JSHandle<GlobalEnv> &env, EcmaEntrypoint ctorFunc,
                                                     const char *name, int length);

    void InitializePromise(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &promiseFuncClass);

    void InitializePromiseJob(const JSHandle<GlobalEnv> &env);

    void InitializeModuleNamespace(const JSHandle<GlobalEnv> &env, const JSHandle<JSHClass> &objFuncClass) const;

    void InitializeCjsModule(const JSHandle<GlobalEnv> &env) const;

    void InitializeCjsExports(const JSHandle<GlobalEnv> &env) const;

    void InitializeCjsRequire(const JSHandle<GlobalEnv> &env) const;

    void InitializeDefaultExportOfScript(const JSHandle<GlobalEnv> &env) const;

    void SetFunction(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &obj, const char *key,
                     EcmaEntrypoint func, int length, kungfu::BuiltinsStubCSigns::ID builtinId =
                     kungfu::BuiltinsStubCSigns::INVALID) const;

    void SetFunction(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &obj, const JSHandle<JSTaggedValue> &key,
                     EcmaEntrypoint func, int length, kungfu::BuiltinsStubCSigns::ID builtinId =
                     kungfu::BuiltinsStubCSigns::INVALID) const;

    void SetFuncToObjAndGlobal(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &globalObject,
                               const JSHandle<JSObject> &obj, const char *key, EcmaEntrypoint func, int length);

    template<int type = JSSymbol::SYMBOL_DEFAULT_TYPE>
    void SetFunctionAtSymbol(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &obj,
                             const JSHandle<JSTaggedValue> &symbol, const char *name, EcmaEntrypoint func,
                             int length) const;

    void SetStringTagSymbol(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &obj, const char *key) const;
    JSHandle<JSTaggedValue> CreateGetter(const JSHandle<GlobalEnv> &env, EcmaEntrypoint func, const char *name,
                                         int length) const;

    void SetConstant(const JSHandle<JSObject> &obj, const char *key, JSTaggedValue value) const;

    void SetGlobalThis(const JSHandle<JSObject> &obj, const char *key, const JSHandle<JSTaggedValue> &globalValue);

    void SetAttribute(const JSHandle<JSObject> &obj, const char *key, const char *value) const;

    void SetNoneAttributeProperty(const JSHandle<JSObject> &obj, const char *key,
                                  const JSHandle<JSTaggedValue> &value) const;

    void StrictModeForbiddenAccessCallerArguments(const JSHandle<GlobalEnv> &env,
                                                  const JSHandle<JSObject> &prototype) const;

    JSHandle<JSTaggedValue> CreateSetter(const JSHandle<GlobalEnv> &env, EcmaEntrypoint func, const char *name,
                                         int length);
    void SetArgumentsSharedAccessor(const JSHandle<GlobalEnv> &env);
    void SetAccessor(const JSHandle<JSObject> &obj, const JSHandle<JSTaggedValue> &key,
                     const JSHandle<JSTaggedValue> &getter, const JSHandle<JSTaggedValue> &setter) const;
    void SetGetter(const JSHandle<JSObject> &obj, const JSHandle<JSTaggedValue> &key,
                   const JSHandle<JSTaggedValue> &getter) const;
    JSHandle<JSObject> InitializeArkTools(const JSHandle<GlobalEnv> &env) const;
    void InitializeGlobalRegExp(JSHandle<JSObject> &obj) const;
    // Using to initialize jsapi container
    JSHandle<JSObject> InitializeArkPrivate(const JSHandle<GlobalEnv> &env) const;
    void SetConstantObject(const JSHandle<JSObject> &obj, const char *key, JSHandle<JSTaggedValue> &value) const;
    void SetFrozenFunction(const JSHandle<GlobalEnv> &env, const JSHandle<JSObject> &obj, const char *key,
                           EcmaEntrypoint func, int length) const;
    void SetNonConstantObject(const JSHandle<JSObject> &obj, const char *key, JSHandle<JSTaggedValue> &value) const;
};
}  // namespace panda::ecmascript
#endif  // ECMASCRIPT_BUILTINS_H
