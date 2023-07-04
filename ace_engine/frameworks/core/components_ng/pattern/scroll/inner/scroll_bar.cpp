/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/scroll/inner/scroll_bar.h"

#include "base/utils/utils.h"
#include "core/animation/curve_animation.h"
#include "core/animation/curves.h"
#include "core/components/scroll/scroll_bar_theme.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t STOP_DURATION = 2000; // 2000ms
constexpr float KEY_TIME_START = 0.0f;
constexpr float KEY_TIME_MIDDLE = 0.7f;
constexpr float KEY_TIME_END = 1.0f;
constexpr int32_t BAR_EXPAND_DURATION = 150; // 150ms, scroll bar width expands from 4dp to 8dp
constexpr int32_t BAR_SHRINK_DURATION = 250; // 250ms, scroll bar width shrinks from 8dp to 4dp
constexpr int32_t BAR_ADAPT_DURATION = 400;  // 400ms, scroll bar adapts to the size changes of components
constexpr double BAR_ADAPT_EPSLION = 1.0;
} // namespace

ScrollBar::ScrollBar()
{
    InitTheme();
}

ScrollBar::ScrollBar(DisplayMode displayMode, ShapeMode shapeMode, PositionMode positionMode)
    : displayMode_(displayMode), shapeMode_(shapeMode), positionMode_(positionMode)
{
    InitTheme();
}

void ScrollBar::InitTheme()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto theme = pipelineContext->GetTheme<ScrollBarTheme>();
    CHECK_NULL_VOID(theme);
    SetInactiveWidth(theme->GetNormalWidth());
    SetNormalWidth(theme->GetNormalWidth());
    SetActiveWidth(theme->GetActiveWidth());
    SetTouchWidth(theme->GetTouchWidth());
    SetReservedHeight(theme->GetReservedHeight());
    SetMinHeight(theme->GetMinHeight());
    SetMinDynamicHeight(theme->GetMinDynamicHeight());
    SetBackgroundColor(theme->GetBackgroundColor());
    SetForegroundColor(theme->GetForegroundColor());
    SetPadding(theme->GetPadding());
    SetScrollable(true);
}

bool ScrollBar::InBarTouchRegion(const Point& point) const
{
    if (NeedScrollBar() && shapeMode_ == ShapeMode::RECT) {
        return touchRegion_.IsInRegion(point);
    }
    return false;
}

bool ScrollBar::InBarActiveRegion(const Point& point) const
{
    if (NeedScrollBar() && shapeMode_ == ShapeMode::RECT) {
        return activeRect_.IsInRegion(point);
    }
    return false;
}

void ScrollBar::FlushBarWidth()
{
    SetBarRegion(paintOffset_, viewPortSize_);
    if (shapeMode_ == ShapeMode::RECT) {
        SetRectTrickRegion(paintOffset_, viewPortSize_, lastOffset_, estimatedHeight_);
    } else {
        SetRoundTrickRegion(paintOffset_, viewPortSize_, lastOffset_, estimatedHeight_);
    }
}

void ScrollBar::UpdateScrollBarRegion(
    const Offset& offset, const Size& size, const Offset& lastOffset, double estimatedHeight)
{
    // return if nothing changes to avoid changing opacity
    if (!positionModeUpdate_ && paintOffset_ == offset && viewPortSize_ == size && lastOffset_ == lastOffset &&
        NearEqual(estimatedHeight_, estimatedHeight, 0.000001f)) {
        return;
    }
    if (!NearZero(estimatedHeight)) {
        paintOffset_ = offset;
        viewPortSize_ = size;
        lastOffset_ = lastOffset;
        estimatedHeight_ = estimatedHeight;
        opacity_ = UINT8_MAX;
        SetBarRegion(offset, size);
        if (shapeMode_ == ShapeMode::RECT) {
            SetRectTrickRegion(offset, size, lastOffset, estimatedHeight);
        } else {
            SetRoundTrickRegion(offset, size, lastOffset, estimatedHeight);
        }
        positionModeUpdate_ = false;
    }
    OnScrollEnd();
}

void ScrollBar::UpdateActiveRectSize(double activeSize)
{
    if (positionMode_ == PositionMode::LEFT || positionMode_ == PositionMode::RIGHT) {
        activeRect_.SetHeight(activeSize);
        touchRegion_.SetHeight(activeSize);
    } else if (positionMode_ == PositionMode::BOTTOM) {
        activeRect_.SetWidth(activeSize);
        touchRegion_.SetWidth(activeSize);
    }
}

void ScrollBar::UpdateActiveRectOffset(double activeMainOffset)
{
    if (positionMode_ == PositionMode::LEFT || positionMode_ == PositionMode::RIGHT) {
        activeMainOffset = std::min(activeMainOffset, barRegionSize_ - activeRect_.Height());
        activeRect_.SetTop(activeMainOffset);
        touchRegion_.SetTop(activeMainOffset);
    } else if (positionMode_ == PositionMode::BOTTOM) {
        activeMainOffset = std::min(activeMainOffset, barRegionSize_ - activeRect_.Width());
        activeRect_.SetLeft(activeMainOffset);
        touchRegion_.SetLeft(activeMainOffset);
    }
}

void ScrollBar::SetBarRegion(const Offset& offset, const Size& size)
{
    double normalWidth = NormalizeToPx(normalWidth_);
    if (shapeMode_ == ShapeMode::RECT) {
        double height = std::max(size.Height() - NormalizeToPx(reservedHeight_), 0.0);
        if (positionMode_ == PositionMode::LEFT) {
            barRect_ = Rect(0.0, 0.0, normalWidth, height) + offset;
        } else if (positionMode_ == PositionMode::RIGHT) {
            barRect_ =
                Rect(size.Width() - normalWidth - NormalizeToPx(padding_.Right()), 0.0, normalWidth, height) + offset;
        } else if (positionMode_ == PositionMode::BOTTOM) {
            auto scrollBarWidth = std::max(size.Width() - NormalizeToPx(reservedHeight_), 0.0);
            barRect_ =
                Rect(0.0, size.Height() - normalWidth - NormalizeToPx(padding_.Bottom()), scrollBarWidth, normalWidth) +
                offset;
        }
    }
}

void ScrollBar::SetRectTrickRegion(
    const Offset& offset, const Size& size, const Offset& lastOffset, double estimatedHeight)
{
    double mainSize = (positionMode_ == PositionMode::BOTTOM ? size.Width() : size.Height());
    barRegionSize_ = std::max(mainSize - NormalizeToPx(reservedHeight_), 0.0);
    if (LessOrEqual(estimatedHeight, 0.0)) {
        return;
    }
    double activeSize = barRegionSize_ * mainSize / estimatedHeight - outBoundary_;
    if (!NearEqual(mainSize, estimatedHeight)) {
        if (!NearZero(outBoundary_)) {
            activeSize = std::max(
                std::max(activeSize, NormalizeToPx(minHeight_) - outBoundary_), NormalizeToPx(minDynamicHeight_));
        } else {
            activeSize = std::max(activeSize, NormalizeToPx(minHeight_));
        }
        double normalWidth = NormalizeToPx(normalWidth_);
        if (LessOrEqual(activeSize, normalWidth)) {
            activeSize = normalWidth;
        }
        double lastMainOffset =
            std::max(positionMode_ == PositionMode::BOTTOM ? lastOffset.GetX() : lastOffset.GetY(), 0.0);
        offsetScale_ = (barRegionSize_ - activeSize) / (estimatedHeight - mainSize);
        double activeMainOffset = offsetScale_ * lastMainOffset;
        bool canUseAnimation = !inSpring && !positionModeUpdate_;
        activeMainOffset = std::min(activeMainOffset, barRegionSize_ - activeSize);
        double inactiveSize = 0.0;
        double inactiveMainOffset = 0.0;
        scrollableOffset_ = activeMainOffset;
        if (positionMode_ == PositionMode::LEFT) {
            inactiveSize = activeRect_.Height();
            inactiveMainOffset = activeRect_.Top();
            if (adaptAnimator_ && adaptAnimator_->IsRunning()) {
                activeRect_ =
                    Rect(-NormalizeToPx(position_), activeRect_.Top(), normalWidth, activeRect_.Height()) + offset;
            } else {
                activeRect_ = Rect(-NormalizeToPx(position_), activeMainOffset, normalWidth, activeSize) + offset;
            }
            touchRegion_ = activeRect_ + Size(NormalizeToPx(touchWidth_), 0);
        } else if (positionMode_ == PositionMode::RIGHT) {
            inactiveSize = activeRect_.Height();
            inactiveMainOffset = activeRect_.Top();
            double x = size.Width() - normalWidth - NormalizeToPx(padding_.Right()) + NormalizeToPx(position_);
            if (adaptAnimator_ && adaptAnimator_->IsRunning()) {
                activeRect_ = Rect(x, activeRect_.Top(), normalWidth, activeRect_.Height()) + offset;
            } else {
                activeRect_ = Rect(x, activeMainOffset, normalWidth, activeSize) + offset;
            }
            // Update the hot region
            touchRegion_ =
                activeRect_ -
                Offset(
                    NormalizeToPx(touchWidth_) - NormalizeToPx(normalWidth_) - NormalizeToPx(padding_.Right()), 0.0) +
                Size(NormalizeToPx(touchWidth_) - NormalizeToPx(normalWidth_), 0);
        } else if (positionMode_ == PositionMode::BOTTOM) {
            inactiveSize = activeRect_.Width();
            inactiveMainOffset = activeRect_.Left();
            auto positionY = size.Height() - normalWidth - NormalizeToPx(padding_.Bottom()) + NormalizeToPx(position_);
            if (adaptAnimator_ && adaptAnimator_->IsRunning()) {
                activeRect_ = Rect(activeRect_.Left(), positionY, activeRect_.Width(), normalWidth) + offset;
            } else {
                activeRect_ = Rect(activeMainOffset, positionY, activeSize, normalWidth) + offset;
            }
            auto hotRegionOffset = Offset(
                0.0, NormalizeToPx(touchWidth_) - NormalizeToPx(normalWidth_) - NormalizeToPx(padding_.Bottom()));
            auto hotRegionSize = Size(0, NormalizeToPx(touchWidth_) - NormalizeToPx(normalWidth_));
            touchRegion_ = activeRect_ - hotRegionOffset + hotRegionSize;
        }
        // If the scrollBar length changes, start the adaptation animation
        if (!NearZero(inactiveSize) && !NearEqual(activeSize, inactiveSize, BAR_ADAPT_EPSLION) && canUseAnimation) {
            PlayAdaptAnimation(activeSize, activeMainOffset, inactiveSize, inactiveMainOffset);
        }
    }
}

void ScrollBar::SetRoundTrickRegion(
    const Offset& offset, const Size& size, const Offset& lastOffset, double estimatedHeight)
{
    double diameter = std::min(size.Width(), size.Height());
    if (!NearEqual(estimatedHeight, diameter)) {
        double maxAngle = bottomAngle_ - topAngle_;
        trickSweepAngle_ = std::max(diameter * maxAngle / estimatedHeight, minAngle_);
        double lastOffsetY = std::max(lastOffset.GetY(), 0.0);
        double trickStartAngle = (maxAngle - trickSweepAngle_) * lastOffsetY / (estimatedHeight - diameter);
        trickStartAngle = std::clamp(0.0, trickStartAngle, maxAngle) - maxAngle * FACTOR_HALF;
        if (positionMode_ == PositionMode::LEFT) {
            if (trickStartAngle > 0.0) {
                trickStartAngle_ = STRAIGHT_ANGLE - trickStartAngle;
            } else {
                trickStartAngle_ = -(trickStartAngle + STRAIGHT_ANGLE);
            }
            trickSweepAngle_ = -trickSweepAngle_;
        } else {
            trickStartAngle_ = trickStartAngle;
        }
    }
}

bool ScrollBar::NeedScrollBar() const
{
    return displayMode_ == DisplayMode::AUTO || displayMode_ == DisplayMode::ON;
}

bool ScrollBar::NeedPaint() const
{
    return NeedScrollBar() && isScrollable_ && GreatNotEqual(normalWidth_.Value(), 0.0) && opacity_ > 0;
}

double ScrollBar::GetNormalWidthToPx() const
{
    return NormalizeToPx(normalWidth_);
}

float ScrollBar::CalcPatternOffset(float scrollBarOffset) const
{
    if (!isDriving_ || NearZero(offsetScale_)) {
        return scrollBarOffset;
    }
    return -scrollBarOffset / offsetScale_;
}

double ScrollBar::NormalizeToPx(const Dimension& dimension) const
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, 0.0);
    return pipelineContext->NormalizeToPx(dimension);
}

void ScrollBar::SetGestureEvent()
{
    if (!touchEvent_) {
        touchEvent_ = MakeRefPtr<TouchEventImpl>([weak = WeakClaim(this)](const TouchEventInfo& info) {
            auto scrollBar = weak.Upgrade();
            CHECK_NULL_VOID(scrollBar);
            if (info.GetTouches().empty()) {
                return;
            }
            auto touch = info.GetTouches().front();
            if (touch.GetTouchType() == TouchType::DOWN) {
                Point point(touch.GetLocalLocation().GetX(), touch.GetLocalLocation().GetY());
                bool inTouchRegion = scrollBar->InBarTouchRegion(point);
                scrollBar->SetPressed(inTouchRegion);
                scrollBar->SetDriving(inTouchRegion);
                if (inTouchRegion && !scrollBar->IsHover()) {
                    scrollBar->PlayGrowAnimation();
                }
                if (scrollBar->scrollEndAnimator_ && !scrollBar->scrollEndAnimator_->IsStopped()) {
                    scrollBar->scrollEndAnimator_->Stop();
                }
                scrollBar->MarkNeedRender();
            }
            if (info.GetTouches().front().GetTouchType() == TouchType::UP) {
                if (scrollBar->IsPressed() && !scrollBar->IsHover()) {
                    scrollBar->PlayShrinkAnimation();
                }
                scrollBar->SetPressed(false);
                scrollBar->MarkNeedRender();
            }
        });
    }
    if (!touchAnimator_) {
        touchAnimator_ = AceType::MakeRefPtr<Animator>(PipelineContext::GetCurrentContext());
    }
}

void ScrollBar::SetMouseEvent()
{
    if (mouseEvent_) {
        return;
    }
    mouseEvent_ = MakeRefPtr<InputEvent>([weak = WeakClaim(this)](MouseInfo& info) {
        auto scrollBar = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(scrollBar);
        Point point(info.GetLocalLocation().GetX(), info.GetLocalLocation().GetY());
        bool inRegion = scrollBar->InBarActiveRegion(point);
        if (inRegion && !scrollBar->IsHover()) {
            if (!scrollBar->IsPressed()) {
                scrollBar->PlayGrowAnimation();
            }
            scrollBar->SetHover(true);
            if (scrollBar->scrollEndAnimator_ && !scrollBar->scrollEndAnimator_->IsStopped()) {
                scrollBar->scrollEndAnimator_->Stop();
            }
            scrollBar->MarkNeedRender();
        }
        if (scrollBar->IsHover() && !inRegion) {
            if (!scrollBar->IsPressed()) {
                scrollBar->PlayShrinkAnimation();
                if (scrollBar->GetDisplayMode() == DisplayMode::AUTO) {
                    scrollBar->PlayBarEndAnimation();
                }
            }
            scrollBar->SetHover(false);
            scrollBar->MarkNeedRender();
        }
    });
}

void ScrollBar::PlayAdaptAnimation(
    double activeSize, double activeMainOffset, double inactiveSize, double inactiveMainOffset)
{
    if (adaptAnimator_ && adaptAnimator_->IsRunning()) {
        return;
    }
    if (!adaptAnimator_) {
        adaptAnimator_ = AceType::MakeRefPtr<Animator>(PipelineContext::GetCurrentContext());
    }
    adaptAnimator_->ClearInterpolators();
    // Animate the mainSize of the ScrollBar
    auto sizeAnimation = AceType::MakeRefPtr<CurveAnimation<double>>(inactiveSize, activeSize, Curves::FRICTION);
    sizeAnimation->AddListener([weakBar = AceType::WeakClaim(this)](double value) {
        auto scrollBar = weakBar.Upgrade();
        if (scrollBar) {
            scrollBar->UpdateActiveRectSize(value);
            scrollBar->MarkNeedRender();
        }
    });
    // Animate the mainOffset of the ScrollBar
    auto offsetAnimation =
        AceType::MakeRefPtr<CurveAnimation<double>>(inactiveMainOffset, activeMainOffset, Curves::FRICTION);
    offsetAnimation->AddListener(
        [weakBar = AceType::WeakClaim(this), inactiveMainOffset, activeMainOffset](double value) {
            auto scrollBar = weakBar.Upgrade();
            if (scrollBar) {
                auto top = scrollBar->GetPositionMode() == PositionMode::BOTTOM ? scrollBar->activeRect_.Left() :
                    scrollBar->activeRect_.Top();
                if (NearEqual(top, activeMainOffset, 0.000001f) || NearEqual(top, inactiveMainOffset, 0.000001f)) {
                    scrollBar->UpdateActiveRectOffset(value);
                } else {
                    scrollBar->UpdateActiveRectOffset(value + scrollBar->scrollableOffset_ - activeMainOffset);
                }
                scrollBar->MarkNeedRender();
            }
        });
    adaptAnimator_->AddInterpolator(sizeAnimation);
    adaptAnimator_->AddInterpolator(offsetAnimation);
    adaptAnimator_->SetDuration(BAR_ADAPT_DURATION);

    UpdateActiveRectSize(inactiveSize);
    UpdateActiveRectOffset(inactiveMainOffset);

    adaptAnimator_->Play();
}

void ScrollBar::PlayGrowAnimation()
{
    if (!touchAnimator_->IsStopped()) {
        touchAnimator_->Stop();
    }
    touchAnimator_->ClearInterpolators();
    auto activeWidth = activeWidth_.ConvertToPx();
    auto inactiveWidth = inactiveWidth_.ConvertToPx();

    auto animation = AceType::MakeRefPtr<CurveAnimation<double>>(inactiveWidth, activeWidth, Curves::SHARP);
    animation->AddListener([weakBar = AceType::WeakClaim(this)](double value) {
        auto scrollBar = weakBar.Upgrade();
        if (scrollBar) {
            scrollBar->normalWidth_ = Dimension(value, DimensionUnit::PX);
            scrollBar->FlushBarWidth();
            scrollBar->MarkNeedRender();
        }
    });
    touchAnimator_->AddInterpolator(animation);
    touchAnimator_->SetDuration(BAR_EXPAND_DURATION);
    touchAnimator_->Play();
}

void ScrollBar::PlayShrinkAnimation()
{
    if (!touchAnimator_->IsStopped()) {
        touchAnimator_->Stop();
    }
    touchAnimator_->ClearInterpolators();
    auto activeWidth = activeWidth_.ConvertToPx();
    auto inactiveWidth = inactiveWidth_.ConvertToPx();

    auto animation = AceType::MakeRefPtr<CurveAnimation<double>>(activeWidth, inactiveWidth, Curves::SHARP);
    animation->AddListener([weakBar = AceType::WeakClaim(this)](double value) {
        auto scrollBar = weakBar.Upgrade();
        if (scrollBar) {
            scrollBar->normalWidth_ = Dimension(value, DimensionUnit::PX);
            scrollBar->FlushBarWidth();
            scrollBar->MarkNeedRender();
        }
    });
    touchAnimator_->AddInterpolator(animation);
    touchAnimator_->SetDuration(BAR_SHRINK_DURATION);
    touchAnimator_->Play();
}

void ScrollBar::PlayBarEndAnimation()
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
        if (scrollBar) {
            scrollBar->opacity_ = value;
            scrollBar->MarkNeedRender();
        }
    });
    scrollEndAnimator_->AddInterpolator(animation);
    scrollEndAnimator_->SetDuration(STOP_DURATION);
    scrollEndAnimator_->Play();
}
} // namespace OHOS::Ace::NG