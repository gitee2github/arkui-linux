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

#include "core/components_ng/pattern/side_bar/side_bar_container_layout_algorithm.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/measure_utils.h"

namespace OHOS::Ace::NG {

namespace {
constexpr int32_t DEFAULT_MIN_CHILDREN_SIZE = 3;
constexpr Dimension DEFAULT_CONTROL_BUTTON_WIDTH = 32.0_vp;
constexpr Dimension DEFAULT_CONTROL_BUTTON_HEIGHT = 32.0_vp;
constexpr Dimension DEFAULT_CONTROL_BUTTON_LEFT = 16.0_vp;
constexpr Dimension DEFAULT_CONTROL_BUTTON_TOP = 48.0_vp;
constexpr Dimension DEFAULT_SIDE_BAR_WIDTH = 200.0_vp;
constexpr Dimension DEFAULT_MIN_SIDE_BAR_WIDTH = 200.0_vp;
constexpr Dimension DEFAULT_MAX_SIDE_BAR_WIDTH = 280.0_vp;
} // namespace

void SideBarContainerLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    const auto& children = layoutWrapper->GetAllChildrenWithBuild();
    if (children.empty() || children.size() < DEFAULT_MIN_CHILDREN_SIZE) {
        LOGE("SideBarContainerLayoutAlgorithm::Measure, children is empty or less than 3.");
        return;
    }

    auto layoutProperty = AceType::DynamicCast<SideBarContainerLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    auto constraint = layoutProperty->GetLayoutConstraint();
    auto idealSize = CreateIdealSize(
        constraint.value(), Axis::HORIZONTAL, layoutProperty->GetMeasureType(MeasureType::MATCH_PARENT), true);
    layoutWrapper->GetGeometryNode()->SetFrameSize(idealSize);

    auto parentWidth = idealSize.Width();
    if (needInitRealSideBarWidth_) {
        InitRealSideBarWidth(layoutWrapper, parentWidth);
    }

    auto begin = children.begin();
    auto sideBarLayoutWrapper = *(++begin);
    MeasureSideBar(layoutProperty, sideBarLayoutWrapper);

    auto contentLayoutWrapper = children.front();
    MeasureSideBarContent(layoutProperty, contentLayoutWrapper, parentWidth);

    auto imgLayoutWrapper = children.back();
    MeasureControlButton(layoutProperty, imgLayoutWrapper, parentWidth);
}

void SideBarContainerLayoutAlgorithm::InitRealSideBarWidth(LayoutWrapper* layoutWrapper, float parentWidth)
{
    auto layoutProperty = AceType::DynamicCast<SideBarContainerLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    auto constraint = layoutProperty->GetLayoutConstraint();
    auto scaleProperty = constraint->scaleProperty;
    auto sideBarWidth = layoutProperty->GetSideBarWidth().value_or(DEFAULT_SIDE_BAR_WIDTH);
    auto minSideBarWidth = layoutProperty->GetMinSideBarWidth().value_or(DEFAULT_MIN_SIDE_BAR_WIDTH);
    auto maxSideBarWidth = layoutProperty->GetMaxSideBarWidth().value_or(DEFAULT_MAX_SIDE_BAR_WIDTH);

    auto sideBarWidthPx = ConvertToPx(sideBarWidth, scaleProperty, parentWidth).value_or(0);
    auto minSideBarWidthPx = ConvertToPx(minSideBarWidth, scaleProperty, parentWidth).value_or(0);
    auto maxSideBarWidthPx = ConvertToPx(maxSideBarWidth, scaleProperty, parentWidth).value_or(0);
    if (minSideBarWidthPx > maxSideBarWidthPx) {
        minSideBarWidthPx = ConvertToPx(DEFAULT_MIN_SIDE_BAR_WIDTH, scaleProperty, parentWidth).value_or(0);
        maxSideBarWidthPx = ConvertToPx(DEFAULT_MAX_SIDE_BAR_WIDTH, scaleProperty, parentWidth).value_or(0);
    }

    if (sideBarWidthPx <= minSideBarWidthPx) {
        realSideBarWidth_ = minSideBarWidthPx;
    } else if (sideBarWidthPx >= maxSideBarWidthPx) {
        realSideBarWidth_ = maxSideBarWidthPx;
    } else {
        realSideBarWidth_ = sideBarWidthPx;
    }
}

void SideBarContainerLayoutAlgorithm::MeasureSideBar(
    const RefPtr<SideBarContainerLayoutProperty>& layoutProperty, const RefPtr<LayoutWrapper>& sideBarLayoutWrapper)
{
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto sideBarIdealSize = CreateIdealSize(
        constraint.value(), Axis::HORIZONTAL, layoutProperty->GetMeasureType(MeasureType::MATCH_PARENT), true);
    sideBarIdealSize.SetWidth(realSideBarWidth_);
    auto sideBarConstraint = layoutProperty->CreateChildConstraint();
    sideBarConstraint.selfIdealSize = OptionalSizeF(sideBarIdealSize);

    realSideBarHeight_ = sideBarIdealSize.Height();
    sideBarLayoutWrapper->Measure(sideBarConstraint);
}

void SideBarContainerLayoutAlgorithm::MeasureSideBarContent(
    const RefPtr<SideBarContainerLayoutProperty>& layoutProperty, const RefPtr<LayoutWrapper>& contentLayoutWrapper,
    float parentWidth)
{
    auto type = layoutProperty->GetSideBarContainerType().value_or(SideBarContainerType::EMBED);
    auto sideBarPosition = layoutProperty->GetSideBarPosition().value_or(SideBarPosition::START);
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto contentWidth = parentWidth;

    if (type == SideBarContainerType::EMBED) {
        if (sideBarStatus_ == SideBarStatus::SHOW) {
            contentWidth -= realSideBarWidth_;
        } else if (sideBarStatus_ == SideBarStatus::CHANGING) {
            contentWidth = (sideBarPosition == SideBarPosition::START)
                               ? (parentWidth - realSideBarWidth_ - currentOffset_)
                               : (parentWidth + currentOffset_);
        }
    }

    auto contentIdealSize = CreateIdealSize(
        constraint.value(), Axis::HORIZONTAL, layoutProperty->GetMeasureType(MeasureType::MATCH_PARENT), true);
    contentIdealSize.SetWidth(contentWidth);
    auto contentConstraint = layoutProperty->CreateChildConstraint();
    contentConstraint.selfIdealSize = OptionalSizeF(contentIdealSize);
    contentLayoutWrapper->Measure(contentConstraint);
}

void SideBarContainerLayoutAlgorithm::MeasureControlButton(const RefPtr<SideBarContainerLayoutProperty>& layoutProperty,
    const RefPtr<LayoutWrapper>& buttonLayoutWrapper, float parentWidth)
{
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto scaleProperty = constraint->scaleProperty;

    auto controlButtonWidth = layoutProperty->GetControlButtonWidth().value_or(DEFAULT_CONTROL_BUTTON_WIDTH);
    auto controlButtonHeight = layoutProperty->GetControlButtonHeight().value_or(DEFAULT_CONTROL_BUTTON_HEIGHT);
    auto controlButtonWidthPx = ConvertToPx(controlButtonWidth, scaleProperty, parentWidth).value_or(0);
    auto controlButtonHeightPx = ConvertToPx(controlButtonHeight, scaleProperty, parentWidth).value_or(0);

    auto controlButtonLayoutConstraint = layoutProperty->CreateChildConstraint();
    controlButtonLayoutConstraint.selfIdealSize.SetWidth(controlButtonWidthPx);
    controlButtonLayoutConstraint.selfIdealSize.SetHeight(controlButtonHeightPx);
    buttonLayoutWrapper->Measure(controlButtonLayoutConstraint);
}

void SideBarContainerLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    const auto& children = layoutWrapper->GetAllChildrenWithBuild();
    if (children.empty() || children.size() < DEFAULT_MIN_CHILDREN_SIZE) {
        LOGE("SideBarContainerLayoutAlgorithm::Measure, children is empty or less than 3.");
        return;
    }

    auto controlButtonLayoutWrapper = children.back();
    auto begin = children.begin();
    auto sideBarLayoutWrapper = *(++begin);
    auto contentLayoutWrapper = children.front();

    LayoutControlButton(layoutWrapper, controlButtonLayoutWrapper);
    LayoutSideBar(layoutWrapper, sideBarLayoutWrapper);
    LayoutSideBarContent(layoutWrapper, contentLayoutWrapper);
}

void SideBarContainerLayoutAlgorithm::LayoutControlButton(
    LayoutWrapper* layoutWrapper, const RefPtr<LayoutWrapper>& buttonLayoutWrapper)
{
    auto layoutProperty = AceType::DynamicCast<SideBarContainerLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    CHECK_NULL_VOID (layoutWrapper->GetGeometryNode());
    auto parentWidth = layoutWrapper->GetGeometryNode()->GetFrameSize().Width();
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto scaleProperty = constraint->scaleProperty;

    auto controlButtonLeft = layoutProperty->GetControlButtonLeft().value_or(DEFAULT_CONTROL_BUTTON_LEFT);
    auto controlButtonTop = layoutProperty->GetControlButtonTop().value_or(DEFAULT_CONTROL_BUTTON_TOP);
    if (LessNotEqual(controlButtonLeft.Value(), 0.0)) {
        controlButtonLeft = DEFAULT_CONTROL_BUTTON_LEFT;
    }

    if (LessNotEqual(controlButtonTop.Value(), 0.0)) {
        controlButtonTop = DEFAULT_CONTROL_BUTTON_TOP;
    }

    auto controlButtonLeftPx = ConvertToPx(controlButtonLeft, scaleProperty, parentWidth).value_or(0);
    auto controlButtonTopPx = ConvertToPx(controlButtonTop, scaleProperty, parentWidth).value_or(0);

    auto imgOffset = OffsetF(controlButtonLeftPx, controlButtonTopPx);
    buttonLayoutWrapper->GetGeometryNode()->SetMarginFrameOffset(imgOffset);
    buttonLayoutWrapper->Layout();
}

void SideBarContainerLayoutAlgorithm::LayoutSideBar(
    LayoutWrapper* layoutWrapper, const RefPtr<LayoutWrapper>& sideBarLayoutWrapper)
{
    auto layoutProperty = AceType::DynamicCast<SideBarContainerLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    CHECK_NULL_VOID (layoutWrapper->GetGeometryNode());
    auto parentWidth = layoutWrapper->GetGeometryNode()->GetFrameSize().Width();
    auto sideBarPosition = layoutProperty->GetSideBarPosition().value_or(SideBarPosition::START);
    float sideBarOffsetX = 0.0f;

    switch (sideBarStatus_) {
        case SideBarStatus::SHOW:
            if (sideBarPosition == SideBarPosition::END) {
                sideBarOffsetX = parentWidth - realSideBarWidth_;
            }
            break;
        case SideBarStatus::HIDDEN:
            if (sideBarPosition == SideBarPosition::START) {
                sideBarOffsetX = -realSideBarWidth_;
            } else {
                sideBarOffsetX = parentWidth;
            }
            break;
        case SideBarStatus::CHANGING:
            if (sideBarPosition == SideBarPosition::START) {
                sideBarOffsetX = currentOffset_;
            } else {
                sideBarOffsetX = parentWidth + currentOffset_;
            }
            break;
        default:
            break;
    }

    sideBarOffset_ = OffsetF(sideBarOffsetX, 0.0f);
    sideBarLayoutWrapper->GetGeometryNode()->SetMarginFrameOffset(sideBarOffset_);
    sideBarLayoutWrapper->Layout();
}

void SideBarContainerLayoutAlgorithm::LayoutSideBarContent(
    LayoutWrapper* layoutWrapper, const RefPtr<LayoutWrapper>& contentLayoutWrapper)
{
    auto layoutProperty = AceType::DynamicCast<SideBarContainerLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);

    auto type = layoutProperty->GetSideBarContainerType().value_or(SideBarContainerType::EMBED);
    auto sideBarPosition = layoutProperty->GetSideBarPosition().value_or(SideBarPosition::START);

    float contentOffsetX = 0.0f;
    if (type == SideBarContainerType::EMBED && sideBarPosition == SideBarPosition::START) {
        if (sideBarStatus_ == SideBarStatus::SHOW) {
            contentOffsetX = realSideBarWidth_;
        } else if (sideBarStatus_ == SideBarStatus::CHANGING) {
            contentOffsetX = realSideBarWidth_ + currentOffset_;
        }
    }

    auto contentOffset = OffsetF(contentOffsetX, 0.0f);
    contentLayoutWrapper->GetGeometryNode()->SetMarginFrameOffset(contentOffset);
    contentLayoutWrapper->Layout();
}

} // namespace OHOS::Ace::NG
