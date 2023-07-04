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

#ifndef ECMASCRIPT_MEM_PARALLEL_MARKER_H
#define ECMASCRIPT_MEM_PARALLEL_MARKER_H

#include "ecmascript/js_hclass.h"
#include "ecmascript/mem/gc_bitset.h"
#include "ecmascript/mem/object_xray.h"
#include "ecmascript/mem/slots.h"
#include "ecmascript/mem/work_manager.h"

namespace panda::ecmascript {
class Heap;
class Region;
class TaggedObject;

static constexpr uint32_t MAIN_THREAD_INDEX = 0;

class Marker {
public:
    explicit Marker(Heap *heap);
    virtual ~Marker() = default;

    virtual void Initialize()
    {
        LOG_GC(DEBUG) << "Marker::Initialize do nothing";
    }

    void MarkRoots(uint32_t threadId);
    void ProcessOldToNew(uint32_t threadId);                  // for HPPGC only semi mode
    void ProcessOldToNew(uint32_t threadId, Region *region);  // for SemiGC
    void ProcessSnapshotRSet(uint32_t threadId);              // for SemiGC

    virtual void ProcessMarkStack([[maybe_unused]] uint32_t threadId)
    {
        LOG_GC(FATAL) << "can not call this method";
    }

protected:
    // non move
    virtual inline void MarkObject([[maybe_unused]] uint32_t threadId, [[maybe_unused]] TaggedObject *object)
    {
        LOG_GC(FATAL) << "can not call this method";
    }

    virtual inline SlotStatus MarkObject([[maybe_unused]] uint32_t threadId, [[maybe_unused]] TaggedObject *object,
                                         [[maybe_unused]] ObjectSlot slot)  // move
    {
        LOG_GC(FATAL) << "can not call this method";
        return SlotStatus::KEEP_SLOT;
    }

    virtual inline void HandleOldToNewRSet(uint32_t threadId, Region *region) = 0;
    virtual inline void HandleRoots(uint32_t threadId, [[maybe_unused]] Root type, ObjectSlot slot) = 0;
    virtual inline void HandleRangeRoots(uint32_t threadId, [[maybe_unused]] Root type, ObjectSlot start,
                                         ObjectSlot end) = 0;
    virtual inline void HandleDerivedRoots(Root type, ObjectSlot base, ObjectSlot derived,
                                           uintptr_t baseOldObject) = 0;
    virtual inline void RecordWeakReference([[maybe_unused]] uint32_t threadId, [[maybe_unused]] JSTaggedType *ref,
                                            [[maybe_unused]] Region *objectRegion)
    {
        LOG_GC(FATAL) << "can not call this method";
    }

    Heap *heap_ {nullptr};
    ObjectXRay objXRay_ {nullptr};
    WorkManager *workManager_ {nullptr};
};

class NonMovableMarker : public Marker {
public:
    explicit NonMovableMarker(Heap *heap) : Marker(heap) {}
    ~NonMovableMarker() = default;

protected:
    void ProcessMarkStack(uint32_t threadId) override;
    inline void MarkObject(uint32_t threadId, TaggedObject *object) override;
    inline void HandleRoots(uint32_t threadId, [[maybe_unused]] Root type, ObjectSlot slot) override;
    inline void HandleRangeRoots(uint32_t threadId, [[maybe_unused]] Root type, ObjectSlot start,
                                 ObjectSlot end) override;
    inline void HandleDerivedRoots(Root type, ObjectSlot base, ObjectSlot derived,
                                   uintptr_t baseOldObject) override;

    inline void HandleOldToNewRSet(uint32_t threadId, Region *region) override;
    inline void RecordWeakReference(uint32_t threadId, JSTaggedType *ref, Region *objectRegion) override;
};

class MovableMarker : public Marker {
public:
    explicit MovableMarker(Heap *heap) : Marker(heap) {}
    ~MovableMarker() = default;

protected:
    inline void HandleRoots(uint32_t threadId, [[maybe_unused]] Root type, ObjectSlot slot) override;
    inline void HandleRangeRoots(uint32_t threadId, [[maybe_unused]] Root type, ObjectSlot start,
                                 ObjectSlot end) override;
    inline void HandleDerivedRoots(Root type, ObjectSlot base, ObjectSlot derived,
                                   uintptr_t baseOldObject) override;
    virtual inline SlotStatus EvacuateObject(uint32_t threadId, TaggedObject *object, const MarkWord &markWord,
                                             ObjectSlot slot) = 0;

    inline void HandleOldToNewRSet(uint32_t threadId, Region *region) override;

    inline uintptr_t AllocateDstSpace(uint32_t threadId, size_t size, bool &shouldPromote);
    inline void UpdateForwardAddressIfSuccess(uint32_t threadId, TaggedObject *object, JSHClass *klass,
                                              uintptr_t toAddress, size_t size, const MarkWord &markWord,
                                              ObjectSlot slot, bool isPromoted = false);
    inline bool UpdateForwardAddressIfFailed(TaggedObject *object, uintptr_t toAddress, size_t size, ObjectSlot slot);
};

class SemiGCMarker : public MovableMarker {
public:
    explicit SemiGCMarker(Heap *heap) : MovableMarker(heap) {}
    ~SemiGCMarker() = default;

    void Initialize() override;

protected:
    void ProcessMarkStack(uint32_t threadId) override;
    inline SlotStatus MarkObject(uint32_t threadId, TaggedObject *object, ObjectSlot slot) override;
    inline SlotStatus EvacuateObject(uint32_t threadId, TaggedObject *object, const MarkWord &markWord,
                                     ObjectSlot slot) override;
    inline void RecordWeakReference(uint32_t threadId, JSTaggedType *ref, Region *objectRegion = nullptr) override;

private:
    inline bool ShouldBePromoted(TaggedObject *object);

    uintptr_t waterLine_ {0};
};

class CompressGCMarker : public MovableMarker {
public:
    explicit CompressGCMarker(Heap *heap) : MovableMarker(heap) {}
    ~CompressGCMarker() = default;

    inline bool NeedEvacuate(Region *region);
    void SetAppSpawn(bool flag)
    {
        isAppSpawn_ = flag;
    }

protected:
    void ProcessMarkStack(uint32_t threadId) override;
    inline SlotStatus MarkObject(uint32_t threadId, TaggedObject *object, ObjectSlot slot) override;

    inline SlotStatus EvacuateObject(uint32_t threadId, TaggedObject *object, const MarkWord &markWord,
                                     ObjectSlot slot) override;
    uintptr_t AllocateForwardAddress(uint32_t threadId, size_t size, JSHClass *hclass, TaggedObject *object);
    inline uintptr_t AllocateAppSpawnSpace(size_t size);
    inline uintptr_t AllocateReadOnlySpace(size_t size);
    inline void RecordWeakReference(uint32_t threadId, JSTaggedType *ref, Region *objectRegion = nullptr) override;

private:
    bool isAppSpawn_ {false};
    os::memory::Mutex mutex_;
};
}  // namespace panda::ecmascript
#endif  // ECMASCRIPT_MEM_PARALLEL_MARKER_H
