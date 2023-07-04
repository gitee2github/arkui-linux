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

#include "ecmascript/js_tagged_value.h"

#include "ecmascript/ecma_macros.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/interpreter/interpreter.h"
#include "ecmascript/js_api/js_api_arraylist.h"
#include "ecmascript/js_api/js_api_deque.h"
#include "ecmascript/js_api/js_api_lightweightset.h"
#include "ecmascript/js_api/js_api_lightweightmap.h"
#include "ecmascript/js_api/js_api_linked_list.h"
#include "ecmascript/js_api/js_api_list.h"
#include "ecmascript/js_api/js_api_plain_array.h"
#include "ecmascript/js_api/js_api_queue.h"
#include "ecmascript/js_api/js_api_stack.h"
#include "ecmascript/js_api/js_api_vector.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_object-inl.h"
#include "ecmascript/js_primitive_ref.h"
#include "ecmascript/js_proxy.h"
#include "ecmascript/js_tagged_value-inl.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/js_typed_array.h"
#include "ecmascript/module/js_module_namespace.h"
#include "ecmascript/tagged_array.h"
#include "ecmascript/object_factory.h"

namespace panda::ecmascript {
JSHandle<EcmaString> GetTypeString(JSThread *thread, PreferredPrimitiveType type)
{
    const GlobalEnvConstants *globalConst = thread->GlobalConstants();
    if (type == NO_PREFERENCE) {
        return JSHandle<EcmaString>::Cast(globalConst->GetHandledDefaultString());
    }
    if (type == PREFER_NUMBER) {
        return JSHandle<EcmaString>::Cast(globalConst->GetHandledNumberString());
    }
    return JSHandle<EcmaString>::Cast(globalConst->GetHandledStringString());
}

JSHandle<JSTaggedValue> JSTaggedValue::ToPropertyKey(JSThread *thread, const JSHandle<JSTaggedValue> &tagged)
{
    if (tagged->IsStringOrSymbol() || tagged->IsNumber()) {
        return tagged;
    }
    JSHandle<JSTaggedValue> key(thread, ToPrimitive(thread, tagged, PREFER_STRING));
    RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
    if (key->IsSymbol()) {
        return key;
    }
    JSHandle<EcmaString> string = ToString(thread, key);
    RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
    return JSHandle<JSTaggedValue>::Cast(string);
}

bool JSTaggedValue::IsInteger() const
{
    if (!IsNumber()) {
        return false;
    }

    if (IsInt()) {
        return true;
    }

    double thisValue = GetDouble();
    // If argument is NaN, +∞, or -∞, return false.
    if (!std::isfinite(thisValue)) {
        return false;
    }

    // If floor(abs(argument)) ≠ abs(argument), return false.
    if (std::floor(std::abs(thisValue)) != std::abs(thisValue)) {
        return false;
    }

    return true;
}

bool JSTaggedValue::IsJSCOWArray() const
{
    // Elements of JSArray are shared and properties are not yet.
    return IsJSArray() && JSArray::Cast(GetTaggedObject())->GetElements().IsCOWArray();
}

bool JSTaggedValue::WithinInt32() const
{
    if (!IsNumber()) {
        return false;
    }

    double doubleValue = GetNumber();
    if (bit_cast<int64_t>(doubleValue) == bit_cast<int64_t>(-0.0)) {
        return false;
    }

    int32_t intvalue = base::NumberHelper::DoubleToInt(doubleValue, base::INT32_BITS);
    return doubleValue == static_cast<double>(intvalue);
}

bool JSTaggedValue::IsZero() const
{
    if (GetRawData() == VALUE_ZERO) {
        return true;
    }
    if (IsDouble()) {
        const double limit = 1e-8;
        return (std::abs(GetDouble() - 0.0) <= limit);
    }
    return false;
}

bool JSTaggedValue::Equal(JSThread *thread, const JSHandle<JSTaggedValue> &x, const JSHandle<JSTaggedValue> &y)
{
    if (x->IsNumber()) {
        if (y->IsNumber()) {
            return StrictNumberEquals(x->ExtractNumber(), y->ExtractNumber());
        }
        if (y->IsString()) {
            JSTaggedNumber yNumber = ToNumber(thread, y);
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
            return StrictNumberEquals(x->ExtractNumber(), yNumber.GetNumber());
        }
        if (y->IsBoolean()) {
            JSTaggedNumber yNumber = ToNumber(thread, y);
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
            return StrictNumberEquals(x->ExtractNumber(), yNumber.GetNumber());
        }
        if (y->IsBigInt()) {
            return Equal(thread, y, x);
        }
        if (y->IsHeapObject() && !y->IsSymbol()) {
            JSHandle<JSTaggedValue> yPrimitive(thread, ToPrimitive(thread, y));
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
            return Equal(thread, x, yPrimitive);
        }
        return false;
    }

    if (x->IsString()) {
        if (y->IsString()) {
            return EcmaStringAccessor::StringsAreEqual(static_cast<EcmaString *>(x->GetTaggedObject()),
                                                       static_cast<EcmaString *>(y->GetTaggedObject()));
        }
        if (y->IsNumber()) {
            JSTaggedNumber xNumber = ToNumber(thread, x);
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
            return StrictNumberEquals(xNumber.GetNumber(), y->ExtractNumber());
        }
        if (y->IsBoolean()) {
            JSTaggedNumber xNumber = ToNumber(thread, x);
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
            JSTaggedNumber yNumber = ToNumber(thread, y);
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
            return StrictNumberEquals(xNumber.GetNumber(), yNumber.GetNumber());
        }
        if (y->IsBigInt()) {
            return Equal(thread, y, x);
        }
        if (y->IsHeapObject() && !y->IsSymbol()) {
            JSHandle<JSTaggedValue> yPrimitive(thread, ToPrimitive(thread, y));
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
            return Equal(thread, x, yPrimitive);
        }
        return false;
    }

    if (x->IsBoolean()) {
        JSTaggedNumber xNumber = ToNumber(thread, x);
        RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
        return Equal(thread, JSHandle<JSTaggedValue>(thread, xNumber), y);
    }

    if (x->IsSymbol()) {
        if (y->IsSymbol()) {
            return x.GetTaggedValue() == y.GetTaggedValue();
        }
        if (y->IsBigInt()) {
            return Equal(thread, y, x);
        }
        if (y->IsHeapObject()) {
            JSHandle<JSTaggedValue> yPrimitive(thread, ToPrimitive(thread, y));
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
            return Equal(thread, x, yPrimitive);
        }
        return false;
    }

    if (x->IsBigInt()) {
        if (y->IsBigInt()) {
            return BigInt::Equal(x.GetTaggedValue(), y.GetTaggedValue());
        }
        if (y->IsString()) {
            JSHandle<JSTaggedValue> yNumber(thread, base::NumberHelper::StringToBigInt(thread, y));
            if (!yNumber->IsBigInt()) {
                return false;
            }
            return BigInt::Equal(x.GetTaggedValue(), yNumber.GetTaggedValue());
        }
        if (y->IsBoolean()) {
            JSHandle<JSTaggedValue> yNumber(thread, ToBigInt(thread, y));
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
            return BigInt::Equal(x.GetTaggedValue(), yNumber.GetTaggedValue());
        }
        if (y->IsNumber()) {
            JSHandle<BigInt> bigint = JSHandle<BigInt>::Cast(x);
            return BigInt::CompareWithNumber(bigint, y) == ComparisonResult::EQUAL;
        }
        if (y->IsHeapObject() && !y->IsSymbol()) {
            JSHandle<JSTaggedValue> yPrimitive(thread, ToPrimitive(thread, y));
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
            return Equal(thread, x, yPrimitive);
        }
        return false;
    }

    if (x->IsHeapObject()) {
        if (y->IsHeapObject()) {
            // if same type, must call Type::StrictEqual()
            JSType xType = x.GetTaggedValue().GetTaggedObject()->GetClass()->GetObjectType();
            JSType yType = y.GetTaggedValue().GetTaggedObject()->GetClass()->GetObjectType();
            if (xType == yType) {
                return StrictEqual(thread, x, y);
            }
        }
        if (y->IsNumber() || y->IsStringOrSymbol() || y->IsBoolean() || y->IsBigInt()) {
            JSHandle<JSTaggedValue> x_primitive(thread, ToPrimitive(thread, x));
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
            return Equal(thread, x_primitive, y);
        }
        return false;
    }

    if (x->IsNull() && y->IsNull()) {
        return true;
    }

    if (x->IsUndefined() && y->IsUndefined()) {
        return true;
    }

    if (x->IsNull() && y->IsUndefined()) {
        return true;
    }

    if (x->IsUndefined() && y->IsNull()) {
        return true;
    }

    return false;
}

ComparisonResult JSTaggedValue::Compare(JSThread *thread, const JSHandle<JSTaggedValue> &x,
                                        const JSHandle<JSTaggedValue> &y)
{
    JSHandle<JSTaggedValue> primX(thread, ToPrimitive(thread, x));
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, ComparisonResult::UNDEFINED);
    JSHandle<JSTaggedValue> primY(thread, ToPrimitive(thread, y));
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, ComparisonResult::UNDEFINED);
    if (primX->IsString() && primY->IsString()) {
        auto xString = static_cast<EcmaString *>(primX->GetTaggedObject());
        auto yString = static_cast<EcmaString *>(primY->GetTaggedObject());
        int result = EcmaStringAccessor::Compare(xString, yString);
        if (result < 0) {
            return ComparisonResult::LESS;
        }
        if (result == 0) {
            return ComparisonResult::EQUAL;
        }
        return ComparisonResult::GREAT;
    }
    if (primX->IsBigInt()) {
        if (primY->IsNumber()) {
            JSHandle<BigInt> bigint = JSHandle<BigInt>::Cast(primX);
            return BigInt::CompareWithNumber(bigint, primY);
        } else if (primY->IsString()) {
            JSHandle<JSTaggedValue> bigY(thread, base::NumberHelper::StringToBigInt(thread, primY));
            if (!bigY->IsBigInt()) {
                return ComparisonResult::UNDEFINED;
            }
            return BigInt::Compare(primX.GetTaggedValue(), bigY.GetTaggedValue());
        } else {
            JSHandle<JSTaggedValue> bigY(thread, ToBigInt(thread, primY));
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, ComparisonResult::UNDEFINED);
            return BigInt::Compare(primX.GetTaggedValue(), bigY.GetTaggedValue());
        }
    }
    if (primY->IsBigInt()) {
        ComparisonResult res = Compare(thread, primY, primX);
        RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, ComparisonResult::UNDEFINED);
        if (res == ComparisonResult::GREAT) {
            return ComparisonResult::LESS;
        } else if (res == ComparisonResult::LESS) {
            return ComparisonResult::GREAT;
        }
        return res;
    }
    JSTaggedNumber xNumber = ToNumber(thread, x);
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, ComparisonResult::UNDEFINED);
    JSTaggedNumber yNumber = ToNumber(thread, y);
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, ComparisonResult::UNDEFINED);
    return StrictNumberCompare(xNumber.GetNumber(), yNumber.GetNumber());
}

bool JSTaggedValue::IsSameTypeOrHClass(JSTaggedValue x, JSTaggedValue y)
{
    if (x.IsNumber() && y.IsNumber()) {
        return true;
    }
    if (x.IsBoolean() && y.IsBoolean()) {
        return true;
    }
    if (x.IsString() && y.IsString()) {
        return true;
    }
    if (x.IsHeapObject() && y.IsHeapObject()) {
        return x.GetTaggedObject()->GetClass() == y.GetTaggedObject()->GetClass();
    }

    return false;
}

JSTaggedValue JSTaggedValue::ToPrimitive(JSThread *thread, const JSHandle<JSTaggedValue> &tagged,
                                         PreferredPrimitiveType type)
{
    if (tagged->IsECMAObject()) {
        EcmaVM *vm = thread->GetEcmaVM();
        JSHandle<JSTaggedValue> keyString = vm->GetGlobalEnv()->GetToPrimitiveSymbol();

        JSHandle<JSTaggedValue> exoticToprim = JSObject::GetMethod(thread, tagged, keyString);
        RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, JSTaggedValue::Exception());
        if (!exoticToprim->IsUndefined()) {
            JSTaggedValue value = GetTypeString(thread, type).GetTaggedValue();
            JSHandle<JSTaggedValue> undefined = thread->GlobalConstants()->GetHandledUndefined();
            EcmaRuntimeCallInfo *info =
                EcmaInterpreter::NewRuntimeCallInfo(thread, exoticToprim, tagged, undefined, 1);
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, JSTaggedValue::Exception());
            info->SetCallArg(value);
            JSTaggedValue valueResult = JSFunction::Call(info);
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, JSTaggedValue::Exception());
            if (!valueResult.IsECMAObject()) {
                return valueResult;
            }
            THROW_TYPE_ERROR_AND_RETURN(thread, "", JSTaggedValue::Exception());
        } else {
            type = (type == NO_PREFERENCE) ? PREFER_NUMBER : type;
            return OrdinaryToPrimitive(thread, tagged, type);
        }
    }
    return tagged.GetTaggedValue();
}

JSTaggedValue JSTaggedValue::OrdinaryToPrimitive(JSThread *thread, const JSHandle<JSTaggedValue> &tagged,
                                                 PreferredPrimitiveType type)
{
    static_assert(PREFER_NUMBER == 0 && PREFER_STRING == 1);
    ASSERT(tagged->IsECMAObject());
    auto globalConst = thread->GlobalConstants();
    for (uint8_t i = 0; i < 2; i++) {  // 2: 2 means value has 2 target types, string or value.
        JSHandle<JSTaggedValue> keyString;
        if ((static_cast<uint8_t>(type) ^ i) != 0) {
            keyString = globalConst->GetHandledToStringString();
        } else {
            keyString = globalConst->GetHandledValueOfString();
        }
        JSHandle<JSTaggedValue> entryfunc = GetProperty(thread, tagged, keyString).GetValue();
        if (entryfunc->IsCallable()) {
            JSHandle<JSTaggedValue> undefined = globalConst->GetHandledUndefined();
            EcmaRuntimeCallInfo *info =
                EcmaInterpreter::NewRuntimeCallInfo(thread, entryfunc, tagged, undefined, 0);
            JSTaggedValue valueResult = JSFunction::Call(info);
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, JSTaggedValue::Exception());
            if (!valueResult.IsECMAObject()) {
                return valueResult;
            }
        }
    }
    THROW_TYPE_ERROR_AND_RETURN(thread, "Cannot convert a illegal value to a Primitive", JSTaggedValue::Undefined());
}

JSHandle<EcmaString> JSTaggedValue::ToString(JSThread *thread, JSTaggedValue val)
{
    JSHandle<JSTaggedValue> tagged(thread, val);
    return ToString(thread, tagged);
}

JSHandle<EcmaString> JSTaggedValue::ToString(JSThread *thread, const JSHandle<JSTaggedValue> &tagged)
{
    if (tagged->IsString()) {
        return JSHandle<EcmaString>(tagged);
    }
    auto globalConst = thread->GlobalConstants();
    if (tagged->IsSpecial()) {
        switch (tagged->GetRawData()) {
            case VALUE_UNDEFINED: {
                return JSHandle<EcmaString>(globalConst->GetHandledUndefinedString());
            }
            case VALUE_NULL: {
                return JSHandle<EcmaString>(globalConst->GetHandledNullString());
            }
            case VALUE_TRUE: {
                return JSHandle<EcmaString>(globalConst->GetHandledTrueString());
            }
            case VALUE_FALSE: {
                return JSHandle<EcmaString>(globalConst->GetHandledFalseString());
            }
            case VALUE_HOLE: {
                return JSHandle<EcmaString>(globalConst->GetHandledEmptyString());
            }
            default:
                break;
        }
    }

    if (tagged->IsNumber()) {
        return base::NumberHelper::NumberToString(thread, tagged.GetTaggedValue());
    }

    if (tagged->IsBigInt()) {
        JSHandle<BigInt> taggedValue(tagged);
        return BigInt::ToString(thread, taggedValue);
    }

    auto emptyStr = globalConst->GetHandledEmptyString();
    if (tagged->IsECMAObject()) {
        JSHandle<JSTaggedValue> primValue(thread, ToPrimitive(thread, tagged, PREFER_STRING));
        RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, JSHandle<EcmaString>(emptyStr));
        return ToString(thread, primValue);
    }
    // Already Include Symbol
    THROW_TYPE_ERROR_AND_RETURN(thread, "Cannot convert a illegal value to a String", JSHandle<EcmaString>(emptyStr));
}

JSTaggedValue JSTaggedValue::CanonicalNumericIndexString(JSThread *thread, const JSHandle<JSTaggedValue> &tagged)
{
    if (tagged->IsNumber()) {
        return tagged.GetTaggedValue();
    }

    if (tagged->IsString()) {
        JSHandle<EcmaString> str = thread->GetEcmaVM()->GetFactory()->NewFromASCII("-0");
        if (EcmaStringAccessor::StringsAreEqual(static_cast<EcmaString *>(tagged->GetTaggedObject()), *str)) {
            return JSTaggedValue(-0.0);
        }
        JSHandle<JSTaggedValue> tmp(thread, ToNumber(thread, tagged));
        if (SameValue(ToString(thread, tmp).GetTaggedValue(), tagged.GetTaggedValue())) {
            return tmp.GetTaggedValue();
        }
    }
    return JSTaggedValue::Undefined();
}

JSHandle<JSObject> JSTaggedValue::ToObject(JSThread *thread, const JSHandle<JSTaggedValue> &tagged)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    if (tagged->IsInt() || tagged->IsDouble()) {
        return JSHandle<JSObject>::Cast(factory->NewJSPrimitiveRef(PrimitiveType::PRIMITIVE_NUMBER, tagged));
    }

    switch (tagged->GetRawData()) {
        case JSTaggedValue::VALUE_UNDEFINED: {
            THROW_TYPE_ERROR_AND_RETURN(thread, "Cannot convert a UNDEFINED value to a JSObject",
                                        JSHandle<JSObject>(thread, JSTaggedValue::Exception()));
        }
        case JSTaggedValue::VALUE_HOLE: {
            THROW_TYPE_ERROR_AND_RETURN(thread, "Cannot convert a HOLE value to a JSObject",
                                        JSHandle<JSObject>(thread, JSTaggedValue::Exception()));
        }
        case JSTaggedValue::VALUE_NULL: {
            THROW_TYPE_ERROR_AND_RETURN(thread, "Cannot convert a NULL value to a JSObject",
                                        JSHandle<JSObject>(thread, JSTaggedValue::Exception()));
        }
        case JSTaggedValue::VALUE_TRUE:
        case JSTaggedValue::VALUE_FALSE: {
            return JSHandle<JSObject>::Cast(factory->NewJSPrimitiveRef(PrimitiveType::PRIMITIVE_BOOLEAN, tagged));
        }
        default: {
            break;
        }
    }

    if (tagged->IsECMAObject()) {
        return JSHandle<JSObject>::Cast(tagged);
    }
    if (tagged->IsSymbol()) {
        return JSHandle<JSObject>::Cast(factory->NewJSPrimitiveRef(PrimitiveType::PRIMITIVE_SYMBOL, tagged));
    }
    if (tagged->IsString()) {
        return JSHandle<JSObject>::Cast(factory->NewJSPrimitiveRef(PrimitiveType::PRIMITIVE_STRING, tagged));
    }
    if (tagged->IsBigInt()) {
        return JSHandle<JSObject>::Cast(factory->NewJSPrimitiveRef(PrimitiveType::PRIMITIVE_BIGINT, tagged));
    }
    THROW_TYPE_ERROR_AND_RETURN(thread, "Cannot convert a Unknown object value to a JSObject",
                                JSHandle<JSObject>(thread, JSTaggedValue::Exception()));
}

// 7.3.1 Get ( O, P )
OperationResult JSTaggedValue::GetProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                           const JSHandle<JSTaggedValue> &key)
{
    if (obj->IsUndefined() || obj->IsNull() || obj->IsHole()) {
        std::string keyStr = EcmaStringAccessor(ToString(thread, key)).ToStdString();
        std::string objStr = EcmaStringAccessor(ToString(thread, obj)).ToStdString();
        std::string message = "Cannot read property ";
        message.append(keyStr).append(" of ").append(objStr);
        THROW_TYPE_ERROR_AND_RETURN(thread, message.c_str(),
                                    OperationResult(thread, JSTaggedValue::Exception(), PropertyMetaData(false)));
    }
    ASSERT_PRINT(JSTaggedValue::IsPropertyKey(key), "Key is not a property key");

    if (obj->IsJSProxy()) {
        return JSProxy::GetProperty(thread, JSHandle<JSProxy>(obj), key);
    }
    if (obj->IsTypedArray()) {
        return JSTypedArray::GetProperty(thread, obj, key);
    }
    if (obj->IsModuleNamespace()) {
        return ModuleNamespace::GetProperty(thread, obj, key);
    }

    if (obj->IsSpecialContainer()) {
        return GetJSAPIProperty(thread, obj, key);
    }

    return JSObject::GetProperty(thread, obj, key);
}

OperationResult JSTaggedValue::GetProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj, uint32_t key)
{
    if (obj->IsUndefined() || obj->IsNull() || obj->IsHole()) {
        std::string objStr = EcmaStringAccessor(ToString(thread, obj)).ToStdString();
        std::string message = "Cannot read property ";
        message.append(ToCString(key)).append(" of ").append(objStr);
        THROW_TYPE_ERROR_AND_RETURN(thread, message.c_str(),
                                    OperationResult(thread, JSTaggedValue::Exception(), PropertyMetaData(false)));
    }

    if (obj->IsJSProxy()) {
        JSHandle<JSTaggedValue> keyHandle(thread, JSTaggedValue(key));
        return JSProxy::GetProperty(thread, JSHandle<JSProxy>(obj), keyHandle);
    }

    if (obj->IsTypedArray()) {
        return JSTypedArray::GetProperty(thread, obj, key);
    }

    if (obj->IsSpecialContainer()) {
        JSHandle<JSTaggedValue> keyHandle = JSHandle<JSTaggedValue>(thread, JSTaggedValue(key));
        return GetJSAPIProperty(thread, obj, keyHandle);
    }

    return JSObject::GetProperty(thread, obj, key);
}

OperationResult JSTaggedValue::GetProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                           const JSHandle<JSTaggedValue> &key, const JSHandle<JSTaggedValue> &receiver)
{
    if (obj->IsUndefined() || obj->IsNull() || obj->IsHole()) {
        std::string keyStr = EcmaStringAccessor(ToString(thread, key)).ToStdString();
        std::string objStr = EcmaStringAccessor(ToString(thread, obj)).ToStdString();
        std::string message = "Cannot read property ";
        message.append(keyStr).append(" of ").append(objStr);
        THROW_TYPE_ERROR_AND_RETURN(thread, message.c_str(),
                                    OperationResult(thread, JSTaggedValue::Exception(), PropertyMetaData(false)));
    }
    ASSERT_PRINT(JSTaggedValue::IsPropertyKey(key), "Key is not a property key");

    if (obj->IsJSProxy()) {
        return JSProxy::GetProperty(thread, JSHandle<JSProxy>(obj), key, receiver);
    }
    if (obj->IsTypedArray()) {
        return JSTypedArray::GetProperty(thread, obj, key, receiver);
    }

    if (obj->IsSpecialContainer()) {
        return GetJSAPIProperty(thread, obj, key);
    }

    return JSObject::GetProperty(thread, obj, key, receiver);
}

// 7.3.3 Set (O, P, V, Throw)
bool JSTaggedValue::SetProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                const JSHandle<JSTaggedValue> &key, const JSHandle<JSTaggedValue> &value, bool mayThrow)
{
    if (obj->IsUndefined() || obj->IsNull() || obj->IsHole()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Obj is not a Valid object", false);
    }

    ASSERT_PRINT(JSTaggedValue::IsPropertyKey(key), "Key is not a property key");

    // 4. Let success be O.[[Set]](P, V, O).
    bool success = false;
    if (obj->IsJSProxy()) {
        success = JSProxy::SetProperty(thread, JSHandle<JSProxy>(obj), key, value, mayThrow);
    } else if (obj->IsTypedArray()) {
        success = JSTypedArray::SetProperty(thread, obj, key, value, mayThrow);
    } else if (obj->IsModuleNamespace()) {
        success = ModuleNamespace::SetProperty(thread, mayThrow);
    } else if (obj->IsSpecialContainer()) {
        success = SetJSAPIProperty(thread, obj, key, value);
    } else {
        success = JSObject::SetProperty(thread, obj, key, value, mayThrow);
    }
    // 5. ReturnIfAbrupt(success).
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, success);
    // 6. If success is false and Throw is true, throw a TypeError exception.
    // have done in JSObject::SetPropert.
    return success;
}

bool JSTaggedValue::SetProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj, uint32_t key,
                                const JSHandle<JSTaggedValue> &value, bool mayThrow)
{
    if (obj->IsUndefined() || obj->IsNull() || obj->IsHole()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Obj is not a Valid object", false);
    }

    // 4. Let success be O.[[Set]](P, V, O).
    bool success = false;
    if (obj->IsJSProxy()) {
        JSHandle<JSTaggedValue> keyHandle(thread, JSTaggedValue(key));
        success = JSProxy::SetProperty(thread, JSHandle<JSProxy>(obj), keyHandle, value, mayThrow);
    } else if (obj->IsTypedArray()) {
        JSHandle<JSTaggedValue> keyHandle(thread, JSTaggedValue(key));
        success = JSTypedArray::SetProperty(thread, obj, keyHandle, value, mayThrow);
    } else if (obj->IsModuleNamespace()) {
        success = ModuleNamespace::SetProperty(thread, mayThrow);
    } else if (obj->IsSpecialContainer()) {
        JSHandle<JSTaggedValue> keyHandle = JSHandle<JSTaggedValue>(thread, JSTaggedValue(key));
        success = SetJSAPIProperty(thread, obj, keyHandle, value);
    } else {
        success = JSObject::SetProperty(thread, obj, key, value, mayThrow);
    }
    // 5. ReturnIfAbrupt(success).
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, success);
    // 6. If success is false and Throw is true, throw a TypeError exception.
    // have done in JSObject::SetPropert.
    return success;
}

bool JSTaggedValue::SetProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                const JSHandle<JSTaggedValue> &key, const JSHandle<JSTaggedValue> &value,
                                const JSHandle<JSTaggedValue> &receiver, bool mayThrow)
{
    if (obj->IsUndefined() || obj->IsNull() || obj->IsHole()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Obj is not a Valid object", false);
    }

    ASSERT_PRINT(JSTaggedValue::IsPropertyKey(key), "Key is not a property key");

    // 4. Let success be O.[[Set]](P, V, O).
    bool success = false;
    if (obj->IsJSProxy()) {
        success = JSProxy::SetProperty(thread, JSHandle<JSProxy>(obj), key, value, receiver, mayThrow);
    } else if (obj->IsTypedArray()) {
        success = JSTypedArray::SetProperty(thread, obj, key, value, receiver, mayThrow);
    } else if (obj->IsModuleNamespace()) {
        success = ModuleNamespace::SetProperty(thread, mayThrow);
    } else if (obj->IsSpecialContainer()) {
        success = SetJSAPIProperty(thread, obj, key, value);
    } else {
        success = JSObject::SetProperty(thread, obj, key, value, receiver, mayThrow);
    }
    // 5. ReturnIfAbrupt(success).
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, success);
    // 6. If success is false and Throw is true, throw a TypeError exception.
    // have done in JSObject::SetPropert.
    return success;
}

bool JSTaggedValue::DeleteProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                   const JSHandle<JSTaggedValue> &key)
{
    if (obj->IsJSProxy()) {
        return JSProxy::DeleteProperty(thread, JSHandle<JSProxy>(obj), key);
    }

    if (obj->IsModuleNamespace()) {
        return ModuleNamespace::DeleteProperty(thread, obj, key);
    }

    if (obj->IsTypedArray()) {
        return JSTypedArray::DeleteProperty(thread, obj, key);
    }

    if (obj->IsSpecialContainer()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Can not delete property in Container Object", false);
    }

    return JSObject::DeleteProperty(thread, JSHandle<JSObject>(obj), key);
}

// 7.3.8 DeletePropertyOrThrow (O, P)
bool JSTaggedValue::DeletePropertyOrThrow(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                          const JSHandle<JSTaggedValue> &key)
{
    if (!obj->IsECMAObject()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Obj is not a valid object", false);
    }
    ASSERT_PRINT(JSTaggedValue::IsPropertyKey(key), "Key is not a property key");

    // 3. Let success be O.[[Delete]](P).
    bool success = DeleteProperty(thread, obj, key);

    // 4. ReturnIfAbrupt(success).
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, success);
    // 5. If success is false, throw a TypeError exception
    if (!success) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Cannot delete property", false);
    }
    return success;
}

// 7.3.7 DefinePropertyOrThrow (O, P, desc)
bool JSTaggedValue::DefinePropertyOrThrow(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                          const JSHandle<JSTaggedValue> &key, const PropertyDescriptor &desc)
{
    // 1. Assert: Type(O) is Object.
    // 2. Assert: IsPropertyKey(P) is true.
    ASSERT_PRINT(obj->IsECMAObject(), "Obj is not a valid object");
    ASSERT_PRINT(JSTaggedValue::IsPropertyKey(key), "Key is not a property key");
    // 3. Let success be ? O.[[DefineOwnProperty]](P, desc).
    bool success = DefineOwnProperty(thread, obj, key, desc);
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, false);
    // 4. If success is false, throw a TypeError exception.
    if (!success) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "", false);
    }
    return success;
}

bool JSTaggedValue::DefineOwnProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                      const JSHandle<JSTaggedValue> &key, const PropertyDescriptor &desc)
{
    if (obj->IsJSArray()) {
        return JSArray::DefineOwnProperty(thread, JSHandle<JSObject>(obj), key, desc);
    }

    if (obj->IsJSProxy()) {
        return JSProxy::DefineOwnProperty(thread, JSHandle<JSProxy>(obj), key, desc);
    }

    if (obj->IsTypedArray()) {
        return JSTypedArray::DefineOwnProperty(thread, obj, key, desc);
    }

    if (obj->IsModuleNamespace()) {
        return ModuleNamespace::DefineOwnProperty(thread, obj, key, desc);
    }

    if (obj->IsSpecialContainer()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Can not defineProperty on Container Object", false);
    }

    return JSObject::DefineOwnProperty(thread, JSHandle<JSObject>(obj), key, desc);
}

bool JSTaggedValue::GetOwnProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                   const JSHandle<JSTaggedValue> &key, PropertyDescriptor &desc)
{
    if (obj->IsJSProxy()) {
        return JSProxy::GetOwnProperty(thread, JSHandle<JSProxy>(obj), key, desc);
    }
    if (obj->IsTypedArray()) {
        return JSTypedArray::GetOwnProperty(thread, obj, key, desc);
    }
    if (obj->IsModuleNamespace()) {
        return ModuleNamespace::GetOwnProperty(thread, obj, key, desc);
    }
    if (obj->IsSpecialContainer()) {
        return GetContainerProperty(thread, obj, key, desc);
    }
    return JSObject::GetOwnProperty(thread, JSHandle<JSObject>(obj), key, desc);
}

bool JSTaggedValue::SetPrototype(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                 const JSHandle<JSTaggedValue> &proto)
{
    if (obj->IsJSProxy()) {
        return JSProxy::SetPrototype(thread, JSHandle<JSProxy>(obj), proto);
    }
    if (obj->IsModuleNamespace()) {
        return ModuleNamespace::SetPrototype(obj, proto);
    }
    if (obj->IsSpecialContainer() || !obj->IsECMAObject()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Can not set Prototype on Container or non ECMA Object", false);
    }

    return JSObject::SetPrototype(thread, JSHandle<JSObject>(obj), proto);
}

JSTaggedValue JSTaggedValue::GetPrototype(JSThread *thread, const JSHandle<JSTaggedValue> &obj)
{
    if (!obj->IsECMAObject()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Can not get Prototype on non ECMA Object", JSTaggedValue::Exception());
    }
    if (obj->IsJSProxy()) {
        return JSProxy::GetPrototype(thread, JSHandle<JSProxy>(obj));
    }
    return JSObject::GetPrototype(JSHandle<JSObject>(obj));
}
bool JSTaggedValue::PreventExtensions(JSThread *thread, const JSHandle<JSTaggedValue> &obj)
{
    if (obj->IsJSProxy()) {
        return JSProxy::PreventExtensions(thread, JSHandle<JSProxy>(obj));
    }
    if (obj->IsModuleNamespace()) {
        return ModuleNamespace::PreventExtensions();
    }
    return JSObject::PreventExtensions(thread, JSHandle<JSObject>(obj));
}

JSHandle<TaggedArray> JSTaggedValue::GetOwnPropertyKeys(JSThread *thread, const JSHandle<JSTaggedValue> &obj)
{
    if (obj->IsJSProxy()) {
        return JSProxy::OwnPropertyKeys(thread, JSHandle<JSProxy>(obj));
    }
    if (obj->IsTypedArray()) {
        return JSTypedArray::OwnPropertyKeys(thread, obj);
    }
    if (obj->IsSpecialContainer()) {
        return GetOwnContainerPropertyKeys(thread, obj);
    }
    if (obj->IsModuleNamespace()) {
        return ModuleNamespace::OwnPropertyKeys(thread, obj);
    }
    return JSObject::GetOwnPropertyKeys(thread, JSHandle<JSObject>(obj));
}

// 7.3.10 HasProperty (O, P)
bool JSTaggedValue::HasProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                const JSHandle<JSTaggedValue> &key)
{
    if (obj->IsJSProxy()) {
        return JSProxy::HasProperty(thread, JSHandle<JSProxy>(obj), key);
    }
    if (obj->IsTypedArray()) {
        return JSTypedArray::HasProperty(thread, obj, key);
    }
    if (obj->IsModuleNamespace()) {
        return ModuleNamespace::HasProperty(thread, obj, key);
    }
    if (obj->IsSpecialContainer()) {
        return HasContainerProperty(thread, obj, key);
    }
    return JSObject::HasProperty(thread, JSHandle<JSObject>(obj), key);
}

bool JSTaggedValue::HasProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj, uint32_t key)
{
    if (obj->IsJSProxy()) {
        JSHandle<JSTaggedValue> keyHandle(thread, JSTaggedValue(key));
        return JSProxy::HasProperty(thread, JSHandle<JSProxy>(obj), keyHandle);
    }
    if (obj->IsTypedArray()) {
        JSHandle<JSTaggedValue> key_handle(thread, JSTaggedValue(key));
        return JSTypedArray::HasProperty(thread, obj, key_handle);
    }
    if (obj->IsSpecialContainer()) {
        return HasContainerProperty(thread, obj, JSHandle<JSTaggedValue>(thread, JSTaggedValue(key)));
    }
    return JSObject::HasProperty(thread, JSHandle<JSObject>(obj), key);
}

// 7.3.11 HasOwnProperty (O, P)
bool JSTaggedValue::HasOwnProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                   const JSHandle<JSTaggedValue> &key)
{
    ASSERT_PRINT(JSTaggedValue::IsPropertyKey(key), "Key is not a property key");

    PropertyDescriptor desc(thread);
    return JSTaggedValue::GetOwnProperty(thread, obj, key, desc);
}

bool JSTaggedValue::GlobalHasOwnProperty(JSThread *thread, const JSHandle<JSTaggedValue> &key)
{
    ASSERT_PRINT(JSTaggedValue::IsPropertyKey(key), "Key is not a property key");

    PropertyDescriptor desc(thread);
    return JSObject::GlobalGetOwnProperty(thread, key, desc);
}

JSTaggedNumber JSTaggedValue::ToIndex(JSThread *thread, const JSHandle<JSTaggedValue> &tagged)
{
    if (tagged->IsInt() && tagged->GetInt() >= 0) {
        return JSTaggedNumber(tagged.GetTaggedValue());
    }
    if (tagged->IsUndefined()) {
        return JSTaggedNumber(0);
    }
    JSTaggedNumber integerIndex = ToNumber(thread, tagged);
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, JSTaggedNumber::Exception());
    if (integerIndex.IsInt() && integerIndex.GetInt() >= 0) {
        return integerIndex;
    }
    double len = base::NumberHelper::TruncateDouble(integerIndex.GetNumber());
    if (len < 0.0 || len > SAFE_NUMBER) {
        THROW_RANGE_ERROR_AND_RETURN(thread, "integerIndex < 0 or integerIndex > SAFE_NUMBER",
                                     JSTaggedNumber::Exception());
    }
    return JSTaggedNumber(len);
}

JSHandle<JSTaggedValue> JSTaggedValue::ToPrototypeOrObj(JSThread *thread, const JSHandle<JSTaggedValue> &obj)
{
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();

    if (obj->IsNumber()) {
        return JSHandle<JSTaggedValue>(thread,
                                       env->GetNumberFunction().GetObject<JSFunction>()->GetFunctionPrototype());
    }
    if (obj->IsBoolean()) {
        return JSHandle<JSTaggedValue>(thread,
                                       env->GetBooleanFunction().GetObject<JSFunction>()->GetFunctionPrototype());
    }
    if (obj->IsString()) {
        return JSHandle<JSTaggedValue>(thread,
                                       env->GetStringFunction().GetObject<JSFunction>()->GetFunctionPrototype());
    }
    if (obj->IsSymbol()) {
        return JSHandle<JSTaggedValue>(thread,
                                       env->GetSymbolFunction().GetObject<JSFunction>()->GetFunctionPrototype());
    }
    if (obj->IsBigInt()) {
        return JSHandle<JSTaggedValue>(thread,
                                       env->GetBigIntFunction().GetObject<JSFunction>()->GetFunctionPrototype());
    }
    return obj;
}

JSTaggedValue JSTaggedValue::GetSuperBase(JSThread *thread, const JSHandle<JSTaggedValue> &obj)
{
    if (obj->IsUndefined()) {
        return JSTaggedValue::Undefined();
    }

    ASSERT(obj->IsECMAObject());
    return JSTaggedValue::GetPrototype(thread, obj);
}

bool JSTaggedValue::HasContainerProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                         const JSHandle<JSTaggedValue> &key)
{
    auto *hclass = obj->GetTaggedObject()->GetClass();
    JSType jsType = hclass->GetObjectType();
    switch (jsType) {
        case JSType::JS_API_ARRAY_LIST: {
            return JSHandle<JSAPIArrayList>::Cast(obj)->Has(key.GetTaggedValue());
        }
        case JSType::JS_API_QUEUE: {
            return JSHandle<JSAPIQueue>::Cast(obj)->Has(key.GetTaggedValue());
        }
        case JSType::JS_API_PLAIN_ARRAY: {
            return JSObject::HasProperty(thread, JSHandle<JSObject>(obj), key);
        }
        case JSType::JS_API_DEQUE: {
            return JSHandle<JSAPIDeque>::Cast(obj)->Has(key.GetTaggedValue());
        }
        case JSType::JS_API_STACK: {
            return JSHandle<JSAPIStack>::Cast(obj)->Has(key.GetTaggedValue());
        }
        case JSType::JS_API_LIST: {
            JSHandle<JSAPIList> list = JSHandle<JSAPIList>::Cast(obj);
            return list->Has(key.GetTaggedValue());
        }
        case JSType::JS_API_LINKED_LIST: {
            JSHandle<JSAPILinkedList> linkedList = JSHandle<JSAPILinkedList>::Cast(obj);
            return linkedList->Has(key.GetTaggedValue());
        }
        case JSType::JS_API_HASH_MAP:
        case JSType::JS_API_HASH_SET:
        case JSType::JS_API_LIGHT_WEIGHT_MAP:
        case JSType::JS_API_LIGHT_WEIGHT_SET:
        case JSType::JS_API_TREE_MAP:
        case JSType::JS_API_TREE_SET: {
            return JSObject::HasProperty(thread, JSHandle<JSObject>(obj), key);
        }
        case JSType::JS_API_VECTOR: {
            return JSHandle<JSAPIVector>::Cast(obj)->Has(key.GetTaggedValue());
        }
        default: {
            UNREACHABLE();
        }
    }
    return false;
}

JSHandle<TaggedArray> JSTaggedValue::GetOwnContainerPropertyKeys(JSThread *thread, const JSHandle<JSTaggedValue> &obj)
{
    auto *hclass = obj->GetTaggedObject()->GetClass();
    JSType jsType = hclass->GetObjectType();
    switch (jsType) {
        case JSType::JS_API_ARRAY_LIST: {
            return JSAPIArrayList::OwnKeys(thread, JSHandle<JSAPIArrayList>::Cast(obj));
        }
        case JSType::JS_API_QUEUE: {
            return JSAPIQueue::OwnKeys(thread, JSHandle<JSAPIQueue>::Cast(obj));
        }
        case JSType::JS_API_PLAIN_ARRAY: {
            return JSObject::GetOwnPropertyKeys(thread, JSHandle<JSObject>(obj));
        }
        case JSType::JS_API_DEQUE: {
            return JSAPIDeque::OwnKeys(thread, JSHandle<JSAPIDeque>::Cast(obj));
        }
        case JSType::JS_API_STACK: {
            return JSAPIStack::OwnKeys(thread, JSHandle<JSAPIStack>::Cast(obj));
        }
        case JSType::JS_API_LIST: {
            return JSAPIList::OwnKeys(thread, JSHandle<JSAPIList>::Cast(obj));
        }
        case JSType::JS_API_LINKED_LIST: {
            return JSAPILinkedList::OwnKeys(thread, JSHandle<JSAPILinkedList>::Cast(obj));
        }
        case JSType::JS_API_HASH_MAP:
        case JSType::JS_API_HASH_SET:
        case JSType::JS_API_LIGHT_WEIGHT_MAP:
        case JSType::JS_API_LIGHT_WEIGHT_SET:
        case JSType::JS_API_TREE_MAP:
        case JSType::JS_API_TREE_SET: {
            return JSObject::GetOwnPropertyKeys(thread, JSHandle<JSObject>(obj));
        }
        case JSType::JS_API_VECTOR: {
            return JSAPIVector::OwnKeys(thread, JSHandle<JSAPIVector>::Cast(obj));
        }
        default: {
            UNREACHABLE();
        }
    }
    return thread->GetEcmaVM()->GetFactory()->EmptyArray();
}

bool JSTaggedValue::GetContainerProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                         const JSHandle<JSTaggedValue> &key, PropertyDescriptor &desc)
{
    if (key->IsInteger()) {
        auto *hclass = obj->GetTaggedObject()->GetClass();
        JSType jsType = hclass->GetObjectType();
        switch (jsType) {
            case JSType::JS_API_ARRAY_LIST: {
                return JSAPIArrayList::GetOwnProperty(thread, JSHandle<JSAPIArrayList>::Cast(obj), key);
            }
            case JSType::JS_API_QUEUE: {
                return JSAPIQueue::GetOwnProperty(thread, JSHandle<JSAPIQueue>::Cast(obj), key);
            }
            case JSType::JS_API_DEQUE: {
                return JSAPIDeque::GetOwnProperty(thread, JSHandle<JSAPIDeque>::Cast(obj), key);
            }
            case JSType::JS_API_STACK: {
                return JSAPIStack::GetOwnProperty(thread, JSHandle<JSAPIStack>::Cast(obj), key);
            }
            case JSType::JS_API_LIST: {
                return JSAPIList::GetOwnProperty(thread, JSHandle<JSAPIList>::Cast(obj), key);
            }
            case JSType::JS_API_LINKED_LIST: {
                return JSAPILinkedList::GetOwnProperty(thread, JSHandle<JSAPILinkedList>::Cast(obj), key);
            }
            case JSType::JS_API_PLAIN_ARRAY: {
                return JSAPIPlainArray::GetOwnProperty(thread, JSHandle<JSAPIPlainArray>::Cast(obj), key);
            }
            case JSType::JS_API_VECTOR: {
                return JSAPIVector::GetOwnProperty(thread, JSHandle<JSAPIVector>::Cast(obj), key);
            }
            default: {
                return JSObject::GetOwnProperty(thread, JSHandle<JSObject>(obj), key, desc);
            }
        }
    } else {
        return JSObject::GetOwnProperty(thread, JSHandle<JSObject>(obj), key, desc);
    }
    return false;
}
JSHandle<JSTaggedValue> JSTaggedValue::ToNumeric(JSThread *thread, JSHandle<JSTaggedValue> tagged)
{
    // 1. Let primValue be ? ToPrimitive(value, number)
    JSHandle<JSTaggedValue> primValue(thread, ToPrimitive(thread, tagged, PREFER_NUMBER));
    RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
    // 2. If Type(primValue) is BigInt, return primValue.
    if (primValue->IsBigInt()) {
        return primValue;
    }
    // 3. Return ? ToNumber(primValue).
    JSTaggedNumber number = ToNumber(thread, primValue);
    RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
    JSHandle<JSTaggedValue> value(thread, number);
    return value;
}
OperationResult JSTaggedValue::GetJSAPIProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                                const JSHandle<JSTaggedValue> &key)
{
    if (key->IsInteger()) {
        auto *hclass = obj->GetTaggedObject()->GetClass();
        JSType jsType = hclass->GetObjectType();
        switch (jsType) {
            case JSType::JS_API_ARRAY_LIST: {
                return JSAPIArrayList::GetProperty(thread, JSHandle<JSAPIArrayList>::Cast(obj), key);
            }
            case JSType::JS_API_LIST: {
                return JSAPIList::GetProperty(thread, JSHandle<JSAPIList>::Cast(obj), key);
            }
            case JSType::JS_API_LINKED_LIST: {
                return JSAPILinkedList::GetProperty(thread, JSHandle<JSAPILinkedList>::Cast(obj), key);
            }
            case JSType::JS_API_QUEUE: {
                return JSAPIQueue::GetProperty(thread, JSHandle<JSAPIQueue>::Cast(obj), key);
            }
            case JSType::JS_API_DEQUE: {
                return JSAPIDeque::GetProperty(thread, JSHandle<JSAPIDeque>::Cast(obj), key);
            }
            case JSType::JS_API_STACK: {
                return JSAPIStack::GetProperty(thread, JSHandle<JSAPIStack>::Cast(obj), key);
            }
            case JSType::JS_API_PLAIN_ARRAY: {
                return JSAPIPlainArray::GetProperty(thread, JSHandle<JSAPIPlainArray>::Cast(obj), key);
            }
            case JSType::JS_API_VECTOR: {
                return JSAPIVector::GetProperty(thread, JSHandle<JSAPIVector>::Cast(obj), key);
            }
            default: {
                return JSObject::GetProperty(thread, JSHandle<JSObject>(obj), key);
            }
        }
    } else {
        return JSObject::GetProperty(thread, JSHandle<JSObject>(obj), key);
    }
    return OperationResult(thread, JSTaggedValue::Exception(), PropertyMetaData(false));
}

bool JSTaggedValue::SetJSAPIProperty(JSThread *thread, const JSHandle<JSTaggedValue> &obj,
                                     const JSHandle<JSTaggedValue> &key,
                                     const JSHandle<JSTaggedValue> &value)
{
    if (key->IsInteger()) {
        auto *hclass = obj->GetTaggedObject()->GetClass();
        JSType jsType = hclass->GetObjectType();
        switch (jsType) {
            case JSType::JS_API_ARRAY_LIST: {
                return JSAPIArrayList::SetProperty(thread, JSHandle<JSAPIArrayList>::Cast(obj), key, value);
            }
            case JSType::JS_API_LIST: {
                return JSAPIList::SetProperty(thread, JSHandle<JSAPIList>::Cast(obj), key, value);
            }
            case JSType::JS_API_LINKED_LIST: {
                return JSAPILinkedList::SetProperty(thread, JSHandle<JSAPILinkedList>::Cast(obj), key, value);
            }
            case JSType::JS_API_QUEUE: {
                return JSAPIQueue::SetProperty(thread, JSHandle<JSAPIQueue>::Cast(obj), key, value);
            }
            case JSType::JS_API_DEQUE: {
                return JSAPIDeque::SetProperty(thread, JSHandle<JSAPIDeque>::Cast(obj), key, value);
            }
            case JSType::JS_API_STACK: {
                return JSAPIStack::SetProperty(thread, JSHandle<JSAPIStack>::Cast(obj), key, value);
            }
            case JSType::JS_API_PLAIN_ARRAY: {
                return JSAPIPlainArray::SetProperty(thread, JSHandle<JSAPIPlainArray>::Cast(obj), key, value);
            }
            case JSType::JS_API_VECTOR: {
                return JSAPIVector::SetProperty(thread, JSHandle<JSAPIVector>::Cast(obj), key, value);
            }
            default: {
                return JSObject::SetProperty(thread, JSHandle<JSObject>::Cast(obj), key, value);
            }
        }
    } else {
        THROW_TYPE_ERROR_AND_RETURN(thread, "Cannot set property on Container", false);
    }
    THROW_TYPE_ERROR_AND_RETURN(thread, "Cannot set property on Container", false);
}
}  // namespace panda::ecmascript
