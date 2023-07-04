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

#ifndef ECMASCRIPT_TAGGED_DICTIONARY_H
#define ECMASCRIPT_TAGGED_DICTIONARY_H

#include "ecmascript/ecma_vm.h"
#include "ecmascript/js_tagged_value-inl.h"
#include "ecmascript/tagged_array-inl.h"
#include "ecmascript/tagged_hash_table.h"

namespace panda::ecmascript {
class NameDictionary : public OrderTaggedHashTable<NameDictionary> {
public:
    using OrderHashTableT = OrderTaggedHashTable<NameDictionary>;
    inline static int GetKeyIndex(int entry)
    {
        return OrderHashTableT::TABLE_HEADER_SIZE + entry * GetEntrySize() + ENTRY_KEY_INDEX;
    }
    inline static int GetValueIndex(int entry)
    {
        return OrderHashTableT::TABLE_HEADER_SIZE + entry * GetEntrySize() + ENTRY_VALUE_INDEX;
    }
    inline static int GetEntryIndex(int entry)
    {
        return OrderHashTableT::TABLE_HEADER_SIZE + entry * GetEntrySize();
    }
    inline static int GetEntrySize()
    {
        return ENTRY_SIZE;
    }
    static int Hash(const JSTaggedValue &key);
    static bool IsMatch(const JSTaggedValue &key, const JSTaggedValue &other);
    static JSHandle<NameDictionary> Create(const JSThread *thread,
                                  int numberOfElements = OrderHashTableT::DEFAULT_ELEMENTS_NUMBER);
    // Returns the property metaData for the property at entry.
    PropertyAttributes GetAttributes(int entry) const;
    void SetAttributes(const JSThread *thread, int entry, const PropertyAttributes &metaData);
    void SetEntry(const JSThread *thread, int entry, const JSTaggedValue &key, const JSTaggedValue &value,
                  const PropertyAttributes &metaData);
    void UpdateValueAndAttributes(const JSThread *thread, int entry, const JSTaggedValue &value,
                                  const PropertyAttributes &metaData);
    void UpdateValue(const JSThread *thread, int entry, const JSTaggedValue &value);
    void UpdateAttributes(int entry, const PropertyAttributes &metaData);
    void ClearEntry(const JSThread *thread, int entry);
    void GetAllKeys(const JSThread *thread, int offset, TaggedArray *keyArray) const;
    void GetAllEnumKeys(const JSThread *thread, int offset, TaggedArray *keyArray, uint32_t *keys) const;
    static inline bool CompKey(const std::pair<JSTaggedValue, PropertyAttributes> &a,
                               const std::pair<JSTaggedValue, PropertyAttributes> &b)
    {
        return a.second.GetDictionaryOrder() < b.second.GetDictionaryOrder();
    }
    DECL_DUMP()

    static constexpr int ENTRY_KEY_INDEX = 0;
    static constexpr int ENTRY_VALUE_INDEX = 1;
    static constexpr int ENTRY_DETAILS_INDEX = 2;
    static constexpr int ENTRY_SIZE = 3;
};

class NumberDictionary : public OrderTaggedHashTable<NumberDictionary> {
public:
    using OrderHashTableT = OrderTaggedHashTable<NumberDictionary>;
    inline static int GetKeyIndex(int entry)
    {
        return OrderHashTableT::TABLE_HEADER_SIZE + entry * GetEntrySize() + ENTRY_KEY_INDEX;
    }
    inline static int GetValueIndex(int entry)
    {
        return OrderHashTableT::TABLE_HEADER_SIZE + entry * GetEntrySize() + ENTRY_VALUE_INDEX;
    }
    inline static int GetEntryIndex(int entry)
    {
        return OrderHashTableT::TABLE_HEADER_SIZE + entry * GetEntrySize();
    }
    inline static int GetEntrySize()
    {
        return ENTRY_SIZE;
    }
    static int Hash(const JSTaggedValue &key);
    static bool IsMatch(const JSTaggedValue &key, const JSTaggedValue &other);
    static JSHandle<NumberDictionary> Create(const JSThread *thread,
                                    int numberOfElements = OrderHashTableT::DEFAULT_ELEMENTS_NUMBER);
    // Returns the property metaData for the property at entry.
    PropertyAttributes GetAttributes(int entry) const;
    void SetAttributes(const JSThread *thread, int entry, const PropertyAttributes &metaData);
    void SetEntry([[maybe_unused]] const JSThread *thread, int entry, const JSTaggedValue &key,
                  const JSTaggedValue &value, const PropertyAttributes &metaData);
    void UpdateValueAndAttributes(const JSThread *thread, int entry, const JSTaggedValue &value,
                                  const PropertyAttributes &metaData);
    void UpdateValue(const JSThread *thread, int entry, const JSTaggedValue &value);
    void UpdateAttributes(int entry, const PropertyAttributes &metaData);
    void ClearEntry(const JSThread *thread, int entry);

    static void GetAllKeys(const JSThread *thread, const JSHandle<NumberDictionary> &obj, int offset,
                           const JSHandle<TaggedArray> &keyArray);
    static void GetAllEnumKeys(const JSThread *thread, const JSHandle<NumberDictionary> &obj, int offset,
                               const JSHandle<TaggedArray> &keyArray, uint32_t *keys);
    static inline bool CompKey(const JSTaggedValue &a, const JSTaggedValue &b)
    {
        ASSERT(a.IsNumber() && b.IsNumber());
        return a.GetNumber() < b.GetNumber();
    }
    DECL_DUMP()

    static constexpr int ENTRY_KEY_INDEX = 0;
    static constexpr int ENTRY_VALUE_INDEX = 1;
    static constexpr int ENTRY_DETAILS_INDEX = 2;
    static constexpr int ENTRY_SIZE = 3;
};
}  // namespace panda::ecmascript
#endif  // ECMASCRIPT_NEW_DICTIONARY_H
