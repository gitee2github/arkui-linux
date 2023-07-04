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

#include "core/components_ng/pattern/tabs/tabs_layout_algorithm.h"

#include "base/geometry/axis.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/log/ace_trace.h"
#include "base/utils/utils.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"

namespace OHOS::Ace::NG {

void TabsLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto layoutProperty = AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto axis = layoutProperty->GetAxis().value_or(Axis::HORIZONTAL);
    auto constraint = layoutProperty->GetLayoutConstraint();
    auto idealSize = CreateIdealSize(constraint.value(), Axis::HORIZONTAL, layoutProperty->GetMeasureType(), true);
    if (GreaterOrEqualToInfinity(idealSize.Width()) || GreaterOrEqualToInfinity(idealSize.Height())) {
        LOGW("Size is infinity.");
        geometryNode->SetFrameSize(SizeF());
        return;
    }
    geometryNode->SetFrameSize(idealSize);
    MinusPaddingToSize(layoutProperty->CreatePaddingAndBorder(), idealSize);
    auto childLayoutConstraint = layoutProperty->CreateChildConstraint();
    childLayoutConstraint.parentIdealSize = OptionalSizeF(idealSize);

    // Measure tab bar.
    auto tabBarWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    SizeF tabBarSize;
    if (tabBarWrapper) {
        tabBarWrapper->Measure(childLayoutConstraint);
        tabBarSize = tabBarWrapper->GetGeometryNode()->GetMarginFrameSize();
    }

    // Measure swiper.
    auto swiperWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    if (swiperWrapper) {
        SizeF parentIdealSize = idealSize;
        if (axis == Axis::HORIZONTAL) {
            childLayoutConstraint.selfIdealSize.SetHeight(childLayoutConstraint.maxSize.Height() - tabBarSize.Height());
            childLayoutConstraint.selfIdealSize.SetWidth(childLayoutConstraint.maxSize.Width());
            parentIdealSize.SetHeight(idealSize.Height() - tabBarSize.Height());
        } else if (axis == Axis::VERTICAL) {
            childLayoutConstraint.selfIdealSize.SetWidth(childLayoutConstraint.maxSize.Width() - tabBarSize.Width());
            childLayoutConstraint.selfIdealSize.SetHeight(childLayoutConstraint.maxSize.Height());
            parentIdealSize.SetWidth(idealSize.Width() - tabBarSize.Width());
        }
        childLayoutConstraint.parentIdealSize = OptionalSizeF(parentIdealSize);
        swiperWrapper->Measure(childLayoutConstraint);
    }
}

void TabsLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto geometryNode = layoutWrapper->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto frameSize = geometryNode->GetFrameSize();
    if (!frameSize.IsPositive()) {
        LOGW("Frame size is not positive.");
        return;
    }

    auto tabBarWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto swiperWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    if (!tabBarWrapper || !swiperWrapper) {
        return;
    }

    auto axis = GetAxis(layoutWrapper);
    auto barPosition = GetBarPosition(layoutWrapper);
    auto tabBarGeometryNode = tabBarWrapper->GetGeometryNode();
    CHECK_NULL_VOID(tabBarGeometryNode);
    auto tabBarFrameSize = tabBarGeometryNode->GetMarginFrameSize();
    auto layoutProperty = DynamicCast<TabsLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    auto padding = layoutProperty->CreatePaddingAndBorder();

    OffsetF tabBarOffset;
    OffsetF swiperOffset;
    if (axis == Axis::HORIZONTAL) {
        float barPosX = (frameSize.MainSize(Axis::HORIZONTAL) - tabBarFrameSize.MainSize(Axis::HORIZONTAL)) / 2;
        if (barPosition == BarPosition::START) {
            tabBarOffset = OffsetF(barPosX, padding.Offset().GetY());
            swiperOffset =
                OffsetF(padding.Offset().GetX(), tabBarFrameSize.MainSize(Axis::VERTICAL) + padding.Offset().GetY());
        } else {
            tabBarOffset = OffsetF(barPosX, frameSize.MainSize(Axis::VERTICAL) -
                                                tabBarFrameSize.MainSize(Axis::VERTICAL) - padding.Offset().GetY());
            swiperOffset = padding.Offset();
        }
    } else {
        float barPosY = (frameSize.MainSize(Axis::VERTICAL) - tabBarFrameSize.MainSize(Axis::VERTICAL)) / 2;
        if (barPosition == BarPosition::START) {
            tabBarOffset = OffsetF(padding.Offset().GetX(), barPosY);
            swiperOffset =
                OffsetF(tabBarFrameSize.MainSize(Axis::HORIZONTAL) + padding.Offset().GetX(), padding.Offset().GetY());
        } else {
            tabBarOffset = OffsetF(frameSize.MainSize(Axis::HORIZONTAL) - tabBarFrameSize.MainSize(Axis::HORIZONTAL) -
                                       padding.Offset().GetX(),
                barPosY);
            swiperOffset = padding.Offset();
        }
    }

    tabBarGeometryNode->SetMarginFrameOffset(tabBarOffset);
    tabBarWrapper->Layout();

    swiperWrapper->GetGeometryNode()->SetMarginFrameOffset(swiperOffset);
    swiperWrapper->Layout();
}

BarPosition TabsLayoutAlgorithm::GetBarPosition(LayoutWrapper* layoutWrapper) const
{
    auto layoutProperty = AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, BarPosition::START);
    return layoutProperty->GetTabBarPosition().value_or(BarPosition::START);
}

Axis TabsLayoutAlgorithm::GetAxis(LayoutWrapper* layoutWrapper) const
{
    auto layoutProperty = AceType::DynamicCast<TabsLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(layoutProperty, Axis::HORIZONTAL);
    return layoutProperty->GetAxis().value_or(Axis::HORIZONTAL);
}

} // namespace OHOS::Ace::NG
