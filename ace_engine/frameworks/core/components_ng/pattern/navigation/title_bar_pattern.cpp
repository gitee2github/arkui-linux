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

#include "core/components_ng/pattern/navigation/title_bar_pattern.h"

#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/navigation/nav_bar_layout_property.h"
#include "core/components_ng/pattern/navigation/title_bar_layout_property.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/text/text_layout_property.h"

namespace OHOS::Ace::NG {

namespace {

void MountBackButton(const RefPtr<TitleBarNode>& hostNode)
{
    auto titleBarLayoutProperty = hostNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    auto backButtonNode = AceType::DynamicCast<FrameNode>(hostNode->GetBackButton());
    CHECK_NULL_VOID(backButtonNode);
    if (titleBarLayoutProperty->GetTitleBarParentTypeValue(TitleBarParentType::NAVBAR) == TitleBarParentType::NAVBAR) {
        auto buttonNode = backButtonNode->GetChildren().front();
        CHECK_NULL_VOID(buttonNode);
        auto backButtonImageNode = AceType::DynamicCast<FrameNode>(buttonNode->GetChildren().front());
        CHECK_NULL_VOID(backButtonImageNode);
        auto backButtonImageLayoutProperty = backButtonImageNode->GetLayoutProperty<ImageLayoutProperty>();
        CHECK_NULL_VOID(backButtonImageLayoutProperty);
        auto navBarNode = AceType::DynamicCast<FrameNode>(hostNode->GetParent());
        CHECK_NULL_VOID(navBarNode);
        auto navBarLayoutProperty = navBarNode->GetLayoutProperty<NavBarLayoutProperty>();
        CHECK_NULL_VOID(navBarLayoutProperty);
        auto hideBackButton = navBarLayoutProperty->GetHideBackButtonValue(false);
        if (hideBackButton) {
            backButtonImageLayoutProperty->UpdateVisibility(VisibleType::GONE);
        } else {
            backButtonImageLayoutProperty->UpdateVisibility(VisibleType::VISIBLE);
        }
        backButtonImageNode->MarkModifyDone();
        return;
    }
    auto backButtonLayoutProperty = backButtonNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(backButtonLayoutProperty);

    if (!titleBarLayoutProperty->HasNoPixMap()) {
        backButtonNode->MarkModifyDone();
        return;
    }

    if (titleBarLayoutProperty->HasImageSource()) {
        backButtonLayoutProperty->UpdateImageSourceInfo(titleBarLayoutProperty->GetImageSourceValue());
        backButtonNode->MarkModifyDone();
        return;
    }

    if (titleBarLayoutProperty->HasPixelMap()) {
        // TODO: use pixelMap
        backButtonNode->MarkModifyDone();
        return;
    }
}

void MountTitle(const RefPtr<TitleBarNode>& hostNode)
{
    auto titleBarLayoutProperty = hostNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    auto titleNode = AceType::DynamicCast<FrameNode>(hostNode->GetTitle());
    CHECK_NULL_VOID(titleNode);
    auto titleLayoutProperty = titleNode->GetLayoutProperty<TextLayoutProperty>();
    if (!titleLayoutProperty) {
        titleNode->MarkModifyDone();
        return;
    }

    auto theme = NavigationGetTheme();
    CHECK_NULL_VOID(theme);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) == NavigationTitleMode::MINI) {
        if (titleBarLayoutProperty->HasHideBackButton() && titleBarLayoutProperty->GetHideBackButtonValue()) {
            titleLayoutProperty->UpdateFontSize(theme->GetTitleFontSize());
        } else {
            titleLayoutProperty->UpdateFontSize(theme->GetTitleFontSizeMin());
        }
    }
}

void MountSubTitle(const RefPtr<TitleBarNode>& hostNode)
{
    auto titleBarLayoutProperty = hostNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    auto subtitleNode = AceType::DynamicCast<FrameNode>(hostNode->GetSubtitle());
    CHECK_NULL_VOID(subtitleNode);
    auto titleLayoutProperty = subtitleNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(titleLayoutProperty);
    subtitleNode->MarkModifyDone();
}

}

void TitleBarPattern::OnModifyDone()
{
    auto hostNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(hostNode);
    MountBackButton(hostNode);
    MountTitle(hostNode);
    MountSubTitle(hostNode);

    // navBar title bar
    auto titleBarLayoutProperty = AceType::DynamicCast<TitleBarLayoutProperty>(hostNode->GetLayoutProperty());
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (titleBarLayoutProperty->GetTitleBarParentTypeValue(TitleBarParentType::NAVBAR) == TitleBarParentType::NAVBAR) {
        auto gesture = hostNode->GetOrCreateGestureEventHub();
        CHECK_NULL_VOID(gesture);
        InitPanEvent(gesture);
    }
}

void TitleBarPattern::InitPanEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID_NOLOG(!panEvent_);

    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        LOGI("Pan event start");
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        if (info.GetInputEventType() == InputEventType::AXIS) {
            return;
        }
        pattern->HandleDragStart(info);
    };

    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        if (info.GetInputEventType() == InputEventType::AXIS) {
            return;
        }
        pattern->HandleDragUpdate(info);
    };

    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        LOGI("Pan event end mainVelocity: %{public}lf", info.GetMainVelocity());
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        if (info.GetInputEventType() == InputEventType::AXIS) {
            return;
        }
        pattern->HandleDragEnd(info.GetMainVelocity());
    };

    auto actionCancelTask = [weak = WeakClaim(this)]() {
        LOGI("Pan event cancel");
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleDragEnd(0.0);
    };

    if (panEvent_) {
        gestureHub->RemovePanEvent(panEvent_);
    }

    panEvent_ = MakeRefPtr<PanEvent>(
        std::move(actionStartTask), std::move(actionUpdateTask), std::move(actionEndTask), std::move(actionCancelTask));
    PanDirection panDirection = { .type = PanDirection::VERTICAL };
    gestureHub->AddPanEvent(panEvent_, panDirection, DEFAULT_PAN_FINGER, DEFAULT_PAN_DISTANCE);
}

void TitleBarPattern::HandleDragStart(const GestureEvent& info)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::FREE) {
        return;
    }
    defaultTitleBarHeight_ = titleBarNode->GetGeometryNode()->GetFrameSize().Height();
    SetMaxTitleBarHeight();
    SetTempTitleBarHeight(static_cast<float>(info.GetOffsetY()));
    minTitleOffsetY_ = (static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()) - minTitleHeight_) / 2;
    maxTitleOffsetY_ = initialTitleOffsetY_;
    moveRatio_ = (maxTitleOffsetY_ - minTitleOffsetY_) /
        (maxTitleBarHeight_ - static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx()));
    titleMoveDistance_ = (tempTitleBarHeight_ - defaultTitleBarHeight_) * moveRatio_;
    defaultTitleOffsetY_ = GetTitleOffsetY();
    SetTempTitleOffsetY();
    defaultSubtitleOffsetY_ = GetSubTitleOffsetY();
    SetTempSubTitleOffsetY();
    titleBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);

    // title font size
    SetDefaultTitleFontSize();
    auto titleBarHeightDiff = maxTitleBarHeight_ - static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    auto titleFontSizeDiff = MAX_TITLE_FONT_SIZE - MIN_TITLE_FONT_SIZE;
    fontSizeRatio_ = titleFontSizeDiff.Value() / titleBarHeightDiff;
    auto tempFontSize = static_cast<float>((tempTitleBarHeight_ - defaultTitleBarHeight_) * fontSizeRatio_ + defaultTitleFontSize_.Value());
    UpdateTitleFontSize(Dimension(tempFontSize, DimensionUnit::VP));

    // subTitle Opacity
    SetDefaultSubtitleOpacity();
    opacityRatio_ = 1.0f / titleBarHeightDiff;
    auto tempOpacity = static_cast<float>((tempTitleBarHeight_ - defaultTitleBarHeight_) * opacityRatio_ + defaultSubtitleOpacity_);
    UpdateSubTitleOpacity(tempOpacity);
}

void TitleBarPattern::HandleDragUpdate(const GestureEvent& info)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::FREE) {
        return;
    }
    SetTempTitleBarHeight(static_cast<float>(info.GetOffsetY()));
    titleMoveDistance_ = (tempTitleBarHeight_ - defaultTitleBarHeight_) * moveRatio_;
    SetTempTitleOffsetY();
    SetTempSubTitleOffsetY();
    titleBarNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);

    // title font size
    auto tempFontSize = static_cast<float>((tempTitleBarHeight_ - defaultTitleBarHeight_) * fontSizeRatio_ + defaultTitleFontSize_.Value());
    UpdateTitleFontSize(Dimension(tempFontSize, DimensionUnit::VP));

    // subTitle Opacity
    auto tempOpacity = static_cast<float>((tempTitleBarHeight_ - defaultTitleBarHeight_) * opacityRatio_ + defaultSubtitleOpacity_);
    UpdateSubTitleOpacity(tempOpacity);
}

void TitleBarPattern::HandleDragEnd(double dragVelocity) {
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleBarLayoutProperty = titleBarNode->GetLayoutProperty<TitleBarLayoutProperty>();
    CHECK_NULL_VOID(titleBarLayoutProperty);
    if (titleBarLayoutProperty->GetTitleModeValue(NavigationTitleMode::FREE) != NavigationTitleMode::FREE) {
        return;
    }
}

void TitleBarPattern::SetMaxTitleBarHeight()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    if (titleBarNode->GetSubtitle()) {
        maxTitleBarHeight_ = static_cast<float>(FULL_DOUBLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    } else {
        maxTitleBarHeight_ = static_cast<float>(FULL_SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    }
}

void TitleBarPattern::SetTempTitleBarHeight(float offsetY)
{
    tempTitleBarHeight_ = defaultTitleBarHeight_ + offsetY;
    if (tempTitleBarHeight_ < static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx())) {
        tempTitleBarHeight_ = static_cast<float>(SINGLE_LINE_TITLEBAR_HEIGHT.ConvertToPx());
    }
    if (tempTitleBarHeight_ > maxTitleBarHeight_) {
        tempTitleBarHeight_ = maxTitleBarHeight_;
    }
}

void TitleBarPattern::SetTempTitleOffsetY()
{
    tempTitleOffsetY_ = defaultTitleOffsetY_ + titleMoveDistance_;
    if (tempTitleOffsetY_ < minTitleOffsetY_) {
        tempTitleOffsetY_ = minTitleOffsetY_;
    }
    if (tempTitleOffsetY_ > maxTitleOffsetY_) {
        tempTitleOffsetY_ = maxTitleOffsetY_;
    }
}

void TitleBarPattern::SetTempSubTitleOffsetY()
{
    tempSubTitleOffsetY_ = tempTitleOffsetY_ + GetTitleHeight();
    if (tempTitleOffsetY_ < minTitleOffsetY_) {
        tempSubTitleOffsetY_ = minTitleOffsetY_;
    }
    if (tempTitleOffsetY_ > maxTitleOffsetY_) {
        tempSubTitleOffsetY_ = maxTitleOffsetY_;
    }
}

void TitleBarPattern::SetDefaultTitleFontSize()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    CHECK_NULL_VOID_NOLOG(titleBarNode->GetTitle());
    auto titleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
    CHECK_NULL_VOID(titleNode);
    auto textLayoutProperty = titleNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    if (defaultTitleBarHeight_ == maxTitleBarHeight_) {
        defaultTitleFontSize_ = textLayoutProperty->GetFontSizeValue(MAX_TITLE_FONT_SIZE);
    } else {
        defaultTitleFontSize_ = textLayoutProperty->GetFontSizeValue(MIN_TITLE_FONT_SIZE);
    }
}

void TitleBarPattern::SetDefaultSubtitleOpacity()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    CHECK_NULL_VOID(titleBarNode->GetSubtitle());
    auto subtitleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle());
    CHECK_NULL_VOID(subtitleNode);
    auto context = subtitleNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    if (defaultTitleBarHeight_ == maxTitleBarHeight_) {
        defaultSubtitleOpacity_ = context->GetOpacityValue(1.0f);
    } else {
        defaultSubtitleOpacity_ = context->GetOpacityValue(0.0f);
    }
}

float TitleBarPattern::GetTitleHeight()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_RETURN(titleBarNode, 0.0f);
    auto titleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
    CHECK_NULL_RETURN(titleNode, 0.0f);
    auto geometryNode = titleNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    return geometryNode->GetFrameSize().Height();
}

float TitleBarPattern::GetTitleOffsetY()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_RETURN(titleBarNode, 0.0f);
    auto titleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
    CHECK_NULL_RETURN(titleNode, 0.0f);
    auto geometryNode = titleNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    return geometryNode->GetMarginFrameOffset().GetY();
}

float TitleBarPattern::GetSubTitleHeight()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_RETURN(titleBarNode, 0.0f);
    auto subTitleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle());
    CHECK_NULL_RETURN(subTitleNode, 0.0f);
    auto geometryNode = subTitleNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    return geometryNode->GetFrameSize().Height();
}

float TitleBarPattern::GetSubTitleOffsetY()
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_RETURN(titleBarNode, 0.0f);
    auto subTitleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle());
    CHECK_NULL_RETURN(subTitleNode, 0.0f);
    auto geometryNode = subTitleNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    return geometryNode->GetMarginFrameOffset().GetY();
}

void TitleBarPattern::UpdateTitleFontSize(const Dimension& tempTitleFontSize)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto titleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetTitle());
    CHECK_NULL_VOID(titleNode);
    auto textLayoutProperty = titleNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    textLayoutProperty->UpdateFontSize(tempTitleFontSize);
    titleNode->MarkModifyDone();
}

void TitleBarPattern::UpdateSubTitleOpacity(const double &value)
{
    auto titleBarNode = AceType::DynamicCast<TitleBarNode>(GetHost());
    CHECK_NULL_VOID(titleBarNode);
    auto subTitleNode = AceType::DynamicCast<FrameNode>(titleBarNode->GetSubtitle());
    CHECK_NULL_VOID(subTitleNode);
    auto context = subTitleNode->GetRenderContext();
    CHECK_NULL_VOID(context);
    context->UpdateOpacity(value);
}

bool TitleBarPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto titleBarLayoutAlgorithm = DynamicCast<TitleBarLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(titleBarLayoutAlgorithm, false);

    initialTitleOffsetY_ = titleBarLayoutAlgorithm->GetInitialTitleOffsetY();
    isInitialTitle_ = titleBarLayoutAlgorithm->IsInitialTitle();
    initialSubtitleOffsetY_ = titleBarLayoutAlgorithm->GetInitialSubtitleOffsetY();
    isInitialSubtitle_ = titleBarLayoutAlgorithm->IsInitialSubtitle();
    minTitleHeight_ = titleBarLayoutAlgorithm->GetMinTitleHeight();
    return true;
}

void TitleBarPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
}

} // namespace OHOS::Ace::NG
