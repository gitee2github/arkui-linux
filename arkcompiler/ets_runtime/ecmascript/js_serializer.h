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

#ifndef ECMASCRIPT_JS_SERIALIZER_H
#define ECMASCRIPT_JS_SERIALIZER_H

#include <map>

#include "ecmascript/ecma_vm.h"
#include "ecmascript/js_date.h"
#include "ecmascript/js_map.h"
#include "ecmascript/js_native_pointer.h"
#include "ecmascript/js_object.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/js_typed_array.h"
#include "ecmascript/jspandafile/js_pandafile.h"
#include "ecmascript/jspandafile/program_object.h"
#include "ecmascript/mem/dyn_chunk.h"
#include "ecmascript/napi/jsnapi_helper.h"
#include "ecmascript/napi/include/jsnapi.h"

using panda::JSValueRef;
namespace panda::ecmascript {
class JSPandaFile;
class ConstantPool;

typedef void* (*DetachFunc)(void *enginePointer, void *objPointer, void *hint, void *detachData);
typedef Local<JSValueRef> (*AttachFunc)(void *enginePointer, void *buffer, void *hint, void *attachData);

enum class SerializationUID : uint8_t {
    // JS special values
    JS_NULL = 0x01,
    JS_UNDEFINED,
    JS_TRUE,
    JS_FALSE,
    HOLE,
    // Number types
    INT32,
    DOUBLE,
    // Not support yet, BigInt type has not been implemented in ark engine
    BIGINT,
    ECMASTRING,
    // Boolean types
    C_TRUE,
    C_FALSE,
    // Tagged object reference mark
    TAGGED_OBJECT_REFERNCE,
    // Support tagged object id reference begin
    JS_DATE,
    JS_REG_EXP,
    JS_PLAIN_OBJECT,
    NATIVE_BINDING_OBJECT,
    JS_SET,
    JS_MAP,
    JS_ARRAY,
    JS_ARRAY_BUFFER,
    JS_SHARED_ARRAY_BUFFER,
    // TypedArray begin
    JS_UINT8_ARRAY,
    JS_UINT8_CLAMPED_ARRAY,
    JS_UINT16_ARRAY,
    JS_UINT32_ARRAY,
    JS_INT8_ARRAY,
    JS_INT16_ARRAY,
    JS_INT32_ARRAY,
    JS_FLOAT32_ARRAY,
    JS_FLOAT64_ARRAY,
    JS_BIGINT64_ARRAY,
    JS_BIGUINT64_ARRAY,
    // TypedArray end
    // Support tagged object id reference end
    // Error UIDs
    JS_ERROR,
    EVAL_ERROR,
    RANGE_ERROR,
    REFERENCE_ERROR,
    TYPE_ERROR,
    AGGREGATE_ERROR,
    URI_ERROR,
    SYNTAX_ERROR,
    OOM_ERROR,
    ERROR_MESSAGE_BEGIN,
    ERROR_MESSAGE_END,
    // Function begin
    CONCURRENT_FUNCTION,
    JS_METHOD,
    NATIVE_METHOD,
    CONSTANT_POOL,
    TAGGED_ARRAY,
    // Function end
    BYTE_ARRAY,
    NATIVE_POINTER,
    UNKNOWN
};

class JSSerializer {
public:
    explicit JSSerializer(JSThread *thread) : thread_(thread) {}
    ~JSSerializer() = default;
    bool SerializeJSTaggedValue(const JSHandle<JSTaggedValue> &value);

    // Return pointer to the buffer and its length, should not use this Serializer anymore after Release
    std::pair<uint8_t *, size_t> ReleaseBuffer();

private:
    bool WriteJSFunction(const JSHandle<JSTaggedValue> &value);
    bool WriteMethod(const JSHandle<JSTaggedValue> &value);
    bool WriteConstantPool(const JSHandle<JSTaggedValue> &value);
    bool WriteTaggedArray(const JSHandle<JSTaggedValue> &value);
    bool WriteByteArray(const JSHandle<JSTaggedValue> &value, DataViewType viewType);
    bool WriteTaggedObject(const JSHandle<JSTaggedValue> &value);
    bool WritePrimitiveValue(const JSHandle<JSTaggedValue> &value);
    bool WriteInt(int32_t value);
    bool WriteDouble(double value);
    bool WriteRawData(const void *data, size_t length);
    bool WriteType(SerializationUID uId);
    bool AllocateBuffer(size_t bytes);
    bool ExpandBuffer(size_t requestedSize);
    bool WriteBoolean(bool value);
    bool WriteJSError(const JSHandle<JSTaggedValue> &value);
    bool WriteJSErrorHeader(JSType type);
    bool WriteJSDate(const JSHandle<JSTaggedValue> &value);
    bool WriteJSArray(const JSHandle<JSTaggedValue> &value);
    bool WriteJSMap(const JSHandle<JSTaggedValue> &value);
    bool WriteJSSet(const JSHandle<JSTaggedValue> &value);
    bool WriteJSRegExp(const JSHandle<JSTaggedValue> &value);
    bool WriteEcmaString(const JSHandle<JSTaggedValue> &value);
    bool WriteJSTypedArray(const JSHandle<JSTaggedValue> &value, SerializationUID uId);
    bool WritePlainObject(const JSHandle<JSTaggedValue> &value);
    bool WriteNativeBindingObject(const JSHandle<JSTaggedValue> &value);
    bool WriteNativePointer(const JSHandle<JSTaggedValue> &value);
    bool WriteJSArrayBuffer(const JSHandle<JSTaggedValue> &value);
    bool WriteDesc(const PropertyDescriptor &desc);
    bool IsNativeBindingObject(std::vector<JSTaggedValue> keyVector);
    bool IsTargetSymbol(JSTaggedValue symbolVal);
    bool IsSerialized(uintptr_t addr) const;
    bool WriteIfSerialized(uintptr_t addr);
    uint32_t GetDataViewTypeIndex(const DataViewType dataViewType);

    NO_MOVE_SEMANTIC(JSSerializer);
    NO_COPY_SEMANTIC(JSSerializer);

    JSThread *thread_;
    uint8_t *buffer_ = nullptr;
    uint64_t sizeLimit_ = 0;
    size_t bufferSize_ = 0;
    size_t bufferCapacity_ = 0;
    // The Reference map is used for check whether a tagged object has been serialized
    // Reference map works only if no gc happens during serialization
    std::map<uintptr_t, uint64_t> referenceMap_;
    uint64_t objectId_ = 0;
};

class JSDeserializer {
public:
    // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    JSDeserializer(JSThread *thread, uint8_t *data, size_t size, void *hint = nullptr)
        : thread_(thread), factory_(thread->GetEcmaVM()->GetFactory()),
        begin_(data), position_(data), end_(data + size), engine_(hint)
    {
    }
    ~JSDeserializer();
    JSHandle<JSTaggedValue> DeserializeJSTaggedValue();

private:
    bool ReadInt(int32_t *value);
    bool ReadObjectId(uint64_t *objectId);
    bool ReadDouble(double *value);
    SerializationUID ReadType();
    JSHandle<JSTaggedValue> ReadJSFunctionBase();
    JSHandle<JSTaggedValue> ReadJSFunction();
    JSHandle<JSTaggedValue> ReadTaggedArray();
    JSHandle<JSTaggedValue> ReadByteArray();
    JSHandle<JSTaggedValue> ReadJSMethod();
    JSHandle<JSTaggedValue> ReadNativeMethod();
    JSHandle<JSTaggedValue> ReadConstantPool();
    JSHandle<JSTaggedValue> ReadJSError(SerializationUID uid);
    JSHandle<JSTaggedValue> ReadJSDate();
    JSHandle<JSTaggedValue> ReadJSArray();
    JSHandle<JSTaggedValue> ReadPlainObject();
    JSHandle<JSTaggedValue> ReadEcmaString();
    JSHandle<JSTaggedValue> ReadJSMap();
    JSHandle<JSTaggedValue> ReadJSSet();
    JSHandle<JSTaggedValue> ReadJSRegExp();
    JSHandle<JSTaggedValue> ReadJSTypedArray(SerializationUID uid);
    JSHandle<JSTaggedValue> ReadNativePointer();
    JSHandle<JSTaggedValue> ReadJSArrayBuffer();
    JSHandle<JSTaggedValue> ReadReference();
    JSHandle<JSTaggedValue> ReadNativeBindingObject();
    bool JudgeType(SerializationUID targetUid);
    void *GetBuffer(uint32_t bufferSize);
    bool ReadJSTaggedValue(JSTaggedValue *originalFlags);
    bool ReadNativePointer(uintptr_t *pointer);
    bool DefinePropertiesAndElements(const JSHandle<JSTaggedValue> &obj);
    bool ReadDesc(PropertyDescriptor *desc);
    bool ReadBoolean(bool *value);
    DataViewType GetDataViewTypeByIndex(uint32_t viewTypeIndex);

    NO_MOVE_SEMANTIC(JSDeserializer);
    NO_COPY_SEMANTIC(JSDeserializer);

    JSThread *thread_ = nullptr;
    ObjectFactory *factory_ = nullptr;
    uint8_t *begin_ = nullptr;
    const uint8_t *position_ = nullptr;
    const uint8_t * const end_ = nullptr;
    uint64_t objectId_ = 0;
    std::map<uint64_t, JSHandle<JSTaggedValue>> referenceMap_;
    void *engine_ = nullptr;
};

class SerializationData {
public:
    SerializationData() : dataSize_(0), value_(nullptr) {}
    ~SerializationData() = default;

    uint8_t* GetData() const
    {
        return value_.get();
    }
    size_t GetSize() const
    {
        return dataSize_;
    }

private:
    struct Deleter {
        void operator()(uint8_t* ptr) const
        {
            free(ptr);
        }
    };

    size_t dataSize_;
    std::unique_ptr<uint8_t, Deleter> value_;

private:
    friend class Serializer;

    NO_COPY_SEMANTIC(SerializationData);
};

class Serializer {
public:
    explicit Serializer(JSThread *thread) : valueSerializer_(thread) {}
    ~Serializer() = default;

    bool WriteValue(JSThread *thread, const JSHandle<JSTaggedValue> &value, const JSHandle<JSTaggedValue> &transfer);
    std::unique_ptr<SerializationData> Release();

private:
    bool PrepareTransfer(JSThread *thread, const JSHandle<JSTaggedValue> &transfer);
    bool FinalizeTransfer(JSThread *thread, const JSHandle<JSTaggedValue> &transfer);

private:
    ecmascript::JSSerializer valueSerializer_;
    std::unique_ptr<SerializationData> data_;
    CVector<int> arrayBufferIdxs_;

    NO_COPY_SEMANTIC(Serializer);
};

class Deserializer {
public:
    explicit Deserializer(JSThread *thread, SerializationData *data, void *hint)
        : valueDeserializer_(thread, data->GetData(), data->GetSize(), hint) {}
    ~Deserializer() = default;

    JSHandle<JSTaggedValue> ReadValue();

private:
    ecmascript::JSDeserializer valueDeserializer_;

    NO_COPY_SEMANTIC(Deserializer);
};
}  // namespace panda::ecmascript

#endif // ECMASCRIPT_JS_SERIALIZER_H