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

#include "ecmascript/mem/linear_space.h"

#include "ecmascript/free_object.h"
#include "ecmascript/js_hclass-inl.h"
#include "ecmascript/mem/allocator-inl.h"
#include "ecmascript/mem/heap.h"

namespace panda::ecmascript {
LinearSpace::LinearSpace(Heap *heap, MemSpaceType type, size_t initialCapacity, size_t maximumCapacity)
    : Space(heap->GetHeapRegionAllocator(), type, initialCapacity, maximumCapacity),
      heap_(heap),
      waterLine_(0)
{
}

uintptr_t LinearSpace::Allocate(size_t size, bool isPromoted)
{
    auto object = allocator_.Allocate(size);
    if (object != 0) {
        return object;
    }
    if (Expand(isPromoted)) {
        if (!isPromoted) {
            heap_->TryTriggerConcurrentMarking();
        }
        object = allocator_.Allocate(size);
    } else if (heap_->GetJSThread()->IsMarking()) {
        // Temporary adjust semi space capacity
        overShootSize_ = heap_->GetEcmaVM()->GetEcmaParamConfiguration().GetSemiSpaceOvershootSize();
        if (Expand(isPromoted)) {
            object = allocator_.Allocate(size);
        }
    }
    return object;
}

bool LinearSpace::Expand(bool isPromoted)
{
    if (committedSize_ >= initialCapacity_ + overShootSize_ + outOfMemoryOvershootSize_) {
        return false;
    }

    uintptr_t top = allocator_.GetTop();
    auto currentRegion = GetCurrentRegion();
    if (currentRegion != nullptr) {
        if (!isPromoted) {
            if (currentRegion->HasAgeMark()) {
                allocateAfterLastGC_ +=
                    currentRegion->GetAllocatedBytes(top) - currentRegion->GetAllocatedBytes(waterLine_);
            } else {
                allocateAfterLastGC_ += currentRegion->GetAllocatedBytes(top);
            }
        } else {
            // For GC
            survivalObjectSize_ += currentRegion->GetAllocatedBytes(top);
        }
        currentRegion->SetHighWaterMark(top);
    }
    Region *region = heapRegionAllocator_->AllocateAlignedRegion(this, DEFAULT_REGION_SIZE, heap_->GetJSThread());
    allocator_.Reset(region->GetBegin(), region->GetEnd());

    AddRegion(region);
    return true;
}

void LinearSpace::Stop()
{
    if (GetCurrentRegion() != nullptr) {
        GetCurrentRegion()->SetHighWaterMark(allocator_.GetTop());
    }
}

void LinearSpace::ResetAllocator()
{
    auto currentRegion = GetCurrentRegion();
    if (currentRegion != nullptr) {
        allocator_.Reset(currentRegion->GetBegin(), currentRegion->GetEnd(), currentRegion->GetHighWaterMark());
    }
}

void LinearSpace::IterateOverObjects(const std::function<void(TaggedObject *object)> &visitor) const
{
    auto current = GetCurrentRegion();
    EnumerateRegions([&](Region *region) {
        auto curPtr = region->GetBegin();
        uintptr_t endPtr = 0;
        if (region == current) {
            auto top = allocator_.GetTop();
            endPtr = curPtr + region->GetAllocatedBytes(top);
        } else {
            endPtr = curPtr + region->GetAllocatedBytes();
        }

        size_t objSize;
        while (curPtr < endPtr) {
            auto freeObject = FreeObject::Cast(curPtr);
            // If curPtr is freeObject, It must to mark unpoison first.
            ASAN_UNPOISON_MEMORY_REGION(freeObject, TaggedObject::TaggedObjectSize());
            if (!freeObject->IsFreeObject()) {
                auto obj = reinterpret_cast<TaggedObject *>(curPtr);
                visitor(obj);
                objSize = obj->GetClass()->SizeFromJSHClass(obj);
            } else {
                freeObject->AsanUnPoisonFreeObject();
                objSize = freeObject->Available();
                freeObject->AsanPoisonFreeObject();
            }
            curPtr += objSize;
            CHECK_OBJECT_SIZE(objSize);
        }
        CHECK_REGION_END(curPtr, endPtr);
    });
}

SemiSpace::SemiSpace(Heap *heap, size_t initialCapacity, size_t maximumCapacity)
    : LinearSpace(heap, MemSpaceType::SEMI_SPACE, initialCapacity, maximumCapacity),
      minimumCapacity_(initialCapacity), newSpaceNativeLimit_(initialCapacity) {}

void SemiSpace::Initialize()
{
    Region *region = heapRegionAllocator_->AllocateAlignedRegion(this, DEFAULT_REGION_SIZE, heap_->GetJSThread());
    AddRegion(region);
    allocator_.Reset(region->GetBegin(), region->GetEnd());
}

void SemiSpace::Restart()
{
    overShootSize_ = 0;
    survivalObjectSize_ = 0;
    allocateAfterLastGC_ = 0;
    Initialize();
}

uintptr_t SemiSpace::AllocateSync(size_t size)
{
    os::memory::LockHolder lock(lock_);
    return Allocate(size, true);
}

bool SemiSpace::SwapRegion(Region *region, SemiSpace *fromSpace)
{
    os::memory::LockHolder lock(lock_);
    if (committedSize_ + region->GetCapacity() > maximumCapacity_ + overShootSize_) {
        return false;
    }
    fromSpace->RemoveRegion(region);

    region->SetGCFlag(RegionGCFlags::IN_NEW_TO_NEW_SET);

    regionList_.AddNodeToFront(region);
    IncreaseCommitted(region->GetCapacity());
    IncreaseObjectSize(region->GetSize());
    survivalObjectSize_ += region->GetAllocatedBytes();
    return true;
}

void SemiSpace::SetWaterLine()
{
    waterLine_ = allocator_.GetTop();
    allocateAfterLastGC_ = 0;
    Region *last = GetCurrentRegion();
    if (last != nullptr) {
        last->SetGCFlag(RegionGCFlags::HAS_AGE_MARK);

        EnumerateRegions([&last](Region *current) {
            if (current != last) {
                current->SetGCFlag(RegionGCFlags::BELOW_AGE_MARK);
            }
        });
        survivalObjectSize_ += last->GetAllocatedBytes(waterLine_);
    }
}

size_t SemiSpace::GetHeapObjectSize() const
{
    return survivalObjectSize_ + allocateAfterLastGC_;
}

size_t SemiSpace::GetSurvivalObjectSize() const
{
    return survivalObjectSize_;
}

void SemiSpace::SetOverShootSize(size_t size)
{
    overShootSize_ = size;
}

bool SemiSpace::AdjustCapacity(size_t allocatedSizeSinceGC)
{
    if (allocatedSizeSinceGC <= initialCapacity_ * GROW_OBJECT_SURVIVAL_RATE / GROWING_FACTOR) {
        return false;
    }
    double curObjectSurvivalRate = static_cast<double>(survivalObjectSize_) / allocatedSizeSinceGC;
    if (curObjectSurvivalRate > GROW_OBJECT_SURVIVAL_RATE) {
        if (initialCapacity_ >= maximumCapacity_) {
            return false;
        }
        size_t newCapacity = initialCapacity_ * GROWING_FACTOR;
        SetInitialCapacity(std::min(newCapacity, maximumCapacity_));
        return true;
    } else if (curObjectSurvivalRate < SHRINK_OBJECT_SURVIVAL_RATE) {
        if (initialCapacity_ <= minimumCapacity_) {
            return false;
        }
        size_t newCapacity = initialCapacity_ / GROWING_FACTOR;
        SetInitialCapacity(std::max(newCapacity, minimumCapacity_));
        return true;
    }
    return false;
}

void SemiSpace::AdjustNativeLimit(size_t previousNativeSize)
{
    if (newSpaceNativeBindingSize_ <= newSpaceNativeLimit_ * GROW_OBJECT_SURVIVAL_RATE / GROWING_FACTOR) {
        return;
    }
    double curObjectSurvivalRate = static_cast<double>(previousNativeSize) / newSpaceNativeBindingSize_;
    if (curObjectSurvivalRate > GROW_OBJECT_SURVIVAL_RATE) {
        if (newSpaceNativeLimit_ >= maximumCapacity_) {
            return;
        }
        size_t newCapacity = newSpaceNativeLimit_ * GROWING_FACTOR;
        newSpaceNativeLimit_ = std::min(newCapacity, maximumCapacity_);
    } else if (curObjectSurvivalRate < SHRINK_OBJECT_SURVIVAL_RATE) {
        if (newSpaceNativeLimit_ <= minimumCapacity_) {
            return;
        }
        size_t newCapacity = newSpaceNativeLimit_ / GROWING_FACTOR;
        newSpaceNativeLimit_ = std::max(newCapacity, minimumCapacity_);
    }
}

size_t SemiSpace::GetAllocatedSizeSinceGC(uintptr_t top) const
{
    size_t currentRegionSize = 0;
    auto currentRegion = GetCurrentRegion();
    if (currentRegion != nullptr) {
        currentRegionSize = currentRegion->GetAllocatedBytes(top);
        if (currentRegion->HasAgeMark()) {
            currentRegionSize -= currentRegion->GetAllocatedBytes(waterLine_);
        }
    }
    return allocateAfterLastGC_ + currentRegionSize;
}

SnapshotSpace::SnapshotSpace(Heap *heap, size_t initialCapacity, size_t maximumCapacity)
    : LinearSpace(heap, MemSpaceType::SNAPSHOT_SPACE, initialCapacity, maximumCapacity) {}

ReadOnlySpace::ReadOnlySpace(Heap *heap, size_t initialCapacity, size_t maximumCapacity)
    : LinearSpace(heap, MemSpaceType::READ_ONLY_SPACE, initialCapacity, maximumCapacity) {}
}  // namespace panda::ecmascript
