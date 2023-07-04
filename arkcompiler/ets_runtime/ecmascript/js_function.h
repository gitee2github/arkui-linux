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

#ifndef ECMASCRIPT_JSFUNCTION_H
#define ECMASCRIPT_JSFUNCTION_H

#include "ecmascript/accessor_data.h"
#include "ecmascript/ecma_macros.h"
#include "ecmascript/js_object-inl.h"
#include "ecmascript/lexical_env.h"

namespace panda::ecmascript {
class JSThread;
struct EcmaRuntimeCallInfo;

class JSFunctionBase : public JSObject {
public:
    CAST_CHECK(JSFunctionBase, IsJSFunctionBase);

    inline void SetConstructor(bool flag)
    {
        JSHClass *hclass = GetJSHClass();
        hclass->SetConstructor(flag);
    }

    static bool SetFunctionName(JSThread *thread, const JSHandle<JSFunctionBase> &func,
                                const JSHandle<JSTaggedValue> &name, const JSHandle<JSTaggedValue> &prefix);
    static JSHandle<JSTaggedValue> GetFunctionName(JSThread *thread, const JSHandle<JSFunctionBase> &func);

    void SetCallNapi(bool isCallNapi)
    {
        JSTaggedValue method = GetMethod();
        Method::Cast(method.GetTaggedObject())->SetCallNapi(isCallNapi);
    }

    bool IsCallNapi() const
    {
        JSTaggedValue method = GetMethod();
        return Method::ConstCast(method.GetTaggedObject())->IsCallNapi();
    }

    FunctionKind GetFunctionKind() const
    {
        JSTaggedValue method = GetMethod();
        return Method::ConstCast(method.GetTaggedObject())->GetFunctionKind();
    }

    static constexpr size_t METHOD_OFFSET = JSObject::SIZE;
    ACCESSORS(Method, METHOD_OFFSET, LAST_OFFSET)
    DEFINE_ALIGN_SIZE(LAST_OFFSET);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSObject, METHOD_OFFSET, LAST_OFFSET)
    DECL_DUMP()
};

static_assert((JSFunctionBase::SIZE % static_cast<uint8_t>(MemAlignment::MEM_ALIGN_OBJECT)) == 0);

class JSFunction : public JSFunctionBase {
public:
    static constexpr int LENGTH_OF_INLINE_PROPERTIES = 3;
    static constexpr int LENGTH_INLINE_PROPERTY_INDEX = 0;
    static constexpr int NAME_INLINE_PROPERTY_INDEX = 1;
    static constexpr int PROTOTYPE_INLINE_PROPERTY_INDEX = 2;
    static constexpr int CLASS_PROTOTYPE_INLINE_PROPERTY_INDEX = 1;

    CAST_CHECK(JSFunction, IsJSFunction);

    static void InitializeJSFunction(JSThread *thread, const JSHandle<GlobalEnv> &env, const JSHandle<JSFunction> &func,
                                     FunctionKind kind = FunctionKind::NORMAL_FUNCTION);
    // ecma6 7.3
    static bool OrdinaryHasInstance(JSThread *thread, const JSHandle<JSTaggedValue> &constructor,
                                    const JSHandle<JSTaggedValue> &obj);

    static JSTaggedValue SpeciesConstructor(const JSHandle<JSFunction> &func,
                                            const JSHandle<JSFunction> &defaultConstructor);

    // ecma6 9.2
    // 7.3.12 Call(F, V, argumentsList)

    static JSTaggedValue Call(EcmaRuntimeCallInfo *info);

    static JSTaggedValue Construct(EcmaRuntimeCallInfo *info);
    static JSTaggedValue Invoke(EcmaRuntimeCallInfo *info, const JSHandle<JSTaggedValue> &key);
    // 9.2.2[[Construct]](argumentsList, newTarget)
    // 9.3.2[[Construct]](argumentsList, newTarget)
    static JSTaggedValue ConstructInternal(EcmaRuntimeCallInfo *info);

    static bool AddRestrictedFunctionProperties(const JSHandle<JSFunction> &func, const JSHandle<JSTaggedValue> &realm);
    static bool MakeConstructor(JSThread *thread, const JSHandle<JSFunction> &func,
                                const JSHandle<JSTaggedValue> &proto, bool writable = true);
    static bool SetFunctionLength(JSThread *thread, const JSHandle<JSFunction> &func, JSTaggedValue length,
                                  bool cfg = true);
    static JSHandle<JSObject> NewJSFunctionPrototype(JSThread *thread, ObjectFactory *factory,
                                                     const JSHandle<JSFunction> &func);
    static JSTaggedValue AccessCallerArgumentsThrowTypeError(EcmaRuntimeCallInfo *argv);
    static JSTaggedValue PrototypeGetter(JSThread *thread, const JSHandle<JSObject> &self);
    static bool PrototypeSetter(JSThread *thread, const JSHandle<JSObject> &self, const JSHandle<JSTaggedValue> &value,
                                bool mayThrow);
    static JSTaggedValue NameGetter(JSThread *thread, const JSHandle<JSObject> &self);
    static bool NameSetter(JSThread *thread, const JSHandle<JSObject> &self, const JSHandle<JSTaggedValue> &value,
                           bool mayThrow);
    static void SetFunctionNameNoPrefix(JSThread *thread, JSFunction *func, JSTaggedValue name);

    inline JSTaggedValue GetFunctionPrototype() const
    {
        ASSERT(HasFunctionPrototype());
        JSTaggedValue protoOrHClass = GetProtoOrHClass();
        if (protoOrHClass.IsJSHClass()) {
            return JSHClass::Cast(protoOrHClass.GetTaggedObject())->GetPrototype();
        }

        return protoOrHClass;
    }

    inline void SetFunctionPrototype(const JSThread *thread, JSTaggedValue proto)
    {
        SetProtoOrHClass(thread, proto);
        if (proto.IsJSHClass()) {
            proto = JSHClass::Cast(proto.GetTaggedObject())->GetPrototype();
        }
        if (proto.IsECMAObject()) {
            proto.GetTaggedObject()->GetClass()->SetIsPrototype(true);
        }
    }

    inline bool HasInitialClass() const
    {
        JSTaggedValue protoOrHClass = GetProtoOrHClass();
        return protoOrHClass.IsJSHClass();
    }

    inline bool HasFunctionPrototype() const
    {
        JSTaggedValue protoOrHClass = GetProtoOrHClass();
        return !protoOrHClass.IsHole();
    }

    inline void SetFunctionLength(const JSThread *thread, JSTaggedValue length)
    {
        ASSERT(!IsPropertiesDict());
        SetPropertyInlinedProps(thread, LENGTH_INLINE_PROPERTY_INDEX, length);
    }

    inline bool IsBase() const
    {
        FunctionKind kind = GetFunctionKind();
        return kind <= FunctionKind::CLASS_CONSTRUCTOR;
    }

    inline bool IsDerivedConstructor() const
    {
        FunctionKind kind = GetFunctionKind();
        return kind == FunctionKind::DERIVED_CONSTRUCTOR;
    }

    inline static bool IsArrowFunction(FunctionKind kind)
    {
        return (kind >= FunctionKind::ARROW_FUNCTION) && (kind <= FunctionKind::ASYNC_ARROW_FUNCTION);
    }

    inline static bool IsClassConstructor(FunctionKind kind)
    {
        return (kind == FunctionKind::CLASS_CONSTRUCTOR) || (kind == FunctionKind::DERIVED_CONSTRUCTOR);
    }

    inline static bool IsConstructorKind(FunctionKind kind)
    {
        return (kind >= FunctionKind::BASE_CONSTRUCTOR) && (kind <= FunctionKind::DERIVED_CONSTRUCTOR);
    }

    inline bool IsBuiltinConstructor()
    {
        FunctionKind kind = GetFunctionKind();
        return kind >= FunctionKind::BUILTIN_PROXY_CONSTRUCTOR && kind <= FunctionKind::BUILTIN_CONSTRUCTOR;
    }

    inline static bool HasPrototype(FunctionKind kind)
    {
        return (kind >= FunctionKind::BASE_CONSTRUCTOR) && (kind <= FunctionKind::ASYNC_GENERATOR_FUNCTION)
            && (kind != FunctionKind::BUILTIN_PROXY_CONSTRUCTOR);
    }

    inline static bool HasAccessor(FunctionKind kind)
    {
        return kind >= FunctionKind::NORMAL_FUNCTION && kind <= FunctionKind::ASYNC_FUNCTION;
    }

    inline bool IsClassConstructor() const
    {
        return GetClass()->IsClassConstructor();
    }

    inline void SetClassConstructor(bool flag)
    {
        GetClass()->SetClassConstructor(flag);
    }

    void SetFunctionExtraInfo(JSThread *thread, void *nativeFunc, const DeleteEntryPoint &deleter,
        void *data, size_t nativeBindingsize = 0);

    JSTaggedValue GetFunctionExtraInfo() const;
    JSTaggedValue GetNativeFunctionExtraInfo() const;
    JSTaggedValue GetRecordName() const;

    static void InitializeJSFunction(JSThread *thread, const JSHandle<JSFunction> &func,
                                     FunctionKind kind = FunctionKind::NORMAL_FUNCTION);
    static JSHClass *GetOrCreateInitialJSHClass(JSThread *thread, const JSHandle<JSFunction> &fun);
    static JSHandle<JSHClass> GetInstanceJSHClass(JSThread *thread, JSHandle<JSFunction> constructor,
                                                  JSHandle<JSTaggedValue> newTarget);

    static constexpr size_t PROTO_OR_DYNCLASS_OFFSET = JSFunctionBase::SIZE;
    ACCESSORS(ProtoOrHClass, PROTO_OR_DYNCLASS_OFFSET, LEXICAL_ENV_OFFSET)
    ACCESSORS(LexicalEnv, LEXICAL_ENV_OFFSET, HOME_OBJECT_OFFSET)
    ACCESSORS(HomeObject, HOME_OBJECT_OFFSET, ECMA_MODULE_OFFSET)
    ACCESSORS(Module, ECMA_MODULE_OFFSET, LAST_OFFSET)
    DEFINE_ALIGN_SIZE(LAST_OFFSET);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunctionBase, PROTO_OR_DYNCLASS_OFFSET, SIZE)
    DECL_DUMP()

private:
    static JSHandle<JSHClass> GetOrCreateDerivedJSHClass(JSThread *thread, JSHandle<JSFunction> derived,
                                                         JSHandle<JSHClass> ctorInitialClass);
};

class JSGeneratorFunction : public JSFunction {
public:
    CAST_CHECK(JSGeneratorFunction, IsGeneratorFunction);

    static constexpr size_t SIZE = JSFunction::SIZE;

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunction, SIZE, SIZE)

    DECL_DUMP()
};

class JSBoundFunction : public JSFunctionBase {
public:
    CAST_CHECK(JSBoundFunction, IsBoundFunction);

    // 9.4.1.2[[Construct]](argumentsList, newTarget)
    static JSTaggedValue ConstructInternal(EcmaRuntimeCallInfo *info);

    static constexpr size_t BOUND_TARGET_OFFSET = JSFunctionBase::SIZE;
    ACCESSORS(BoundTarget, BOUND_TARGET_OFFSET, BOUND_THIS_OFFSET);
    ACCESSORS(BoundThis, BOUND_THIS_OFFSET, BOUND_ARGUMENTS_OFFSET);
    ACCESSORS(BoundArguments, BOUND_ARGUMENTS_OFFSET, SIZE);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunctionBase, BOUND_TARGET_OFFSET, SIZE)

    DECL_DUMP()
};

class JSProxyRevocFunction : public JSFunction {
public:
    CAST_CHECK(JSProxyRevocFunction, IsProxyRevocFunction);

    static void ProxyRevocFunctions(const JSThread *thread, const JSHandle<JSProxyRevocFunction> &revoker);

    static constexpr size_t REVOCABLE_PROXY_OFFSET = JSFunction::SIZE;
    ACCESSORS(RevocableProxy, REVOCABLE_PROXY_OFFSET, SIZE);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunction, REVOCABLE_PROXY_OFFSET, SIZE)

    DECL_DUMP()
};

// ResolveFunction/RejectFunction
class JSPromiseReactionsFunction : public JSFunction {
public:
    CAST_CHECK(JSPromiseReactionsFunction, IsJSPromiseReactionFunction);

    static constexpr size_t PROMISE_OFFSET = JSFunction::SIZE;
    ACCESSORS(Promise, PROMISE_OFFSET, ALREADY_RESOLVED_OFFSET);
    ACCESSORS(AlreadyResolved, ALREADY_RESOLVED_OFFSET, SIZE);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunction, PROMISE_OFFSET, SIZE)

    DECL_DUMP()
};

// ExecutorFunction
class JSPromiseExecutorFunction : public JSFunction {
public:
    CAST_CHECK(JSPromiseExecutorFunction, IsJSPromiseExecutorFunction);

    static constexpr size_t CAPABILITY_OFFSET = JSFunction::SIZE;
    ACCESSORS(Capability, CAPABILITY_OFFSET, SIZE);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunction, CAPABILITY_OFFSET, SIZE)

    DECL_DUMP()
};

class JSPromiseAllResolveElementFunction : public JSFunction {
public:
    CAST_CHECK(JSPromiseAllResolveElementFunction, IsJSPromiseAllResolveElementFunction);

    static constexpr size_t INDEX_OFFSET = JSFunction::SIZE;
    ACCESSORS(Index, INDEX_OFFSET, VALUES_OFFSET);
    ACCESSORS(Values, VALUES_OFFSET, CAPABILITIES_OFFSET);
    ACCESSORS(Capabilities, CAPABILITIES_OFFSET, REMAINING_ELEMENTS_OFFSET);
    ACCESSORS(RemainingElements, REMAINING_ELEMENTS_OFFSET, ALREADY_CALLED_OFFSET);
    ACCESSORS(AlreadyCalled, ALREADY_CALLED_OFFSET, SIZE);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunction, INDEX_OFFSET, SIZE)

    DECL_DUMP()
};

// PromiseAnyRejectElementFunction
class JSPromiseAnyRejectElementFunction : public JSFunction {
public:
    CAST_CHECK(JSPromiseAnyRejectElementFunction, IsJSPromiseAnyRejectElementFunction);

    static constexpr size_t ERRORS_OFFSET = JSFunction::SIZE;

    ACCESSORS(Errors, ERRORS_OFFSET, CAPABILITY_OFFSET);
    ACCESSORS(Capability, CAPABILITY_OFFSET, REMAINING_ELEMENTS_OFFSET);
    ACCESSORS(RemainingElements, REMAINING_ELEMENTS_OFFSET, ALREADY_CALLED_OFFSET);
    ACCESSORS(AlreadyCalled, ALREADY_CALLED_OFFSET, INDEX_OFFSET);
    ACCESSORS_PRIMITIVE_FIELD(Index, uint32_t, INDEX_OFFSET, LAST_OFFSET);
    DEFINE_ALIGN_SIZE(LAST_OFFSET);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunction, ERRORS_OFFSET, INDEX_OFFSET)

    DECL_DUMP()
};

// PromiseAllSettledElementFunction
class JSPromiseAllSettledElementFunction : public JSFunction {
public:
    CAST_CHECK(JSPromiseAllSettledElementFunction, IsJSPromiseAllSettledElementFunction);

    static constexpr size_t ALREADY_CALLED_OFFSET = JSFunction::SIZE;
    ACCESSORS(AlreadyCalled, ALREADY_CALLED_OFFSET, VALUES_OFFSET);
    ACCESSORS(Values, VALUES_OFFSET, CAPABILITY_OFFSET);
    ACCESSORS(Capability, CAPABILITY_OFFSET, REMAINING_ELEMENTS_OFFSET);
    ACCESSORS(RemainingElements, REMAINING_ELEMENTS_OFFSET, INDEX_OFFSET);
    ACCESSORS_PRIMITIVE_FIELD(Index, uint32_t, INDEX_OFFSET, LAST_OFFSET);
    DEFINE_ALIGN_SIZE(LAST_OFFSET);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunction, ALREADY_CALLED_OFFSET, INDEX_OFFSET)

    DECL_DUMP()
};

// PromiseFinallyFunction
class JSPromiseFinallyFunction : public JSFunction {
public:
    CAST_CHECK(JSPromiseFinallyFunction, IsJSPromiseFinallyFunction);

    static constexpr size_t CONSTRUCTOR_OFFSET = JSFunction::SIZE;
    ACCESSORS(Constructor, CONSTRUCTOR_OFFSET, ONFINALLY_OFFSET);
    ACCESSORS(OnFinally, ONFINALLY_OFFSET, SIZE);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunction, CONSTRUCTOR_OFFSET, SIZE)

    DECL_DUMP()
};

// ValueThunkOrThrowReason
class JSPromiseValueThunkOrThrowerFunction : public JSFunction {
public:
    CAST_CHECK(JSPromiseValueThunkOrThrowerFunction, IsJSPromiseValueThunkOrThrowerFunction);

    static constexpr size_t RESULT_OFFSET = JSFunction::SIZE;
    ACCESSORS(Result, RESULT_OFFSET, SIZE);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunction, RESULT_OFFSET, SIZE)

    DECL_DUMP()
};

class JSIntlBoundFunction : public JSFunction {
public:
    CAST_CHECK(JSIntlBoundFunction, IsJSIntlBoundFunction);

    static JSTaggedValue IntlNameGetter(JSThread *thread, const JSHandle<JSObject> &self);

    static constexpr size_t NUMBER_FORMAT_OFFSET = JSFunction::SIZE;

    ACCESSORS(NumberFormat, NUMBER_FORMAT_OFFSET, DATETIME_FORMAT_OFFSET);
    ACCESSORS(DateTimeFormat, DATETIME_FORMAT_OFFSET, COLLATOR_OFFSET);
    ACCESSORS(Collator, COLLATOR_OFFSET, SIZE);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunction, NUMBER_FORMAT_OFFSET, SIZE)
    DECL_DUMP()
};

class JSAsyncGeneratorFunction : public JSFunction {
public:
    CAST_CHECK(JSAsyncGeneratorFunction, IsAsyncGeneratorFunction);
    static constexpr size_t  SIZE = JSFunction::SIZE;
    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunction, SIZE, SIZE)
    DECL_DUMP()
};

class JSAsyncFromSyncIterUnwarpFunction : public JSFunction {
public:
    CAST_CHECK(JSAsyncFromSyncIterUnwarpFunction, IsJSAsyncFromSyncIterUnwarpFunction);
    static constexpr size_t DONE_OFFSET = JSFunction::SIZE;
    ACCESSORS(Done, DONE_OFFSET, SIZE);

    DECL_VISIT_OBJECT_FOR_JS_OBJECT(JSFunction, DONE_OFFSET, SIZE);
    DECL_DUMP()
};

}  // namespace panda::ecmascript

#endif  // ECMASCRIPT_JSFUNCTION_H
