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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_SCROLL_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_SCROLL_PATTERN_H

#include "base/geometry/axis.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scroll/scroll_edge_effect.h"
#include "core/components_ng/pattern/scroll/scroll_event_hub.h"
#include "core/components_ng/pattern/scroll/scroll_layout_algorithm.h"
#include "core/components_ng/pattern/scroll/scroll_layout_property.h"
#include "core/components_ng/pattern/scroll/scroll_paint_property.h"
#include "core/components_ng/pattern/scroll/scroll_paint_method.h"
#include "core/components_ng/pattern/scroll/scroll_position_controller.h"
#include "core/components_ng/pattern/scroll_bar/proxy/scroll_bar_proxy.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"

namespace OHOS::Ace::NG {

class ScrollPattern : public ScrollablePattern {
    DECLARE_ACE_TYPE(ScrollPattern, ScrollablePattern);

public:
    ScrollPattern() = default;
    ~ScrollPattern() override
    {
        animator_ = nullptr;
        positionController_ = nullptr;
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    bool UsResRegion() override
    {
        return false;
    }
    
    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<ScrollLayoutProperty>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<ScrollPaintProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        auto layoutAlgorithm = MakeRefPtr<ScrollLayoutAlgorithm>(currentOffset_);
        return layoutAlgorithm;
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        auto paint = MakeRefPtr<ScrollPaintMethod>();
        paint->SetScrollBar(GetScrollBar());
        auto scrollEffect = GetScrollEdgeEffect();
        if (scrollEffect && scrollEffect->IsFadeEffect()) {
            paint->SetEdgeEffect(scrollEffect);
        }
        return paint;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<ScrollEventHub>();
    }

    virtual bool ReachMaxCount() const
    {
        return true;
    }

    bool IsScrollable() const override
    {
        return GetAxis() != Axis::NONE;
    }

    bool OnScrollCallback(float offset, int32_t source) override;
    void OnScrollEndCallback() override;

    double GetCurrentPosition() const
    {
        return currentOffset_;
    }

    void ResetPosition();

    Offset GetCurrentOffset() const
    {
        if (GetAxis() == Axis::HORIZONTAL) {
            return Offset{currentOffset_, 0};
        }
        return Offset{0, currentOffset_};
    }

    float GetScrollableDistance() const
    {
        return scrollableDistance_;
    }

    bool IsRowReverse() const
    {
        // TODO: not consider rightToLeft
        return direction_ == FlexDirection::ROW_REVERSE;
    }

    bool IsColReverse() const
    {
        return  direction_ == FlexDirection::COLUMN_REVERSE;
    }

    RefPtr<ScrollPositionController> GetScrollPositionController() const
    {
        return positionController_;
    }

    void SetScrollPositionController(const RefPtr<ScrollPositionController>& positionController)
    {
        positionController_ = positionController;
    }

    void SetDirection(FlexDirection direction)
    {
        direction_ = direction;
    }

    bool IsAtTop() const override;
    bool IsAtBottom() const override;

    bool UpdateCurrentOffset(float offset, int32_t source) override;
    void AnimateTo(float position, float duration, const RefPtr<Curve>& curve, bool limitDuration = true,
        const std::function<void()>& onFinish = nullptr);
    void ScrollToEdge(ScrollEdgeType scrollEdgeType, bool smooth);
    void ScrollBy(float pixelX, float pixelY, bool smooth, const std::function<void()>& onFinish = nullptr);
    bool ScrollPage(bool reverse, bool smooth, const std::function<void()>& onFinish = nullptr);
    void JumpToPosition(float position, int32_t source = SCROLL_FROM_JUMP);
    bool ScrollPageCheck(float delta, int32_t source);
    void AdjustOffset(float& delta, int32_t source);

protected:
    void DoJump(float position, int32_t source = SCROLL_FROM_JUMP);

private:
    void OnModifyDone() override;
    void OnAttachToFrameNode() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    bool IsCrashTop() const;
    bool IsCrashBottom() const;
    bool IsScrollOutOnEdge(float delta) const;
    void HandleCrashTop() const;
    void HandleCrashBottom() const;

    void RegisterScrollEventTask();
    void RegisterScrollBarEventTask();
    void CreateOrStopAnimator();
    void HandleScrollEffect();
    void HandleScrollBarOutBoundary();
    void ValidateOffset(int32_t source);
    void HandleScrollPosition(float scroll, int32_t scrollState);
    void SetEdgeEffectCallback(const RefPtr<ScrollEdgeEffect>& scrollEffect) override;
    void AddScrollEdgeEffect(RefPtr<ScrollEdgeEffect> scrollEffect);
    void UpdateScrollBarOffset() override;
    void FireOnScrollStart();
    void FireOnScrollStop();

    RefPtr<Animator> animator_;
    RefPtr<ScrollPositionController> positionController_;
    float currentOffset_ = 0.0f;
    float lastOffset_ = 0.0f;
    float scrollableDistance_ = 0.0f;
    float viewPortLength_ = 0.0f;
    SizeF viewPort_;
    SizeF viewPortExtent_;
    FlexDirection direction_ { FlexDirection::COLUMN };
    bool scrollStop_ = false;
    bool scrollAbort_ = false;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_SCROLL_PATTERN_H
