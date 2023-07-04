/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "base/geometry/axis.h"
#include "base/geometry/point.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components/scroll/scrollable.h"
#include "core/components_ng/pattern/scroll/effect/scroll_fade_effect.h"
#include "core/components_ng/pattern/scroll/scroll_spring_effect.h"

namespace OHOS::Ace::NG {
void ScrollablePattern::SetAxis(Axis axis)
{
    if (axis_ == axis) {
        return;
    }
    axis_ = axis;
    if (scrollBar_) {
        scrollBar_->SetPositionMode(axis_ == Axis::HORIZONTAL ? PositionMode::BOTTOM : PositionMode::RIGHT);
    }
    auto gestureHub = GetGestureHub();
    CHECK_NULL_VOID(gestureHub);
    if (scrollableEvent_) {
        gestureHub->RemoveScrollableEvent(scrollableEvent_);
        scrollableEvent_->SetAxis(axis);
        gestureHub->AddScrollableEvent(scrollableEvent_);
    }
    if (scrollEffect_) {
        gestureHub->RemoveScrollEdgeEffect(scrollEffect_);
        gestureHub->AddScrollEdgeEffect(GetAxis(), scrollEffect_);
    }
}

const RefPtr<GestureEventHub>& ScrollablePattern::GetGestureHub()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(hub, nullptr);
    return hub->GetOrCreateGestureEventHub();
}

const RefPtr<InputEventHub>& ScrollablePattern::GetInputHub()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_RETURN(host, nullptr);
    return hub->GetOrCreateInputEventHub();
}

bool ScrollablePattern::OnScrollCallback(float offset, int32_t source)
{
    if (source == SCROLL_FROM_START) {
        return true;
    }
    if (scrollBar_ && scrollBar_->IsDriving()) {
        offset = scrollBar_->CalcPatternOffset(offset);
        source = SCROLL_FROM_BAR;
    }
    return UpdateCurrentOffset(offset, source);
}

bool ScrollablePattern::OnScrollPosition(double offset, int32_t source)
{
    if (coordinationEvent_ && isReactInParentMovement_) {
        auto onScroll = coordinationEvent_->GetOnScroll();
        if (onScroll) {
            onScroll(offset);
            return false;
        }
    }
    auto isAtTop = (IsAtTop() && Positive(offset));
    if (isAtTop && source == SCROLL_FROM_UPDATE && !isReactInParentMovement_ && (axis_ == Axis::VERTICAL)) {
        isReactInParentMovement_ = true;
        if (coordinationEvent_) {
            auto onScrollStart = coordinationEvent_->GetOnScrollStartEvent();
            if (onScrollStart) {
                onScrollStart();
            }
        }
    }
    if (source == SCROLL_FROM_START) {
        if (scrollBarProxy_) {
            scrollBarProxy_->StopScrollBarAnimator();
        }
    }
    return true;
}

void ScrollablePattern::OnScrollEnd()
{
    if (coordinationEvent_ && isReactInParentMovement_) {
        isReactInParentMovement_ = false;
        auto onScrollEnd = coordinationEvent_->GetOnScrollEndEvent();
        if (onScrollEnd) {
            onScrollEnd();
            return;
        }
    }
    if (scrollBar_) {
        scrollBar_->SetDriving(false);
        scrollBar_->OnScrollEnd();
    }
    if (scrollBarProxy_) {
        scrollBarProxy_->StartScrollBarAnimator();
    }
}

void ScrollablePattern::AddScrollEvent()
{
    auto gestureHub = GetGestureHub();
    CHECK_NULL_VOID(gestureHub);
    if (scrollableEvent_) {
        gestureHub->RemoveScrollableEvent(scrollableEvent_);
    }
    scrollableEvent_ = MakeRefPtr<ScrollableEvent>(GetAxis());
    auto scrollCallback = [weak = WeakClaim(this)](double offset, int32_t source) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        if (!pattern->OnScrollPosition(offset, source)) {
            return false;
        }
        return pattern->OnScrollCallback(static_cast<float>(offset), source);
    };
    scrollableEvent_->SetScrollPositionCallback(std::move(scrollCallback));
    auto scrollEnd = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnScrollEnd();
        pattern->OnScrollEndCallback();
    };
    scrollableEvent_->SetScrollEndCallback(std::move(scrollEnd));
    gestureHub->AddScrollableEvent(scrollableEvent_);
}

void ScrollablePattern::SetEdgeEffect(EdgeEffect edgeEffect)
{
    auto gestureHub = GetGestureHub();
    CHECK_NULL_VOID(gestureHub);
    if (scrollEffect_ && (edgeEffect != scrollEffect_->GetEdgeEffect())) {
        gestureHub->RemoveScrollEdgeEffect(scrollEffect_);
        scrollEffect_.Reset();
    }
    if (edgeEffect == EdgeEffect::SPRING && !scrollEffect_) {
        auto springEffect = AceType::MakeRefPtr<ScrollSpringEffect>();
        CHECK_NULL_VOID(springEffect);
        springEffect->SetOutBoundaryCallback([weak = AceType::WeakClaim(this)]() {
            auto pattern = weak.Upgrade();
            CHECK_NULL_RETURN_NOLOG(pattern, false);
            return pattern->IsAtTop() || pattern->IsAtBottom();
        });
        // add callback to springEdgeEffect
        SetEdgeEffectCallback(springEffect);
        scrollEffect_ = springEffect;
        gestureHub->AddScrollEdgeEffect(GetAxis(), scrollEffect_);
    }
    if (edgeEffect == EdgeEffect::FADE && !scrollEffect_) {
        auto fadeEdgeEffect = AceType::MakeRefPtr<ScrollFadeEffect>(Color::GRAY);
        CHECK_NULL_VOID(fadeEdgeEffect);
        fadeEdgeEffect->SetHandleOverScrollCallback([weakScroll = AceType::WeakClaim(this)]() -> void {
            auto list = weakScroll.Upgrade();
            CHECK_NULL_VOID_NOLOG(list);
            auto host = list->GetHost();
            CHECK_NULL_VOID_NOLOG(host);
            host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        });
        SetEdgeEffectCallback(fadeEdgeEffect);
        fadeEdgeEffect->InitialEdgeEffect();
        scrollEffect_ = fadeEdgeEffect;
        gestureHub->AddScrollEdgeEffect(GetAxis(), scrollEffect_);
    }
}

bool ScrollablePattern::HandleEdgeEffect(float offset, int32_t source, const SizeF& size)
{
    // check edgeEffect is not springEffect
    if (scrollEffect_ && scrollEffect_->IsFadeEffect() && source != SCROLL_FROM_BAR) {    // handle edge effect
        if ((IsAtTop() && Positive(offset)) || (IsAtBottom() && Negative(offset))) {
            scrollEffect_->HandleOverScroll(GetAxis(), -offset, size);
        }
    }
    if (!(scrollEffect_ && scrollEffect_->IsSpringEffect()) ||
        source == SCROLL_FROM_BAR || source == SCROLL_FROM_JUMP) {
        if (IsAtTop() && Positive(offset)) {
            return false;
        }
        if (IsAtBottom() && Negative(offset)) {
            return false;
        }
    }
    return true;
}

void ScrollablePattern::RegisterScrollBarEventTask()
{
    CHECK_NULL_VOID(scrollBar_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = GetGestureHub();
    auto inputHub = GetInputHub();
    CHECK_NULL_VOID(gestureHub);
    CHECK_NULL_VOID(inputHub);
    scrollBar_->SetGestureEvent();
    scrollBar_->SetMouseEvent();
    scrollBar_->SetMarkNeedRenderFunc([weak = AceType::WeakClaim(AceType::RawPtr(host))]() {
        auto host = weak.Upgrade();
        CHECK_NULL_VOID(host);
        host->MarkNeedRenderOnly();
    });
    gestureHub->AddTouchEvent(scrollBar_->GetTouchEvent());
    inputHub->AddOnMouseEvent(scrollBar_->GetMouseEvent());
}

void ScrollablePattern::SetScrollBar(DisplayMode displayMode)
{
    if (displayMode == DisplayMode::OFF) {
        if (scrollBar_) {
            auto gestureHub = GetGestureHub();
            if (gestureHub) {
                gestureHub->RemoveTouchEvent(scrollBar_->GetTouchEvent());
            }
            scrollBar_->MarkNeedRender();
            scrollBar_.Reset();
        }
    } else if (!scrollBar_) {
        scrollBar_ = AceType::MakeRefPtr<ScrollBar>(displayMode);
        // set the scroll bar style
        if (GetAxis() == Axis::HORIZONTAL) {
            scrollBar_->SetPositionMode(PositionMode::BOTTOM);
        }
        RegisterScrollBarEventTask();
        if (displayMode == DisplayMode::AUTO) {
            scrollBar_->OnScrollEnd();
        }
    } else if (scrollBar_->GetDisplayMode() != displayMode) {
        scrollBar_->SetDisplayMode(displayMode);
    } else {
        return;
    }
    if (scrollBar_) {
        UpdateScrollBarOffset();
    }
}

void ScrollablePattern::SetScrollBar(const std::unique_ptr<ScrollBarProperty>& property)
{
    if (!property) {
        SetScrollBar(DisplayMode::AUTO);
        return;
    }
    auto displayMode = property->GetScrollBarMode().value_or(DisplayMode::AUTO);
    SetScrollBar(displayMode);
    if (scrollBar_) {
        auto barColor = property->GetScrollBarColor();
        if (barColor) {
            scrollBar_->SetForegroundColor(barColor.value());
        }
        auto barWidth = property->GetScrollBarWidth();
        if (barWidth) {
            scrollBar_->SetInactiveWidth(barWidth.value());
            scrollBar_->SetNormalWidth(barWidth.value());
            scrollBar_->SetActiveWidth(barWidth.value());
            scrollBar_->SetTouchWidth(barWidth.value());
        }
    }
}

void ScrollablePattern::UpdateScrollBarRegion(float offset, float estimatedHeight, Size viewPort)
{
    // inner scrollbar
    if (scrollBar_) {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto layoutPriority = host->GetLayoutProperty();
        CHECK_NULL_VOID(layoutPriority);
        auto paddingOffset = layoutPriority->CreatePaddingAndBorder().Offset();
        auto mainSize = axis_ == Axis::VERTICAL ? viewPort.Height() : viewPort.Width();
        bool scrollable = GreatNotEqual(estimatedHeight, mainSize);
        scrollBar_->SetScrollable(IsScrollable() && scrollable);
        Offset scrollOffset = { offset, offset }; // fit for w/h switched.
        Offset viewOffset = { paddingOffset.GetX(), paddingOffset.GetY() };
        scrollBar_->UpdateScrollBarRegion(viewOffset, viewPort, scrollOffset, estimatedHeight);
        scrollBar_->MarkNeedRender();
    }

    // outer scrollbar
    if (scrollBarProxy_) {
        estimatedHeight_ = estimatedHeight - (GetAxis() == Axis::VERTICAL ? viewPort.Height() : viewPort.Width());
        barOffset_ = -offset;
        scrollBarProxy_->NotifyScrollBar(AceType::WeakClaim(this));
    }
}

void ScrollablePattern::SetScrollBarProxy(const RefPtr<ScrollBarProxy>& scrollBarProxy)
{
    CHECK_NULL_VOID(scrollBarProxy);
    auto scrollFunction = [weak = WeakClaim(this)](double offset, int32_t source) {
        if (source != SCROLL_FROM_START) {
            auto pattern = weak.Upgrade();
            if (!pattern || pattern->GetAxis() == Axis::NONE) {
                return false;
            }
            return pattern->UpdateCurrentOffset(offset, SCROLL_FROM_BAR);
        }
        return true;
    };
    ScrollableNodeInfo nodeInfo = { AceType::WeakClaim(this), std::move(scrollFunction) };
    scrollBarProxy->RegisterScrollableNode(nodeInfo);
    scrollBarProxy_ = scrollBarProxy;
}
} // namespace OHOS::Ace::NG