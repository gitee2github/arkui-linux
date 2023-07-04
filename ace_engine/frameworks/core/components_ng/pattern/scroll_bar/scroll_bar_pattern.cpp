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

#include "core/components_ng/pattern/scroll_bar/scroll_bar_pattern.h"

#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t STOP_DURATION = 2000; // 2000ms
constexpr float KEY_TIME_START = 0.0f;
constexpr float KEY_TIME_MIDDLE = 0.7f;
constexpr float KEY_TIME_END = 1.0f;
} // namespace

void ScrollBarPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    host->GetRenderContext()->SetClipToFrame(true);
}

void ScrollBarPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<ScrollBarLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);

    auto oldDisplayMode = displayMode_;
    displayMode_ = layoutProperty->GetDisplayMode().value_or(DisplayMode::AUTO);
    if (scrollBarProxy_ &&
        (!scrollEndAnimator_ || (oldDisplayMode != displayMode_ && displayMode_ == DisplayMode::AUTO))) {
        scrollBarProxy_->StartScrollBarAnimator();
    }
    if (displayMode_ == DisplayMode::ON) {
        SetOpacity(UINT8_MAX);
    }

    auto axis = layoutProperty->GetAxis().value_or(Axis::VERTICAL);
    if (axis_ == axis && scrollableEvent_) {
        LOGD("Direction not changed, need't resister scroll event again.");
        return;
    }

    axis_ = axis;
    // scrollPosition callback
    auto offsetTask = [weak = WeakClaim(this)](double offset, int32_t source) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        if (source == SCROLL_FROM_START) {
            pattern->StopAnimator();
            return true;
        }
        return pattern->UpdateCurrentOffset(offset, source);
    };
    auto scrollEndTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (pattern->GetDisplayMode() == DisplayMode::AUTO) {
            pattern->StartAnimator();
        }
    };

    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    if (scrollableEvent_) {
        gestureHub->RemoveScrollableEvent(scrollableEvent_);
    }
    scrollableEvent_ = MakeRefPtr<ScrollableEvent>(axis);
    scrollableEvent_->SetScrollPositionCallback(std::move(offsetTask));
    scrollableEvent_->SetScrollEndCallback(std::move(scrollEndTask));
    gestureHub->AddScrollableEvent(scrollableEvent_);
}

bool ScrollBarPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }

    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto layoutAlgorithm = DynamicCast<ScrollBarLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithm, false);
    scrollableDistance_ = layoutAlgorithm->GetScrollableDistance();
    return false;
}

bool ScrollBarPattern::IsAtTop() const
{
    return LessOrEqual(currentOffset_, 0.0);
}

bool ScrollBarPattern::IsAtBottom() const
{
    return GreatOrEqual(currentOffset_, scrollableDistance_);
}

void ScrollBarPattern::ValidateOffset(int32_t source)
{
    if (scrollableDistance_ <= 0.0f) {
        return;
    }
    currentOffset_ = std::clamp(currentOffset_, 0.0f, scrollableDistance_);
}

bool ScrollBarPattern::UpdateCurrentOffset(float delta, int32_t source)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    if (NearZero(delta) || axis_ == Axis::NONE) {
        return false;
    }
    if ((IsAtBottom() && delta > 0.0f) || (IsAtTop() && delta < 0.0f)) {
        return false;
    }

    lastOffset_ = currentOffset_;
    currentOffset_ += delta;
    ValidateOffset(source);
    if (scrollBarProxy_ && lastOffset_ != currentOffset_) {
        scrollBarProxy_->NotifyScrollableNode(-delta, AceType::WeakClaim(this));
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    return true;
}

void ScrollBarPattern::StartAnimator()
{
    if (scrollEndAnimator_ && !scrollEndAnimator_->IsStopped()) {
        scrollEndAnimator_->Stop();
    }
    if (scrollEndAnimator_) {
        scrollEndAnimator_->Play();
        return;
    }

    scrollEndAnimator_ = AceType::MakeRefPtr<Animator>(PipelineContext::GetCurrentContext());
    auto hiddenStartKeyframe = AceType::MakeRefPtr<Keyframe<int32_t>>(KEY_TIME_START, UINT8_MAX);
    auto hiddenMiddleKeyframe = AceType::MakeRefPtr<Keyframe<int32_t>>(KEY_TIME_MIDDLE, UINT8_MAX);
    auto hiddenEndKeyframe = AceType::MakeRefPtr<Keyframe<int32_t>>(KEY_TIME_END, 0);
    hiddenMiddleKeyframe->SetCurve(Curves::LINEAR);
    hiddenEndKeyframe->SetCurve(Curves::FRICTION);

    auto animation = AceType::MakeRefPtr<KeyframeAnimation<int32_t>>();
    animation->AddKeyframe(hiddenStartKeyframe);
    animation->AddKeyframe(hiddenMiddleKeyframe);
    animation->AddKeyframe(hiddenEndKeyframe);
    animation->AddListener([weakBar = AceType::WeakClaim(this)](int32_t value) {
        auto scrollBar = weakBar.Upgrade();
        CHECK_NULL_VOID(scrollBar);
        scrollBar->SetOpacity(value);
    });
    scrollEndAnimator_->AddInterpolator(animation);
    scrollEndAnimator_->SetDuration(STOP_DURATION);
    scrollEndAnimator_->Play();
}

void ScrollBarPattern::StopAnimator()
{
    if (scrollEndAnimator_ && !scrollEndAnimator_->IsStopped()) {
        scrollEndAnimator_->Stop();
    }
    SetOpacity(UINT8_MAX);
}

void ScrollBarPattern::SetOpacity(uint8_t value)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateOpacity(static_cast<double>(value) / UINT8_MAX);
    host->MarkNeedRenderOnly();
}

} // namespace OHOS::Ace::NG
