/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
 
#include "ecmascript/js_api/js_api_plain_array.h"

#include "ecmascript/containers/containers_errors.h"
#include "ecmascript/interpreter/interpreter.h"
#include "ecmascript/js_api/js_api_plain_array_iterator.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/object_factory.h"

namespace panda::ecmascript {
using ContainerError = containers::ContainerError;
using ErrorFlag = containers::ErrorFlag;
void JSAPIPlainArray::Add(JSThread *thread, const JSHandle<JSAPIPlainArray> &obj, JSHandle<JSTaggedValue> key,
                          JSHandle<JSTaggedValue> value)
{
    JSHandle<TaggedArray> keyArray(thread, obj->GetKeys());
    JSHandle<TaggedArray> valueArray(thread, obj->GetValues());
    int32_t size = obj->GetLength();
    int32_t index = obj->BinarySearch(*keyArray, 0, size, key.GetTaggedValue().GetNumber());
    if (index >= 0) {
        keyArray->Set(thread, index, key);
        valueArray->Set(thread, index, value);
        return;
    }
    index ^= 0xFFFFFFFF;
    if (index < size) {
        obj->AdjustArray(thread, *keyArray, index, size, true);
        obj->AdjustArray(thread, *valueArray, index, size, true);
    }
    uint32_t capacity = valueArray->GetLength();
    if (size + 1 >= static_cast<int32_t>(capacity)) {
        uint32_t newCapacity = capacity << 1U;
        keyArray =
            thread->GetEcmaVM()->GetFactory()->CopyArray(keyArray, capacity, newCapacity);
        valueArray =
            thread->GetEcmaVM()->GetFactory()->CopyArray(valueArray, capacity, newCapacity);
        obj->SetKeys(thread, keyArray);
        obj->SetValues(thread, valueArray);
    }
    keyArray->Set(thread, index, key);
    valueArray->Set(thread, index, value);
    size++;
    obj->SetLength(size);
}

JSHandle<TaggedArray> JSAPIPlainArray::CreateSlot(const JSThread *thread, const uint32_t capacity)
{
    ASSERT_PRINT(capacity > 0, "size must be a non-negative integer");
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<TaggedArray> taggedArray = factory->NewTaggedArray(capacity, JSTaggedValue::Hole());
    return taggedArray;
}

bool JSAPIPlainArray::AdjustForward(JSThread *thread, int32_t index, int32_t forwardSize)
{
    int32_t size = GetLength();
    TaggedArray *keys = TaggedArray::Cast(GetKeys().GetTaggedObject());
    TaggedArray *values = TaggedArray::Cast(GetValues().GetTaggedObject());
    AdjustArray(thread, keys, index + forwardSize, index, false);
    AdjustArray(thread, values, index + forwardSize, index, false);
    size = size - forwardSize;
    SetLength(size);
    return true;
}

void JSAPIPlainArray::AdjustArray(JSThread *thread, TaggedArray *srcArray, int32_t fromIndex,
                                  int32_t toIndex, bool direction)
{
    int32_t size = GetLength();
    int32_t idx = size - 1;
    if (direction) {
        while (fromIndex < toIndex) {
            JSTaggedValue value = srcArray->Get(idx);
            srcArray->Set(thread, idx + 1, value);
            idx--;
            fromIndex++;
        }
    } else {
        int32_t moveSize = size - fromIndex;
        for (int32_t i = 0; i < moveSize; i++) {
            if ((fromIndex + i) < size) {
                JSTaggedValue value = srcArray->Get(fromIndex + i);
                srcArray->Set(thread, toIndex + i, value);
            } else {
                srcArray->Set(thread, toIndex + i, JSTaggedValue::Hole());
            }
        }
    }
}

int32_t JSAPIPlainArray::BinarySearch(TaggedArray *array, int32_t fromIndex, int32_t toIndex, int32_t key)
{
    int32_t low = fromIndex;
    int32_t high = toIndex - 1;
    while (low <= high) {
        int32_t mid = static_cast<int32_t>(static_cast<uint32_t>(low + high) >> 1U);
        int32_t midVal = static_cast<int32_t>(array->Get(mid).GetNumber());
        if (midVal < key) {
            low = mid + 1;
        } else {
            if (midVal <= key) {
                return mid;
            }
            high = mid - 1;
        }
    }
    return -(low + 1);
}

void JSAPIPlainArray::Clear(JSThread *thread)
{
    TaggedArray *keys = TaggedArray::Cast(GetKeys().GetTaggedObject());
    TaggedArray *values = TaggedArray::Cast(GetValues().GetTaggedObject());
    int32_t size = GetLength();
    for (int32_t index = 0; index < size; index++) {
        keys->Set(thread, index, JSTaggedValue::Hole());
        values->Set(thread, index, JSTaggedValue::Hole());
    }
    SetLength(0);
}

JSTaggedValue JSAPIPlainArray::RemoveRangeFrom(JSThread *thread, int32_t index, int32_t batchSize)
{
    int32_t size = GetLength();
    if (index < 0 || index >= size) {
        std::ostringstream oss;
        oss << "The value of \"index\" is out of range. It must be >= 0 && <= " << (size - 1)
            << ". Received value is: " << index;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, JSTaggedValue::Exception());
    }
    if (batchSize < 1) {
        std::ostringstream oss;
        oss << "The value of \"size\" is out of range. It must be > 0" << ". Received value is: " << batchSize;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, JSTaggedValue::Exception());
    }
    int32_t safeSize = (size - (index + batchSize)) < 0 ? size - index : batchSize;
    AdjustForward(thread, index, safeSize);
    return JSTaggedValue(safeSize);
}

JSTaggedValue JSAPIPlainArray::Set(JSThread *thread, const JSHandle<JSAPIPlainArray> &obj,
                                   const uint32_t index, JSTaggedValue value)
{
    JSHandle<JSTaggedValue> key(thread, JSTaggedValue(index));
    JSHandle<JSTaggedValue> valueHandle(thread, value);
    JSAPIPlainArray::Add(thread, obj, key, valueHandle);
    return JSTaggedValue::Undefined();
}

bool JSAPIPlainArray::GetOwnProperty(JSThread *thread, const JSHandle<JSAPIPlainArray> &obj,
                                     const JSHandle<JSTaggedValue> &key)
{
    TaggedArray *keyArray = TaggedArray::Cast(obj->GetKeys().GetTaggedObject());
    int32_t size = obj->GetLength();
    int32_t index = obj->BinarySearch(keyArray, 0, size, key.GetTaggedValue().GetInt());
    if (index < 0 || index >= size) {
        std::ostringstream oss;
        oss << "The value of \"index\" is out of range. It must be >= 0 && <= " << (size - 1)
            << ". Received value is: " << index;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, false);
    }

    obj->Get(key.GetTaggedValue());
    return true;
}

OperationResult JSAPIPlainArray::GetProperty(JSThread *thread, const JSHandle<JSAPIPlainArray> &obj,
                                             const JSHandle<JSTaggedValue> &key)
{
    TaggedArray *keyArray = TaggedArray::Cast(obj->GetKeys().GetTaggedObject());
    int32_t size = obj->GetLength();
    int32_t index = obj->BinarySearch(keyArray, 0, size, key.GetTaggedValue().GetInt());
    if (index < 0 || index >= size) {
        std::ostringstream oss;
        oss << "The value of \"index\" is out of range. It must be >= 0 && <= " << (size - 1)
            << ". Received value is: " << index;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, OperationResult(thread,
                                                                        JSTaggedValue::Exception(),
                                                                        PropertyMetaData(false)));
    }
    
    return OperationResult(thread, obj->Get(JSTaggedValue(index)), PropertyMetaData(false));
}

bool JSAPIPlainArray::SetProperty(JSThread *thread, const JSHandle<JSAPIPlainArray> &obj,
                                  const JSHandle<JSTaggedValue> &key,
                                  const JSHandle<JSTaggedValue> &value)
{
    TaggedArray *keyArray = TaggedArray::Cast(obj->GetKeys().GetTaggedObject());
    int32_t size = obj->GetLength();
    int32_t index = obj->BinarySearch(keyArray, 0, size, key.GetTaggedValue().GetInt());
    if (index < 0 || index >= size) {
        return false;
    }

    obj->Set(thread, obj, index, value.GetTaggedValue());
    return true;
}

JSHandle<JSAPIPlainArray> JSAPIPlainArray::Clone(JSThread *thread, const JSHandle<JSAPIPlainArray> &obj)
{
    JSHandle<TaggedArray> srckeys(thread, obj->GetKeys());
    JSHandle<TaggedArray> srcvalues(thread, obj->GetValues());
    auto factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSAPIPlainArray> newPlainArray = factory->NewJSAPIPlainArray(0);

    int32_t length = obj->GetLength();
    newPlainArray->SetLength(length);
    JSHandle<TaggedArray> srcKeyArray(thread, obj->GetKeys());
    JSHandle<TaggedArray> srcValueArray(thread, obj->GetValues());
    
    JSHandle<TaggedArray> dstKeyArray = factory->NewAndCopyTaggedArray(srcKeyArray, length, length);
    JSHandle<TaggedArray> dstValueArray = factory->NewAndCopyTaggedArray(srcValueArray, length, length);

    newPlainArray->SetKeys(thread, dstKeyArray);
    newPlainArray->SetValues(thread, dstValueArray);
    return newPlainArray;
}

bool JSAPIPlainArray::Has(const int32_t key)
{
    int32_t size = GetLength();
    TaggedArray *keyArray = TaggedArray::Cast(GetKeys().GetTaggedObject());
    int32_t index = BinarySearch(keyArray, 0, size, key);
    if (index < 0) {
        return false;
    }
    return true;
}

JSTaggedValue JSAPIPlainArray::Get(const JSTaggedValue key)
{
    int32_t size = GetLength();
    TaggedArray *keyArray = TaggedArray::Cast(GetKeys().GetTaggedObject());
    int32_t index = BinarySearch(keyArray, 0, size, key.GetNumber());
    if (index < 0) {
        return JSTaggedValue::Undefined();
    }
    TaggedArray *values = TaggedArray::Cast(GetValues().GetTaggedObject());
    return values->Get(index);
}

JSHandle<JSTaggedValue> JSAPIPlainArray::GetIteratorObj(JSThread *thread, const JSHandle<JSAPIPlainArray> &obj,
                                                        IterationKind kind)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSTaggedValue> iter =
        JSHandle<JSTaggedValue>::Cast(factory->NewJSAPIPlainArrayIterator(obj, kind));
    return iter;
}

JSTaggedValue JSAPIPlainArray::ForEach(JSThread *thread, const JSHandle<JSTaggedValue> &thisHandle,
                                       const JSHandle<JSTaggedValue> &callbackFn,
                                       const JSHandle<JSTaggedValue> &thisArg)
{
    JSAPIPlainArray *plainarray = JSAPIPlainArray::Cast(thisHandle->GetTaggedObject());
    int32_t length = plainarray->GetLength();
    JSHandle<JSTaggedValue> undefined = thread->GlobalConstants()->GetHandledUndefined();
    JSHandle<TaggedArray> keyArray(thread, plainarray->GetKeys());
    JSHandle<TaggedArray> valueArray(thread, plainarray->GetValues());
    for (int32_t k = 0; k < length; k++) {
        JSTaggedValue kValue = valueArray->Get(k);
        JSTaggedValue key = keyArray->Get(k);
        EcmaRuntimeCallInfo *info =
            EcmaInterpreter::NewRuntimeCallInfo(thread, callbackFn, thisArg, undefined, 3);  // 3: three args
        RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, JSTaggedValue::Exception());
        info->SetCallArg(kValue, key, thisHandle.GetTaggedValue());
        JSTaggedValue funcResult = JSFunction::Call(info);
        RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, funcResult);
    }
    return JSTaggedValue::Undefined();
}

JSTaggedValue JSAPIPlainArray::ToString(JSThread *thread, const JSHandle<JSAPIPlainArray> &plainarray)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    std::u16string sepStr = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(",");
    std::u16string colonStr = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(":");

    int32_t length = plainarray->GetLength();
    std::u16string concatStr = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes("");
    std::u16string concatStrNew = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes("");
    JSMutableHandle<JSTaggedValue> valueHandle(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> keyHandle(thread, JSTaggedValue::Undefined());
    for (int32_t k = 0; k < length; k++) {
        std::u16string valueStr;
        valueHandle.Update(plainarray->GetValueAt(thread, k));
        if (!valueHandle->IsUndefined() && !valueHandle->IsNull()) {
            JSHandle<EcmaString> valueStringHandle = JSTaggedValue::ToString(thread, valueHandle);
            RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
            valueStr = EcmaStringAccessor(valueStringHandle).ToU16String();
        }

        std::u16string nextStr;
        keyHandle.Update(plainarray->GetKeyAt(k));
        if (!keyHandle->IsUndefined() && !keyHandle->IsNull()) {
            JSHandle<EcmaString> keyStringHandle = JSTaggedValue::ToString(thread, keyHandle);
            RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
            nextStr = EcmaStringAccessor(keyStringHandle).ToU16String();
        }

        nextStr.append(colonStr);
        nextStr.append(valueStr);
        if (k > 0) {
            concatStr.append(sepStr);
            concatStr.append(nextStr);
            continue;
        }
        concatStr.append(nextStr);
    }

    char16_t *char16tData = concatStr.data();
    auto *uint16tData = reinterpret_cast<uint16_t *>(char16tData);
    uint32_t u16strSize = concatStr.size();
    return factory->NewFromUtf16Literal(uint16tData, u16strSize).GetTaggedValue();
}

JSTaggedValue JSAPIPlainArray::GetIndexOfKey(int32_t key)
{
    int32_t size = GetLength();
    TaggedArray *keyArray = TaggedArray::Cast(GetKeys().GetTaggedObject());
    int32_t index = BinarySearch(keyArray, 0, size, key);
    if (index < 0) {
        return JSTaggedValue(-1);
    }
    return JSTaggedValue(index);
}

JSTaggedValue JSAPIPlainArray::GetIndexOfValue(JSTaggedValue value)
{
    TaggedArray *values = TaggedArray::Cast(GetValues().GetTaggedObject());
    int32_t size = GetLength();
    int32_t index = -1;
    for (int32_t i = 0; i < size; i++) {
        if (JSTaggedValue::SameValue(values->Get(i), value)) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        return JSTaggedValue(-1);
    }
    return JSTaggedValue(index);
}

bool JSAPIPlainArray::IsEmpty()
{
    int32_t length = GetLength();
    return length == 0;
}

JSTaggedValue JSAPIPlainArray::GetKeyAt(int32_t index)
{
    int32_t size = GetLength();
    TaggedArray *keyArray = TaggedArray::Cast(GetKeys().GetTaggedObject());
    if (index < 0 || index >= size) {
        return JSTaggedValue::Undefined();
    }
    return keyArray->Get(index);
}

JSTaggedValue JSAPIPlainArray::GetValueAt(JSThread *thread, int32_t index)
{
    int32_t size = GetLength();
    if (index < 0 || index >= size) {
        std::ostringstream oss;
        oss << "The value of \"index\" is out of range. It must be >= 0 && <= " << (size - 1)
            << ". Received value is: " << index;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, JSTaggedValue::Exception());
    }
    TaggedArray *values = TaggedArray::Cast(GetValues().GetTaggedObject());
    return values->Get(index);
}

JSTaggedValue JSAPIPlainArray::Remove(JSThread *thread, JSTaggedValue key)
{
    int32_t size = GetLength();
    TaggedArray *keyArray = TaggedArray::Cast(GetKeys().GetTaggedObject());
    int32_t index = BinarySearch(keyArray, 0, size, key.GetNumber());
    if (index < 0 || index >= size) {
        return JSTaggedValue::Undefined();
    }
    TaggedArray *values = TaggedArray::Cast(GetValues().GetTaggedObject());
    JSTaggedValue value = values->Get(index);
    AdjustForward(thread, index, 1); // 1 means the length of array
    return value;
}

JSTaggedValue JSAPIPlainArray::RemoveAt(JSThread *thread, JSTaggedValue index)
{
    int32_t size = GetLength();
    int32_t seat = index.GetNumber();
    if (seat < 0 || seat >= size) {
        return JSTaggedValue::Undefined();
    }
    TaggedArray *values = TaggedArray::Cast(GetValues().GetTaggedObject());
    JSTaggedValue value = values->Get(seat);
    AdjustForward(thread, seat, 1);
    return value;
}

bool JSAPIPlainArray::SetValueAt(JSThread *thread, JSTaggedValue index, JSTaggedValue value)
{
    int32_t size = GetLength();
    int32_t seat = index.GetNumber();
    if (seat < 0 || seat >= size) {
        std::ostringstream oss;
        oss << "The value of \"index\" is out of range. It must be >= 0 && <= " << (size - 1)
            << ". Received value is: " << seat;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, false);
    }
    TaggedArray *values = TaggedArray::Cast(GetValues().GetTaggedObject());
    values->Set(thread, seat, value);
    return true;
}
} // namespace panda::ecmascript
