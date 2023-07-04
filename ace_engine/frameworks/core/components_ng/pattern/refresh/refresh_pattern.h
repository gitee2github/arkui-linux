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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_REFRESH_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_REFRESH_PATTERN_H

#include <string>

#include "frameworks/base/geometry/dimension.h"
#include "frameworks/base/geometry/ng/offset_t.h"
#include "frameworks/base/memory/referenced.h"
#include "frameworks/base/utils/noncopyable.h"
#include "frameworks/core/components_ng/pattern/pattern.h"
#include "frameworks/core/components_ng/pattern/refresh/refresh_event_hub.h"
#include "frameworks/core/components_ng/pattern/refresh/refresh_layout_algorithm.h"
#include "frameworks/core/components_ng/pattern/refresh/refresh_layout_property.h"
#include "frameworks/core/components_ng/pattern/refresh/refresh_render_property.h"
#include "frameworks/core/components_ng/pattern/text/text_layout_property.h"
#include "frameworks/core/components_ng/property/property.h"

namespace OHOS::Ace::NG {

class RefreshPattern : public Pattern {
    DECLARE_ACE_TYPE(RefreshPattern, Pattern);

public:
    RefreshPattern() = default;
    ~RefreshPattern() override = default;

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<RefreshLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<RefreshLayoutAlgorithm>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<RefreshRenderProperty>();
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<RefreshEventHub>();
    }

    bool IsAtomicNode() const override
    {
        return false;
    }
    void OnModifyDone() override;
    void FireStateChange(int32_t value);
    void FireRefreshing();
    void FireChangeEvent(const std::string& value);
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, bool skipMeasure, bool skipLayout) override;
    void OnActive() override {}
    void CheckCoordinationEvent();
    RefPtr<FrameNode> FindScrollableChild();

private:
    void InitPanEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleDragStart();
    void HandleDragUpdate(float delta);
    void HandleDragEnd();
    void HandleDragCancel();
    void TriggerRefresh();
    void TriggerInActive();
    void TriggerDone();
    void TriggerFinish();
    void TriggerStatusChange(RefreshStatus newStatus);
    void TransitionPeriodAnimation();
    void RefreshStatusChange(RefreshStatus newStatus);
    void LoadingProgressExit();
    void LoadingProgressAppear();
    void LoadingProgressRecycle();
    void UpdateLoadingProgress(int32_t state, float ratio);
    void ReplaceLoadingProgressNode();
    void LoadingProgressReset();
    void OnExitAnimationFinish();
    void ResetLoadingProgressColor();
    float GetFollowRatio();
    float GetFadeAwayRatio();
    float GetScrollOffset(float delta);
    void UpdateLoadingMarginTop(float top);
    RefreshStatus refreshStatus_ = RefreshStatus::INACTIVE;
    RefPtr<PanEvent> panEvent_;
    OffsetF scrollOffset_;

    bool isRefreshing_ = false;
    float triggerLoadingDistance_ = 0.0f;
    RefPtr<FrameNode> progressChild_;
    WeakPtr<FrameNode> scrollableNode_;
    ACE_DISALLOW_COPY_AND_MOVE(RefreshPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_TEXT_PATTERN_H
