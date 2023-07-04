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

#include "core/components_ng/pattern/panel/sliding_panel_pattern.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/animation/friction_motion.h"
#include "core/animation/spring_animation.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/property.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {

constexpr int32_t ANIMATION_BASE_DURATION = 256;
constexpr Dimension BLANK_MIN_HEIGHT = 8.0_vp;
constexpr Dimension DRAG_UP_THRESHOLD = 48.0_vp;
constexpr double VELOCITY_THRESHOLD = 1000.0; // Move 1000px per second.

} // namespace

void SlidingPanelPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<SlidingPanelLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    InitPanEvent(gestureHub);
    Update();
    if (layoutProperty->GetHasDragBarValue(true)) {
        auto dragBar = GetDragBarNode();
        CHECK_NULL_VOID(dragBar);
        auto dragBarPattern = dragBar->GetPattern<DragBarPattern>();
        CHECK_NULL_VOID(dragBarPattern);
        if (dragBarPattern && !(dragBarPattern->HasClickArrowCallback())) {
            SetDragBarCallBack();
        }
    }
    auto isShow = layoutProperty->GetIsShowValue(false);
    if (isShow_.has_value() && isShow != isShow_.value_or(false)) {
        isShowQueue_.push(isShow);
        if (isShowQueue_.size() == 1 && isShowQueue_.front()) {
            invisibleFlag_ = false;
        }
        return;
    }
    invisibleFlag_ = !invisibleFlag_.has_value() ? !isShow : false;
}

void SlidingPanelPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
}

bool SlidingPanelPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto layoutAlgorithm = DynamicCast<SlidingPanelLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithm, false);
    InitializeLayoutProps();
    isFirstLayout_ = layoutAlgorithm->GetIsFirstLayout();
    fullHeight_ = layoutAlgorithm->GetFullHeight();
    halfHeight_ = layoutAlgorithm->GetHalfHeight();
    miniHeight_ = layoutAlgorithm->GetMiniHeight();
    return true;
}

void SlidingPanelPattern::Update()
{
    auto layoutProperty = GetLayoutProperty<SlidingPanelLayoutProperty>();
    type_ = layoutProperty->GetPanelType().value_or(PanelType::FOLDABLE_BAR);
    if (!mode_.has_value()) {
        mode_ = layoutProperty->GetPanelMode() == PanelMode::AUTO
                    ? PanelMode::FULL
                    : layoutProperty->GetPanelMode().value_or(PanelMode::HALF);
        return;
    }
    auto mode = layoutProperty->GetPanelMode() == PanelMode::AUTO
                    ? PanelMode::FULL
                    : layoutProperty->GetPanelMode().value_or(PanelMode::HALF);
    auto isShow = layoutProperty->GetIsShowValue(true);
    if (mode_.value() != mode && isShow_.has_value() && isShow_.value() == isShow) {
        mode_ = mode;
        CheckPanelModeAndType();
        AnimateTo(defaultBlankHeights_[mode_.value_or(PanelMode::HALF)], mode_.value_or(PanelMode::HALF));
        if (previousMode_ != mode_.value_or(PanelMode::HALF)) {
            FireSizeChangeEvent();
        }
    }
}

void SlidingPanelPattern::InitializeLayoutProps()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto child = host->GetChildren();
    if (child.empty() || child.size() != 1) {
        LOGE("Children size wrong in slide panel modal");
        return;
    }

    auto maxSize = host->GetGeometryNode()->GetFrameSize();
    auto layoutProperty = GetLayoutProperty<SlidingPanelLayoutProperty>();
    auto defaultFullHeight = Dimension(maxSize.Height() - BLANK_MIN_HEIGHT.ConvertToPx());
    auto fullHeight = layoutProperty->GetFullHeight().value_or(defaultFullHeight).ConvertToPx();
    auto halfHeight = layoutProperty->GetHalfHeight().value_or(Dimension(maxSize.Height() / 2)).ConvertToPx();
    auto miniHeight =
        layoutProperty->GetMiniHeight().value_or(Dimension(DRAG_UP_THRESHOLD.ConvertToPx())).ConvertToPx();
    defaultBlankHeights_[PanelMode::FULL] = maxSize.Height() - fullHeight;
    defaultBlankHeights_[PanelMode::HALF] = maxSize.Height() - halfHeight;
    defaultBlankHeights_[PanelMode::MINI] = maxSize.Height() - miniHeight;
    CheckHeightValidity();
    fullHalfBoundary_ = defaultBlankHeights_[PanelMode::FULL] +
                        (defaultBlankHeights_[PanelMode::HALF] - defaultBlankHeights_[PanelMode::FULL]) / 2.0;
    halfMiniBoundary_ = defaultBlankHeights_[PanelMode::HALF] +
                        (defaultBlankHeights_[PanelMode::MINI] - defaultBlankHeights_[PanelMode::HALF]) / 2.0;
    fullMiniBoundary_ = defaultBlankHeights_[PanelMode::FULL] +
                        (defaultBlankHeights_[PanelMode::MINI] - defaultBlankHeights_[PanelMode::FULL]) / 2.0;
    minBlankHeight_ = BLANK_MIN_HEIGHT.ConvertToPx();

    if (!isShow_.has_value()) {
        FirstLayout();
        return;
    }
    auto isShow = layoutProperty->GetIsShowValue(false);
    if (isShow_.value() != isShow) {
        IsShowChanged(isShow);
        return;
    }
    HeightDynamicUpdate();
}

void SlidingPanelPattern::FirstLayout()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    isFirstLayout_ = false;
    auto layoutProperty = GetLayoutProperty<SlidingPanelLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto maxSize = host->GetGeometryNode()->GetFrameSize();
    if (layoutProperty->GetIsShowValue(false) == true) {
        CheckPanelModeAndType();
        UpdateCurrentOffset(maxSize.Height());
        AnimateTo(defaultBlankHeights_[mode_.value_or(PanelMode::HALF)], mode_.value_or(PanelMode::HALF));
        if (previousMode_ != mode_.value_or(PanelMode::HALF)) {
            FireSizeChangeEvent();
        }
        isShow_ = true;
        if (layoutProperty->GetHasDragBarValue(true)) {
            auto dragBar = GetDragBarNode();
            CHECK_NULL_VOID(dragBar);
            auto dragBarPattern = dragBar->GetPattern<DragBarPattern>();
            CHECK_NULL_VOID(dragBarPattern);
            dragBarPattern->ShowInPanelMode(mode_.value_or(PanelMode::HALF));
        }
        return;
    }
    auto rootHeight = PipelineContext::GetCurrentRootHeight();
    CheckPanelModeAndType();
    currentOffset_ = rootHeight;
    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    isShow_ = false;
}

void SlidingPanelPattern::IsShowChanged(bool isShow)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = GetLayoutProperty<SlidingPanelLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto hasDragBar = layoutProperty->GetHasDragBarValue(true);
    if (isShow) {
        isShow_ = true;
        CheckPanelModeAndType();
        AnimateTo(defaultBlankHeights_[mode_.value_or(PanelMode::HALF)], mode_.value_or(PanelMode::HALF));
        if (previousMode_ != mode_.value_or(PanelMode::HALF)) {
            FireSizeChangeEvent();
        }
        if (hasDragBar) {
            auto dragBar = GetDragBarNode();
            CHECK_NULL_VOID(dragBar);
            auto dragBarPattern = dragBar->GetPattern<DragBarPattern>();
            CHECK_NULL_VOID(dragBarPattern);
            dragBarPattern->ShowInPanelMode(mode_.value_or(PanelMode::HALF));
        }
        return;
    }
    isShow_ = false;
    auto rootHeight = PipelineContext::GetCurrentRootHeight();
    AnimateTo(rootHeight, mode_.value_or(PanelMode::HALF));
    if (hasDragBar) {
        auto dragBar = GetDragBarNode();
        CHECK_NULL_VOID(dragBar);
        auto dragBarPattern = dragBar->GetPattern<DragBarPattern>();
        CHECK_NULL_VOID(dragBarPattern);
        dragBarPattern->ShowInPanelMode(mode_.value_or(PanelMode::HALF));
    }
}

void SlidingPanelPattern::HeightDynamicUpdate()
{
    if (isShow_.value_or(false) == true && !isDrag_ && !isAnimating_) {
        switch (previousMode_) {
            case PanelMode::FULL:
                if (!NearEqual(currentOffset_, defaultBlankHeights_[PanelMode::FULL])) {
                    AnimateTo(defaultBlankHeights_[PanelMode::FULL], PanelMode::FULL);
                }
                break;
            case PanelMode::HALF:
                if (!NearEqual(currentOffset_, defaultBlankHeights_[PanelMode::HALF])) {
                    AnimateTo(defaultBlankHeights_[PanelMode::HALF], PanelMode::HALF);
                }
                break;
            case PanelMode::MINI:
                if (!NearEqual(currentOffset_, defaultBlankHeights_[PanelMode::MINI])) {
                    AnimateTo(defaultBlankHeights_[PanelMode::MINI], PanelMode::MINI);
                }
                break;
            default:
                break;
        }
    }
}

void SlidingPanelPattern::CheckHeightValidity()
{
    auto minBlank = BLANK_MIN_HEIGHT.ConvertToPx();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto geometryNode = host->GetGeometryNode();

    auto maxBlank = static_cast<double>(geometryNode->GetFrameSize().Height());
    defaultBlankHeights_[PanelMode::MINI] = std::clamp(defaultBlankHeights_[PanelMode::MINI], minBlank, maxBlank);
    defaultBlankHeights_[PanelMode::HALF] = std::clamp(defaultBlankHeights_[PanelMode::HALF], minBlank, maxBlank);
    defaultBlankHeights_[PanelMode::FULL] = std::clamp(defaultBlankHeights_[PanelMode::FULL], minBlank, maxBlank);
}

void SlidingPanelPattern::CheckPanelModeAndType()
{
    // This parameter does not take effect when PanelMode is set to Half and PanelType is set to minibar
    if (mode_.value_or(PanelMode::HALF) == PanelMode::HALF && type_ == PanelType::MINI_BAR) {
        mode_ = PanelMode::MINI;
    }

    // This parameter does not take effect when PanelMode is set to Mini and PanelType is set to temporary
    if (mode_.value_or(PanelMode::HALF) == PanelMode::MINI && type_ == PanelType::TEMP_DISPLAY) {
        mode_ = PanelMode::HALF;
    }
}

void SlidingPanelPattern::InitPanEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (panEvent_) {
        return;
    }
    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& startInfo) {
        LOGI("Pan event start");
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleDragStart(startInfo.GetLocalLocation());
        }
    };

    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleDragUpdate(info);
        }
    };

    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        LOGI("Pan event end mainVelocity: %{public}lf", info.GetMainVelocity());
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleDragEnd(info.GetMainVelocity());
        }
    };
    auto actionCancelTask = [weak = WeakClaim(this)]() {
        LOGI("Pan event cancel");
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleDragEnd({});
        }
    };
    PanDirection panDirection;
    panDirection.type = PanDirection::VERTICAL;
    float distance = DEFAULT_PAN_DISTANCE;
    auto host = GetHost();
    if (host) {
        auto context = host->GetContext();
        if (context) {
            distance = static_cast<float>(
                context->NormalizeToPx(Dimension(DEFAULT_PAN_DISTANCE, DimensionUnit::VP))); // convert VP to Px
        }
    }
    panEvent_ = MakeRefPtr<PanEvent>(
        std::move(actionStartTask), std::move(actionUpdateTask), std::move(actionEndTask), std::move(actionCancelTask));
    gestureHub->AddPanEvent(panEvent_, panDirection, 1, distance);
}

void SlidingPanelPattern::HandleDragStart(const Offset& startPoint) // const GestureEvent& info
{
    if (isAnimating_ || !isShow_.value_or(false)) {
        return;
    }
    isDrag_ = true;
    dragStartCurrentOffset_ = currentOffset_;
}

void SlidingPanelPattern::HandleDragUpdate(const GestureEvent& info)
{
    if (isAnimating_ || !isShow_.value_or(false)) {
        return;
    }
    auto mainDelta = static_cast<float>(info.GetMainDelta());
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto tempOffset = currentOffset_;
    UpdateCurrentOffset(mainDelta);
    if (NearEqual(currentOffset_, tempOffset)) {
        LOGI("Offset is not changed, needn't measure.");
        return;
    }
    FireHeightChangeEvent();
    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
}

void SlidingPanelPattern::HandleDragEnd(float dragVelocity)
{
    if (isAnimating_ || !isShow_.value_or(false)) {
        return;
    }
    auto dragLen = currentOffset_ - dragStartCurrentOffset_;
    type_ = GetPanelType();
    switch (type_) {
        case PanelType::MINI_BAR: { // FULL & MINI
            CalculateModeTypeMini(dragLen, dragVelocity);
            break;
        }
        case PanelType::FOLDABLE_BAR: { // FULL & HALF & MINI
            CalculateModeTypeFold(dragLen, dragVelocity);
            break;
        }
        case PanelType::TEMP_DISPLAY: { // FULL & HALF
            CalculateModeTypeTemp(dragLen, dragVelocity);
            break;
        }
        default: {
            LOGE("Unsupported type:%{public}d", type_);
            return;
        }
    }
    AnimateTo(defaultBlankHeights_[mode_.value_or(PanelMode::HALF)], mode_.value_or(PanelMode::HALF));
    if (previousMode_ != mode_.value_or(PanelMode::HALF)) {
        FireSizeChangeEvent();
        previousMode_ = mode_.value_or(PanelMode::HALF);
    }
    isDrag_ = false;
}

void SlidingPanelPattern::CalculateModeTypeMini(float dragLen, float velocity) // FULL & MINI
{
    float currentPostion = currentOffset_;
    if (std::abs(velocity) < VELOCITY_THRESHOLD) {
        // Drag velocity not reached to threshold, mode based on the location.
        if (currentPostion < fullMiniBoundary_) {
            mode_ = PanelMode::FULL;
        } else {
            mode_ = PanelMode::MINI;
        }
    } else {
        // Drag velocity reached to threshold, mode based on the drag direction.
        if (velocity > 0.0) {
            mode_ = PanelMode::MINI;
        } else {
            mode_ = PanelMode::FULL;
        }
    }
}

void SlidingPanelPattern::CalculateModeTypeFold(float dragLen, float velocity) // // FULL & HALF & MINI
{
    float currentPostion = currentOffset_;
    if (std::abs(velocity) < VELOCITY_THRESHOLD) {
        // Drag velocity not reached to threshold, mode based on the location.
        if (currentPostion < fullHalfBoundary_) {
            mode_ = PanelMode::FULL;
        } else if (currentPostion < halfMiniBoundary_) {
            mode_ = PanelMode::HALF;
        } else {
            mode_ = PanelMode::MINI;
        }
    } else {
        // Drag velocity reached to threshold, mode based on the drag direction.
        if (velocity > 0.0) {
            if (currentPostion < defaultBlankHeights_[PanelMode::HALF]) {
                mode_ = PanelMode::HALF;
            } else {
                mode_ = PanelMode::MINI;
            }
        } else {
            if (currentPostion > defaultBlankHeights_[PanelMode::HALF]) {
                mode_ = PanelMode::HALF;
            } else {
                mode_ = PanelMode::FULL;
            }
        }
    }
}

void SlidingPanelPattern::CalculateModeTypeTemp(float dragLen, float velocity) // FULL & HALF
{
    float currentPostion = currentOffset_;
    if (std::abs(velocity) < VELOCITY_THRESHOLD) {
        // Drag velocity not reached to threshold, mode based on the location.
        if (currentPostion < fullHalfBoundary_) {
            mode_ = PanelMode::FULL;
        } else {
            mode_ = PanelMode::HALF;
        }
    } else {
        // Drag velocity reached to threshold, mode based on the drag direction.
        if (velocity > 0.0) {
            mode_ = PanelMode::HALF;
        } else {
            mode_ = PanelMode::FULL;
        }
    }
}

void SlidingPanelPattern::AnimateTo(float targetLocation, PanelMode mode)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    animator_ = AceType::MakeRefPtr<Animator>(host->GetContext());
    isAnimating_ = true;
    animator_->ClearInterpolators();
    animator_->ClearAllListeners();
    if (animator_->IsRunning()) {
        animator_->Stop();
    }
    animator_->AddStopListener([weak = WeakClaim(this), mode]() {
        auto panel = weak.Upgrade();
        CHECK_NULL_VOID(panel);
        auto layoutProperty = panel->GetLayoutProperty<SlidingPanelLayoutProperty>();
        CHECK_NULL_VOID(layoutProperty);
        if (layoutProperty->GetHasDragBarValue(true)) {
            auto dragBar = panel->GetDragBarNode();
            CHECK_NULL_VOID(dragBar);
            auto dragBarPattern = dragBar->GetPattern<DragBarPattern>();
            CHECK_NULL_VOID(dragBarPattern);
            dragBarPattern->ShowInPanelMode(mode);
        }
        if (!panel->isShowQueue_.empty() && !panel->isShowQueue_.front()) {
            auto panelNode = panel->GetHost();
            panel->invisibleFlag_ = true;
            panelNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        }
        panel->OnAnimationStop();
    });
    AppendBlankHeightAnimation(targetLocation, mode);
    auto geometryNode = host->GetGeometryNode();
    auto frameSize = geometryNode->GetFrameSize();
    auto dragRange = frameSize.Height();
    animator_->SetDuration(GetAnimationDuration(targetLocation - currentOffset_, dragRange)); // inner
    animator_->SetFillMode(FillMode::FORWARDS);
    animator_->Forward();
}

void SlidingPanelPattern::AppendBlankHeightAnimation(float targetLocation, PanelMode mode)
{
    auto springProperty = AceType::MakeRefPtr<SpringProperty>(1.0f, 100.0f, 20.0f);
    auto heightAnimation = AceType::MakeRefPtr<SpringAnimation>(springProperty);
    heightAnimation->AddListener(
        [weak = AceType::WeakClaim(this), start = currentOffset_, end = targetLocation, mode](float value) {
            auto panel = weak.Upgrade();
            if (!panel) {
                LOGE("Panel is null.");
                return;
            }
            if (value > 1.0) {
                auto layoutProperty = panel->GetLayoutProperty<SlidingPanelLayoutProperty>();
                CHECK_NULL_VOID(layoutProperty);
                if (layoutProperty->GetHasDragBarValue(true)) {
                    auto dragBar = panel->GetDragBarNode();
                    CHECK_NULL_VOID(dragBar);
                    auto dragBarPattern = dragBar->GetPattern<DragBarPattern>();
                    CHECK_NULL_VOID(dragBarPattern);
                    dragBarPattern->ShowInPanelMode(mode);
                }
            }
            panel->UpdateCurrentOffsetOnAnimate((end - start) * value + start);
            panel->FireHeightChangeEvent();
            panel->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
        });
    animator_->AddInterpolator(heightAnimation);
}

RefPtr<FrameNode> SlidingPanelPattern::GetDragBarNode()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto column = AceType::DynamicCast<FrameNode>(host->GetChildAtIndex(0));
    CHECK_NULL_RETURN(column, nullptr);
    auto dragBar = AceType::DynamicCast<FrameNode>(column->GetChildAtIndex(0));
    return dragBar;
}

int32_t SlidingPanelPattern::GetAnimationDuration(float delta, float dragRange) const
{
    if (NearZero(dragRange)) {
        return 0;
    }
    // duration is in millisecond
    return static_cast<int32_t>(((std::abs(delta) / dragRange) + 1.0) * ANIMATION_BASE_DURATION);
}

void SlidingPanelPattern::OnAnimationStop()
{
    if (!isShowQueue_.empty()) {
        isShowQueue_.pop();
        if (!isShowQueue_.empty() && isShowQueue_.front()) {
            invisibleFlag_ = false;
            auto host = GetHost();
            host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        }
    }
    isAnimating_ = false;
}

void SlidingPanelPattern::UpdateCurrentOffset(float offset)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    currentOffset_ = currentOffset_ + offset;
    currentOffset_ = currentOffset_ <= static_cast<float>(BLANK_MIN_HEIGHT.ConvertToPx())
                         ? static_cast<float>(BLANK_MIN_HEIGHT.ConvertToPx())
                         : currentOffset_;
    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
}

void SlidingPanelPattern::UpdateCurrentOffsetOnAnimate(float currentOffset)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    currentOffset_ = currentOffset;
    currentOffset_ = currentOffset_ <= static_cast<float>(BLANK_MIN_HEIGHT.ConvertToPx())
                         ? static_cast<float>(BLANK_MIN_HEIGHT.ConvertToPx())
                         : currentOffset_;
    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
}

PanelType SlidingPanelPattern::GetPanelType() const
{
    auto slidingLayoutProperty = GetLayoutProperty<SlidingPanelLayoutProperty>();
    CHECK_NULL_RETURN(slidingLayoutProperty, PanelType::FOLDABLE_BAR);
    return slidingLayoutProperty->GetPanelType().value_or(PanelType::FOLDABLE_BAR);
}

PanelMode SlidingPanelPattern::GetPanelMode() const
{
    auto slidingLayoutProperty = GetLayoutProperty<SlidingPanelLayoutProperty>();
    CHECK_NULL_RETURN(slidingLayoutProperty, PanelMode::HALF);
    return slidingLayoutProperty->GetPanelMode().value_or(PanelMode::HALF);
}

void SlidingPanelPattern::FireSizeChangeEvent()
{
    auto slidingPanelEventHub = GetEventHub<SlidingPanelEventHub>();
    CHECK_NULL_VOID(slidingPanelEventHub);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto frameSize = host->GetGeometryNode()->GetFrameSize();
    auto layoutProperty = GetLayoutProperty<SlidingPanelLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    float height = 0.0f;
    if (layoutProperty->GetHasDragBarValue(true)) {
        auto dragBar = GetDragBarNode();
        CHECK_NULL_VOID(dragBar);
        auto dragBarFrameSize = dragBar->GetGeometryNode()->GetFrameSize();
        height = std::floor(
            frameSize.Height() - defaultBlankHeights_[mode_.value_or(PanelMode::HALF)] - dragBarFrameSize.Height());
    }
    height = std::floor(frameSize.Height() - defaultBlankHeights_[mode_.value_or(PanelMode::HALF)]);
    float width = std::floor(frameSize.Width());
    slidingPanelEventHub->FireSizeChangeEvent(mode_.value_or(PanelMode::HALF), width, height);
    previousMode_ = mode_.value_or(PanelMode::HALF);
}

void SlidingPanelPattern::FireHeightChangeEvent()
{
    auto slidingPanelEventHub = GetEventHub<SlidingPanelEventHub>();
    CHECK_NULL_VOID(slidingPanelEventHub);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);

    auto currentHeight = static_cast<float>(geometryNode->GetFrameSize().Height() - currentOffset_);
    slidingPanelEventHub->FireHeightChangeEvent(currentHeight);
}

void SlidingPanelPattern::SetDragBarCallBack()
{
    auto dragBar = GetDragBarNode();
    CHECK_NULL_VOID(dragBar);
    auto dragBarPattern = dragBar->GetPattern<DragBarPattern>();
    CHECK_NULL_VOID(dragBarPattern);
    dragBarPattern->SetClickArrowCallback([weak = WeakClaim(this)]() {
        auto panel = weak.Upgrade();
        CHECK_NULL_VOID(panel);
        panel->previousMode_ = panel->mode_.value_or(PanelMode::HALF);
        if (panel->mode_.value_or(PanelMode::HALF) == PanelMode::MINI) {
            panel->mode_ = panel->type_ == PanelType::MINI_BAR ? PanelMode::FULL : PanelMode::HALF;
        } else if (panel->mode_.value_or(PanelMode::HALF) == PanelMode::FULL) {
            panel->mode_ = panel->type_ == PanelType::MINI_BAR ? PanelMode::MINI : PanelMode::HALF;
        } else {
            LOGD("not support click in half mode");
        }
        panel->AnimateTo(panel->defaultBlankHeights_[panel->mode_.value_or(PanelMode::HALF)],
            panel->mode_.value_or(PanelMode::HALF));
        if (panel->previousMode_ != panel->mode_.value_or(PanelMode::HALF)) {
            panel->FireSizeChangeEvent();
        }
    });
}

void SlidingPanelPattern::MarkDirtyNode(PropertyChangeFlag extraFlag)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(extraFlag);
}

void SlidingPanelPattern::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    Pattern::ToJsonValue(json);
    auto layoutProperty = GetLayoutProperty<SlidingPanelLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    static const char* PANEL_TYPE[] = { "PanelType.Minibar", "PanelType.Foldable", "PanelType.Temporary" };
    json->Put(
        "type", PANEL_TYPE[static_cast<int32_t>(layoutProperty->GetPanelType().value_or(PanelType::FOLDABLE_BAR))]);
    static const char* PANEL_MODE[] = { "PanelMode.Mini", "PanelMode.Half", "PanelMode.Full" };
    json->Put("mode", PANEL_MODE[static_cast<int32_t>(layoutProperty->GetPanelMode().value_or(PanelMode::HALF))]);
    json->Put("dragBar", layoutProperty->GetHasDragBar().value_or(true) ? "true" : "false");
    json->Put("show", layoutProperty->GetIsShow().value_or(true) ? "true" : "false");
    json->Put("miniHeight", layoutProperty->GetMiniHeight().value_or(miniHeight_).ToString().c_str());
    json->Put("halfHeight", layoutProperty->GetHalfHeight().value_or(halfHeight_).ToString().c_str());
    json->Put("fullHeight", layoutProperty->GetFullHeight().value_or(fullHeight_).ToString().c_str());
    json->Put(
        "backgroundMask", layoutProperty->GetBackgroundColor().value_or(Color::TRANSPARENT).ColorToString().c_str());
}
} // namespace OHOS::Ace::NG