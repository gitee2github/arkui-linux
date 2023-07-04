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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_LAYOUT_ALGORITHM_H

#include <map>
#include <optional>

#include "base/geometry/axis.h"
#include "base/memory/referenced.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/list/list_layout_property.h"
#include "core/components_v2/list/list_component.h"
#include "core/components_v2/list/list_properties.h"

namespace OHOS::Ace::NG {
class PipelineContext;

enum class ScrollIndexAlignment {
    ALIGN_TOP = 0,
    ALIGN_BOTTOM = 1,
};

struct ListItemInfo {
    float startPos;
    float endPos;
    bool isGroup;
};

// TextLayoutAlgorithm acts as the underlying text layout.
class ACE_EXPORT ListLayoutAlgorithm : public LayoutAlgorithm {
    DECLARE_ACE_TYPE(ListLayoutAlgorithm, LayoutAlgorithm);

public:
    using PositionMap = std::map<int32_t, ListItemInfo>;
    static const int32_t LAST_ITEM = -1;

    ListLayoutAlgorithm() = default;

    ~ListLayoutAlgorithm() override = default;

    void OnReset() override {}

    const PositionMap& GetItemPosition() const
    {
        return itemPosition_;
    }

    void SetItemsPosition(const PositionMap& itemPosition)
    {
        itemPosition_ = itemPosition;
    }

    void SetOverScrollFeature()
    {
        overScrollFeature_ = true;
    }

    void SetCanOverScroll(bool canOverScroll)
    {
        canOverScroll_ = canOverScroll;
    }

    void SetIndex(int32_t index)
    {
        jumpIndex_ = index;
    }

    void SetIndexAlignment(ScrollIndexAlignment align)
    {
        scrollIndexAlignment_ = align;
    }

    void SetCurrentDelta(float offset)
    {
        currentDelta_ = offset;
        currentOffset_ = offset;
    }

    float GetCurrentOffset() const
    {
        return currentOffset_;
    }

    float GetContentMainSize() const
    {
        return contentMainSize_;
    }

    void SetPrevContentMainSize(float mainSize)
    {
        prevContentMainSize_ = mainSize;
    }

    int32_t GetStartIndex() const
    {
        return itemPosition_.empty() ? 0 : itemPosition_.begin()->first;
    }

    int32_t GetEndIndex() const
    {
        return itemPosition_.empty() ? 0 : itemPosition_.rbegin()->first;
    }

    int32_t GetMaxListItemIndex() const
    {
        return totalItemCount_ - 1;
    }

    void SetSpaceWidth(float spaceWidth)
    {
        spaceWidth_ = spaceWidth;
    }

    float GetSpaceWidth() const
    {
        return spaceWidth_;
    }

    float GetEstimateOffset() const
    {
        return estimateOffset_;
    }

    float GetStartPosition() const
    {
        if (itemPosition_.empty()) {
            return 0.0f;
        }
        if (GetStartIndex() == 0) {
            return itemPosition_.begin()->second.startPos;
        }
        return itemPosition_.begin()->second.startPos - spaceWidth_;
    }

    float GetEndPosition() const
    {
        if (itemPosition_.empty()) {
            return 0.0f;
        }
        if (GetEndIndex() == totalItemCount_ - 1) {
            return itemPosition_.rbegin()->second.endPos;
        }
        return itemPosition_.rbegin()->second.endPos + spaceWidth_;
    }

    std::list<WeakPtr<FrameNode>>& GetItemGroupList()
    {
        return itemGroupList_;
    }

    void SetChainOffsetCallback(std::function<float(int32_t)> func)
    {
        chainOffsetFunc_ = std::move(func);
    }

    void SetChainInterval(float interval)
    {
        chainInterval_ = interval;
    }

    float GetChildMaxCrossSize(LayoutWrapper* layoutWrapper, Axis axis) const;

    void Measure(LayoutWrapper* layoutWrapper) override;

    void Layout(LayoutWrapper* layoutWrapper) override;

    void LayoutForward(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, Axis axis,
        int32_t startIndex, float startPos);
    void LayoutBackward(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, Axis axis,
        int32_t endIndex, float endPos);

    virtual int32_t GetLanes() const
    {
        return 1;
    }

protected:
    virtual void UpdateListItemConstraint(
        Axis axis, const OptionalSizeF& selfIdealSize, LayoutConstraintF& contentConstraint);
    virtual int32_t LayoutALineForward(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
        Axis axis, int32_t& currentIndex, float startPos, float& endPos);
    virtual int32_t LayoutALineBackward(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
        Axis axis, int32_t& currentIndex, float endPos, float& startPos);
    virtual float CalculateLaneCrossOffset(float crossSize, float childCrossSize);
    virtual void CalculateLanes(const RefPtr<ListLayoutProperty>& layoutProperty,
        const LayoutConstraintF& layoutConstraint, std::optional<float> crossSizeOptional, Axis axis) {};
    virtual int32_t GetLanesFloor(LayoutWrapper* layoutWrapper, int32_t index)
    {
        return index;
    }

    virtual void SetCacheCount(LayoutWrapper* layoutWrapper, int32_t cachedCount);
    void SetListItemGroupParam(const RefPtr<LayoutWrapper>& layoutWrapper, float referencePos, bool forwardLayout,
        const RefPtr<ListLayoutProperty>& layoutProperty) const;
    static void SetListItemIndex(const RefPtr<LayoutWrapper>& layoutWrapper, int32_t index);
    void CheckListItemGroupRecycle(
        LayoutWrapper* layoutWrapper, int32_t index, float referencePos, bool forwardLayout) const;
    void AdjustPostionForListItemGroup(LayoutWrapper* layoutWrapper, Axis axis, int32_t index);
    void SetItemInfo(int32_t index, ListItemInfo&& info)
    {
        itemPosition_[index] = info;
    }

private:
    void MeasureList(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, Axis axis);

    void CalculateEstimateOffset();

    std::pair<int32_t, float> LayoutOrRecycleCachedItems(
        LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, Axis axis);

    std::pair<int32_t, float> RequestNewItemsForward(LayoutWrapper* layoutWrapper,
        const LayoutConstraintF& layoutConstraint, int32_t startIndex, float startPos, Axis axis);

    std::pair<int32_t, float> RequestNewItemsBackward(LayoutWrapper* layoutWrapper,
        const LayoutConstraintF& layoutConstraint, int32_t startIndex, float startPos, Axis axis);

    void CreateItemGroupList(LayoutWrapper* layoutWrapper);
    void OnSurfaceChanged(LayoutWrapper* layoutWrapper);

    std::optional<int32_t> jumpIndex_;
    ScrollIndexAlignment scrollIndexAlignment_ = ScrollIndexAlignment::ALIGN_TOP;

    PositionMap itemPosition_;
    float currentOffset_ = 0.0f;
    float currentDelta_ = 0.0f;
    float startMainPos_ = 0.0f;
    float endMainPos_ = 0.0f;

    float spaceWidth_ = 0.0f;
    bool overScrollFeature_ = false;
    bool canOverScroll_ = false;

    int32_t totalItemCount_ = 0;

    V2::ListItemAlign listItemAlign_ = V2::ListItemAlign::START;

    float estimateOffset_ = 0.0f;

    bool mainSizeIsDefined_ = false;
    float contentMainSize_ = 0.0f;
    float prevContentMainSize_ = 0.0f;
    float paddingBeforeContent_ = 0.0f;
    float paddingAfterContent_ = 0.0f;

    V2::StickyStyle stickyStyle_ = V2::StickyStyle::NONE;
    std::list<WeakPtr<FrameNode>> itemGroupList_;

    std::function<float(int32_t)> chainOffsetFunc_;
    float chainInterval_ = 0.0f;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_LAYOUT_ALGORITHM_H
