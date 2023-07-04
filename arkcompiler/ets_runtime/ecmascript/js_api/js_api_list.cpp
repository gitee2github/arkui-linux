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

#include "ecmascript/js_api/js_api_list.h"

#include "ecmascript/containers/containers_errors.h"
#include "ecmascript/js_api/js_api_list_iterator.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_tagged_number.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tagged_list.h"

namespace panda::ecmascript {
using ContainerError = containers::ContainerError;
using ErrorFlag = containers::ErrorFlag;
void JSAPIList::Add(JSThread *thread, const JSHandle<JSAPIList> &list, const JSHandle<JSTaggedValue> &value)
{
    JSHandle<TaggedSingleList> singleList(thread, list->GetSingleList());
    JSTaggedValue newList = TaggedSingleList::Add(thread, singleList, value);
    list->SetSingleList(thread, newList);
}

JSTaggedValue JSAPIList::GetFirst()
{
    JSTaggedValue res = TaggedSingleList::Cast(GetSingleList().GetTaggedObject())->GetFirst();
    if (res.IsHole()) {
        return JSTaggedValue::Undefined();
    }
    return res;
}

JSTaggedValue JSAPIList::GetLast()
{
    JSTaggedValue res = TaggedSingleList::Cast(GetSingleList().GetTaggedObject())->GetLast();
    if (res.IsHole()) {
        return JSTaggedValue::Undefined();
    }
    return res;
}

JSTaggedValue JSAPIList::Insert(JSThread *thread, const JSHandle<JSAPIList> &list, const JSHandle<JSTaggedValue> &value,
                                const int index)
{
    JSHandle<TaggedSingleList> singleList(thread, list->GetSingleList());
    int nodeLength = singleList->Length();
    if (index < 0 || index > nodeLength) {
        std::ostringstream oss;
        oss << "The value of \"index\" is out of range. It must be >= 0 && <= " << nodeLength
            << ". Received value is: " << index;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, JSTaggedValue::Exception());
    }
    JSTaggedValue newList = TaggedSingleList::Insert(thread, singleList, value, index);
    list->SetSingleList(thread, newList);
    return JSTaggedValue::True();
}

JSTaggedValue JSAPIList::Set(JSThread *thread, const JSHandle<JSAPIList> &list,
                             const int index, const JSHandle<JSTaggedValue> &value)
{
    JSHandle<TaggedSingleList> singleList(thread, list->GetSingleList());
    int nodeLength = singleList->Length();
    if (index < 0 || index >= nodeLength) {
        std::ostringstream oss;
        oss << "The value of \"index\" is out of range. It must be >= 0 && <= " << (nodeLength - 1)
            << ". Received value is: " << index;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, JSTaggedValue::Exception());
    }
    TaggedSingleList::Set(thread, singleList, index, value);
    RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
    return value.GetTaggedValue();
}

bool JSAPIList::Has(const JSTaggedValue &element)
{
    TaggedSingleList *singleList = TaggedSingleList::Cast(GetSingleList().GetTaggedObject());
    return singleList->Has(element);
}

bool JSAPIList::IsEmpty()
{
    return TaggedSingleList::Cast(GetSingleList().GetTaggedObject())->IsEmpty();
}

JSTaggedValue JSAPIList::Get(const int index)
{
    TaggedSingleList *singleList = TaggedSingleList::Cast(GetSingleList().GetTaggedObject());
    int nodeLength = singleList->Length();
    if (index < 0 || index >= nodeLength) {
        return JSTaggedValue::Undefined();
    }
    return singleList->Get(index);
}

JSTaggedValue JSAPIList::GetIndexOf(const JSTaggedValue &element)
{
    TaggedSingleList *singleList = TaggedSingleList::Cast(GetSingleList().GetTaggedObject());
    return JSTaggedValue(singleList->GetIndexOf(element));
}

JSTaggedValue JSAPIList::GetLastIndexOf(const JSTaggedValue &element)
{
    TaggedSingleList *singleList = TaggedSingleList::Cast(GetSingleList().GetTaggedObject());
    return JSTaggedValue(singleList->GetLastIndexOf(element));
}

void JSAPIList::Clear(JSThread *thread)
{
    TaggedSingleList *singleList = TaggedSingleList::Cast(GetSingleList().GetTaggedObject());
    if (singleList->NumberOfNodes() > 0) {
        singleList->Clear(thread);
    }
}

JSTaggedValue JSAPIList::RemoveByIndex(JSThread *thread, const JSHandle<JSAPIList> &list, const int &index)
{
    JSHandle<TaggedSingleList> singleList(thread, list->GetSingleList());
    int nodeLength = singleList->Length();
    if (index < 0 || index >= nodeLength) {
        int size = (nodeLength > 0) ? (nodeLength - 1) : 0;
        std::ostringstream oss;
        oss << "The value of \"index\" is out of range. It must be >= 0 && <= " << size
            << ". Received value is: " << index;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, JSTaggedValue::Exception());
    }
    return singleList->RemoveByIndex(thread, index);
}

JSTaggedValue JSAPIList::Remove(JSThread *thread, const JSTaggedValue &element)
{
    TaggedSingleList *singleList = TaggedSingleList::Cast(GetSingleList().GetTaggedObject());
    return singleList->Remove(thread, element);
}

JSTaggedValue JSAPIList::ReplaceAllElements(JSThread *thread, const JSHandle<JSTaggedValue> &thisHandle,
                                            const JSHandle<JSTaggedValue> &callbackFn,
                                            const JSHandle<JSTaggedValue> &thisArg)
{
    JSHandle<JSAPIList> list = JSHandle<JSAPIList>::Cast(thisHandle);
    JSHandle<TaggedSingleList> singleList(thread, list->GetSingleList());
    return TaggedSingleList::ReplaceAllElements(thread, thisHandle, callbackFn, thisArg, singleList);
}

JSTaggedValue JSAPIList::Sort(JSThread *thread, const JSHandle<JSTaggedValue> &thisHandle,
                              const JSHandle<JSTaggedValue> &callbackFn)
{
    JSHandle<JSAPIList> list = JSHandle<JSAPIList>::Cast(thisHandle);
    JSHandle<TaggedSingleList> singleList(thread, list->GetSingleList());
    return TaggedSingleList::Sort(thread, callbackFn, singleList);
}

JSTaggedValue JSAPIList::Equal(JSThread *thread, const JSHandle<JSAPIList> &list)
{
    JSHandle<TaggedSingleList> compareList(thread, list->GetSingleList());
    return TaggedSingleList::Cast(GetSingleList().GetTaggedObject())->Equal(compareList);
}

JSTaggedValue JSAPIList::ConvertToArray(const JSThread *thread, const JSHandle<JSAPIList> &list)
{
    JSHandle<TaggedSingleList> singleList(thread, list->GetSingleList());
    return TaggedSingleList::ConvertToArray(thread, singleList);
}

JSTaggedValue JSAPIList::GetSubList(JSThread *thread, const JSHandle<JSAPIList> &list,
                                    const int fromIndex, const int toIndex)
{
    JSHandle<TaggedSingleList> singleList(thread, list->GetSingleList());
    int nodeLength = singleList->Length();
    int32_t size = nodeLength > toIndex ? toIndex : nodeLength;
    if (fromIndex < 0 || fromIndex >= size) {
        std::ostringstream oss;
        oss << "The value of \"fromIndex\" is out of range. It must be >= 0 && <= "
            << (size - 1) << ". Received value is: " << fromIndex;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, JSTaggedValue::Exception());
    }
    if (toIndex < 0 || toIndex <= fromIndex || toIndex >= nodeLength) {
        std::ostringstream oss;
        oss << "The value of \"toIndex\" is out of range. It must be >= 0 && <= "
            << nodeLength << ". Received value is: " << toIndex;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, JSTaggedValue::Exception());
    }
    int len = TaggedSingleList::ELEMENTS_START_INDEX + (toIndex - fromIndex + 1) * TaggedSingleList::ENTRY_SIZE;
    JSHandle<TaggedArray> newElement = thread->GetEcmaVM()->GetFactory()->NewTaggedArray(len);
    JSHandle<TaggedSingleList> subSingleList = JSHandle<TaggedSingleList>::Cast(newElement);
    JSHandle<JSAPIList> sublist = thread->GetEcmaVM()->GetFactory()->NewJSAPIList();
    TaggedSingleList::GetSubList(thread, singleList, fromIndex, toIndex, subSingleList);
    sublist->SetSingleList(thread, subSingleList);
    return sublist.GetTaggedValue();
}

JSHandle<TaggedArray> JSAPIList::OwnKeys(JSThread *thread, const JSHandle<JSAPIList> &list)
{
    JSHandle<TaggedSingleList> singleList(thread, list->GetSingleList());
    return TaggedSingleList::OwnKeys(thread, singleList);
}

bool JSAPIList::GetOwnProperty(JSThread *thread, const JSHandle<JSAPIList> &list, const JSHandle<JSTaggedValue> &key)
{
    uint32_t index = 0;
    if (UNLIKELY(!JSTaggedValue::ToElementIndex(key.GetTaggedValue(), &index))) {
        JSHandle<EcmaString> result = JSTaggedValue::ToString(thread, key.GetTaggedValue());
        CString errorMsg =
            "The type of \"index\" can not obtain attributes of no-number type. Received value is: "
            + ConvertToString(*result);
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::TYPE_ERROR, errorMsg.c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, false);
    }
    JSHandle<TaggedSingleList> singleList(thread, list->GetSingleList());
    uint32_t length = static_cast<uint32_t>(singleList->Length());
    if (index >= length) {
        std::ostringstream oss;
        oss << "The value of \"index\" is out of range. It must be > " << (length - 1)
            << ". Received value is: " << index;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, false);
    }
    list->Get(index);
    return true;
}

OperationResult JSAPIList::GetProperty(JSThread *thread, const JSHandle<JSAPIList> &list,
                                       const JSHandle<JSTaggedValue> &key)
{
    JSHandle<TaggedSingleList> singleList(thread, list->GetSingleList());
    int nodeLength = singleList->Length();
    int index = key->GetInt();
    if (index < 0 || index >= nodeLength) {
        std::ostringstream oss;
        oss << "The value of \"index\" is out of range. It must be >= 0 && <= " << (nodeLength - 1)
            << ". Received value is: " << index;
        JSTaggedValue error = ContainerError::BusinessError(thread, ErrorFlag::RANGE_ERROR, oss.str().c_str());
        THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error, OperationResult(thread,
                                                                        JSTaggedValue::Exception(),
                                                                        PropertyMetaData(false)));
    }
    
    return OperationResult(thread, singleList->Get(index), PropertyMetaData(false));
}

bool JSAPIList::SetProperty(JSThread *thread, const JSHandle<JSAPIList> &obj,
                            const JSHandle<JSTaggedValue> &key,
                            const JSHandle<JSTaggedValue> &value)
{
    JSHandle<TaggedSingleList> singleList(thread, obj->GetSingleList());
    int nodeLength = singleList->Length();
    int index = key->GetInt();
    if (index < 0 || index >= nodeLength) {
        return false;
    }

    TaggedSingleList::Set(thread, singleList, index, value);
    return true;
}
}  // namespace panda::ecmascript
