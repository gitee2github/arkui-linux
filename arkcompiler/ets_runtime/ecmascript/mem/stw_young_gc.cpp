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

#include "ecmascript/mem/stw_young_gc.h"

#include "ecmascript/ecma_vm.h"
#include "ecmascript/mem/clock_scope.h"
#include "ecmascript/mem/concurrent_marker.h"
#include "ecmascript/mem/heap-inl.h"
#include "ecmascript/mem/mark_stack.h"
#include "ecmascript/mem/mem.h"
#include "ecmascript/mem/parallel_marker-inl.h"
#include "ecmascript/mem/space-inl.h"
#include "ecmascript/mem/tlab_allocator-inl.h"
#include "ecmascript/mem/visitor.h"
#include "ecmascript/mem/gc_stats.h"
#include "ecmascript/ecma_string_table.h"
#include "ecmascript/runtime_call_id.h"

namespace panda::ecmascript {
STWYoungGC::STWYoungGC(Heap *heap, bool parallelGC)
    : heap_(heap), parallelGC_(parallelGC), workManager_(heap->GetWorkManager())
{
}

void STWYoungGC::RunPhases()
{
    MEM_ALLOCATE_AND_GC_TRACE(heap_->GetEcmaVM(), STWYoungGC_RunPhases);
    [[maybe_unused]] ClockScope clockScope;

    ECMA_BYTRACE_NAME(HITRACE_TAG_ARK, "STWYoungGC::RunPhases");
    if (heap_->CheckOngoingConcurrentMarking()) {
        LOG_GC(DEBUG) << "STWYoungGC after ConcurrentMarking";
        heap_->GetConcurrentMarker()->Reset();  // HPPGC use mark result to move TaggedObject.
    }
    Initialize();
    Mark();
    Sweep();
    Finish();
    heap_->GetEcmaVM()->GetEcmaGCStats()->StatisticSTWYoungGC(clockScope.GetPauseTime(), semiCopiedSize_,
                                                              promotedSize_, commitSize_);
    LOG_GC(DEBUG) << "STWYoungGC::RunPhases " << clockScope.TotalSpentTime();
}

void STWYoungGC::Initialize()
{
    ECMA_BYTRACE_NAME(HITRACE_TAG_ARK, "STWYoungGC::Initialize");
    heap_->Prepare();
    commitSize_ = heap_->GetNewSpace()->GetCommittedSize();
    heap_->SwapNewSpace();
    workManager_->Initialize(TriggerGCType::YOUNG_GC, ParallelGCTaskPhase::SEMI_HANDLE_GLOBAL_POOL_TASK);
    heap_->GetSemiGCMarker()->Initialize();
    promotedSize_ = 0;
    semiCopiedSize_ = 0;
}

void STWYoungGC::Mark()
{
    ECMA_BYTRACE_NAME(HITRACE_TAG_ARK, "STWYoungGC::Mark");
    auto region = heap_->GetOldSpace()->GetCurrentRegion();

    if (parallelGC_) {
        heap_->PostParallelGCTask(ParallelGCTaskPhase::SEMI_HANDLE_THREAD_ROOTS_TASK);
        heap_->PostParallelGCTask(ParallelGCTaskPhase::SEMI_HANDLE_SNAPSHOT_TASK);
        heap_->GetSemiGCMarker()->ProcessOldToNew(0, region);
    } else {
        heap_->GetSemiGCMarker()->ProcessOldToNew(0, region);
        heap_->GetSemiGCMarker()->ProcessSnapshotRSet(MAIN_THREAD_INDEX);
        heap_->GetSemiGCMarker()->MarkRoots(MAIN_THREAD_INDEX);
        heap_->GetSemiGCMarker()->ProcessMarkStack(MAIN_THREAD_INDEX);
    }
    heap_->WaitRunningTaskFinished();

    auto totalThreadCount = Taskpool::GetCurrentTaskpool()->GetTotalThreadNum() + 1;  // gc thread and main thread
    for (uint32_t i = 0; i < totalThreadCount; i++) {
        SlotNeedUpdate needUpdate(nullptr, ObjectSlot(0));
        while (workManager_->GetSlotNeedUpdate(i, &needUpdate)) {
            UpdatePromotedSlot(needUpdate.first, needUpdate.second);
        }
    }
}

void STWYoungGC::Sweep()
{
    ECMA_BYTRACE_NAME(HITRACE_TAG_ARK, "STWYoungGC::Sweep");
    auto totalThreadCount = static_cast<uint32_t>(
        Taskpool::GetCurrentTaskpool()->GetTotalThreadNum() + 1);  // gc thread and main thread
    for (uint32_t i = 0; i < totalThreadCount; i++) {
        ProcessQueue *queue = workManager_->GetWeakReferenceQueue(i);
        while (true) {
            auto obj = queue->PopBack();
            if (UNLIKELY(obj == nullptr)) {
                break;
            }
            ObjectSlot slot(ToUintPtr(obj));
            JSTaggedValue value(slot.GetTaggedType());
            auto header = value.GetTaggedWeakRef();
            MarkWord markWord(header);
            if (markWord.IsForwardingAddress()) {
                TaggedObject *dst = markWord.ToForwardingAddress();
                auto weakRef = JSTaggedValue(JSTaggedValue(dst).CreateAndGetWeakRef()).GetRawTaggedObject();
                slot.Update(weakRef);
            } else {
                slot.Clear();
            }
        }
    }

    auto stringTable = heap_->GetEcmaVM()->GetEcmaStringTable();
    WeakRootVisitor gcUpdateWeak = [](TaggedObject *header) {
        Region *objectRegion = Region::ObjectAddressToRange(reinterpret_cast<TaggedObject *>(header));
        if (!objectRegion->InYoungSpace()) {
            return header;
        }

        MarkWord markWord(header);
        if (markWord.IsForwardingAddress()) {
            TaggedObject *dst = markWord.ToForwardingAddress();
            return dst;
        }
        return reinterpret_cast<TaggedObject *>(ToUintPtr(nullptr));
    };
    stringTable->SweepWeakReference(gcUpdateWeak);
    heap_->GetEcmaVM()->GetJSThread()->IterateWeakEcmaGlobalStorage(gcUpdateWeak);
    heap_->GetEcmaVM()->ProcessReferences(gcUpdateWeak);
}

void STWYoungGC::Finish()
{
    ECMA_BYTRACE_NAME(HITRACE_TAG_ARK, "STWYoungGC::Finish");
    workManager_->Finish(semiCopiedSize_, promotedSize_);
    heap_->Resume(YOUNG_GC);
}
}  // namespace panda::ecmascript
