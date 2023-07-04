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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_ITEM_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_ITEM_PATTERN_H

#include "base/memory/referenced.h"
#include "base/utils/noncopyable.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/list/list_item_event_hub.h"
#include "core/components_ng/pattern/list/list_item_layout_property.h"
#include "core/components_ng/pattern/list/list_layout_property.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/syntax/shallow_builder.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {

enum class ListItemSwipeIndex {
    SWIPER_END = -1,
    ITEM_CHILD = 0,
    SWIPER_START = 1,
};

class ACE_EXPORT ListItemPattern : public Pattern {
    DECLARE_ACE_TYPE(ListItemPattern, Pattern);

public:
    explicit ListItemPattern(const RefPtr<ShallowBuilder>& shallowBuilder) : shallowBuilder_(shallowBuilder) {}
    ~ListItemPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    void BeforeCreateLayoutWrapper() override
    {
        if (shallowBuilder_ && !shallowBuilder_->IsExecuteDeepRenderDone()) {
            shallowBuilder_->ExecuteDeepRender();
            shallowBuilder_.Reset();
        }
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::SCOPE, true };
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<ListItemLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<ListItemEventHub>();
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override;

    void SetStartNode(const RefPtr<NG::UINode>& startNode);

    void SetEndNode(const RefPtr<NG::UINode>& endNode);

    SizeF GetContentSize() const;

    void HandleDragStart(const GestureEvent& info);
    void HandleDragUpdate(const GestureEvent& info);
    void HandleDragEnd(const GestureEvent& info);

    V2::SwipeEdgeEffect GetEdgeEffect();
    void MarkDirtyNode();
    void UpdatePostion(float delta);

    bool HasStartNode() const
    {
        return startNodeIndex_ >= 0;
    }

    bool HasEndNode() const
    {
        return endNodeIndex_ >= 0;
    }

    ListItemSwipeIndex GetSwiperIndex()
    {
        return swiperIndex_;
    }

    RefPtr<FrameNode> GetListFrameNode() const;

    Axis GetAxis() const;

    void SetSwiperItemForList();

    void SwiperReset();

    static float CalculateFriction(float gamma);

    void MarkIsSelected(bool isSelected);

    bool IsSelected() const
    {
        return isSelected_;
    }

    bool Selectable() const
    {
        return selectable_;
    }

    void SetSelectable(bool selectable)
    {
        selectable_ = selectable;
    }

    int32_t GetIndexInList() const
    {
        return indexInList_;
    }

    void SetIndexInList(int32_t index)
    {
        indexInList_ = index;
    }

    int32_t GetIndexInListItemGroup() const
    {
        return indexInListItemGroup_;
    }

    void SetIndexInListItemGroup(int32_t index)
    {
        indexInListItemGroup_ = index;
    }

protected:
    void OnModifyDone() override;

private:
    void InitSwiperAction(bool axisChanged);
    float GetFriction();
    void StartSpringMotion(float start, float end, float velocity);

    RefPtr<ShallowBuilder> shallowBuilder_;

    int32_t indexInList_ = 0;
    int32_t indexInListItemGroup_ = -1;

    // swiperAction
    int32_t startNodeIndex_ = -1;
    int32_t endNodeIndex_ = -1;
    int32_t childNodeIndex_ = 0;

    float curOffset_ = 0.0f;
    float startNodeSize_ = 0.0f;
    float endNodeSize_ = 0.0f;
    Axis axis_ = Axis::NONE;
    ListItemSwipeIndex swiperIndex_ = ListItemSwipeIndex::ITEM_CHILD;

    RefPtr<PanEvent> panEvent_;
    RefPtr<Animator> springController_;
    RefPtr<SpringMotion> springMotion_;

    // selectable
    bool selectable_ = true;
    bool isSelected_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(ListItemPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_ITEM_PATTERN_H
