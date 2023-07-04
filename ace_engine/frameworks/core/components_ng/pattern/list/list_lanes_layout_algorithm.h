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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_LANES_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_LANES_LAYOUT_ALGORITHM_H

#include "core/components_ng/pattern/list/list_layout_algorithm.h"

namespace OHOS::Ace::NG {

// TextLayoutAlgorithm acts as the underlying text layout.
class ACE_EXPORT ListLanesLayoutAlgorithm : public ListLayoutAlgorithm {
    DECLARE_ACE_TYPE(ListLanesLayoutAlgorithm, ListLayoutAlgorithm);
public:
    void SwapLanesItemRange(std::map<int32_t, int32_t>& itemRange)
    {
        lanesItemRange_.swap(itemRange);
    }
    void SetLanes(int32_t lanes)
    {
        lanes_ = lanes;
    }
    int32_t GetLanes() const override
    {
        return lanes_;
    }

    static int32_t CalculateLanesParam(std::optional<float>& minLaneLength, std::optional<float>& maxLaneLength,
        int32_t lanes, std::optional<float> crossSizeOptional);

protected:
    void UpdateListItemConstraint(Axis axis, const OptionalSizeF& selfIdealSize,
        LayoutConstraintF& contentConstraint) override;
    int32_t LayoutALineForward(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
        Axis axis, int32_t& currentIndex, float startPos, float& endPos) override;
    int32_t LayoutALineBackward(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
        Axis axis, int32_t& currentIndex, float endPos, float& startPos) override;
    float CalculateLaneCrossOffset(float crossSize, float childCrossSize) override;
    void CalculateLanes(const RefPtr<ListLayoutProperty>& layoutProperty,
        const LayoutConstraintF& layoutConstraint, std::optional<float> crossSizeOptional, Axis axis) override;
    int32_t GetLanesFloor(LayoutWrapper* layoutWrapper, int32_t index) override;
    void SetCacheCount(LayoutWrapper* layoutWrapper, int32_t cachedCount) override;

private:
    static void ModifyLaneLength(
        std::optional<float>& minLaneLength, std::optional<float>& maxLaneLength, float crossSize);
    int32_t FindLanesStartIndex(LayoutWrapper* layoutWrapper, int32_t index);
    static int32_t FindLanesStartIndex(LayoutWrapper* layoutWrapper, int32_t startIndex, int32_t index);

    int32_t lanes_ = 1;
    bool lanesChanged_ = false;
    std::optional<float> minLaneLength_;
    std::optional<float> maxLaneLength_;
    std::map<int32_t, int32_t> lanesItemRange_;

    LayoutConstraintF groupLayoutConstraint_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_LAYOUT_ALGORITHM_H