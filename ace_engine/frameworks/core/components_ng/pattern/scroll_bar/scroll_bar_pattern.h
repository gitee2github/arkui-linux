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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_BAR_SCROLL_BAR_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_BAR_SCROLL_BAR_PATTERN_H

#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scroll/inner/scroll_bar.h"
#include "core/components_ng/pattern/scroll/scroll_event_hub.h"
#include "core/components_ng/pattern/scroll_bar/scroll_bar_layout_algorithm.h"
#include "core/components_ng/pattern/scroll_bar/scroll_bar_layout_property.h"
#include "core/components_ng/pattern/scroll_bar/proxy/scroll_bar_proxy.h"

namespace OHOS::Ace::NG {

class ScrollBarPattern : public Pattern {
    DECLARE_ACE_TYPE(ScrollBarPattern, Pattern);

public:
    ScrollBarPattern() = default;
    ~ScrollBarPattern() override
    {
        scrollEndAnimator_ = nullptr;
        scrollBarProxy_ = nullptr;
        scrollableEvent_ = nullptr;
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<ScrollBarLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        auto layoutAlgorithm = MakeRefPtr<ScrollBarLayoutAlgorithm>(currentOffset_);
        return layoutAlgorithm;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<ScrollEventHub>();
    }

    float GetCurrentPosition() const
    {
        return currentOffset_;
    }

    void SetCurrentPosition(float currentOffset)
    {
        currentOffset_ = currentOffset;
    }

    Axis GetAxis() const
    {
        return axis_;
    }

    float GetScrollableDistance() const
    {
        return scrollableDistance_;
    }

    void SetScrollBarProxy(const RefPtr<ScrollBarProxy>& scrollBarProxy)
    {
        scrollBarProxy_ = scrollBarProxy;
    }

    const DisplayMode& GetDisplayMode() const
    {
        return displayMode_;
    }

    float GetControlDistance() const
    {
        return controlDistance_;
    }

    void SetControlDistance(float controlDistance)
    {
        controlDistance_ = controlDistance;
    }

    float GetScrollOffset() const
    {
        return scrollOffset_;
    }

    void SetScrollOffset(float scrollOffset)
    {
        scrollOffset_ = scrollOffset;
    }

    bool IsAtTop() const;
    bool IsAtBottom() const;
    bool UpdateCurrentOffset(float offset, int32_t source);

    // disappear Animator
    void StartAnimator();
    void StopAnimator();
    void SetOpacity(uint8_t value);

private:
    void OnModifyDone() override;
    void OnAttachToFrameNode() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void ValidateOffset(int32_t source);

    RefPtr<Animator> scrollEndAnimator_;
    RefPtr<ScrollBarProxy> scrollBarProxy_;
    RefPtr<ScrollableEvent> scrollableEvent_;
    Axis axis_ = Axis::VERTICAL;
    DisplayMode displayMode_ { DisplayMode::AUTO };
    float currentOffset_ = 0.0f;
    float lastOffset_ = 0.0f;
    float scrollableDistance_ = 0.0f;
    float controlDistance_ = 0.0f;
    float scrollOffset_ = 0.0f;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_BAR_SCROLL_BAR_PATTERN_H
