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

#include "ecmascript/js_serializer.h"

#if defined(PANDA_TARGET_IOS)
#include <stdlib.h>
#elif defined(PANDA_TARGET_MACOS)
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif

#include <vector>

#include "ecmascript/base/array_helper.h"
#include "ecmascript/base/typed_array_helper-inl.h"
#include "ecmascript/base/typed_array_helper.h"
#include "ecmascript/global_env.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_arraybuffer.h"
#include "ecmascript/js_hclass.h"
#include "ecmascript/js_regexp.h"
#include "ecmascript/js_set.h"
#include "ecmascript/js_typed_array.h"
#include "ecmascript/linked_hash_table.h"
#include "ecmascript/shared_mm/shared_mm.h"

#include "securec.h"

namespace panda::ecmascript {
using TypedArrayHelper = base::TypedArrayHelper;
constexpr size_t INITIAL_CAPACITY = 64;
constexpr int CAPACITY_INCREASE_RATE = 2;

bool JSSerializer::WriteType(SerializationUID id)
{
    uint8_t rawId = static_cast<uint8_t>(id);
    return WriteRawData(&rawId, sizeof(rawId));
}

// Write JSTaggedValue could be either a pointer to a HeapObject or a value
bool JSSerializer::SerializeJSTaggedValue(const JSHandle<JSTaggedValue> &value)
{
    [[maybe_unused]] EcmaHandleScope scope(thread_);
    if (!value->IsHeapObject()) {
        if (!WritePrimitiveValue(value)) {
            return false;
        }
    } else {
        if (!WriteTaggedObject(value)) {
            return false;
        }
    }
    return true;
}

// Write JSTaggedValue that is pure value
bool JSSerializer::WritePrimitiveValue(const JSHandle<JSTaggedValue> &value)
{
    if (value->IsNull()) {
        return WriteType(SerializationUID::JS_NULL);
    }
    if (value->IsUndefined()) {
        return WriteType(SerializationUID::JS_UNDEFINED);
    }
    if (value->IsTrue()) {
        return WriteType(SerializationUID::JS_TRUE);
    }
    if (value->IsFalse()) {
        return WriteType(SerializationUID::JS_FALSE);
    }
    if (value->IsInt()) {
        return WriteInt(value->GetInt());
    }
    if (value->IsDouble()) {
        return WriteDouble(value->GetDouble());
    }
    if (value->IsHole()) {
        return WriteType(SerializationUID::HOLE);
    }
    return false;
}

bool JSSerializer::WriteInt(int32_t value)
{
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::INT32)) {
        return false;
    }
    if (!WriteRawData(&value, sizeof(value))) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

bool JSSerializer::WriteDouble(double value)
{
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::DOUBLE)) {
        return false;
    }
    if (!WriteRawData(&value, sizeof(value))) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

bool JSSerializer::WriteBoolean(bool value)
{
    if (value) {
        return WriteType(SerializationUID::C_TRUE);
    }
    return WriteType(SerializationUID::C_FALSE);
}

bool JSSerializer::WriteRawData(const void *data, size_t length)
{
    if (length <= 0) {
        return false;
    }
    if ((bufferSize_ + length) > bufferCapacity_) {
        if (!AllocateBuffer(length)) {
            return false;
        }
    }
    if (memcpy_s(buffer_ + bufferSize_, bufferCapacity_ - bufferSize_, data, length) != EOK) {
        LOG_FULL(ERROR) << "Failed to memcpy_s Data";
        return false;
    }
    bufferSize_ += length;
    return true;
}

bool JSSerializer::AllocateBuffer(size_t bytes)
{
    // Get internal heap size
    if (sizeLimit_ == 0) {
        uint64_t heapSize = thread_->GetEcmaVM()->GetJSOptions().GetSerializerBufferSizeLimit();
        sizeLimit_ = heapSize;
    }
    size_t oldSize = bufferSize_;
    size_t newSize = oldSize + bytes;
    if (newSize > sizeLimit_) {
        return false;
    }
    if (bufferCapacity_ == 0) {
        if (bytes < INITIAL_CAPACITY) {
            buffer_ = reinterpret_cast<uint8_t *>(malloc(INITIAL_CAPACITY));
            if (buffer_ != nullptr) {
                bufferCapacity_ = INITIAL_CAPACITY;
                return true;
            } else {
                return false;
            }
        } else {
            buffer_ = reinterpret_cast<uint8_t *>(malloc(bytes));
            if (buffer_ != nullptr) {
                bufferCapacity_ = bytes;
                return true;
            } else {
                return false;
            }
        }
    }
    if (newSize > bufferCapacity_) {
        if (!ExpandBuffer(newSize)) {
            return false;
        }
    }
    return true;
}

bool JSSerializer::ExpandBuffer(size_t requestedSize)
{
    size_t newCapacity = bufferCapacity_ * CAPACITY_INCREASE_RATE;
    newCapacity = std::max(newCapacity, requestedSize);
    if (newCapacity > sizeLimit_) {
        return false;
    }
    uint8_t *newBuffer = reinterpret_cast<uint8_t *>(malloc(newCapacity));
    if (newBuffer == nullptr) {
        return false;
    }
    if (memcpy_s(newBuffer, newCapacity, buffer_, bufferSize_) != EOK) {
        LOG_FULL(ERROR) << "Failed to memcpy_s Data";
        free(newBuffer);
        return false;
    }
    free(buffer_);
    buffer_ = newBuffer;
    bufferCapacity_ = newCapacity;
    return true;
}

// Transfer ownership of buffer, should not use this Serializer after release
std::pair<uint8_t *, size_t> JSSerializer::ReleaseBuffer()
{
    auto res = std::make_pair(buffer_, bufferSize_);
    buffer_ = nullptr;
    bufferSize_ = 0;
    bufferCapacity_ = 0;
    objectId_ = 0;
    return res;
}

bool JSSerializer::IsSerialized(uintptr_t addr) const
{
    if (referenceMap_.find(addr) != referenceMap_.end()) {
        return true;
    }
    return false;
}

bool JSSerializer::WriteIfSerialized(uintptr_t addr)
{
    size_t oldSize = bufferSize_;
    auto iter = referenceMap_.find(addr);
    if (iter == referenceMap_.end()) {
        return false;
    }
    uint64_t id = iter->second;
    if (!WriteType(SerializationUID::TAGGED_OBJECT_REFERNCE)) {
        return false;
    }
    if (!WriteRawData(&id, sizeof(uint64_t))) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

// Write HeapObject
bool JSSerializer::WriteTaggedObject(const JSHandle<JSTaggedValue> &value)
{
    uintptr_t addr = reinterpret_cast<uintptr_t>(value.GetTaggedValue().GetTaggedObject());
    bool serialized = IsSerialized(addr);
    if (serialized) {
        return WriteIfSerialized(addr);
    }
    referenceMap_.emplace(addr, objectId_);
    objectId_++;

    TaggedObject *taggedObject = value->GetTaggedObject();
    JSType type = taggedObject->GetClass()->GetObjectType();
    switch (type) {
        case JSType::JS_ERROR:
        case JSType::JS_EVAL_ERROR:
        case JSType::JS_RANGE_ERROR:
        case JSType::JS_REFERENCE_ERROR:
        case JSType::JS_TYPE_ERROR:
        case JSType::JS_AGGREGATE_ERROR:
        case JSType::JS_URI_ERROR:
        case JSType::JS_SYNTAX_ERROR:
        case JSType::JS_OOM_ERROR:
            return WriteJSError(value);
        case JSType::JS_DATE:
            return WriteJSDate(value);
        case JSType::JS_ARRAY:
            return WriteJSArray(value);
        case JSType::JS_MAP:
            return WriteJSMap(value);
        case JSType::JS_SET:
            return WriteJSSet(value);
        case JSType::JS_REG_EXP:
            return WriteJSRegExp(value);
        case JSType::JS_INT8_ARRAY:
            return WriteJSTypedArray(value, SerializationUID::JS_INT8_ARRAY);
        case JSType::JS_UINT8_ARRAY:
            return WriteJSTypedArray(value, SerializationUID::JS_UINT8_ARRAY);
        case JSType::JS_UINT8_CLAMPED_ARRAY:
            return WriteJSTypedArray(value, SerializationUID::JS_UINT8_CLAMPED_ARRAY);
        case JSType::JS_INT16_ARRAY:
            return WriteJSTypedArray(value, SerializationUID::JS_INT16_ARRAY);
        case JSType::JS_UINT16_ARRAY:
            return WriteJSTypedArray(value, SerializationUID::JS_UINT16_ARRAY);
        case JSType::JS_INT32_ARRAY:
            return WriteJSTypedArray(value, SerializationUID::JS_INT32_ARRAY);
        case JSType::JS_UINT32_ARRAY:
            return WriteJSTypedArray(value, SerializationUID::JS_UINT32_ARRAY);
        case JSType::JS_FLOAT32_ARRAY:
            return WriteJSTypedArray(value, SerializationUID::JS_FLOAT32_ARRAY);
        case JSType::JS_FLOAT64_ARRAY:
            return WriteJSTypedArray(value, SerializationUID::JS_FLOAT64_ARRAY);
        case JSType::JS_BIGINT64_ARRAY:
            return WriteJSTypedArray(value, SerializationUID::JS_BIGINT64_ARRAY);
        case JSType::JS_BIGUINT64_ARRAY:
            return WriteJSTypedArray(value, SerializationUID::JS_BIGUINT64_ARRAY);
        case JSType::JS_ARRAY_BUFFER:
        case JSType::JS_SHARED_ARRAY_BUFFER:
            return WriteJSArrayBuffer(value);
        case JSType::STRING:
            return WriteEcmaString(value);
        case JSType::JS_OBJECT:
            return WritePlainObject(value);
        case JSType::JS_NATIVE_POINTER:
            return WriteNativePointer(value);
        case JSType::JS_ASYNC_FUNCTION:  // means CONCURRENT_FUNCTION
            return WriteJSFunction(value);
        case JSType::METHOD:
            return WriteMethod(value);
        case JSType::CONSTANT_POOL:
            return WriteConstantPool(value);
        case JSType::TAGGED_ARRAY:
            return WriteTaggedArray(value);
        default:
            break;
    }
    return false;
}

bool JSSerializer::WriteTaggedArray(const JSHandle<JSTaggedValue> &value)
{
    JSHandle<TaggedArray> taggedArray = JSHandle<TaggedArray>::Cast(value);
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::TAGGED_ARRAY)) {
        return false;
    }
    uint32_t len = taggedArray->GetLength();
    if (!WriteInt(len)) {
        bufferSize_ = oldSize;
        return false;
    }
    JSMutableHandle<JSTaggedValue> val(thread_, JSTaggedValue::Undefined());
    for (uint32_t i = 0; i < len; i++) {
        val.Update(taggedArray->Get(i));
        if (!SerializeJSTaggedValue(val)) {
            bufferSize_ = oldSize;
            return false;
        }
    }
    return true;
}

bool JSSerializer::WriteByteArray(const JSHandle<JSTaggedValue> &value, DataViewType viewType)
{
    JSHandle<ByteArray> byteArray = JSHandle<ByteArray>::Cast(value);
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::BYTE_ARRAY)) {
        return false;
    }
    uint32_t arrayLength = byteArray->GetLength();
    if (!WriteInt(arrayLength)) {
        bufferSize_ = oldSize;
        return false;
    }
    uint32_t viewTypeIndex = GetDataViewTypeIndex(viewType);
    if (!WriteInt(viewTypeIndex)) {
        bufferSize_ = oldSize;
        return false;
    }
    JSMutableHandle<JSTaggedValue> val(thread_, JSTaggedValue::Undefined());
    for (uint32_t i = 0; i < arrayLength; i++) {
        val.Update(byteArray->Get(thread_, i, viewType));
        if (!SerializeJSTaggedValue(val)) {
            bufferSize_ = oldSize;
            return false;
        }
    }
    return true;
}

uint32_t JSSerializer::GetDataViewTypeIndex(const DataViewType viewType)
{
    uint32_t index = 0;
    switch (viewType) {
        case DataViewType::INT8:
            index = 1; // 1 : DataViewType::INT8
            break;
        case DataViewType::UINT8:
            index = 2; // 2 : DataViewType::UINT8
            break;
        case DataViewType::UINT8_CLAMPED:
            index = 3; // 3 : DataViewType::UINT8_CLAMPED
            break;
        case DataViewType::INT16:
            index = 4; // 4 : DataViewType::INT16
            break;
        case DataViewType::UINT16:
            index = 5; // 5 : DataViewType::UINT16
            break;
        case DataViewType::INT32:
            index = 6; // 6 : DataViewType::INT32
            break;
        case DataViewType::UINT32:
            index = 7; // 7 : DataViewType::UINT32
            break;
        case DataViewType::FLOAT32:
            index = 8; // 8 : DataViewType::FLOAT32
            break;
        case DataViewType::FLOAT64:
            index = 9; // 9 : DataViewType::FLOAT64
            break;
        case DataViewType::BIGINT64:
            index = 10; // 10 : DataViewType::BIGINT64
            break;
        case DataViewType::BIGUINT64:
            index = 11; // 11 : DataViewType::BIGUINT64
            break;
        default:
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
    }
    return index;
}

bool JSSerializer::WriteConstantPool(const JSHandle<JSTaggedValue> &value)
{
    JSHandle<ConstantPool> constPool = JSHandle<ConstantPool>::Cast(value);
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::CONSTANT_POOL)) {
        return false;
    }
    uint32_t len = constPool->GetLength();
    if (!WriteInt(len)) {
        bufferSize_ = oldSize;
        return false;
    }

    const panda_file::File::IndexHeader *indexHeader = constPool->GetIndexHeader();
    if (!WriteRawData(&indexHeader, sizeof(uintptr_t))) {
        bufferSize_ = oldSize;
        return false;
    }
    const JSPandaFile *jsPandaFile = constPool->GetJSPandaFile();
    if (!jsPandaFile->IsNewVersion()) {
        LOG_ECMA(ERROR) << "Serialize function only support panda file with new ISA";
        return false;
    }
    if (!WriteRawData(&jsPandaFile, sizeof(uintptr_t))) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

bool JSSerializer::WriteMethod(const JSHandle<JSTaggedValue> &value)
{
    JSHandle<Method> method = JSHandle<Method>::Cast(value);
    size_t oldSize = bufferSize_;
    if (method->IsNativeWithCallField()) {
        if (!WriteType(SerializationUID::NATIVE_METHOD)) {
            return false;
        }
        const void *nativeFunc = method->GetNativePointer();
        if (!WriteRawData(&nativeFunc, sizeof(uintptr_t))) {
            bufferSize_ = oldSize;
            return false;
        }
    } else {
        if (!WriteType(SerializationUID::JS_METHOD)) {
            return false;
        }
        const MethodLiteral *methodLiteral = method->GetMethodLiteral();
        if (!WriteRawData(&methodLiteral, sizeof(uintptr_t))) {
            bufferSize_ = oldSize;
            return false;
        }
        JSHandle<JSTaggedValue> constPool(thread_, method->GetConstantPool());
        if (!SerializeJSTaggedValue(constPool)) {
            bufferSize_ = oldSize;
            return false;
        }
    }
    return true;
}

bool JSSerializer::WriteJSFunction(const JSHandle<JSTaggedValue> &value)
{
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::CONCURRENT_FUNCTION)) {
        return false;
    }
    JSHandle<JSFunction> func = JSHandle<JSFunction>::Cast(value);
    JSHandle<JSTaggedValue> method(thread_, func->GetMethod());
    if (!SerializeJSTaggedValue(method)) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

bool JSSerializer::WriteJSError(const JSHandle<JSTaggedValue> &value)
{
    size_t oldSize = bufferSize_;
    TaggedObject *taggedObject = value->GetTaggedObject();
    JSType errorType = taggedObject->GetClass()->GetObjectType();
    if (!WriteJSErrorHeader(errorType)) {
        return false;
    }
    auto globalConst = thread_->GlobalConstants();
    JSHandle<JSTaggedValue> handleMsg = globalConst->GetHandledMessageString();
    JSHandle<JSTaggedValue> msg = JSObject::GetProperty(thread_, value, handleMsg).GetValue();
    // Write error message
    if (!SerializeJSTaggedValue(msg)) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

bool JSSerializer::WriteJSErrorHeader(JSType type)
{
    switch (type) {
        case JSType::JS_ERROR:
            return WriteType(SerializationUID::JS_ERROR);
        case JSType::JS_EVAL_ERROR:
            return WriteType(SerializationUID::EVAL_ERROR);
        case JSType::JS_RANGE_ERROR:
            return WriteType(SerializationUID::RANGE_ERROR);
        case JSType::JS_REFERENCE_ERROR:
            return WriteType(SerializationUID::REFERENCE_ERROR);
        case JSType::JS_TYPE_ERROR:
            return WriteType(SerializationUID::TYPE_ERROR);
        case JSType::JS_AGGREGATE_ERROR:
            return WriteType(SerializationUID::AGGREGATE_ERROR);
        case JSType::JS_URI_ERROR:
            return WriteType(SerializationUID::URI_ERROR);
        case JSType::JS_SYNTAX_ERROR:
            return WriteType(SerializationUID::SYNTAX_ERROR);
        case JSType::JS_OOM_ERROR:
            return WriteType(SerializationUID::OOM_ERROR);
        default:
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
    }
    return false;
}

bool JSSerializer::WriteJSDate(const JSHandle<JSTaggedValue> &value)
{
    JSHandle<JSDate> date = JSHandle<JSDate>::Cast(value);
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::JS_DATE)) {
        return false;
    }
    if (!WritePlainObject(value)) {
        bufferSize_ = oldSize;
        return false;
    }
    double timeValue = date->GetTimeValue().GetDouble();
    if (!WriteDouble(timeValue)) {
        bufferSize_ = oldSize;
        return false;
    }
    double localOffset = date->GetLocalOffset().GetDouble();
    if (!WriteDouble(localOffset)) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

bool JSSerializer::WriteJSArray(const JSHandle<JSTaggedValue> &value)
{
    JSHandle<JSArray> array = JSHandle<JSArray>::Cast(value);
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::JS_ARRAY)) {
        return false;
    }
    if (!WritePlainObject(value)) {
        bufferSize_ = oldSize;
        return false;
    }
    uint32_t arrayLength = static_cast<uint32_t>(array->GetLength().GetInt());
    if (!WriteInt(arrayLength)) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

bool JSSerializer::WriteEcmaString(const JSHandle<JSTaggedValue> &value)
{
    JSHandle<EcmaString> string = JSHandle<EcmaString>::Cast(value);
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::ECMASTRING)) {
        return false;
    }
    bool isUtf8 = EcmaStringAccessor(string).IsUtf8();
    // write utf encode flag
    if (!WriteBoolean(isUtf8)) {
        bufferSize_ = oldSize;
        return false;
    }
    size_t length = EcmaStringAccessor(string).GetLength();
    if (!WriteInt(static_cast<int32_t>(length))) {
        bufferSize_ = oldSize;
        return false;
    }
    // skip writeRawData for empty EcmaString
    if (length == 0) {
        return true;
    }
    if (isUtf8) {
        const uint8_t *data = EcmaStringAccessor(string).GetDataUtf8();
        const uint8_t strEnd = '\0';
        if (!WriteRawData(data, length) || !WriteRawData(&strEnd, sizeof(uint8_t))) {
            bufferSize_ = oldSize;
            return false;
        }
    } else {
        const uint16_t *data = EcmaStringAccessor(string).GetDataUtf16();
        if (!WriteRawData(data, length * sizeof(uint16_t))) {
            bufferSize_ = oldSize;
            return false;
        }
    }
    return true;
}

bool JSSerializer::WriteJSMap(const JSHandle<JSTaggedValue> &value)
{
    JSHandle<JSMap> map = JSHandle<JSMap>::Cast(value);
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::JS_MAP)) {
        return false;
    }
    if (!WritePlainObject(value)) {
        bufferSize_ = oldSize;
        return false;
    }
    int size = map->GetSize();
    if (!WriteInt(size)) {
        bufferSize_ = oldSize;
        return false;
    }
    JSMutableHandle<JSTaggedValue> key(thread_, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> val(thread_, JSTaggedValue::Undefined());
    for (int i = 0; i < size; i++) {
        key.Update(map->GetKey(i));
        if (!SerializeJSTaggedValue(key)) {
            bufferSize_ = oldSize;
            return false;
        }
        val.Update(map->GetValue(i));
        if (!SerializeJSTaggedValue(val)) {
            bufferSize_ = oldSize;
            return false;
        }
    }
    return true;
}

bool JSSerializer::WriteJSSet(const JSHandle<JSTaggedValue> &value)
{
    JSHandle<JSSet> set = JSHandle<JSSet>::Cast(value);
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::JS_SET)) {
        return false;
    }
    if (!WritePlainObject(value)) {
        bufferSize_ = oldSize;
        return false;
    }
    int size = set->GetSize();
    if (!WriteInt(size)) {
        bufferSize_ = oldSize;
        return false;
    }
    JSMutableHandle<JSTaggedValue> val(thread_, JSTaggedValue::Undefined());
    for (int i = 0; i < size; i++) {
        val.Update(set->GetValue(i));
        if (!SerializeJSTaggedValue(val)) {
            bufferSize_ = oldSize;
            return false;
        }
    }
    return true;
}

bool JSSerializer::WriteJSRegExp(const JSHandle<JSTaggedValue> &value)
{
    JSHandle<JSRegExp> regExp = JSHandle<JSRegExp>::Cast(value);
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::JS_REG_EXP)) {
        return false;
    }
    if (!WritePlainObject(value)) {
        bufferSize_ = oldSize;
        return false;
    }
    uint32_t bufferSize = regExp->GetLength();
    if (!WriteInt(static_cast<int32_t>(bufferSize))) {
        bufferSize_ = oldSize;
        return false;
    }
    // Write Accessor(ByteCodeBuffer) which is a pointer to a dynamic buffer
    JSHandle<JSTaggedValue> bufferValue(thread_, regExp->GetByteCodeBuffer());
    JSHandle<JSNativePointer> np = JSHandle<JSNativePointer>::Cast(bufferValue);
    void *dynBuffer = np->GetExternalPointer();
    if (!WriteRawData(dynBuffer, bufferSize)) {
        bufferSize_ = oldSize;
        return false;
    }
    // Write Accessor(OriginalSource)
    JSHandle<JSTaggedValue> originalSource(thread_, regExp->GetOriginalSource());
    if (!SerializeJSTaggedValue(originalSource)) {
        bufferSize_ = oldSize;
        return false;
    }
    // Write Accessor(OriginalFlags)
    JSHandle<JSTaggedValue> originalFlags(thread_, regExp->GetOriginalFlags());
    if (!SerializeJSTaggedValue(originalFlags)) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

bool JSSerializer::WriteJSTypedArray(const JSHandle<JSTaggedValue> &value, SerializationUID uId)
{
    JSHandle<JSTypedArray> typedArray = JSHandle<JSTypedArray>::Cast(value);
    size_t oldSize = bufferSize_;
    if (!WriteType(uId)) {
        return false;
    }
    if (!WritePlainObject(value)) {
        bufferSize_ = oldSize;
        return false;
    }
    [[maybe_unused]] DataViewType viewType = TypedArrayHelper::GetType(typedArray);
    // Write ACCESSORS(ViewedArrayBuffer) which is a pointer to an ArrayBuffer
    JSHandle<JSTaggedValue> viewedArrayBufferOrByteArray(thread_, typedArray->GetViewedArrayBuffer());
    bool isViewedArrayBuffer = false;
    if (viewedArrayBufferOrByteArray->IsArrayBuffer() || viewedArrayBufferOrByteArray->IsSharedArrayBuffer()) {
        isViewedArrayBuffer = true;
        if (!WriteBoolean(isViewedArrayBuffer)) {
            bufferSize_ = oldSize;
            return false;
        }
        if (!SerializeJSTaggedValue(viewedArrayBufferOrByteArray)) {
            bufferSize_ = oldSize;
            return false;
        }
    } else {
        if (!WriteBoolean(isViewedArrayBuffer)) {
            bufferSize_ = oldSize;
            return false;
        }
        if (!WriteByteArray(viewedArrayBufferOrByteArray, viewType)) {
            bufferSize_ = oldSize;
            return false;
        }
    }

    // Write ACCESSORS(TypedArrayName)
    JSHandle<JSTaggedValue> typedArrayName(thread_, typedArray->GetTypedArrayName());
    if (!SerializeJSTaggedValue(typedArrayName)) {
        bufferSize_ = oldSize;
        return false;
    }
    // Write ACCESSORS(ByteLength)
    JSTaggedValue byteLength(typedArray->GetByteLength());
    if (!WriteRawData(&byteLength, sizeof(JSTaggedValue))) {
        bufferSize_ = oldSize;
        return false;
    }
    // Write ACCESSORS(ByteOffset)
    JSTaggedValue byteOffset(typedArray->GetByteOffset());
    if (!WriteRawData(&byteOffset, sizeof(JSTaggedValue))) {
        bufferSize_ = oldSize;
        return false;
    }
    // Write ACCESSORS(ArrayLength)
    JSTaggedValue arrayLength(typedArray->GetArrayLength());
    if (!WriteRawData(&arrayLength, sizeof(JSTaggedValue))) {
        bufferSize_ = oldSize;
        return false;
    }
    // Write ACCESSORS(ContentType)
    ContentType contentType = typedArray->GetContentType();
    if (!WriteRawData(&contentType, sizeof(ContentType))) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

bool JSSerializer::WriteNativePointer(const JSHandle<JSTaggedValue> &value)
{
    size_t oldSize = bufferSize_;
    if (!WriteType(SerializationUID::NATIVE_POINTER)) {
        return false;
    }
    JSTaggedValue pointer = value.GetTaggedValue();
    if (!WriteRawData(&pointer, sizeof(JSTaggedValue))) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

bool JSSerializer::WriteJSArrayBuffer(const JSHandle<JSTaggedValue> &value)
{
    size_t oldSize = bufferSize_;
    JSHandle<JSArrayBuffer> arrayBuffer = JSHandle<JSArrayBuffer>::Cast(value);

    if (arrayBuffer->IsDetach()) {
        return false;
    }

    bool shared = arrayBuffer->GetShared();
    if (shared) {
        if (!WriteType(SerializationUID::JS_SHARED_ARRAY_BUFFER)) {
            return false;
        }
    } else {
        if (!WriteType(SerializationUID::JS_ARRAY_BUFFER)) {
            return false;
        }
    }

    // Write Accessors(ArrayBufferByteLength)
    uint32_t arrayLength = arrayBuffer->GetArrayBufferByteLength();
    if (!WriteInt(arrayLength)) {
        bufferSize_ = oldSize;
        return false;
    }

    // write Accessor shared which indicate the C memory is shared
    if (!WriteBoolean(shared)) {
        bufferSize_ = oldSize;
        return false;
    }

    if (shared) {
        JSHandle<JSNativePointer> np(thread_, arrayBuffer->GetArrayBufferData());
        void *buffer = np->GetExternalPointer();
        JSSharedMemoryManager::GetInstance()->CreateOrLoad(&buffer, arrayLength);
        uint64_t bufferAddr = (uint64_t)buffer;
        if (!WriteRawData(&bufferAddr, sizeof(uint64_t))) {
            bufferSize_ = oldSize;
            return false;
        }
    } else {
        // Write Accessors(ArrayBufferData) which is a pointer to a Buffer
        JSHandle<JSNativePointer> np(thread_, arrayBuffer->GetArrayBufferData());
        void *buffer = np->GetExternalPointer();
        if (!WriteRawData(buffer, arrayLength)) {
            bufferSize_ = oldSize;
            return false;
        }
    }

    // write obj properties
    if (!WritePlainObject(value)) {
        bufferSize_ = oldSize;
        return false;
    }

    return true;
}

bool JSSerializer::IsNativeBindingObject(std::vector<JSTaggedValue> keyVector)
{
    if (keyVector.size() < 2) { // 2:detachSymbol, attachSymbol
        return false;
    }
    JSHandle<GlobalEnv> env = thread_->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSTaggedValue> detach = env->GetDetachSymbol();
    JSHandle<JSTaggedValue> attach = env->GetAttachSymbol();
    JSMutableHandle<JSTaggedValue> detachKey(thread_, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> attachKey(thread_, JSTaggedValue::Undefined());
    uint32_t keyLength = keyVector.size();
    for (uint32_t i = 0; i < keyLength - 1; i++) {
        if (keyVector[i].IsSymbol() && keyVector[i + 1].IsSymbol()) {
            detachKey.Update(keyVector[i]);
            attachKey.Update(keyVector[i + 1]);
            if (JSTaggedValue::Equal(thread_, detach, detachKey) || JSTaggedValue::Equal(thread_, attach, attachKey)) {
                return true;
            }
        }
    }
    return false;
}

bool JSSerializer::IsTargetSymbol(JSTaggedValue symbolVal)
{
    JSHandle<GlobalEnv> env = thread_->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSTaggedValue> detach = env->GetDetachSymbol();
    JSHandle<JSTaggedValue> attach = env->GetAttachSymbol();
    if (JSTaggedValue::Equal(thread_, detach, JSHandle<JSTaggedValue>(thread_, symbolVal)) ||
        JSTaggedValue::Equal(thread_, attach, JSHandle<JSTaggedValue>(thread_, symbolVal))) {
        return true;
    }
    return false;
}

bool JSSerializer::WritePlainObject(const JSHandle<JSTaggedValue> &objValue)
{
    JSHandle<JSObject> obj = JSHandle<JSObject>::Cast(objValue);
    std::vector<JSTaggedValue> keyVector;
    uint32_t propertiesLength = obj->GetNumberOfKeys();
    JSObject::GetAllKeys(obj, keyVector);
    if (keyVector.size() != propertiesLength) {
        return false;
    }

    if (IsNativeBindingObject(keyVector)) {
        return WriteNativeBindingObject(objValue);
    }

    if (!WriteType(SerializationUID::JS_PLAIN_OBJECT)) {
        return false;
    }
    size_t oldSize = bufferSize_;
    if (!WriteInt(static_cast<int32_t>(propertiesLength))) {
        bufferSize_ = oldSize;
        return false;
    }
    JSMutableHandle<JSTaggedValue> propertyKey(thread_, JSTaggedValue::Undefined());
    for (uint32_t i = 0; i < propertiesLength; i++) {
        if (keyVector.empty()) {
            bufferSize_ = oldSize;
            return false;
        }
        propertyKey.Update(keyVector[i]);
        if (!SerializeJSTaggedValue(propertyKey)) {
            bufferSize_ = oldSize;
            return false;
        }
        PropertyDescriptor desc(thread_);
        JSObject::OrdinaryGetOwnProperty(thread_, obj, propertyKey, desc);
        if (!WriteDesc(desc)) {
            bufferSize_ = oldSize;
            return false;
        }
    }

    uint32_t elementsLength = obj->GetNumberOfElements();
    if (!WriteInt(static_cast<int32_t>(elementsLength))) {
        bufferSize_ = oldSize;
        return false;
    }
    keyVector.clear();
    JSObject::GetALLElementKeysIntoVector(thread_, obj, keyVector);
    // Write elements' description attributes and value
    if (keyVector.size() != elementsLength) {
        bufferSize_ = oldSize;
        return false;
    }
    JSMutableHandle<JSTaggedValue> elementKey(thread_, JSTaggedValue::Undefined());
    for (uint32_t i = 0; i < elementsLength; i++) {
        elementKey.Update(keyVector[i]);
        if (!SerializeJSTaggedValue(elementKey)) {
            bufferSize_ = oldSize;
            return false;
        }
        PropertyDescriptor desc(thread_);
        JSObject::OrdinaryGetOwnProperty(thread_, obj, elementKey, desc);
        if (!WriteDesc(desc)) {
            bufferSize_ = oldSize;
            return false;
        }
    }
    return true;
}

bool JSSerializer::WriteNativeBindingObject(const JSHandle<JSTaggedValue> &objValue)
{
    JSHandle<JSObject> obj = JSHandle<JSObject>::Cast(objValue);
    size_t oldSize = bufferSize_;
    JSHandle<GlobalEnv> env = thread_->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSTaggedValue> detach = env->GetDetachSymbol();
    JSHandle<JSTaggedValue> attach = env->GetAttachSymbol();
    if (!WriteType(SerializationUID::NATIVE_BINDING_OBJECT)) {
        return false;
    }
    int32_t paramCount = obj->GetNativePointerFieldCount();
    void *enginePointer = nullptr;
    void *objPointer = nullptr;
    void *hint = nullptr;
    void *detachData = nullptr;
    void *attachData = nullptr;
    if (paramCount == 5) { // 5 : enginePointer, objPointer, hint, detachData, attachData
        enginePointer = obj->GetNativePointerField(0);
        objPointer = obj->GetNativePointerField(1);
        hint = obj->GetNativePointerField(2); // 2 : hint
        detachData = obj->GetNativePointerField(3); // 3 : detachData
        attachData = obj->GetNativePointerField(4); // 4 : attachData
    }
    // Write custom object's values: AttachFunc*, buffer*
    JSHandle<JSTaggedValue> detachVal = JSObject::GetProperty(thread_, obj, detach).GetRawValue();
    JSHandle<JSTaggedValue> attackVal = JSObject::GetProperty(thread_, obj, attach).GetRawValue();
    DetachFunc detachNative = reinterpret_cast<DetachFunc>(JSNativePointer::Cast(
        detachVal.GetTaggedValue().GetTaggedObject())->GetExternalPointer());
    if (detachNative == nullptr) {
        return false;
    }
    void *buffer = detachNative(enginePointer, objPointer, hint, detachData);
    AttachFunc attachNative = reinterpret_cast<AttachFunc>(JSNativePointer::Cast(
        attackVal.GetTaggedValue().GetTaggedObject())->GetExternalPointer());
    if (!WriteRawData(&attachNative, sizeof(uintptr_t))) {
        bufferSize_ = oldSize;
        return false;
    }
    if (!WriteRawData(&buffer, sizeof(uintptr_t))) {
        bufferSize_ = oldSize;
        return false;
    }
    if (!WriteRawData(&hint, sizeof(uintptr_t))) {
        bufferSize_ = oldSize;
        return false;
    }
    if (!WriteRawData(&attachData, sizeof(uintptr_t))) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

bool JSSerializer::WriteDesc(const PropertyDescriptor &desc)
{
    size_t oldSize = bufferSize_;
    bool isWritable = desc.IsWritable();
    if (!WriteBoolean(isWritable)) {
        bufferSize_ = oldSize;
        return false;
    }
    bool isEnumerable = desc.IsEnumerable();
    if (!WriteBoolean(isEnumerable)) {
        bufferSize_ = oldSize;
        return false;
    }
    bool isConfigurable = desc.IsConfigurable();
    if (!WriteBoolean(isConfigurable)) {
        bufferSize_ = oldSize;
        return false;
    }
    bool hasWritable = desc.HasWritable();
    if (!WriteBoolean(hasWritable)) {
        bufferSize_ = oldSize;
        return false;
    }
    bool hasEnumerable = desc.HasEnumerable();
    if (!WriteBoolean(hasEnumerable)) {
        bufferSize_ = oldSize;
        return false;
    }
    bool hasConfigurable = desc.HasConfigurable();
    if (!WriteBoolean(hasConfigurable)) {
        bufferSize_ = oldSize;
        return false;
    }
    JSHandle<JSTaggedValue> value = desc.GetValue();
    if (!SerializeJSTaggedValue(value)) {
        bufferSize_ = oldSize;
        return false;
    }
    return true;
}

SerializationUID JSDeserializer::ReadType()
{
    SerializationUID uid;
    if (position_ >= end_) {
        return SerializationUID::UNKNOWN;
    }
    uid = static_cast<SerializationUID>(*position_);
    if (uid < SerializationUID::JS_NULL || uid > SerializationUID::NATIVE_POINTER) {
        return SerializationUID::UNKNOWN;
    }
    position_++;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return uid;
}

bool JSDeserializer::ReadInt(int32_t *value)
{
    size_t len = sizeof(int32_t);
    if (len > static_cast<size_t>(end_ - position_)) {
        return false;
    }
    if (memcpy_s(value, len, position_, len) != EOK) {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
    position_ += len;
    return true;
}

bool JSDeserializer::ReadObjectId(uint64_t *objectId)
{
    size_t len = sizeof(uint64_t);
    if (len > static_cast<size_t>(end_ - position_)) {
        return false;
    }
    if (memcpy_s(objectId, len, position_, len) != EOK) {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
    position_ += len;
    return true;
}

bool JSDeserializer::ReadDouble(double *value)
{
    size_t len = sizeof(double);
    if (len > static_cast<size_t>(end_ - position_)) {
        return false;
    }
    if (memcpy_s(value, len, position_, len) != EOK) {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
    position_ += len;
    return true;
}

JSDeserializer::~JSDeserializer()
{
    referenceMap_.clear();
    free(begin_);
    begin_ = nullptr;
}

JSHandle<JSTaggedValue> JSDeserializer::DeserializeJSTaggedValue()
{
    SerializationUID uid = ReadType();
    if (uid == SerializationUID::UNKNOWN) {
        return JSHandle<JSTaggedValue>();
    }
    switch (uid) {
        case SerializationUID::JS_NULL:
            return JSHandle<JSTaggedValue>(thread_, JSTaggedValue::Null());
        case SerializationUID::JS_UNDEFINED:
            return JSHandle<JSTaggedValue>(thread_, JSTaggedValue::Undefined());
        case SerializationUID::JS_TRUE:
            return JSHandle<JSTaggedValue>(thread_, JSTaggedValue::True());
        case SerializationUID::JS_FALSE:
            return JSHandle<JSTaggedValue>(thread_, JSTaggedValue::False());
        case SerializationUID::HOLE:
            return JSHandle<JSTaggedValue>(thread_, JSTaggedValue::Hole());
        case SerializationUID::INT32: {
            int32_t value;
            if (!ReadInt(&value)) {
                return JSHandle<JSTaggedValue>();
            }
            return JSHandle<JSTaggedValue>(thread_, JSTaggedValue(value));
        }
        case SerializationUID::DOUBLE: {
            double value;
            if (!ReadDouble(&value)) {
                return JSHandle<JSTaggedValue>();
            }
            return JSHandle<JSTaggedValue>(thread_, JSTaggedValue(value));
        }
        case SerializationUID::JS_ERROR:
        case SerializationUID::EVAL_ERROR:
        case SerializationUID::RANGE_ERROR:
        case SerializationUID::REFERENCE_ERROR:
        case SerializationUID::TYPE_ERROR:
        case SerializationUID::AGGREGATE_ERROR:
        case SerializationUID::URI_ERROR:
        case SerializationUID::SYNTAX_ERROR:
        case SerializationUID::OOM_ERROR:
            return ReadJSError(uid);
        case SerializationUID::JS_DATE:
            return ReadJSDate();
        case SerializationUID::JS_PLAIN_OBJECT:
            return ReadPlainObject();
        case SerializationUID::NATIVE_BINDING_OBJECT:
            return ReadNativeBindingObject();
        case SerializationUID::JS_ARRAY:
            return ReadJSArray();
        case SerializationUID::ECMASTRING:
            return ReadEcmaString();
        case SerializationUID::JS_MAP:
            return ReadJSMap();
        case SerializationUID::JS_SET:
            return ReadJSSet();
        case SerializationUID::JS_REG_EXP:
            return ReadJSRegExp();
        case SerializationUID::JS_INT8_ARRAY:
            return ReadJSTypedArray(SerializationUID::JS_INT8_ARRAY);
        case SerializationUID::JS_UINT8_ARRAY:
            return ReadJSTypedArray(SerializationUID::JS_UINT8_ARRAY);
        case SerializationUID::JS_UINT8_CLAMPED_ARRAY:
            return ReadJSTypedArray(SerializationUID::JS_UINT8_CLAMPED_ARRAY);
        case SerializationUID::JS_INT16_ARRAY:
            return ReadJSTypedArray(SerializationUID::JS_INT16_ARRAY);
        case SerializationUID::JS_UINT16_ARRAY:
            return ReadJSTypedArray(SerializationUID::JS_UINT16_ARRAY);
        case SerializationUID::JS_INT32_ARRAY:
            return ReadJSTypedArray(SerializationUID::JS_INT32_ARRAY);
        case SerializationUID::JS_UINT32_ARRAY:
            return ReadJSTypedArray(SerializationUID::JS_UINT32_ARRAY);
        case SerializationUID::JS_FLOAT32_ARRAY:
            return ReadJSTypedArray(SerializationUID::JS_FLOAT32_ARRAY);
        case SerializationUID::JS_FLOAT64_ARRAY:
            return ReadJSTypedArray(SerializationUID::JS_FLOAT64_ARRAY);
        case SerializationUID::JS_BIGINT64_ARRAY:
            return ReadJSTypedArray(SerializationUID::JS_BIGINT64_ARRAY);
        case SerializationUID::JS_BIGUINT64_ARRAY:
            return ReadJSTypedArray(SerializationUID::JS_BIGUINT64_ARRAY);
        case SerializationUID::NATIVE_POINTER:
            return ReadNativePointer();
        case SerializationUID::JS_SHARED_ARRAY_BUFFER:
        case SerializationUID::JS_ARRAY_BUFFER:
            return ReadJSArrayBuffer();
        case SerializationUID::TAGGED_OBJECT_REFERNCE:
            return ReadReference();
        case SerializationUID::CONCURRENT_FUNCTION:
            return ReadJSFunction();
        case SerializationUID::TAGGED_ARRAY:
            return ReadTaggedArray();
        case SerializationUID::JS_METHOD:
            return ReadJSMethod();
        case SerializationUID::NATIVE_METHOD:
            return ReadNativeMethod();
        case SerializationUID::CONSTANT_POOL:
            return ReadConstantPool();
        default:
            return JSHandle<JSTaggedValue>();
    }
}

JSHandle<JSTaggedValue> JSDeserializer::ReadTaggedArray()
{
    int32_t len = 0;
    if (!JudgeType(SerializationUID::INT32) || !ReadInt(&len)) {
        return JSHandle<JSTaggedValue>();
    }
    JSHandle<TaggedArray> taggedArray = factory_->NewTaggedArray(len);
    JSHandle<JSTaggedValue> arrayTag(taggedArray);
    referenceMap_.emplace(objectId_++, arrayTag);
    for (int32_t i = 0; i < len; i++) {
        JSHandle<JSTaggedValue> val = DeserializeJSTaggedValue();
        taggedArray->Set(thread_, i, val.GetTaggedValue());
    }
    return arrayTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadByteArray()
{
    int32_t arrayLength = 0;
    if (!JudgeType(SerializationUID::INT32) || !ReadInt(&arrayLength)) {
        return JSHandle<JSTaggedValue>();
    }
    int32_t viewTypeIndex = 0;
    if (!JudgeType(SerializationUID::INT32) || !ReadInt(&viewTypeIndex)) {
        return JSHandle<JSTaggedValue>();
    }
    DataViewType viewType = GetDataViewTypeByIndex(viewTypeIndex);
    uint32_t arrayType = TypedArrayHelper::GetSizeFromType(viewType);
    JSHandle<ByteArray> byteArray = factory_->NewByteArray(arrayLength, arrayType);
    for (int32_t i = 0; i < arrayLength; i++) {
        JSHandle<JSTaggedValue> val = DeserializeJSTaggedValue();
        byteArray->Set(i, viewType, val.GetTaggedType());
    }
    return JSHandle<JSTaggedValue>(byteArray);
}

DataViewType JSDeserializer::GetDataViewTypeByIndex(uint32_t viewTypeIndex)
{
    DataViewType viewType;
    switch (viewTypeIndex) {
        case 1: // 1 : DataViewType::INT8
            viewType = DataViewType::INT8;
            break;
        case 2: // 2 : DataViewType::UINT8
            viewType = DataViewType::UINT8;
            break;
        case 3: // 3 : DataViewType::UINT8_CLAMPED
            viewType = DataViewType::UINT8_CLAMPED;
            break;
        case 4: // 4 : DataViewType::INT16
            viewType = DataViewType::INT16;
            break;
        case 5: // 5 : DataViewType::UINT16
            viewType = DataViewType::UINT16;
            break;
        case 6: // 6 : DataViewType::INT32
            viewType = DataViewType::INT32;
            break;
        case 7: // 7 : DataViewType::UINT32
            viewType = DataViewType::UINT32;
            break;
        case 8: // 8 : DataViewType::FLOAT32
            viewType = DataViewType::FLOAT32;
            break;
        case 9: // 9 : DataViewType::FLOAT64
            viewType = DataViewType::FLOAT64;
            break;
        case 10: // 10 : DataViewType::BIGINT64
            viewType = DataViewType::BIGINT64;
            break;
        case 11: // 11 : DataViewType::BIGUINT64
            viewType = DataViewType::BIGUINT64;
            break;
        default:
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
    }
    return viewType;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadConstantPool()
{
    int32_t len = 0;
    if (!JudgeType(SerializationUID::INT32) || !ReadInt(&len)) {
        return JSHandle<JSTaggedValue>();
    }
    int32_t cacheLength = len - 2; // 2 : RESERVED_POOL_LENGTH
    JSHandle<ConstantPool> constPool = factory_->NewConstantPool(cacheLength);
    JSHandle<JSTaggedValue> constPoolTag(constPool);
    referenceMap_.emplace(objectId_++, constPoolTag);

    uintptr_t indexHeader;
    if (!ReadNativePointer(&indexHeader)) {
        return JSHandle<JSTaggedValue>();
    }
    uintptr_t jsPandafile;
    if (!ReadNativePointer(&jsPandafile)) {
        return JSHandle<JSTaggedValue>();
    }
    JSPandaFile *pf = reinterpret_cast<JSPandaFile *>(jsPandafile);
    if (!pf->IsNewVersion()) {
        LOG_ECMA(ERROR) << "Deserialize function only support panda file with new ISA";
        return JSHandle<JSTaggedValue>();
    }
    constPool->SetJSPandaFile(pf);
    constPool->SetIndexHeader(reinterpret_cast<panda_file::File::IndexHeader *>(indexHeader));
    return constPoolTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadJSMethod()
{
    uintptr_t methodLiteral;
    if (!ReadNativePointer(&methodLiteral)) {
        return JSHandle<JSTaggedValue>();
    }
    JSHandle<Method> method = factory_->NewMethod(reinterpret_cast<MethodLiteral *>(methodLiteral));
    JSHandle<JSTaggedValue> methodTag(method);
    referenceMap_.emplace(objectId_++, methodTag);
    JSHandle<JSTaggedValue> constPool = DeserializeJSTaggedValue();
    method->SetConstantPool(thread_, constPool.GetTaggedValue());
    return methodTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadNativeMethod()
{
    uintptr_t nativeFunc;
    if (!ReadNativePointer(&nativeFunc)) {
        return JSHandle<JSTaggedValue>();
    }
    JSHandle<Method> method = factory_->NewMethodForNativeFunction(reinterpret_cast<void *>(nativeFunc));
    JSHandle<JSTaggedValue> methodTag(method);
    referenceMap_.emplace(objectId_++, methodTag);
    return methodTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadJSFunction()
{
    JSHandle<GlobalEnv> env = thread_->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> func = factory_->NewJSFunction(env, nullptr, FunctionKind::CONCURRENT_FUNCTION);
    JSHandle<JSTaggedValue> funcTag(func);
    referenceMap_.emplace(objectId_++, funcTag);
    JSHandle<JSTaggedValue> methodVal = DeserializeJSTaggedValue();
    JSHandle<Method> method = JSHandle<Method>::Cast(methodVal);
    func->SetMethod(thread_, method);
    return funcTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadJSError(SerializationUID uid)
{
    base::ErrorType errorType;
    switch (uid) {
        case SerializationUID::JS_ERROR:
            errorType = base::ErrorType::ERROR;
            break;
        case SerializationUID::EVAL_ERROR:
            errorType = base::ErrorType::EVAL_ERROR;
            break;
        case SerializationUID::RANGE_ERROR:
            errorType = base::ErrorType::RANGE_ERROR;
            break;
        case SerializationUID::REFERENCE_ERROR:
            errorType = base::ErrorType::REFERENCE_ERROR;
            break;
        case SerializationUID::TYPE_ERROR:
            errorType = base::ErrorType::TYPE_ERROR;
            break;
        case SerializationUID::AGGREGATE_ERROR:
            errorType = base::ErrorType::AGGREGATE_ERROR;
            break;
        case SerializationUID::URI_ERROR:
            errorType = base::ErrorType::URI_ERROR;
            break;
        case SerializationUID::SYNTAX_ERROR:
            errorType = base::ErrorType::SYNTAX_ERROR;
            break;
        case SerializationUID::OOM_ERROR:
            errorType = base::ErrorType::OOM_ERROR;
            break;
        default:
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
    }
    JSHandle<JSTaggedValue> msg = DeserializeJSTaggedValue();
    JSHandle<EcmaString> handleMsg(msg);
    JSHandle<JSTaggedValue> errorTag = JSHandle<JSTaggedValue>::Cast(factory_->NewJSError(errorType, handleMsg));
    referenceMap_.emplace(objectId_++, errorTag);
    return errorTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadJSDate()
{
    JSHandle<GlobalEnv> env = thread_->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> dateFunction(env->GetDateFunction());
    JSHandle<JSDate> date = JSHandle<JSDate>::Cast(factory_->NewJSObjectByConstructor(dateFunction));
    JSHandle<JSTaggedValue> dateTag = JSHandle<JSTaggedValue>::Cast(date);
    referenceMap_.emplace(objectId_++, dateTag);
    if (!JudgeType(SerializationUID::JS_PLAIN_OBJECT) || !DefinePropertiesAndElements(dateTag)) {
        return JSHandle<JSTaggedValue>();
    }
    double timeValue = 0.0;
    if (!JudgeType(SerializationUID::DOUBLE) || !ReadDouble(&timeValue)) {
        return JSHandle<JSTaggedValue>();
    }
    date->SetTimeValue(thread_, JSTaggedValue(timeValue));
    double localOffset = 0.0;
    if (!JudgeType(SerializationUID::DOUBLE) || !ReadDouble(&localOffset)) {
        return JSHandle<JSTaggedValue>();
    }
    date->SetLocalOffset(thread_, JSTaggedValue(localOffset));
    return dateTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadJSArray()
{
    JSHandle<JSArray> jsArray = thread_->GetEcmaVM()->GetFactory()->NewJSArray();
    JSHandle<JSTaggedValue> arrayTag = JSHandle<JSTaggedValue>::Cast(jsArray);
    referenceMap_.emplace(objectId_++, arrayTag);
    if (!JudgeType(SerializationUID::JS_PLAIN_OBJECT) || !DefinePropertiesAndElements(arrayTag)) {
        return JSHandle<JSTaggedValue>();
    }
    int32_t arrLength;
    if (!JudgeType(SerializationUID::INT32) || !ReadInt(&arrLength)) {
        return JSHandle<JSTaggedValue>();
    }
    jsArray->SetLength(thread_, JSTaggedValue(arrLength));
    return arrayTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadEcmaString()
{
    int32_t stringLength;
    bool isUtf8 = false;
    if (!ReadBoolean(&isUtf8)) {
        return JSHandle<JSTaggedValue>();
    }
    if (!JudgeType(SerializationUID::INT32) || !ReadInt(&stringLength)) {
        return JSHandle<JSTaggedValue>();
    }
    JSHandle<JSTaggedValue> stringTag;
    if (isUtf8) {
        if (stringLength == 0) {
            JSHandle<JSTaggedValue> emptyString = JSHandle<JSTaggedValue>::Cast(factory_->GetEmptyString());
            referenceMap_.emplace(objectId_++, emptyString);
            return emptyString;
        }

        uint8_t *string = reinterpret_cast<uint8_t*>(GetBuffer(stringLength + 1));
        if (string == nullptr) {
            return JSHandle<JSTaggedValue>();
        }

        JSHandle<EcmaString> ecmaString = factory_->NewFromUtf8(string, stringLength);
        stringTag = JSHandle<JSTaggedValue>(ecmaString);
        referenceMap_.emplace(objectId_++, stringTag);
    } else {
        uint16_t *string = reinterpret_cast<uint16_t*>(GetBuffer(stringLength * sizeof(uint16_t)));
        if (string == nullptr) {
            return JSHandle<JSTaggedValue>();
        }
        JSHandle<EcmaString> ecmaString = factory_->NewFromUtf16(string, stringLength);
        stringTag = JSHandle<JSTaggedValue>(ecmaString);
        referenceMap_.emplace(objectId_++, stringTag);
    }
    return stringTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadPlainObject()
{
    JSHandle<GlobalEnv> env = thread_->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> objFunc(env->GetObjectFunction());
    JSHandle<JSObject> jsObject = thread_->GetEcmaVM()->GetFactory()->NewJSObjectByConstructor(objFunc);
    JSHandle<JSTaggedValue> objTag = JSHandle<JSTaggedValue>::Cast(jsObject);
    referenceMap_.emplace(objectId_++, objTag);
    if (!DefinePropertiesAndElements(objTag)) {
        return JSHandle<JSTaggedValue>();
    }
    return objTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadNativeBindingObject()
{
    uintptr_t funcPointer;
    if (!ReadNativePointer(&funcPointer)) {
        return JSHandle<JSTaggedValue>();
    }
    AttachFunc attachFunc = reinterpret_cast<AttachFunc>(funcPointer);
    if (attachFunc == nullptr) {
        return JSHandle<JSTaggedValue>();
    }
    uintptr_t bufferPointer;
    if (!ReadNativePointer(&bufferPointer)) {
        return JSHandle<JSTaggedValue>();
    }
    uintptr_t hint;
    if (!ReadNativePointer(&hint)) {
        return JSHandle<JSTaggedValue>();
    }
    uintptr_t attachData;
    if (!ReadNativePointer(&attachData)) {
        return JSHandle<JSTaggedValue>();
    }
    Local<JSValueRef> attachVal = attachFunc(engine_, reinterpret_cast<void *>(bufferPointer),
        reinterpret_cast<void *>(hint), reinterpret_cast<void *>(attachData));
    if (attachVal.IsEmpty()) {
        LOG_ECMA(ERROR) << "NativeBindingObject is empty";
        attachVal = JSValueRef::Undefined(thread_->GetEcmaVM());
    }
    return JSNApiHelper::ToJSHandle(attachVal);
}

JSHandle<JSTaggedValue> JSDeserializer::ReadJSMap()
{
    JSHandle<GlobalEnv> env = thread_->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> mapFunction(env->GetBuiltinsMapFunction());
    JSHandle<JSMap> jsMap = JSHandle<JSMap>::Cast(factory_->NewJSObjectByConstructor(mapFunction));
    JSHandle<JSTaggedValue> mapTag = JSHandle<JSTaggedValue>::Cast(jsMap);
    referenceMap_.emplace(objectId_++, mapTag);
    if (!JudgeType(SerializationUID::JS_PLAIN_OBJECT) || !DefinePropertiesAndElements(mapTag)) {
        return JSHandle<JSTaggedValue>();
    }
    int32_t size;
    if (!JudgeType(SerializationUID::INT32) || !ReadInt(&size)) {
        return JSHandle<JSTaggedValue>();
    }
    JSHandle<LinkedHashMap> linkedMap = LinkedHashMap::Create(thread_);
    jsMap->SetLinkedMap(thread_, linkedMap);
    for (int32_t i = 0; i < size; i++) {
        JSHandle<JSTaggedValue> key = DeserializeJSTaggedValue();
        if (key.IsEmpty()) {
            return JSHandle<JSTaggedValue>();
        }
        JSHandle<JSTaggedValue> value = DeserializeJSTaggedValue();
        if (value.IsEmpty()) {
            return JSHandle<JSTaggedValue>();
        }
        JSMap::Set(thread_, jsMap, key, value);
    }
    return mapTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadJSSet()
{
    JSHandle<GlobalEnv> env = thread_->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> setFunction(env->GetBuiltinsSetFunction());
    JSHandle<JSSet> jsSet = JSHandle<JSSet>::Cast(factory_->NewJSObjectByConstructor(setFunction));
    JSHandle<JSTaggedValue> setTag = JSHandle<JSTaggedValue>::Cast(jsSet);
    referenceMap_.emplace(objectId_++, setTag);
    if (!JudgeType(SerializationUID::JS_PLAIN_OBJECT) || !DefinePropertiesAndElements(setTag)) {
        return JSHandle<JSTaggedValue>();
    }
    int32_t size;
    if (!JudgeType(SerializationUID::INT32) || !ReadInt(&size)) {
        return JSHandle<JSTaggedValue>();
    }
    JSHandle<LinkedHashSet> linkedSet = LinkedHashSet::Create(thread_);
    jsSet->SetLinkedSet(thread_, linkedSet);
    for (int32_t i = 0; i < size; i++) {
        JSHandle<JSTaggedValue> key = DeserializeJSTaggedValue();
        if (key.IsEmpty()) {
            return JSHandle<JSTaggedValue>();
        }
        JSSet::Add(thread_, jsSet, key);
    }
    return setTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadJSRegExp()
{
    JSHandle<GlobalEnv> env = thread_->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> regexpFunction(env->GetRegExpFunction());
    JSHandle<JSObject> obj = factory_->NewJSObjectByConstructor(regexpFunction);
    JSHandle<JSRegExp> regExp = JSHandle<JSRegExp>::Cast(obj);
    JSHandle<JSTaggedValue> regexpTag = JSHandle<JSTaggedValue>::Cast(regExp);
    referenceMap_.emplace(objectId_++, regexpTag);
    if (!JudgeType(SerializationUID::JS_PLAIN_OBJECT) || !DefinePropertiesAndElements(regexpTag)) {
        return JSHandle<JSTaggedValue>();
    }
    int32_t bufferSize;
    if (!JudgeType(SerializationUID::INT32) || !ReadInt(&bufferSize)) {
        return JSHandle<JSTaggedValue>();
    }
    void *buffer = GetBuffer(bufferSize);
    if (buffer == nullptr) {
        return JSHandle<JSTaggedValue>();
    }
    factory_->NewJSRegExpByteCodeData(regExp, buffer, bufferSize);
    JSHandle<JSTaggedValue> originalSource = DeserializeJSTaggedValue();
    regExp->SetOriginalSource(thread_, originalSource);
    JSHandle<JSTaggedValue> originalFlags = DeserializeJSTaggedValue();
    regExp->SetOriginalFlags(thread_, originalFlags);
    return regexpTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadJSTypedArray(SerializationUID uid)
{
    JSHandle<GlobalEnv> env = thread_->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSTaggedValue> target;
    JSHandle<JSObject> obj;
    JSHandle<JSTaggedValue> objTag;
    switch (uid) {
        case SerializationUID::JS_INT8_ARRAY: {
            target = env->GetInt8ArrayFunction();
            break;
        }
        case SerializationUID::JS_UINT8_ARRAY: {
            target = env->GetUint8ArrayFunction();
            break;
        }
        case SerializationUID::JS_UINT8_CLAMPED_ARRAY: {
            target = env->GetUint8ClampedArrayFunction();
            break;
        }
        case SerializationUID::JS_INT16_ARRAY: {
            target = env->GetInt16ArrayFunction();
            break;
        }
        case SerializationUID::JS_UINT16_ARRAY: {
            target = env->GetUint16ArrayFunction();
            break;
        }
        case SerializationUID::JS_INT32_ARRAY: {
            target = env->GetInt32ArrayFunction();
            break;
        }
        case SerializationUID::JS_UINT32_ARRAY: {
            target = env->GetUint32ArrayFunction();
            break;
        }
        case SerializationUID::JS_FLOAT32_ARRAY: {
            target = env->GetFloat32ArrayFunction();
            break;
        }
        case SerializationUID::JS_FLOAT64_ARRAY: {
            target = env->GetFloat64ArrayFunction();
            break;
        }
        case SerializationUID::JS_BIGINT64_ARRAY: {
            target = env->GetBigInt64ArrayFunction();
            break;
        }
        case SerializationUID::JS_BIGUINT64_ARRAY: {
            target = env->GetBigUint64ArrayFunction();
            break;
        }
        default:
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
    }
    JSHandle<JSTypedArray> typedArray =
        JSHandle<JSTypedArray>::Cast(factory_->NewJSObjectByConstructor(JSHandle<JSFunction>(target)));
    obj = JSHandle<JSObject>::Cast(typedArray);
    objTag = JSHandle<JSTaggedValue>::Cast(obj);
    referenceMap_.emplace(objectId_++, objTag);
    if (!JudgeType(SerializationUID::JS_PLAIN_OBJECT) || !DefinePropertiesAndElements(objTag)) {
        return JSHandle<JSTaggedValue>();
    }

    bool isViewedArrayBuffer = false;
    if (!ReadBoolean(&isViewedArrayBuffer)) {
        return JSHandle<JSTaggedValue>();
    }
    JSHandle<JSTaggedValue> viewedArrayBufferOrByteArray;
    if (isViewedArrayBuffer) {
        viewedArrayBufferOrByteArray = DeserializeJSTaggedValue();
    } else {
        if (!JudgeType(SerializationUID::BYTE_ARRAY)) {
            return JSHandle<JSTaggedValue>();
        }
        viewedArrayBufferOrByteArray = ReadByteArray();
    }
    if (viewedArrayBufferOrByteArray.IsEmpty()) {
        return JSHandle<JSTaggedValue>();
    }
    typedArray->SetViewedArrayBuffer(thread_, viewedArrayBufferOrByteArray);

    JSHandle<JSTaggedValue> typedArrayName = DeserializeJSTaggedValue();
    if (typedArrayName.IsEmpty()) {
        return JSHandle<JSTaggedValue>();
    }
    typedArray->SetTypedArrayName(thread_, typedArrayName);

    JSTaggedValue byteLength;
    if (!ReadJSTaggedValue(&byteLength) || !byteLength.IsNumber()) {
        return JSHandle<JSTaggedValue>();
    }
    typedArray->SetByteLength(byteLength.GetNumber());

    JSTaggedValue byteOffset;
    if (!ReadJSTaggedValue(&byteOffset) || !byteOffset.IsNumber()) {
        return JSHandle<JSTaggedValue>();
    }
    typedArray->SetByteOffset(byteOffset.GetNumber());

    JSTaggedValue arrayLength;
    if (!ReadJSTaggedValue(&arrayLength) || !byteOffset.IsNumber()) {
        return JSHandle<JSTaggedValue>();
    }
    typedArray->SetArrayLength(arrayLength.GetNumber());

    ContentType *contentType = reinterpret_cast<ContentType*>(GetBuffer(sizeof(ContentType)));
    if (contentType == nullptr) {
        return JSHandle<JSTaggedValue>();
    }
    typedArray->SetContentType(*contentType);
    return objTag;
}

JSHandle<JSTaggedValue> JSDeserializer::ReadNativePointer()
{
    JSTaggedValue pointer;
    if (!ReadJSTaggedValue(&pointer)) {
        return JSHandle<JSTaggedValue>();
    }
    return JSHandle<JSTaggedValue>(thread_, pointer);
}

JSHandle<JSTaggedValue> JSDeserializer::ReadJSArrayBuffer()
{
    // read access length
    int32_t arrayLength;
    if (!JudgeType(SerializationUID::INT32) || !ReadInt(&arrayLength)) {
        return JSHandle<JSTaggedValue>();
    }
    // read access shared
    bool shared = false;
    if (!ReadBoolean(&shared)) {
        return JSHandle<JSTaggedValue>();
    }
    // create jsarraybuffer
    JSHandle<JSTaggedValue> arrayBufferTag;
    if (shared) {
        uint64_t *bufferAddr = reinterpret_cast<uint64_t*>(GetBuffer(sizeof(uint64_t)));
        void *bufferData = ToVoidPtr(*bufferAddr);
        JSHandle<JSArrayBuffer> arrayBuffer = factory_->NewJSSharedArrayBuffer(bufferData, arrayLength);
        arrayBufferTag = JSHandle<JSTaggedValue>::Cast(arrayBuffer);
        referenceMap_.emplace(objectId_++, arrayBufferTag);
    } else {
        void *fromBuffer = GetBuffer(arrayLength);
        if (fromBuffer == nullptr) {
            return arrayBufferTag;
        }
        JSHandle<JSArrayBuffer> arrayBuffer = factory_->NewJSArrayBuffer(arrayLength);
        arrayBufferTag = JSHandle<JSTaggedValue>::Cast(arrayBuffer);
        referenceMap_.emplace(objectId_++, arrayBufferTag);
        JSHandle<JSNativePointer> np(thread_, arrayBuffer->GetArrayBufferData());
        void *toBuffer = np->GetExternalPointer();
        if (memcpy_s(toBuffer, arrayLength, fromBuffer, arrayLength) != EOK) {
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
        }
    }
    // read jsarraybuffer properties
    if (!JudgeType(SerializationUID::JS_PLAIN_OBJECT) || !DefinePropertiesAndElements(arrayBufferTag)) {
        return JSHandle<JSTaggedValue>();
    }

    return arrayBufferTag;
}

bool JSDeserializer::ReadJSTaggedValue(JSTaggedValue *value)
{
    size_t len = sizeof(JSTaggedValue);
    if (len > static_cast<size_t>(end_ - position_)) {
        return false;
    }
    if (memcpy_s(value, len, position_, len) != EOK) {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
    position_ += len;
    return true;
}

bool JSDeserializer::ReadNativePointer(uintptr_t *value)
{
    size_t len = sizeof(uintptr_t);
    if (len > static_cast<size_t>(end_ - position_)) {
        return false;
    }
    if (memcpy_s(value, len, position_, len) != EOK) {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
    position_ += len;
    return true;
}

void *JSDeserializer::GetBuffer(uint32_t bufferSize)
{
    const uint8_t *buffer = nullptr;
    if (bufferSize > static_cast<size_t>(end_ - position_)) {
        return nullptr;
    }
    buffer = position_;
    position_ += bufferSize;
    uint8_t *retBuffer = const_cast<uint8_t *>(buffer);
    return static_cast<void *>(retBuffer);
}

JSHandle<JSTaggedValue> JSDeserializer::ReadReference()
{
    uint64_t objId;
    if (!ReadObjectId(&objId)) {
        return JSHandle<JSTaggedValue>();
    }
    auto objIter = referenceMap_.find(objId);
    if (objIter == referenceMap_.end()) {
        return JSHandle<JSTaggedValue>();
    }
    return objIter->second;
}

bool JSDeserializer::JudgeType(SerializationUID targetUid)
{
    if (ReadType() != targetUid) {
        return false;
    }
    return true;
}

bool JSDeserializer::DefinePropertiesAndElements(const JSHandle<JSTaggedValue> &obj)
{
    int32_t propertyLength;
    if (!JudgeType(SerializationUID::INT32) || !ReadInt(&propertyLength)) {
        return false;
    }
    for (int32_t i = 0; i < propertyLength; i++) {
        JSHandle<JSTaggedValue> key = DeserializeJSTaggedValue();
        if (key.IsEmpty()) {
            return false;
        }
        PropertyDescriptor desc(thread_);
        if (!ReadDesc(&desc)) {
            return false;
        }
        if (!JSTaggedValue::DefineOwnProperty(thread_, obj, key, desc)) {
            return false;
        }
    }

    int32_t elementLength;
    if (!JudgeType(SerializationUID::INT32) || !ReadInt(&elementLength)) {
        return false;
    }
    for (int32_t i = 0; i < elementLength; i++) {
        JSHandle<JSTaggedValue> key = DeserializeJSTaggedValue();
        if (key.IsEmpty()) {
            return false;
        }
        PropertyDescriptor desc(thread_);
        if (!ReadDesc(&desc)) {
            return false;
        }
        if (!JSTaggedValue::DefineOwnProperty(thread_, obj, key, desc)) {
            return false;
        }
    }
    return true;
}

bool JSDeserializer::ReadDesc(PropertyDescriptor *desc)
{
    bool isWritable = false;
    if (!ReadBoolean(&isWritable)) {
        return false;
    }
    bool isEnumerable = false;
    if (!ReadBoolean(&isEnumerable)) {
        return false;
    }
    bool isConfigurable = false;
    if (!ReadBoolean(&isConfigurable)) {
        return false;
    }
    bool hasWritable = false;
    if (!ReadBoolean(&hasWritable)) {
        return false;
    }
    bool hasEnumerable = false;
    if (!ReadBoolean(&hasEnumerable)) {
        return false;
    }
    bool hasConfigurable = false;
    if (!ReadBoolean(&hasConfigurable)) {
        return false;
    }
    JSHandle<JSTaggedValue> value = DeserializeJSTaggedValue();
    if (value.IsEmpty()) {
        return false;
    }
    desc->SetValue(value);
    if (hasWritable) {
        desc->SetWritable(isWritable);
    }
    if (hasEnumerable) {
        desc->SetEnumerable(isEnumerable);
    }
    if (hasConfigurable) {
        desc->SetConfigurable(isConfigurable);
    }
    return true;
}

bool JSDeserializer::ReadBoolean(bool *value)
{
    SerializationUID uid = ReadType();
    if (uid == SerializationUID::C_TRUE) {
        *value = true;
        return true;
    }
    if (uid == SerializationUID::C_FALSE) {
        *value = false;
        return true;
    }
    return false;
}

bool Serializer::WriteValue(
    JSThread *thread, const JSHandle<JSTaggedValue> &value, const JSHandle<JSTaggedValue> &transfer)
{
    if (data_ != nullptr) {
        return false;
    }
    data_.reset(new SerializationData);
    if (!PrepareTransfer(thread, transfer)) {
        return false;
    }
    if (!valueSerializer_.SerializeJSTaggedValue(value)) {
        return false;
    }
    if (!FinalizeTransfer(thread, transfer)) {
        return false;
    }
    std::pair<uint8_t*, size_t> pair = valueSerializer_.ReleaseBuffer();
    data_->value_.reset(pair.first);
    data_->dataSize_ = pair.second;
    return true;
}

std::unique_ptr<SerializationData> Serializer::Release()
{
    return std::move(data_);
}

bool Serializer::PrepareTransfer(JSThread *thread, const JSHandle<JSTaggedValue> &transfer)
{
    if (transfer->IsUndefined()) {
        return true;
    }
    if (!transfer->IsJSArray()) {
        return false;
    }
    int len = base::ArrayHelper::GetArrayLength(thread, transfer);
    int k = 0;
    while (k < len) {
        bool exists = JSTaggedValue::HasProperty(thread, transfer, k);
        if (exists) {
            JSHandle<JSTaggedValue> element = JSArray::FastGetPropertyByValue(thread, transfer, k);
            if (!element->IsArrayBuffer()) {
                return false;
            }
            arrayBufferIdxs_.emplace_back(k);
        }
        k++;
    }
    return true;
}

bool Serializer::FinalizeTransfer(JSThread *thread, const JSHandle<JSTaggedValue> &transfer)
{
    for (int idx : arrayBufferIdxs_) {
        JSHandle<JSTaggedValue> element = JSArray::FastGetPropertyByValue(thread, transfer, idx);
        JSArrayBuffer::Cast(element->GetTaggedObject())->Detach(thread);
    }
    return true;
}

JSHandle<JSTaggedValue> Deserializer::ReadValue()
{
    return valueDeserializer_.DeserializeJSTaggedValue();
}
}  // namespace panda::ecmascript