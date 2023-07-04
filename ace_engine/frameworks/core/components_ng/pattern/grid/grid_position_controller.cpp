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

#include "core/components_ng/pattern/grid/grid_position_controller.h"

#include "base/geometry/axis.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/grid/grid_pattern.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {

void GridPositionController::JumpTo(int32_t index, int32_t /* source */)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_VOID(pattern);
    auto gridPattern = AceType::DynamicCast<GridPattern>(pattern);
    gridPattern->UpdateStartIndex(index);
}

bool GridPositionController::AnimateTo(const Dimension& position, float duration, const RefPtr<Curve>& curve)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_RETURN(pattern, false);
    auto gridPattern = AceType::DynamicCast<GridPattern>(pattern);
    return gridPattern->AnimateTo(position.ConvertToPx(), duration, curve);
}

void GridPositionController::ScrollBy(double /* pixelX */, double /* pixelY */, bool /* smooth */) {}

Axis GridPositionController::GetScrollDirection() const
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_RETURN(pattern, Axis::NONE);
    auto gridPattern = AceType::DynamicCast<GridPattern>(pattern);
    if (gridPattern) {
        return gridPattern->GetGridLayoutInfo().axis_;
    }
    return Axis::VERTICAL;
}

void GridPositionController::ScrollToEdge(ScrollEdgeType scrollEdgeType, bool /* smooth */)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_VOID(pattern);
    auto gridPattern = AceType::DynamicCast<GridPattern>(pattern);

    if ((gridPattern->GetGridLayoutInfo().axis_ == Axis::VERTICAL && scrollEdgeType == ScrollEdgeType::SCROLL_TOP) ||
        (gridPattern->GetGridLayoutInfo().axis_ == Axis::HORIZONTAL && scrollEdgeType == ScrollEdgeType::SCROLL_LEFT)) {
        gridPattern->UpdateStartIndex(0);
    } else if ((gridPattern->GetGridLayoutInfo().axis_ == Axis::VERTICAL &&
                   scrollEdgeType == ScrollEdgeType::SCROLL_BOTTOM) ||
               (gridPattern->GetGridLayoutInfo().axis_ == Axis::HORIZONTAL &&
                   scrollEdgeType == ScrollEdgeType::SCROLL_RIGHT)) {
        gridPattern->UpdateStartIndex(gridPattern->GetGridLayoutInfo().childrenCount_ - 1);
    }
}

void GridPositionController::ScrollPage(bool reverse, bool /* smooth */)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_VOID(pattern);
    auto gridPattern = AceType::DynamicCast<GridPattern>(pattern);
    if (gridPattern->GetGridLayoutInfo().axis_ != Axis::NONE) {
        gridPattern->ScrollPage(reverse);
    }
}

Offset GridPositionController::GetCurrentOffset() const
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_RETURN(pattern, Offset::Zero());
    auto gridPattern = AceType::DynamicCast<GridPattern>(pattern);
    auto axis = gridPattern->GetGridLayoutInfo().axis_;
    if (axis == Axis::NONE) {
        return Offset::Zero();
    }

    auto pxOffset = gridPattern->GetGridLayoutInfo().currentOffset_;
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, Offset::Zero());
    auto vpOffset = Dimension(pxOffset, DimensionUnit::PX).ConvertToVp();
    return (axis == Axis::HORIZONTAL) ? Offset(vpOffset, 0) : Offset(0, vpOffset);
}

} // namespace OHOS::Ace::NG
