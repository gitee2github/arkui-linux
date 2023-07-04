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

#ifndef ECMASCRIPT_LAYOUT_INFO_H
#define ECMASCRIPT_LAYOUT_INFO_H

#include "ecmascript/js_object.h"
#include "ecmascript/property_attributes.h"
#include "ecmascript/tagged_array.h"

namespace panda::ecmascript {
struct Properties {
    JSTaggedValue key_;
    JSTaggedValue attr_;
};

class LayoutInfo : private TaggedArray {
public:
    static constexpr int MIN_PROPERTIES_LENGTH = JSObject::MIN_PROPERTIES_LENGTH;
    static constexpr int MAX_PROPERTIES_LENGTH = PropertyAttributes::MAX_CAPACITY_OF_PROPERTIES;
    static constexpr uint32_t ELEMENTS_INDEX_LOG2 = 1;
    static constexpr uint32_t ATTR_INDEX_OFFSET = 1;

    inline static LayoutInfo *Cast(TaggedObject *obj)
    {
        ASSERT(JSTaggedValue(obj).IsTaggedArray());
        return reinterpret_cast<LayoutInfo *>(obj);
    }

    int GetPropertiesCapacity() const;
    int NumberOfElements() const;
    void SetNumberOfElements(const JSThread *thread, int properties);
    uint32_t GetKeyIndex(int index) const;
    uint32_t GetAttrIndex(int index) const;
    void SetPropertyInit(const JSThread *thread, int index, const JSTaggedValue &key, const PropertyAttributes &attr);
    void SetKey(const JSThread *thread, int index, const JSTaggedValue &key);
    void SetNormalAttr(const JSThread *thread, int index, const PropertyAttributes &attr);
    JSTaggedValue GetKey(int index) const;
    PropertyAttributes GetAttr(int index) const;
    JSTaggedValue GetSortedKey(int index) const;
    uint32_t GetSortedIndex(int index) const;
    void SetSortedIndex(const JSThread *thread, int index, int sortedIndex);
    void AddKey(const JSThread *thread, int index, const JSTaggedValue &key, const PropertyAttributes &attr);

    inline uint32_t GetLength() const
    {
        return TaggedArray::GetLength();
    }

    inline Properties *GetProperties() const
    {
        return reinterpret_cast<Properties *>(reinterpret_cast<uintptr_t>(this) + TaggedArray::DATA_OFFSET);
    }

    static inline uint32_t ComputeArrayLength(uint32_t properties_number)
    {
        return (properties_number << ELEMENTS_INDEX_LOG2);
    }

    static inline uint32_t ComputeGrowCapacity(uint32_t old_capacity)
    {
        uint32_t new_capacity = old_capacity + MIN_PROPERTIES_LENGTH;
        return new_capacity > MAX_PROPERTIES_LENGTH ? MAX_PROPERTIES_LENGTH : new_capacity;
    }

    int FindElementWithCache(JSThread *thread, JSHClass *cls, JSTaggedValue key, int propertiesNumber);
    int BinarySearch(JSTaggedValue key, int propertiesNumber);
    void GetAllKeys(const JSThread *thread, int end, int offset, TaggedArray *keyArray,
                    const JSHandle<JSObject> object);
    void GetAllKeys(int end, std::vector<JSTaggedValue> &keyVector, const JSHandle<JSObject> object);
    void GetAllEnumKeys(const JSThread *thread, int end, int offset, TaggedArray *keyArray, uint32_t *keys,
                        const JSHandle<JSObject> object);

    DECL_DUMP()

private:
    bool IsUninitializedProperty(const JSHandle<JSObject> object, uint32_t index);
};
}  // namespace panda::ecmascript

#endif  // ECMASCRIPT_LAYOUT_INFO_H
