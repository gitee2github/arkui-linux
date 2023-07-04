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

#ifndef ECMASCRIPT_MEM_HEAP_H
#define ECMASCRIPT_MEM_HEAP_H

#include "ecmascript/base/config.h"
#include "ecmascript/frames.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/mem/linear_space.h"
#include "ecmascript/mem/mark_stack.h"
#include "ecmascript/mem/sparse_space.h"
#include "ecmascript/mem/work_manager.h"
#include "ecmascript/taskpool/taskpool.h"

namespace panda::ecmascript {
class ConcurrentMarker;
class ConcurrentSweeper;
class EcmaVM;
class FullGC;
class HeapRegionAllocator;
class HeapTracker;
class Marker;
class MemController;
class NativeAreaAllocator;
class ParallelEvacuator;
class PartialGC;
class STWYoungGC;
class JSNativePointer;

enum class MarkType : uint8_t {
    MARK_YOUNG,
    MARK_FULL
};

enum class MemGrowingType : uint8_t {
    HIGH_THROUGHPUT,
    CONSERVATIVE,
    PRESSURE
};

enum class IdleHeapSizePtr : uint8_t {
    IDLE_HEAP_SIZE_1 = 0,
    IDLE_HEAP_SIZE_2,
    IDLE_HEAP_SIZE_3
};

struct IdleData {
    int64_t idleHeapObjectSize1 {0};
    int64_t idleHeapObjectSize2 {0};
    int64_t idleHeapObjectSize3 {0};
    IdleHeapSizePtr curPtr_ {IdleHeapSizePtr::IDLE_HEAP_SIZE_1};

    static constexpr int64_t REST_HEAP_GROWTH_LIMIT = 200_KB;
    bool CheckIsRest()
    {
        if (abs(idleHeapObjectSize1 - idleHeapObjectSize2) < REST_HEAP_GROWTH_LIMIT &&
            abs(idleHeapObjectSize2 - idleHeapObjectSize3) < REST_HEAP_GROWTH_LIMIT) {
            return true;
        }
        return false;
    }

    void SetNextValue(int64_t idleHeapObjectSize)
    {
        switch (curPtr_) {
            case IdleHeapSizePtr::IDLE_HEAP_SIZE_1:
                idleHeapObjectSize1 = idleHeapObjectSize;
                curPtr_ = IdleHeapSizePtr::IDLE_HEAP_SIZE_2;
                break;
            case IdleHeapSizePtr::IDLE_HEAP_SIZE_2:
                idleHeapObjectSize2 = idleHeapObjectSize;
                curPtr_ = IdleHeapSizePtr::IDLE_HEAP_SIZE_3;
                break;
            case IdleHeapSizePtr::IDLE_HEAP_SIZE_3:
                idleHeapObjectSize3 = idleHeapObjectSize;
                curPtr_ = IdleHeapSizePtr::IDLE_HEAP_SIZE_1;
                break;
            default:
                LOG_ECMA(FATAL) << "this branch is unreachable";
                UNREACHABLE();
        }
    }
};

enum class HeapMode {
    NORMAL,
    SPAWN,
    SHARE,
};

class Heap {
public:
    explicit Heap(EcmaVM *ecmaVm);
    ~Heap() = default;
    NO_COPY_SEMANTIC(Heap);
    NO_MOVE_SEMANTIC(Heap);
    void Initialize();
    void Destroy();
    void Prepare();
    void Resume(TriggerGCType gcType);
    void ResumeForAppSpawn();
    void CompactHeapBeforeFork();
    void DisableParallelGC();
    void EnableParallelGC();
    // fixme: Rename NewSpace to YoungSpace.
    // This is the active young generation space that the new objects are allocated in
    // or copied into (from the other semi space) during semi space GC.
    SemiSpace *GetNewSpace() const
    {
        return activeSemiSpace_;
    }

    /*
     * Return the original active space where the objects are to be evacuated during semi space GC.
     * This should be invoked only in the evacuation phase of semi space GC.
     * fixme: Get rid of this interface or make it safe considering the above implicit limitation / requirement.
     */
    SemiSpace *GetFromSpaceDuringEvacuation() const
    {
        return inactiveSemiSpace_;
    }

    OldSpace *GetOldSpace() const
    {
        return oldSpace_;
    }

    NonMovableSpace *GetNonMovableSpace() const
    {
        return nonMovableSpace_;
    }

    HugeObjectSpace *GetHugeObjectSpace() const
    {
        return hugeObjectSpace_;
    }

    MachineCodeSpace *GetMachineCodeSpace() const
    {
        return machineCodeSpace_;
    }

    SnapshotSpace *GetSnapshotSpace() const
    {
        return snapshotSpace_;
    }

    ReadOnlySpace *GetReadOnlySpace() const
    {
        return readOnlySpace_;
    }

    AppSpawnSpace *GetAppSpawnSpace() const
    {
        return appSpawnSpace_;
    }

    SparseSpace *GetSpaceWithType(MemSpaceType type) const
    {
        switch (type) {
            case MemSpaceType::OLD_SPACE:
                return oldSpace_;
                break;
            case MemSpaceType::NON_MOVABLE:
                return nonMovableSpace_;
                break;
            case MemSpaceType::MACHINE_CODE_SPACE:
                return machineCodeSpace_;
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    STWYoungGC *GetSTWYoungGC() const
    {
        return stwYoungGC_;
    }

    PartialGC *GetPartialGC() const
    {
        return partialGC_;
    }

    FullGC *GetFullGC() const
    {
        return fullGC_;
    }

    ConcurrentSweeper *GetSweeper() const
    {
        return sweeper_;
    }

    ParallelEvacuator *GetEvacuator() const
    {
        return evacuator_;
    }

    ConcurrentMarker *GetConcurrentMarker() const
    {
        return concurrentMarker_;
    }

    Marker *GetNonMovableMarker() const
    {
        return nonMovableMarker_;
    }

    Marker *GetSemiGCMarker() const
    {
        return semiGCMarker_;
    }

    Marker *GetCompressGCMarker() const
    {
        return compressGCMarker_;
    }

    EcmaVM *GetEcmaVM() const
    {
        return ecmaVm_;
    }

    JSThread *GetJSThread() const
    {
        return thread_;
    }

    WorkManager *GetWorkManager() const
    {
        return workManager_;
    }

    MemController *GetMemController() const
    {
        return memController_;
    }

    /*
     * For object allocations.
     */

    // Young
    inline TaggedObject *AllocateYoungOrHugeObject(JSHClass *hclass);
    inline TaggedObject *AllocateYoungOrHugeObject(JSHClass *hclass, size_t size);
    inline TaggedObject *AllocateReadOnlyOrHugeObject(JSHClass *hclass);
    inline TaggedObject *AllocateReadOnlyOrHugeObject(JSHClass *hclass, size_t size);
    inline TaggedObject *AllocateYoungOrHugeObject(size_t size);
    inline uintptr_t AllocateYoungSync(size_t size);
    inline TaggedObject *TryAllocateYoungGeneration(JSHClass *hclass, size_t size);
    // Old
    inline TaggedObject *AllocateOldOrHugeObject(JSHClass *hclass);
    inline TaggedObject *AllocateOldOrHugeObject(JSHClass *hclass, size_t size);
    // Non-movable
    inline TaggedObject *AllocateNonMovableOrHugeObject(JSHClass *hclass);
    inline TaggedObject *AllocateNonMovableOrHugeObject(JSHClass *hclass, size_t size);
    inline TaggedObject *AllocateClassClass(JSHClass *hclass, size_t size);
    // Huge
    inline TaggedObject *AllocateHugeObject(JSHClass *hclass, size_t size);
    inline TaggedObject *AllocateHugeObject(size_t size);
    // Machine code
    inline TaggedObject *AllocateMachineCodeObject(JSHClass *hclass, size_t size);
    // Snapshot
    inline uintptr_t AllocateSnapshotSpace(size_t size);

    NativeAreaAllocator *GetNativeAreaAllocator() const
    {
        return nativeAreaAllocator_;
    }

    HeapRegionAllocator *GetHeapRegionAllocator() const
    {
        return heapRegionAllocator_;
    }

    /*
     * GC triggers.
     */

    void CollectGarbage(TriggerGCType gcType);

    void CheckAndTriggerOldGC(size_t size = 0);

    /*
     * Parallel GC related configurations and utilities.
     */

    void PostParallelGCTask(ParallelGCTaskPhase taskPhase);

    bool IsParallelGCEnabled() const
    {
        return parallelGC_;
    }
    void ChangeGCParams(bool inBackground);
    void TriggerIdleCollection(int idleMicroSec);
    void NotifyMemoryPressure(bool inHighMemoryPressure);
    bool CheckCanDistributeTask();

    void WaitRunningTaskFinished();

    void TryTriggerConcurrentMarking();
    void AdjustBySurvivalRate(size_t originalNewSpaceSize);
    void TriggerConcurrentMarking();

    /*
     * Wait for existing concurrent marking tasks to be finished (if any).
     * Return true if there's ongoing concurrent marking.
     */
    bool CheckOngoingConcurrentMarking();

    /*
     * Functions invoked during GC.
     */

    void SetMarkType(MarkType markType)
    {
        markType_ = markType;
    }

    bool IsFullMark() const
    {
        return markType_ == MarkType::MARK_FULL;
    }

    inline void SwapNewSpace();
    inline void SwapOldSpace();

    inline bool MoveYoungRegionSync(Region *region);
    inline void MergeToOldSpaceSync(LocalSpace *localSpace);

    template<class Callback>
    void EnumerateOldSpaceRegions(const Callback &cb, Region *region = nullptr) const;

    template<class Callback>
    void EnumerateNonNewSpaceRegions(const Callback &cb) const;

    template<class Callback>
    void EnumerateNonNewSpaceRegionsWithRecord(const Callback &cb) const;

    template<class Callback>
    void EnumerateNewSpaceRegions(const Callback &cb) const;

    template<class Callback>
    void EnumerateSnapshotSpaceRegions(const Callback &cb) const;

    template<class Callback>
    void EnumerateNonMovableRegions(const Callback &cb) const;

    template<class Callback>
    inline void EnumerateRegions(const Callback &cb) const;

    inline void ClearSlotsRange(Region *current, uintptr_t freeStart, uintptr_t freeEnd);

    void WaitAllTasksFinished();
    void WaitConcurrentMarkingFinished();

    MemGrowingType GetMemGrowingType() const
    {
        return memGrowingtype_;
    }

    void SetMemGrowingType(MemGrowingType memGrowingType)
    {
        memGrowingtype_ = memGrowingType;
    }

    inline size_t GetCommittedSize() const;

    inline size_t GetHeapObjectSize() const;

    inline int32_t GetHeapObjectCount() const;

    size_t GetPromotedSize() const
    {
        return promotedSize_;
    }

    size_t GetArrayBufferSize() const;

    uint32_t GetMaxMarkTaskCount() const
    {
        return maxMarkTaskCount_;
    }

    uint32_t GetMaxEvacuateTaskCount() const
    {
        return maxEvacuateTaskCount_;
    }

    /*
     * Heap tracking will be used by tools like heap profiler etc.
     */

    void StartHeapTracking(HeapTracker *tracker)
    {
        WaitAllTasksFinished();
        parallelGC_ = false;
        tracker_ = tracker;
    }

    void StopHeapTracking()
    {
        WaitAllTasksFinished();
        parallelGC_ = true;
        tracker_ = nullptr;
    }

    inline void OnAllocateEvent(TaggedObject* address, size_t size);
    inline void OnMoveEvent(uintptr_t address, TaggedObject* forwardAddress, size_t size);
    void AddToKeptObjects(JSHandle<JSTaggedValue> value) const;
    void ClearKeptObjects() const;
    /*
     * Funtions used by heap verification.
     */

    template<class Callback>
    void IterateOverObjects(const Callback &cb) const;

    bool IsAlive(TaggedObject *object) const;
    bool ContainObject(TaggedObject *object) const;

    size_t VerifyHeapObjects() const;
    size_t VerifyOldToNewRSet() const;
    void StatisticHeapObject(TriggerGCType gcType) const;
    void PrintHeapInfo(TriggerGCType gcType) const;

    bool OldSpaceExceedCapacity(size_t size) const
    {
        size_t totalSize = oldSpace_->GetCommittedSize() + hugeObjectSpace_->GetCommittedSize() + size;
        return totalSize >= oldSpace_->GetMaximumCapacity() + oldSpace_->GetOutOfMemoryOvershootSize();
    }

    bool OldSpaceExceedLimit() const
    {
        size_t totalSize = oldSpace_->GetHeapObjectSize() + hugeObjectSpace_->GetHeapObjectSize();
        return totalSize >= oldSpace_->GetInitialCapacity();
    }

    void AdjustSpaceSizeForAppSpawn();
#if ECMASCRIPT_ENABLE_HEAP_VERIFY
    bool IsVerifying() const
    {
        return isVerifying_;
    }
#endif
    static bool ShouldMoveToRoSpace(JSHClass *hclass, TaggedObject *object)
    {
        return hclass->IsString() && !Region::ObjectAddressToRange(object)->InHugeObjectSpace();
    }

    bool IsFullMarkRequested() const
    {
        return fullMarkRequested_;
    }

    void SetFullMarkRequestedState(bool fullMarkRequested)
    {
        fullMarkRequested_ = fullMarkRequested;
    }

    void ShouldThrowOOMError(bool shouldThrow)
    {
        shouldThrowOOMError_ = shouldThrow;
    }

    void SetHeapMode(HeapMode mode)
    {
        mode_ = mode;
    }

    void ThrowOutOfMemoryError(size_t size, std::string functionName);

    void IncreaseNativeBindingSize(bool nonMovable, size_t size);
    void IncreaseNativeBindingSize(JSNativePointer *object);
    void ResetNativeBindingSize()
    {
        activeSemiSpace_->ResetNativeBindingSize();
        nonNewSpaceNativeBindingSize_ = 0;
    }

    size_t GetNativeBindingSize() const
    {
        return activeSemiSpace_->GetNativeBindingSize() + nonNewSpaceNativeBindingSize_;
    }

    size_t GetNonNewSpaceNativeBindingSize() const
    {
        return nonNewSpaceNativeBindingSize_;
    }
private:
    static constexpr int64_t WAIT_FOR_APP_START_UP = 200;
    static constexpr int IDLE_TIME_REMARK = 10;
    static constexpr int IDLE_TIME_LIMIT = 15;  // if idle time over 15ms we can do something
    static constexpr int MIN_OLD_GC_LIMIT = 10000;  // 10s
    static constexpr int REST_HEAP_GROWTH_LIMIT = 2_MB;
    void FatalOutOfMemoryError(size_t size, std::string functionName);
    void RecomputeLimits();
    void AdjustOldSpaceLimit();
    // record lastRegion for each space, which will be used in ReclaimRegions()
    void PrepareRecordRegionsForReclaim();
    TriggerGCType SelectGCType() const;
    void IncreaseTaskCount();
    void ReduceTaskCount();
    void WaitClearTaskFinished();
    void InvokeWeakNodeSecondPassCallback();
    inline void ReclaimRegions(TriggerGCType gcType);

    class ParallelGCTask : public Task {
    public:
        ParallelGCTask(int32_t id, Heap *heap, ParallelGCTaskPhase taskPhase)
            : Task(id), heap_(heap), taskPhase_(taskPhase) {};
        ~ParallelGCTask() override = default;
        bool Run(uint32_t threadIndex) override;

        NO_COPY_SEMANTIC(ParallelGCTask);
        NO_MOVE_SEMANTIC(ParallelGCTask);

    private:
        Heap *heap_ {nullptr};
        ParallelGCTaskPhase taskPhase_;
    };

    class AsyncClearTask : public Task {
    public:
        AsyncClearTask(int32_t id, Heap *heap, TriggerGCType type)
            : Task(id), heap_(heap), gcType_(type) {}
        ~AsyncClearTask() override = default;
        bool Run(uint32_t threadIndex) override;

        NO_COPY_SEMANTIC(AsyncClearTask);
        NO_MOVE_SEMANTIC(AsyncClearTask);
    private:
        Heap *heap_;
        TriggerGCType gcType_;
    };

    EcmaVM *ecmaVm_ {nullptr};
    JSThread *thread_ {nullptr};

    /*
     * Heap spaces.
     */

    /*
     * Young generation spaces where most new objects are allocated.
     * (only one of the spaces is active at a time in semi space GC).
     */
    SemiSpace *activeSemiSpace_ {nullptr};
    SemiSpace *inactiveSemiSpace_ {nullptr};

    // Old generation spaces where some long living objects are allocated or promoted.
    OldSpace *oldSpace_ {nullptr};
    OldSpace *compressSpace_ {nullptr};
    ReadOnlySpace *readOnlySpace_ {nullptr};
    AppSpawnSpace *appSpawnSpace_ {nullptr};
    // Spaces used for special kinds of objects.
    NonMovableSpace *nonMovableSpace_ {nullptr};
    MachineCodeSpace *machineCodeSpace_ {nullptr};
    HugeObjectSpace *hugeObjectSpace_ {nullptr};
    SnapshotSpace *snapshotSpace_ {nullptr};

    /*
     * Garbage collectors collecting garbage in different scopes.
     */

    /*
     * Semi sapce GC which collects garbage only in young spaces.
     * This is however optional for now because the partial GC also covers its functionality.
     */
    STWYoungGC *stwYoungGC_ {nullptr};

    /*
     * The mostly used partial GC which collects garbage in young spaces,
     * and part of old spaces if needed determined by GC heuristics.
     */
    PartialGC *partialGC_ {nullptr};

    // Full collector which collects garbage in all valid heap spaces.
    FullGC *fullGC_ {nullptr};

    // Concurrent marker which coordinates actions of GC markers and mutators.
    ConcurrentMarker *concurrentMarker_ {nullptr};

    // Concurrent sweeper which coordinates actions of sweepers (in spaces excluding young semi spaces) and mutators.
    ConcurrentSweeper *sweeper_ {nullptr};

    // Parallel evacuator which evacuates objects from one space to another one.
    ParallelEvacuator *evacuator_ {nullptr};

    /*
     * Different kinds of markers used by different collectors.
     * Depending on the collector algorithm, some markers can do simple marking
     *  while some others need to handle object movement.
     */
    Marker *nonMovableMarker_ {nullptr};
    Marker *semiGCMarker_ {nullptr};
    Marker *compressGCMarker_ {nullptr};

    // Work manager managing the tasks mostly generated in the GC mark phase.
    WorkManager *workManager_ {nullptr};

    MarkType markType_ {MarkType::MARK_YOUNG};

    bool parallelGC_ {true};
    bool fullGCRequested_ {false};
    bool fullMarkRequested_ {false};
    bool oldSpaceLimitAdjusted_ {false};
    bool shouldThrowOOMError_ {false};
    bool runningSecondPassCallbacks_ {false};
    bool enableIdleGC_ {true};
    bool waitForStartUp_ {true};
    bool couldIdleGC_ {false};
    HeapMode mode_ { HeapMode::NORMAL };

    size_t globalSpaceAllocLimit_ {0};
    size_t promotedSize_ {0};
    size_t semiSpaceCopiedSize_ {0};
    size_t nonNewSpaceNativeBindingSize_{0};
    size_t globalSpaceNativeLimit_ {0};
    size_t idleHeapObjectSize_ {0};
    size_t idleOldSpace_ {16_MB};
    size_t triggerRestIdleSize_ {0};
    MemGrowingType memGrowingtype_ {MemGrowingType::HIGH_THROUGHPUT};

    bool clearTaskFinished_ {true};
    os::memory::Mutex waitClearTaskFinishedMutex_;
    os::memory::ConditionVariable waitClearTaskFinishedCV_;
    int64_t idleTime_ {0};
    uint32_t runningTaskCount_ {0};
    // parallel marker task number.
    uint32_t maxMarkTaskCount_ {0};
    // parallel evacuator task number.
    uint32_t maxEvacuateTaskCount_ {0};
    os::memory::Mutex waitTaskFinishedMutex_;
    os::memory::ConditionVariable waitTaskFinishedCV_;

    /*
     * The memory controller providing memory statistics (by allocations and coleections),
     * which is used for GC heuristics.
     */
    MemController *memController_ {nullptr};

    // Region allocators.
    NativeAreaAllocator *nativeAreaAllocator_ {nullptr};
    HeapRegionAllocator *heapRegionAllocator_ {nullptr};

    // The tracker tracking heap object allocation and movement events.
    HeapTracker *tracker_ {nullptr};

    IdleData *idleData_;

#if ECMASCRIPT_ENABLE_HEAP_VERIFY
    bool isVerifying_ {false};
#endif
};
}  // namespace panda::ecmascript

#endif  // ECMASCRIPT_MEM_HEAP_H
