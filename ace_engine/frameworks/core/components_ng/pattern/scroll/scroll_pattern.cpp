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

#include "core/components_ng/pattern/scroll/scroll_pattern.h"

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/utils/utils.h"
#include "core/components/scroll/scrollable.h"
#include "core/components_ng/pattern/scroll/scroll_edge_effect.h"
#include "core/components_ng/pattern/scroll/scroll_event_hub.h"
#include "core/components_ng/pattern/scroll/scroll_layout_algorithm.h"
#include "core/components_ng/pattern/scroll/scroll_layout_property.h"
#include "core/components_ng/pattern/scroll/scroll_paint_property.h"
#include "core/components_ng/pattern/scroll/scroll_spring_effect.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/property/property.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {

namespace {

constexpr int32_t SCROLL_NONE = 0;
constexpr int32_t SCROLL_TOUCH_DOWN = 1;
constexpr int32_t SCROLL_TOUCH_UP = 2;
constexpr float SCROLL_RATIO = 0.52f;
constexpr float SCROLL_BY_SPEED = 250.0f; // move 250 pixels per second
constexpr float SCROLL_MAX_TIME = 300.0f; // Scroll Animate max time 0.3 second
constexpr float UNIT_CONVERT = 1000.0f;    // 1s convert to 1000ms

float CalculateFriction(float gamma)
{
    return static_cast<float>(SCROLL_RATIO * std::pow(1.0 - gamma, SQUARE));
}

float CalculateOffsetByFriction(float extentOffset, float delta, float friction)
{
    if (NearZero(friction)) {
        return delta;
    }
    float deltaToLimit = extentOffset / friction;
    if (delta < deltaToLimit) {
        return delta * friction;
    }
    return extentOffset + delta - deltaToLimit;
}

} // namespace

void ScrollPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToBounds(true);
}

void ScrollPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<ScrollLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto paintProperty = host->GetPaintProperty<ScrollPaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    auto axis = layoutProperty->GetAxis().value_or(Axis::VERTICAL);
    if (axis != GetAxis()) {
        SetAxis(axis);
        ResetPosition();
    }
    if (!GetScrollableEvent()) {
        AddScrollEvent();
        RegisterScrollEventTask();
    }
    SetEdgeEffect(layoutProperty->GetEdgeEffect().value_or(EdgeEffect::NONE));
    SetScrollBar(paintProperty->GetScrollBarProperty());
}

void ScrollPattern::RegisterScrollEventTask()
{
    auto scrollableEvent = GetScrollableEvent();
    CHECK_NULL_VOID(scrollableEvent);
    auto eventHub = GetHost()->GetEventHub<ScrollEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto onScrollEvent = eventHub->GetOnScrollEvent();
    if (onScrollEvent) {
        scrollableEvent->SetOnScrollCallback(std::move(onScrollEvent));
    }
    auto scrollBeginEvent = eventHub->GetScrollBeginEvent();
    if (scrollBeginEvent) {
        scrollableEvent->SetScrollBeginCallback(std::move(scrollBeginEvent));
    }
    auto scrollFrameBeginEvent = eventHub->GetScrollFrameBeginEvent();
    if (scrollFrameBeginEvent) {
        scrollableEvent->SetScrollFrameBeginCallback(std::move(scrollFrameBeginEvent));
    }
}

bool ScrollPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto layoutAlgorithm = DynamicCast<ScrollLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithm, false);
    currentOffset_ = layoutAlgorithm->GetCurrentOffset();
    scrollableDistance_ = layoutAlgorithm->GetScrollableDistance();
    viewPortLength_ = layoutAlgorithm->GetViewPort();
    viewPort_ = layoutAlgorithm->GetViewPortSize();
    viewPortExtent_ = layoutAlgorithm->GetViewPortExtent();
    UpdateScrollBarOffset();
    if (config.frameSizeChange) {
        if (GetScrollBar() != nullptr) {
            GetScrollBar()->OnScrollEnd();
        }
    }
    if (scrollStop_) {
        FireOnScrollStop();
        scrollStop_ = false;
    }
    return false;
}

void ScrollPattern::FireOnScrollStart()
{
    if (scrollAbort_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<ScrollEventHub>();
    CHECK_NULL_VOID_NOLOG(hub);
    auto onScrollStart = hub->GetScrollStartEvent();
    CHECK_NULL_VOID_NOLOG(onScrollStart);
    onScrollStart();
}

void ScrollPattern::FireOnScrollStop()
{
    if (scrollAbort_) {
        scrollAbort_ = false;
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<ScrollEventHub>();
    CHECK_NULL_VOID_NOLOG(hub);
    auto onScrollStop = hub->GetScrollStopEvent();
    CHECK_NULL_VOID_NOLOG(onScrollStop);
    onScrollStop();
}

bool ScrollPattern::OnScrollCallback(float offset, int32_t source)
{
    if (source != SCROLL_FROM_START) {
        if (GetAxis() == Axis::NONE) {
            return false;
        }
        if (animator_ && !animator_->IsStopped()) {
            return false;
        }
        auto adjustOffset = static_cast<float>(offset);
        auto scrollBar = GetScrollBar();
        if (scrollBar && scrollBar->IsDriving()) {
            adjustOffset = scrollBar->CalcPatternOffset(adjustOffset);
            source = SCROLL_FROM_BAR;
        }
        AdjustOffset(adjustOffset, source);
        return UpdateCurrentOffset(adjustOffset, source);
    } else {
        if (animator_ && !animator_->IsStopped()) {
            scrollAbort_ = true;
            animator_->Stop();
        }
        FireOnScrollStart();
    }
    return true;
}

void ScrollPattern::OnScrollEndCallback()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<ScrollEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto scrollEndEvent = eventHub->GetScrollEndEvent();
    if (scrollEndEvent) {
        scrollEndEvent();
    }
    scrollStop_ = true;
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void ScrollPattern::ResetPosition()
{
    currentOffset_ = 0.0f;
    lastOffset_ = 0.0f;
}

bool ScrollPattern::IsAtTop() const
{
    return GreatOrEqual(currentOffset_, 0.0);
}

bool ScrollPattern::IsAtBottom() const
{
    bool atBottom = LessOrEqual(currentOffset_, -scrollableDistance_);
    // TODO: ignore ReachMaxCount
    return atBottom;
}

bool ScrollPattern::ScrollPageCheck(float delta, int32_t source)
{
    return true;
}

void ScrollPattern::HandleScrollBarOutBoundary()
{
    auto scrollBar = GetScrollBar();
    if (scrollBar && scrollBar->NeedScrollBar()) {
        scrollBar->SetOutBoundary(std::abs(GetScrollBarOutBoundaryExtent()));
    }
}

void ScrollPattern::AdjustOffset(float& delta, int32_t source)
{
    if (NearZero(delta) || NearZero(viewPortLength_) || source == SCROLL_FROM_ANIMATION ||
        source == SCROLL_FROM_ANIMATION_SPRING) {
        return;
    }
    // the distance above the top, if lower than top, it is zero
    float overScrollPastStart = 0.0f;
    // the distance below the bottom, if higher than bottom, it is zero
    float overScrollPastEnd = 0.0f;
    float overScrollPast = 0.0f;
    // TODO: not consider rowReverse or colReverse
    overScrollPastStart = std::max(currentOffset_, 0.0f);
    overScrollPastEnd = std::max(-scrollableDistance_ - currentOffset_, 0.0f);
    // do not adjust offset if direction opposite from the overScroll direction when out of boundary
    if ((overScrollPastStart > 0.0f && delta < 0.0f) || (overScrollPastEnd > 0.0f && delta > 0.0f)) {
        return;
    }
    overScrollPast = std::max(overScrollPastStart, overScrollPastEnd);
    if (overScrollPast == 0.0f) {
        return;
    }
    float friction = CalculateFriction((overScrollPast - std::abs(delta)) / viewPortLength_);
    float direction = delta > 0.0f ? 1.0f : -1.0f;
    delta = direction * CalculateOffsetByFriction(overScrollPast, std::abs(delta), friction);
}

void ScrollPattern::ValidateOffset(int32_t source)
{
    if (scrollableDistance_ <= 0.0f) {
        return;
    }

    // restrict position between top and bottom
    if (IsRestrictBoundary() || source == SCROLL_FROM_JUMP ||
        source == SCROLL_FROM_BAR || source == SCROLL_FROM_ROTATE) {
        if (GetAxis() == Axis::HORIZONTAL) {
            if (IsRowReverse()) {
                currentOffset_ = std::clamp(currentOffset_, 0.0f, scrollableDistance_);
            } else {
                currentOffset_ = std::clamp(currentOffset_, -scrollableDistance_, 0.0f);
            }
        } else {
            currentOffset_ = std::clamp(currentOffset_, -scrollableDistance_, 0.0f);
        }
    } else {
        if (currentOffset_ > 0) {
            SetScrollBarOutBoundaryExtent(currentOffset_);
        } else if ((-currentOffset_) >= (GetMainSize(viewPortExtent_) - GetMainSize(viewPort_)) &&
            ReachMaxCount()) {
            SetScrollBarOutBoundaryExtent((-currentOffset_) - (GetMainSize(viewPortExtent_) - GetMainSize(viewPort_)));
        }
        HandleScrollBarOutBoundary();
    }
}

void ScrollPattern::HandleScrollPosition(float scroll, int32_t scrollState)
{
    auto scrollableEvent = GetScrollableEvent();
    CHECK_NULL_VOID_NOLOG(scrollableEvent);
    const auto& onScroll = scrollableEvent->GetOnScrollCallback();
    CHECK_NULL_VOID_NOLOG(onScroll);
    // not consider async call
    Dimension scrollX(0, DimensionUnit::VP);
    Dimension scrollY(0, DimensionUnit::VP);
    Dimension scrollPx(scroll, DimensionUnit::PX);
    auto scrollVpValue = scrollPx.ConvertToVp();
    if (GetAxis() == Axis::HORIZONTAL) {
        scrollX.SetValue(scrollVpValue);
    } else {
        scrollY.SetValue(scrollVpValue);
    }
    onScroll(scrollX, scrollY);
}

bool ScrollPattern::IsCrashTop() const
{
    bool scrollUpToReachTop = LessNotEqual(lastOffset_, 0.0) && GreatOrEqual(currentOffset_, 0.0);
    bool scrollDownToReachTop = GreatNotEqual(lastOffset_, 0.0) && LessOrEqual(currentOffset_, 0.0);
    return scrollUpToReachTop || scrollDownToReachTop;
}

bool ScrollPattern::IsCrashBottom() const
{
    float minExtent = -scrollableDistance_;
    bool scrollDownToReachEnd = GreatNotEqual(lastOffset_, minExtent) && LessOrEqual(currentOffset_, minExtent);
    bool scrollUpToReachEnd = LessNotEqual(lastOffset_, minExtent) && GreatOrEqual(currentOffset_, minExtent);
    return (scrollUpToReachEnd || scrollDownToReachEnd) && ReachMaxCount();
}

void ScrollPattern::HandleCrashTop() const
{
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ScrollEventHub>();
    CHECK_NULL_VOID(eventHub);
    const auto& onScrollEdge = eventHub->GetScrollEdgeEvent();
    CHECK_NULL_VOID_NOLOG(onScrollEdge);
    // not consider async call
    if (GetAxis() == Axis::HORIZONTAL) {
        onScrollEdge(ScrollEdge::LEFT);
        return;
    }
    onScrollEdge(ScrollEdge::TOP);
}

void ScrollPattern::HandleCrashBottom() const
{
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ScrollEventHub>();
    CHECK_NULL_VOID(eventHub);
    const auto& onScrollEdge = eventHub->GetScrollEdgeEvent();
    CHECK_NULL_VOID_NOLOG(onScrollEdge);
    if (GetAxis() == Axis::HORIZONTAL) {
        onScrollEdge(ScrollEdge::RIGHT);
        return;
    }
    onScrollEdge(ScrollEdge::BOTTOM);
}

bool ScrollPattern::UpdateCurrentOffset(float delta, int32_t source)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    if (NearZero(delta)) {
        return false;
    }
    // TODO: ignore handle refresh
    if (!HandleEdgeEffect(delta, source, viewPort_)) {
        return false;
    }
    // TODO: scrollBar effect!!
    lastOffset_ = currentOffset_;
    currentOffset_ += delta;
    ValidateOffset(source);
    int32_t touchState = SCROLL_NONE;
    if (source == SCROLL_FROM_UPDATE) {
        touchState = SCROLL_TOUCH_DOWN;
    } else if (source == SCROLL_FROM_ANIMATION || source == SCROLL_FROM_ANIMATION_SPRING) {
        touchState = SCROLL_TOUCH_UP;
    }
    HandleScrollPosition(-delta, touchState);
    if (IsCrashTop()) {
        HandleCrashTop();
    } else if (IsCrashBottom()) {
        HandleCrashBottom();
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    return true;
}

void ScrollPattern::CreateOrStopAnimator()
{
    if (!animator_) {
        animator_ = AceType::MakeRefPtr<Animator>(PipelineBase::GetCurrentContext());
        return;
    }
    if (!animator_->IsStopped()) {
        scrollAbort_ = true;
        animator_->Stop();
    }
    animator_->ClearInterpolators();
}

void ScrollPattern::AnimateTo(float position, float duration, const RefPtr<Curve>& curve, bool limitDuration,
    const std::function<void()>& onFinish)
{
    LOGD("scroll pattern, from %{public}f to %{public}f", currentOffset_, position);
    if (!IsScrollableStopped()) {
        scrollAbort_ = true;
        StopScrollable();
    }
    CreateOrStopAnimator();
    // TODO: no accessibility event
    auto animation = AceType::MakeRefPtr<CurveAnimation<float>>(currentOffset_, position, curve);
    animation->AddListener([weakScroll = AceType::WeakClaim(this)](float value) {
        auto scroll = weakScroll.Upgrade();
        CHECK_NULL_VOID_NOLOG(scroll);
        scroll->DoJump(value);
    });
    animator_->AddInterpolator(animation);
    animator_->SetDuration(static_cast<int32_t>(limitDuration ? std::min(duration, SCROLL_MAX_TIME) : duration));
    animator_->ClearStopListeners();
    animator_->Play();
    // TODO: expand stop listener
    animator_->AddStopListener([onFinish, weak = AceType::WeakClaim(this)]() {
        auto scroll = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(scroll);
        scroll->scrollStop_ = true;
        auto host = scroll->GetHost();
        CHECK_NULL_VOID_NOLOG(host);
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        CHECK_NULL_VOID_NOLOG(onFinish);
        onFinish();
    });
    FireOnScrollStart();
}

void ScrollPattern::ScrollToEdge(ScrollEdgeType scrollEdgeType, bool smooth)
{
    if (scrollEdgeType == ScrollEdgeType::SCROLL_NONE) {
        return;
    }
    float distance = scrollEdgeType == ScrollEdgeType::SCROLL_TOP ? -currentOffset_ :
        (-scrollableDistance_ - currentOffset_);
    ScrollBy(distance, distance, smooth);
}

void ScrollPattern::ScrollBy(float pixelX, float pixelY, bool smooth, const std::function<void()>& onFinish)
{
    float distance = (GetAxis() == Axis::VERTICAL) ? pixelY : pixelX;
    if (NearZero(distance)) {
        return;
    }
    float position = currentOffset_ + distance;
    if (smooth) {
        AnimateTo(position, fabs(distance) * UNIT_CONVERT / SCROLL_BY_SPEED, Curves::EASE_OUT, true, onFinish);
        return;
    }
    JumpToPosition(position);
}

bool ScrollPattern::ScrollPage(bool reverse, bool smooth, const std::function<void()>& onFinish)
{
    float distance = reverse ? viewPortLength_ : -viewPortLength_;
    ScrollBy(distance, distance, smooth, onFinish);
    return true;
}

void ScrollPattern::JumpToPosition(float position, int32_t source)
{
    // If an animation is playing, stop it.
    if (animator_) {
        if (!animator_->IsStopped()) {
            animator_->Stop();
        }
        animator_->ClearInterpolators();
    }
    DoJump(position, source);
}

void ScrollPattern::DoJump(float position, int32_t source)
{
    float setPosition = (GetAxis() == Axis::HORIZONTAL && IsRowReverse()) ? -position : position;
    if (!NearEqual(currentOffset_, setPosition)) {
        UpdateCurrentOffset(setPosition - currentOffset_, source);
    }
}

void ScrollPattern::SetEdgeEffectCallback(const RefPtr<ScrollEdgeEffect>& scrollEffect)
{
    scrollEffect->SetCurrentPositionCallback([weakScroll = AceType::WeakClaim(this)]() -> double {
        auto scroll = weakScroll.Upgrade();
        CHECK_NULL_RETURN_NOLOG(scroll, 0.0);
        return scroll->GetCurrentPosition();
    });
    scrollEffect->SetLeadingCallback([weakScroll = AceType::WeakClaim(this)]() -> double {
        auto scroll = weakScroll.Upgrade();
        if (scroll && !scroll->IsRowReverse() && !scroll->IsColReverse() && scroll->GetScrollableDistance() > 0) {
            return -scroll->GetScrollableDistance();
        }
        return 0.0;
    });
    scrollEffect->SetTrailingCallback([weakScroll = AceType::WeakClaim(this)]() -> double {
        auto scroll = weakScroll.Upgrade();
        if (scroll && (scroll->IsRowReverse() || scroll->IsColReverse())) {
            return scroll->GetScrollableDistance();
        }
        return 0.0;
    });
    scrollEffect->SetInitLeadingCallback([weakScroll = AceType::WeakClaim(this)]() -> double {
        auto scroll = weakScroll.Upgrade();
        if (scroll && !scroll->IsRowReverse() && !scroll->IsColReverse() && scroll->GetScrollableDistance() > 0) {
            return -scroll->GetScrollableDistance();
        }
        return 0.0;
    });
    scrollEffect->SetInitTrailingCallback([weakScroll = AceType::WeakClaim(this)]() -> double {
        auto scroll = weakScroll.Upgrade();
        if (scroll && (scroll->IsRowReverse() || scroll->IsColReverse())) {
            return scroll->GetScrollableDistance();
        }
        return 0.0;
    });
}

void ScrollPattern::UpdateScrollBarOffset()
{
    if (!GetScrollBar() && !GetScrollBarProxy()) {
        return;
    }
    Size size(viewPort_.Width(), viewPort_.Height());
    auto estimatedHeight = (GetAxis() == Axis::HORIZONTAL) ? viewPortExtent_.Width() : viewPortExtent_.Height();
    UpdateScrollBarRegion(-currentOffset_, estimatedHeight, size);
}

} // namespace OHOS::Ace::NG
