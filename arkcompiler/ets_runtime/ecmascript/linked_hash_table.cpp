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

#include "ecmascript/linked_hash_table.h"

#include "ecmascript/js_object-inl.h"
#include "ecmascript/object_factory.h"

namespace panda::ecmascript {
template<typename Derived, typename HashObject>
JSHandle<Derived> LinkedHashTable<Derived, HashObject>::Create(const JSThread *thread, int numberOfElements)
{
    ASSERT_PRINT(numberOfElements > 0, "size must be a non-negative integer");
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    auto capacity = static_cast<uint32_t>(numberOfElements);
    ASSERT_PRINT(helpers::math::IsPowerOfTwo(capacity), "capacity must be pow of '2'");
    int length = ELEMENTS_START_INDEX + numberOfElements + numberOfElements * (HashObject::ENTRY_SIZE + 1);

    auto table = JSHandle<Derived>(factory->NewTaggedArray(length));
    table->SetNumberOfElements(thread, 0);
    table->SetNumberOfDeletedElements(thread, 0);
    table->SetCapacity(thread, capacity);
    return table;
}

template<typename Derived, typename HashObject>
JSHandle<Derived> LinkedHashTable<Derived, HashObject>::Insert(const JSThread *thread, const JSHandle<Derived> &table,
                                                               const JSHandle<JSTaggedValue> &key,
                                                               const JSHandle<JSTaggedValue> &value)
{
    ASSERT(IsKey(key.GetTaggedValue()));
    int hash = LinkedHash::Hash(key.GetTaggedValue());
    int entry = table->FindElement(key.GetTaggedValue());
    if (entry != -1) {
        table->SetValue(thread, entry, value.GetTaggedValue());
        return table;
    }

    JSHandle<Derived> newTable = GrowCapacity(thread, table);

    uint32_t bucket = newTable->HashToBucket(hash);
    entry = newTable->NumberOfElements() + newTable->NumberOfDeletedElements();
    newTable->InsertNewEntry(thread, bucket, entry);
    newTable->SetKey(thread, entry, key.GetTaggedValue());
    newTable->SetValue(thread, entry, value.GetTaggedValue());
    newTable->SetNumberOfElements(thread, newTable->NumberOfElements() + 1);

    return newTable;
}

template<typename Derived, typename HashObject>
JSHandle<Derived> LinkedHashTable<Derived, HashObject>::InsertWeakRef(const JSThread *thread,
    const JSHandle<Derived> &table, const JSHandle<JSTaggedValue> &key, const JSHandle<JSTaggedValue> &value)
{
    ASSERT(IsKey(key.GetTaggedValue()));
    int hash = LinkedHash::Hash(key.GetTaggedValue());
    int entry = table->FindElement(key.GetTaggedValue());
    if (entry != -1) {
        table->SetValue(thread, entry, value.GetTaggedValue());
        return table;
    }

    JSHandle<Derived> newTable = GrowCapacity(thread, table);

    uint32_t bucket = newTable->HashToBucket(hash);
    entry = newTable->NumberOfElements() + newTable->NumberOfDeletedElements();
    newTable->InsertNewEntry(thread, bucket, entry);
    JSTaggedValue weakKey(key->CreateAndGetWeakRef());
    newTable->SetKey(thread, entry, weakKey);
    // The ENTRY_VALUE_INDEX of LinkedHashSet is 0. SetValue will cause the overwitten key.
    if (std::is_same_v<LinkedHashMap, Derived>) {
        newTable->SetValue(thread, entry, value.GetTaggedValue());
    }
    newTable->SetNumberOfElements(thread, newTable->NumberOfElements() + 1);

    return newTable;
}

template<typename Derived, typename HashObject>
JSHandle<Derived> LinkedHashTable<Derived, HashObject>::GrowCapacity(const JSThread *thread,
    const JSHandle<Derived> &table, int numberOfAddedElements)
{
    if (table->HasSufficientCapacity(numberOfAddedElements)) {
        return table;
    }
    int newCapacity = ComputeCapacity(table->NumberOfElements() + numberOfAddedElements);
    JSHandle<Derived> newTable = Create(thread, newCapacity);
    table->Rehash(thread, *newTable);
    return newTable;
}

template<typename Derived, typename HashObject>
JSHandle<Derived> LinkedHashTable<Derived, HashObject>::Remove(const JSThread *thread, const JSHandle<Derived> &table,
                                                               const JSHandle<JSTaggedValue> &key)
{
    int entry = table->FindElement(key.GetTaggedValue());
    if (entry == -1) {
        return table;
    }

    table->RemoveEntry(thread, entry);
    return Shrink(thread, table);
}

template<typename Derived, typename HashObject>
JSHandle<Derived> LinkedHashTable<Derived, HashObject>::Shrink(const JSThread *thread, const JSHandle<Derived> &table,
    int additionalCapacity)
{
    int newCapacity = ComputeCapacityWithShrink(table->Capacity(), table->NumberOfElements() + additionalCapacity);
    if (newCapacity == table->Capacity()) {
        return table;
    }

    JSHandle<Derived> newTable = Create(thread, newCapacity);

    table->Rehash(thread, *newTable);
    return newTable;
}

// LinkedHashMap
JSHandle<LinkedHashMap> LinkedHashMap::Create(const JSThread *thread, int numberOfElements)
{
    return LinkedHashTable<LinkedHashMap, LinkedHashMapObject>::Create(thread, numberOfElements);
}

JSHandle<LinkedHashMap> LinkedHashMap::Delete(const JSThread *thread, const JSHandle<LinkedHashMap> &obj,
                                              const JSHandle<JSTaggedValue> &key)
{
    return LinkedHashTable<LinkedHashMap, LinkedHashMapObject>::Remove(thread, obj, key);
}

JSHandle<LinkedHashMap> LinkedHashMap::Set(const JSThread *thread, const JSHandle<LinkedHashMap> &obj,
    const JSHandle<JSTaggedValue> &key, const JSHandle<JSTaggedValue> &value)
{
    return LinkedHashTable<LinkedHashMap, LinkedHashMapObject>::Insert(thread, obj, key, value);
}

JSHandle<LinkedHashMap> LinkedHashMap::SetWeakRef(const JSThread *thread, const JSHandle<LinkedHashMap> &obj,
    const JSHandle<JSTaggedValue> &key, const JSHandle<JSTaggedValue> &value)
{
    return LinkedHashTable<LinkedHashMap, LinkedHashMapObject>::InsertWeakRef(thread, obj, key, value);
}

JSTaggedValue LinkedHashMap::Get(JSTaggedValue key) const
{
    int entry = FindElement(key);
    if (entry == -1) {
        return JSTaggedValue::Undefined();
    }
    return GetValue(entry);
}

bool LinkedHashMap::Has(JSTaggedValue key) const
{
    int entry = FindElement(key);
    return entry != -1;
}

JSHandle<LinkedHashMap> LinkedHashMap::Clear(const JSThread *thread, const JSHandle<LinkedHashMap> &table)
{
    JSHandle<LinkedHashMap> newMap = LinkedHashMap::Create(thread);
    if (table->Capacity() > 0) {
        table->SetNextTable(thread, newMap.GetTaggedValue());
        table->SetNumberOfDeletedElements(thread, -1);
    }
    return newMap;
}

JSHandle<LinkedHashMap> LinkedHashMap::Shrink(const JSThread *thread, const JSHandle<LinkedHashMap> &table,
                                              int additionalCapacity)
{
    return LinkedHashTable<LinkedHashMap, LinkedHashMapObject>::Shrink(thread, table, additionalCapacity);
}

// LinkedHashSet
JSHandle<LinkedHashSet> LinkedHashSet::Create(const JSThread *thread, int numberOfElements)
{
    return LinkedHashTable<LinkedHashSet, LinkedHashSetObject>::Create(thread, numberOfElements);
}

JSHandle<LinkedHashSet> LinkedHashSet::Delete(const JSThread *thread, const JSHandle<LinkedHashSet> &obj,
                                              const JSHandle<JSTaggedValue> &key)
{
    return LinkedHashTable<LinkedHashSet, LinkedHashSetObject>::Remove(thread, obj, key);
}

JSHandle<LinkedHashSet> LinkedHashSet::Add(const JSThread *thread, const JSHandle<LinkedHashSet> &obj,
                                           const JSHandle<JSTaggedValue> &key)
{
    return LinkedHashTable<LinkedHashSet, LinkedHashSetObject>::Insert(thread, obj, key, key);
}

JSHandle<LinkedHashSet> LinkedHashSet::AddWeakRef(const JSThread *thread, const JSHandle<LinkedHashSet> &obj,
    const JSHandle<JSTaggedValue> &key)
{
    return LinkedHashTable<LinkedHashSet, LinkedHashSetObject>::InsertWeakRef(thread, obj, key, key);
}

bool LinkedHashSet::Has(JSTaggedValue key) const
{
    int entry = FindElement(key);
    return entry != -1;
}

JSHandle<LinkedHashSet> LinkedHashSet::Clear(const JSThread *thread, const JSHandle<LinkedHashSet> &table)
{
    JSHandle<LinkedHashSet> newSet = LinkedHashSet::Create(thread);
    if (table->Capacity() > 0) {
        table->SetNextTable(thread, newSet.GetTaggedValue());
        table->SetNumberOfDeletedElements(thread, -1);
    }
    return newSet;
}

JSHandle<LinkedHashSet> LinkedHashSet::Shrink(const JSThread *thread, const JSHandle<LinkedHashSet> &table,
    int additionalCapacity)
{
    return LinkedHashTable<LinkedHashSet, LinkedHashSetObject>::Shrink(thread, table, additionalCapacity);
}

int LinkedHash::Hash(JSTaggedValue key)
{
    if (key.IsSymbol()) {
        auto symbolString = JSSymbol::Cast(key.GetTaggedObject());
        return symbolString->GetHashField();
    }
    if (key.IsString()) {
        auto keyString = reinterpret_cast<EcmaString *>(key.GetTaggedObject());
        return EcmaStringAccessor(keyString).GetHashcode();
    }
    if (key.IsECMAObject()) {
        int32_t hash = ECMAObject::Cast(key.GetTaggedObject())->GetHash();
        if (hash == 0) {
            uint64_t keyValue = key.GetRawData();
            hash = static_cast<int32_t>(
                GetHash32(reinterpret_cast<uint8_t *>(&keyValue), sizeof(keyValue) / sizeof(uint8_t)));
            ECMAObject::Cast(key.GetTaggedObject())->SetHash(hash);
        }
        return hash;
    }

    // Int, Double, Special and HeapObject(except symbol and string)
    if (key.IsDouble()) {
        key = JSTaggedValue::TryCastDoubleToInt32(key.GetDouble());
    }
    uint64_t keyValue = key.GetRawData();
    return GetHash32(reinterpret_cast<uint8_t *>(&keyValue), sizeof(keyValue) / sizeof(uint8_t));
}
}  // namespace panda::ecmascript
