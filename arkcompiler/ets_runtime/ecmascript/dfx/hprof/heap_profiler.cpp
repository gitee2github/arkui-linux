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

#include "ecmascript/dfx/hprof/heap_profiler.h"

#include "ecmascript/dfx/hprof/heap_snapshot.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/mem/assert_scope.h"
#include "ecmascript/mem/concurrent_sweeper.h"
#include "ecmascript/mem/heap-inl.h"
#include "ecmascript/dfx/hprof/file_stream.h"

namespace panda::ecmascript {
HeapProfiler::HeapProfiler(const EcmaVM *vm) : vm_(vm), chunk_(vm->GetNativeAreaAllocator())
{
    jsonSerializer_ = GetChunk()->New<HeapSnapshotJSONSerializer>();
    if (UNLIKELY(jsonSerializer_ == nullptr)) {
        LOG_FULL(FATAL) << "alloc snapshot json serializer failed";
        UNREACHABLE();
    }
}
HeapProfiler::~HeapProfiler()
{
    ClearSnapshot();
    GetChunk()->Delete(jsonSerializer_);
    jsonSerializer_ = nullptr;
}

void HeapProfiler::UpdateHeapObjects(HeapSnapshot *snapshot)
{
    vm_->CollectGarbage(TriggerGCType::OLD_GC);
    vm_->GetHeap()->GetSweeper()->EnsureAllTaskFinished();
    snapshot->UpdateNodes();
}

bool HeapProfiler::DumpHeapSnapshot(DumpFormat dumpFormat, Stream *stream, Progress *progress,
                                    bool isVmMode, bool isPrivate)
{
    [[maybe_unused]] bool heapClean = ForceFullGC(vm_);
    ASSERT(heapClean);
    LOG_ECMA(INFO) << "HeapProfiler DumpSnapshot start";
    size_t heapSize = vm_->GetHeap()->GetHeapObjectSize();
    LOG_ECMA(ERROR) << "HeapProfiler DumpSnapshot heap size " << heapSize;
    int32_t heapCount = vm_->GetHeap()->GetHeapObjectCount();
    if (progress != nullptr) {
        progress->ReportProgress(0, heapCount);
    }
    HeapSnapshot *snapshot = MakeHeapSnapshot(SampleType::ONE_SHOT, isVmMode, isPrivate);
    if (progress != nullptr) {
        progress->ReportProgress(heapCount, heapCount);
    }
    ASSERT(snapshot != nullptr);
    if (!stream->Good()) {
        FileStream newStream(GenDumpFileName(dumpFormat));
        return jsonSerializer_->Serialize(snapshot, &newStream);
    }
    return jsonSerializer_->Serialize(snapshot, stream);
}

bool HeapProfiler::StartHeapTracking(double timeInterval, bool isVmMode, Stream *stream,
                                     bool traceAllocation, bool newThread)
{
    HeapSnapshot *snapshot = MakeHeapSnapshot(SampleType::REAL_TIME, isVmMode, false, traceAllocation);
    if (snapshot == nullptr) {
        return false;
    }

    UpdateHeapObjects(snapshot);
    heapTracker_ = std::make_unique<HeapTracker>(snapshot, timeInterval, stream);
    const_cast<EcmaVM *>(vm_)->StartHeapTracking(heapTracker_.get());

    if (newThread) {
        heapTracker_->StartTracing();
    }

    return true;
}

bool HeapProfiler::UpdateHeapTracking(Stream *stream)
{
    HeapSnapshot *snapshot = hprofs_.at(0);
    if (snapshot == nullptr) {
        return false;
    }

    snapshot->RecordSampleTime();
    UpdateHeapObjects(snapshot);

    if (stream != nullptr) {
        snapshot->PushHeapStat(stream);
    }

    return true;
}

bool HeapProfiler::StopHeapTracking(Stream *stream, Progress *progress, bool newThread)
{
    if (heapTracker_ == nullptr) {
        return false;
    }
    int32_t heapCount = vm_->GetHeap()->GetHeapObjectCount();

    const_cast<EcmaVM *>(vm_)->StopHeapTracking();
    if (newThread) {
        heapTracker_->StopTracing();
    }

    HeapSnapshot *snapshot = hprofs_.at(0);
    if (snapshot == nullptr) {
        return false;
    }

    if (progress != nullptr) {
        progress->ReportProgress(0, heapCount);
    }
    snapshot->FinishSnapshot();
    if (progress != nullptr) {
        progress->ReportProgress(heapCount, heapCount);
    }
    return jsonSerializer_->Serialize(snapshot, stream);
}

std::string HeapProfiler::GenDumpFileName(DumpFormat dumpFormat)
{
    CString filename("hprof_");
    switch (dumpFormat) {
        case DumpFormat::JSON:
            filename.append(GetTimeStamp());
            break;
        case DumpFormat::BINARY:
            filename.append("unimplemented");
            break;
        case DumpFormat::OTHER:
            filename.append("unimplemented");
            break;
        default:
            filename.append("unimplemented");
            break;
    }
    filename.append(".heapsnapshot");
    return CstringConvertToStdString(filename);
}

CString HeapProfiler::GetTimeStamp()
{
    std::time_t timeSource = std::time(nullptr);
    struct tm tm {
    };
    struct tm *timeData = localtime_r(&timeSource, &tm);
    if (timeData == nullptr) {
        LOG_FULL(FATAL) << "localtime_r failed";
        UNREACHABLE();
    }
    CString stamp;
    const int TIME_START = 1900;
    stamp.append(ToCString(timeData->tm_year + TIME_START))
        .append("-")
        .append(ToCString(timeData->tm_mon + 1))
        .append("-")
        .append(ToCString(timeData->tm_mday))
        .append("_")
        .append(ToCString(timeData->tm_hour))
        .append("-")
        .append(ToCString(timeData->tm_min))
        .append("-")
        .append(ToCString(timeData->tm_sec));
    return stamp;
}

bool HeapProfiler::ForceFullGC(const EcmaVM *vm)
{
    if (vm->IsInitialized()) {
        const_cast<Heap *>(vm->GetHeap())->CollectGarbage(TriggerGCType::FULL_GC);
        return true;
    }
    return false;
}

HeapSnapshot *HeapProfiler::MakeHeapSnapshot(SampleType sampleType, bool isVmMode, bool isPrivate, bool traceAllocation)
{
    LOG_ECMA(INFO) << "HeapProfiler::MakeHeapSnapshot";
    DISALLOW_GARBAGE_COLLECTION;
    const_cast<Heap *>(vm_->GetHeap())->Prepare();
    switch (sampleType) {
        case SampleType::ONE_SHOT: {
            auto *snapshot = GetChunk()->New<HeapSnapshot>(vm_, isVmMode, isPrivate, traceAllocation, GetChunk());
            if (snapshot == nullptr) {
                LOG_FULL(FATAL) << "alloc snapshot failed";
                UNREACHABLE();
            }
            snapshot->BuildUp();
            AddSnapshot(snapshot);
            return snapshot;
        }
        case SampleType::REAL_TIME: {
            auto *snapshot = GetChunk()->New<HeapSnapshot>(vm_, isVmMode, isPrivate, traceAllocation, GetChunk());
            if (snapshot == nullptr) {
                LOG_FULL(FATAL) << "alloc snapshot failed";
                UNREACHABLE();
            }
            AddSnapshot(snapshot);
            snapshot->PrepareSnapshot();
            return snapshot;
        }
        default:
            return nullptr;
    }
}

void HeapProfiler::AddSnapshot(HeapSnapshot *snapshot)
{
    if (hprofs_.size() >= MAX_NUM_HPROF) {
        ClearSnapshot();
    }
    ASSERT(snapshot != nullptr);
    hprofs_.emplace_back(snapshot);
}

void HeapProfiler::ClearSnapshot()
{
    for (auto *snapshot : hprofs_) {
        GetChunk()->Delete(snapshot);
    }
    hprofs_.clear();
}
}  // namespace panda::ecmascript
