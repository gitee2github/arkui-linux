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

#include "core/components_ng/layout/layout_property.h"

#include <optional>

#include "base/geometry/ng/size_t.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
std::string VisibleTypeToString(VisibleType type)
{
    static const LinearEnumMapNode<VisibleType, std::string> visibilityMap[] = {
        { VisibleType::VISIBLE, "Visibility.Visible" },
        { VisibleType::INVISIBLE, "Visibility.Hidden" },
        { VisibleType::GONE, "Visibility.None" },
    };
    auto idx = BinarySearchFindIndex(visibilityMap, ArraySize(visibilityMap), type);
    if (idx >= 0) {
        return visibilityMap[idx].value;
    }
    return "Visibility.Visible";
}

std::string TextDirectionToString(TextDirection type)
{
    static const LinearEnumMapNode<TextDirection, std::string> toStringMap[] = {
        { TextDirection::LTR, "Direction.Ltr" },
        { TextDirection::RTL, "Direction.Rtl" },
        { TextDirection::INHERIT, "Direction.Inherit" },
        { TextDirection::AUTO, "Direction.Auto" },
    };
    auto idx = BinarySearchFindIndex(toStringMap, ArraySize(toStringMap), type);
    if (idx >= 0) {
        return toStringMap[idx].value;
    }
    return "Direction.Ltr";
}
} // namespace

void LayoutProperty::Reset()
{
    layoutConstraint_.reset();
    calcLayoutConstraint_.reset();
    padding_.reset();
    margin_.reset();
    borderWidth_.reset();
    magicItemProperty_.reset();
    positionProperty_.reset();
    measureType_.reset();
    layoutDirection_.reset();
    propVisibility_.reset();
    CleanDirty();
}

void LayoutProperty::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    ACE_PROPERTY_TO_JSON_VALUE(calcLayoutConstraint_, MeasureProperty);
    ACE_PROPERTY_TO_JSON_VALUE(positionProperty_, PositionProperty);
    ACE_PROPERTY_TO_JSON_VALUE(magicItemProperty_, MagicItemProperty);
    ACE_PROPERTY_TO_JSON_VALUE(flexItemProperty_, FlexItemProperty);
    ACE_PROPERTY_TO_JSON_VALUE(borderWidth_, BorderWidthProperty);
    ACE_PROPERTY_TO_JSON_VALUE(gridProperty_, GridProperty);

    if (padding_) {
        json->Put("padding", padding_->ToJsonString().c_str());
    } else {
        json->Put("padding", "0.00vp");
    }

    if (margin_) {
        json->Put("margin", margin_->ToJsonString().c_str());
    } else {
        json->Put("margin", "0.00vp");
    }

    json->Put("visibility", VisibleTypeToString(propVisibility_.value_or(VisibleType::VISIBLE)).c_str());
    json->Put("direction", TextDirectionToString(GetLayoutDirection()).c_str());
}

RefPtr<LayoutProperty> LayoutProperty::Clone() const
{
    auto layoutProperty = MakeRefPtr<LayoutProperty>();
    Clone(layoutProperty);
    return layoutProperty;
}

void LayoutProperty::Clone(RefPtr<LayoutProperty> layoutProperty) const
{
    layoutProperty->UpdateLayoutProperty(this);
}

void LayoutProperty::UpdateLayoutProperty(const LayoutProperty* layoutProperty)
{
    layoutConstraint_ = layoutProperty->layoutConstraint_;
    if (layoutProperty->gridProperty_) {
        gridProperty_ = std::make_unique<GridProperty>(*layoutProperty->gridProperty_);
    }
    if (layoutProperty->calcLayoutConstraint_) {
        calcLayoutConstraint_ = std::make_unique<MeasureProperty>(*layoutProperty->calcLayoutConstraint_);
    }
    if (layoutProperty->padding_) {
        padding_ = std::make_unique<PaddingProperty>(*layoutProperty->padding_);
    }
    if (layoutProperty->margin_) {
        margin_ = std::make_unique<PaddingProperty>(*layoutProperty->margin_);
    }
    if (layoutProperty->borderWidth_) {
        borderWidth_ = std::make_unique<BorderWidthProperty>(*layoutProperty->borderWidth_);
    }
    if (layoutProperty->magicItemProperty_) {
        magicItemProperty_ = std::make_unique<MagicItemProperty>(*layoutProperty->magicItemProperty_);
    }
    if (layoutProperty->positionProperty_) {
        positionProperty_ = std::make_unique<PositionProperty>(*layoutProperty->positionProperty_);
    }
    if (layoutProperty->flexItemProperty_) {
        flexItemProperty_ = std::make_unique<FlexItemProperty>(*layoutProperty->flexItemProperty_);
    }
    propVisibility_ = layoutProperty->GetVisibility();
    measureType_ = layoutProperty->measureType_;
    layoutDirection_ = layoutProperty->layoutDirection_;
    propertyChangeFlag_ = layoutProperty->propertyChangeFlag_;
}

void LayoutProperty::UpdateCalcLayoutProperty(const MeasureProperty& constraint)
{
    if (!calcLayoutConstraint_) {
        calcLayoutConstraint_ = std::make_unique<MeasureProperty>(constraint);
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
        return;
    }
    if (*calcLayoutConstraint_ == constraint) {
        return;
    }
    calcLayoutConstraint_->selfIdealSize = constraint.selfIdealSize;
    calcLayoutConstraint_->maxSize = constraint.maxSize;
    calcLayoutConstraint_->minSize = constraint.minSize;
    propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
}

void LayoutProperty::UpdateLayoutConstraint(const LayoutConstraintF& parentConstraint)
{
    layoutConstraint_ = parentConstraint;
    if (margin_) {
        // TODO: add margin is negative case.
        auto margin = CreateMargin();
        MinusPaddingToSize(margin, layoutConstraint_->maxSize);
        MinusPaddingToSize(margin, layoutConstraint_->minSize);
        MinusPaddingToSize(margin, layoutConstraint_->percentReference);
        MinusPaddingToSize(margin, layoutConstraint_->selfIdealSize);
        MinusPaddingToSize(margin, layoutConstraint_->parentIdealSize);
    }
    if (calcLayoutConstraint_) {
        if (calcLayoutConstraint_->maxSize.has_value()) {
            layoutConstraint_->UpdateMaxSizeWithCheck(ConvertToSize(calcLayoutConstraint_->maxSize.value(),
                parentConstraint.scaleProperty, parentConstraint.percentReference));
        }
        if (calcLayoutConstraint_->minSize.has_value()) {
            layoutConstraint_->UpdateMinSizeWithCheck(ConvertToSize(calcLayoutConstraint_->minSize.value(),
                parentConstraint.scaleProperty, parentConstraint.percentReference));
        }
        if (calcLayoutConstraint_->selfIdealSize.has_value()) {
            LOGD("CalcLayoutConstraint->selfIdealSize = %{public}s",
                calcLayoutConstraint_->selfIdealSize.value().ToString().c_str());
            layoutConstraint_->UpdateIllegalSelfIdealSizeWithCheck(
                ConvertToOptionalSize(calcLayoutConstraint_->selfIdealSize.value(), parentConstraint.scaleProperty,
                    parentConstraint.percentReference));
        }
    }

    CheckSelfIdealSize();
    CheckBorderAndPadding();
    CheckAspectRatio();
}

void LayoutProperty::CheckBorderAndPadding()
{
    auto selfWidth = layoutConstraint_->selfIdealSize.Width();
    auto selfHeight = layoutConstraint_->selfIdealSize.Height();
    if (!selfWidth && !selfHeight) {
        return;
    }
    auto selfWidthFloat = selfWidth.value_or(Infinity<float>());
    auto selfHeightFloat = selfHeight.value_or(Infinity<float>());
    auto paddingWithBorder = CreatePaddingAndBorder();
    auto deflateWidthF = paddingWithBorder.Width();
    auto deflateHeightF = paddingWithBorder.Height();
    if (LessOrEqual(deflateWidthF, selfWidthFloat) && LessOrEqual(deflateHeightF, selfHeightFloat)) {
        return;
    }
    if (GreatNotEqual(deflateWidthF, selfWidthFloat)) {
        layoutConstraint_->selfIdealSize.SetWidth(deflateWidthF);
    }
    if (GreatNotEqual(deflateHeightF, selfHeightFloat)) {
        layoutConstraint_->selfIdealSize.SetHeight(deflateHeightF);
    }
}

void LayoutProperty::CheckAspectRatio()
{
    auto hasAspectRatio = magicItemProperty_ ? magicItemProperty_->HasAspectRatio() : false;
    if (!hasAspectRatio) {
        return;
    }
    auto aspectRatio = magicItemProperty_->GetAspectRatioValue();
    // Adjust by aspect ratio, firstly pick height based on width. It means that when width, height and aspectRatio are
    // all set, the height is not used.
    auto maxWidth = layoutConstraint_->maxSize.Width();
    auto maxHeight = layoutConstraint_->maxSize.Height();
    if (maxHeight > maxWidth / aspectRatio) {
        maxHeight = maxWidth / aspectRatio;
    }
    layoutConstraint_->maxSize.SetWidth(maxWidth);
    layoutConstraint_->maxSize.SetHeight(maxHeight);
    std::optional<float> selfWidth;
    std::optional<float> selfHeight;
    if (layoutConstraint_->selfIdealSize.Width()) {
        selfWidth = layoutConstraint_->selfIdealSize.Width().value();
        selfHeight = selfWidth.value() / aspectRatio;
        if (selfHeight > maxHeight) {
            selfHeight = maxHeight;
            selfWidth = selfHeight.value() * aspectRatio;
        }
    } else if (layoutConstraint_->selfIdealSize.Height()) {
        selfHeight = layoutConstraint_->selfIdealSize.Height().value();
        selfWidth = selfHeight.value() * aspectRatio;
        if (selfWidth > maxWidth) {
            selfWidth = maxWidth;
            selfHeight = selfWidth.value() / aspectRatio;
        }
    }

    if (selfHeight) {
        layoutConstraint_->selfIdealSize.SetHeight(selfHeight);
    }
    if (selfWidth) {
        layoutConstraint_->selfIdealSize.SetWidth(selfWidth);
    }
    // TODO: after measure done, need to check AspectRatio again.
}

void LayoutProperty::BuildGridProperty(const RefPtr<FrameNode>& host)
{
    CHECK_NULL_VOID_NOLOG(gridProperty_);
    auto parent = host->GetAncestorNodeOfFrame();
    while (parent) {
        if (parent->GetTag() == V2::GRIDCONTAINER_ETS_TAG) {
            auto containerLayout = parent->GetLayoutProperty();
            gridProperty_->UpdateContainer(containerLayout, host);
            UpdateUserDefinedIdealSize(CalcSize(CalcLength(gridProperty_->GetWidth()), std::nullopt));
            break;
        }
        parent = parent->GetAncestorNodeOfFrame();
    }
}

void LayoutProperty::UpdateGridProperty(std::optional<int32_t> span, std::optional<int32_t> offset, GridSizeType type)
{
    if (!gridProperty_) {
        gridProperty_ = std::make_unique<GridProperty>();
    }

    bool isSpanUpdated = (span.has_value() && gridProperty_->UpdateSpan(span.value(), type));
    bool isOffsetUpdated = (offset.has_value() && gridProperty_->UpdateOffset(offset.value(), type));
    if (isSpanUpdated || isOffsetUpdated) {
        propertyChangeFlag_ = propertyChangeFlag_ | PROPERTY_UPDATE_MEASURE;
    }
}

bool LayoutProperty::UpdateGridOffset(const RefPtr<FrameNode>& host)
{
    CHECK_NULL_RETURN_NOLOG(gridProperty_, false);
    auto optOffset = gridProperty_->GetOffset();
    if (optOffset == UNDEFINED_DIMENSION) {
        return false;
    }

    RefPtr<FrameNode> parent = host->GetAncestorNodeOfFrame();
    auto parentOffset = parent->GetOffsetRelativeToWindow();
    auto globalOffset = gridProperty_->GetContainerPosition();

    OffsetF offset(optOffset.ConvertToPx(), 0);
    offset = offset + globalOffset - parentOffset;
    const auto& geometryNode = host->GetGeometryNode();
    if (offset.GetX() == geometryNode->GetFrameOffset().GetX()) {
        return false;
    }
    offset.SetY(geometryNode->GetFrameOffset().GetY());
    geometryNode->SetFrameOffset(offset);
    return true;
}

void LayoutProperty::CheckSelfIdealSize()
{
    if (measureType_ == MeasureType::MATCH_PARENT) {
        layoutConstraint_->UpdateIllegalSelfIdealSizeWithCheck(layoutConstraint_->parentIdealSize);
    }
    if (!calcLayoutConstraint_) {
        return;
    }
    if (calcLayoutConstraint_->minSize.has_value()) {
        layoutConstraint_->selfIdealSize.UpdateSizeWhenLarger(ConvertToSize(calcLayoutConstraint_->minSize.value(),
            layoutConstraint_->scaleProperty, layoutConstraint_->percentReference));
    }
    if (calcLayoutConstraint_->maxSize.has_value()) {
        layoutConstraint_->selfIdealSize.UpdateSizeWhenSmaller(ConvertToSize(calcLayoutConstraint_->maxSize.value(),
            layoutConstraint_->scaleProperty, layoutConstraint_->percentReference));
    }
}

LayoutConstraintF LayoutProperty::CreateChildConstraint() const
{
    CHECK_NULL_RETURN(layoutConstraint_, {});
    auto layoutConstraint = contentConstraint_.value();
    layoutConstraint.parentIdealSize = layoutConstraint.selfIdealSize;
    // update max size when ideal size has value.
    if (layoutConstraint.parentIdealSize.Width()) {
        layoutConstraint.maxSize.SetWidth(layoutConstraint.parentIdealSize.Width().value());
        layoutConstraint.percentReference.SetWidth(layoutConstraint.parentIdealSize.Width().value());
    }
    if (layoutConstraint.parentIdealSize.Height()) {
        layoutConstraint.maxSize.SetHeight(layoutConstraint.parentIdealSize.Height().value());
        layoutConstraint.percentReference.SetHeight(layoutConstraint.parentIdealSize.Height().value());
    }
    // for child constraint, reset current selfIdealSize and minSize.
    layoutConstraint.selfIdealSize.Reset();
    layoutConstraint.minSize.Reset();
    return layoutConstraint;
}

void LayoutProperty::UpdateContentConstraint()
{
    CHECK_NULL_VOID(layoutConstraint_);
    contentConstraint_ = layoutConstraint_.value();
    // update percent reference when parent has size.
    if (contentConstraint_->parentIdealSize.Width()) {
        contentConstraint_->percentReference.SetWidth(contentConstraint_->parentIdealSize.Width().value());
    }
    if (contentConstraint_->parentIdealSize.Height()) {
        contentConstraint_->percentReference.SetHeight(contentConstraint_->parentIdealSize.Height().value());
    }
    if (padding_) {
        auto paddingF = ConvertToPaddingPropertyF(
            *padding_, contentConstraint_->scaleProperty, contentConstraint_->percentReference.Width());
        contentConstraint_->MinusPaddingOnBothSize(paddingF.left, paddingF.right, paddingF.top, paddingF.bottom);
    }
    if (borderWidth_) {
        auto borderWidthF = ConvertToBorderWidthPropertyF(
            *borderWidth_, contentConstraint_->scaleProperty, contentConstraint_->percentReference.Width());
        contentConstraint_->MinusPaddingOnBothSize(
            borderWidthF.leftDimen, borderWidthF.rightDimen, borderWidthF.topDimen, borderWidthF.bottomDimen);
    }
}

PaddingPropertyF LayoutProperty::CreatePaddingAndBorder()
{
    if (layoutConstraint_.has_value()) {
        auto padding = ConvertToPaddingPropertyF(
            padding_, ScaleProperty::CreateScaleProperty(), layoutConstraint_->percentReference.Width());
        auto borderWidth = ConvertToBorderWidthPropertyF(
            borderWidth_, ScaleProperty::CreateScaleProperty(), layoutConstraint_->percentReference.Width());

        return PaddingPropertyF { padding.left.value_or(0) + borderWidth.leftDimen.value_or(0),
            padding.right.value_or(0) + borderWidth.rightDimen.value_or(0),
            padding.top.value_or(0) + borderWidth.topDimen.value_or(0),
            padding.bottom.value_or(0) + borderWidth.bottomDimen.value_or(0) };
    }
    auto padding = ConvertToPaddingPropertyF(
        padding_, ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth());
    auto borderWidth = ConvertToBorderWidthPropertyF(
        borderWidth_, ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth());

    return PaddingPropertyF { padding.left.value_or(0) + borderWidth.leftDimen.value_or(0),
        padding.right.value_or(0) + borderWidth.rightDimen.value_or(0),
        padding.top.value_or(0) + borderWidth.topDimen.value_or(0),
        padding.bottom.value_or(0) + borderWidth.bottomDimen.value_or(0) };
}

PaddingPropertyF LayoutProperty::CreatePaddingAndBorderWithDefault(float paddingHorizontalDefault,
    float paddingVerticalDefault, float borderHorizontalDefault, float borderVerticalDefault)
{
    if (layoutConstraint_.has_value()) {
        auto padding = ConvertToPaddingPropertyF(
            padding_, ScaleProperty::CreateScaleProperty(), layoutConstraint_->percentReference.Width());
        auto borderWidth = ConvertToBorderWidthPropertyF(
            borderWidth_, ScaleProperty::CreateScaleProperty(), layoutConstraint_->percentReference.Width());
        return PaddingPropertyF { padding.left.value_or(paddingHorizontalDefault) +
                                      borderWidth.leftDimen.value_or(borderHorizontalDefault),
            padding.right.value_or(paddingHorizontalDefault) + borderWidth.rightDimen.value_or(borderHorizontalDefault),
            padding.top.value_or(paddingVerticalDefault) + borderWidth.topDimen.value_or(borderVerticalDefault),
            padding.bottom.value_or(paddingVerticalDefault) + borderWidth.bottomDimen.value_or(borderVerticalDefault) };
    }
    auto padding = ConvertToPaddingPropertyF(
        padding_, ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth());
    auto borderWidth = ConvertToBorderWidthPropertyF(
        borderWidth_, ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth());

    return PaddingPropertyF { padding.left.value_or(paddingHorizontalDefault) +
                                  borderWidth.leftDimen.value_or(borderHorizontalDefault),
        padding.right.value_or(paddingHorizontalDefault) + borderWidth.rightDimen.value_or(borderHorizontalDefault),
        padding.top.value_or(paddingVerticalDefault) + borderWidth.topDimen.value_or(borderVerticalDefault),
        padding.bottom.value_or(paddingVerticalDefault) + borderWidth.bottomDimen.value_or(borderVerticalDefault) };
}

PaddingPropertyF LayoutProperty::CreatePaddingWithoutBorder()
{
    if (layoutConstraint_.has_value()) {
        return ConvertToPaddingPropertyF(
            padding_, layoutConstraint_->scaleProperty, layoutConstraint_->percentReference.Width());
    }

    return ConvertToPaddingPropertyF(
        padding_, ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth());
}

MarginPropertyF LayoutProperty::CreateMargin()
{
    if (layoutConstraint_.has_value()) {
        return ConvertToMarginPropertyF(
            margin_, layoutConstraint_->scaleProperty, layoutConstraint_->percentReference.Width());
    }

    return ConvertToMarginPropertyF(
        margin_, ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth());
}

void LayoutProperty::SetHost(const WeakPtr<FrameNode>& host)
{
    host_ = host;
}

RefPtr<FrameNode> LayoutProperty::GetHost() const
{
    return host_.Upgrade();
}

void LayoutProperty::OnVisibilityUpdate(VisibleType visible) const
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->OnVisibleChange(visible == VisibleType::VISIBLE);
    auto parent = host->GetAncestorNodeOfFrame();
    if (parent) {
        parent->MarkNeedSyncRenderTree();
        parent->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }
}

} // namespace OHOS::Ace::NG
