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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_FRAME_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_FRAME_NODE_H

#include <functional>
#include <list>
#include <utility>

#include "base/geometry/ng/point_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/thread/cancelable_callback.h"
#include "base/thread/task_executor.h"
#include "base/utils/macros.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/event/input_event_hub.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/property/accessibility_property.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/paint_property.h"
#include "core/components_ng/render/paint_wrapper.h"
#include "core/components_ng/render/render_context.h"
#include "core/components_v2/inspector/inspector_node.h"

namespace OHOS::Ace::NG {
class PipelineContext;
class Pattern;
class StateModifyTask;
class UITask;

constexpr int32_t DEFAULT_FRAME_SLOT = -1;

// FrameNode will display rendering region in the screen.
class ACE_EXPORT FrameNode : public UINode {
    DECLARE_ACE_TYPE(FrameNode, UINode);

public:
    // create a new child element with new element tree.
    static RefPtr<FrameNode> CreateFrameNodeWithTree(
        const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern);

    static RefPtr<FrameNode> GetOrCreateFrameNode(
        const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator);

    // create a new element with new pattern.
    static RefPtr<FrameNode> CreateFrameNode(
        const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern, bool isRoot = false);

    // get element with nodeId from node map.
    static RefPtr<FrameNode> GetFrameNode(const std::string& tag, int32_t nodeId);

    // avoid use creator function, use CreateFrameNode
    FrameNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern, bool isRoot = false);

    ~FrameNode() override;

    int32_t FrameCount() const override
    {
        return 1;
    }

    void InitializePatternAndContext();

    virtual void MarkModifyDone();

    void MarkDirtyNode(PropertyChangeFlag extraFlag = PROPERTY_UPDATE_NORMAL) override;

    void MarkDirtyNode(
        bool isMeasureBoundary, bool isRenderBoundary, PropertyChangeFlag extraFlag = PROPERTY_UPDATE_NORMAL);

    void FlushUpdateAndMarkDirty() override;

    void MarkNeedFrameFlushDirty(PropertyChangeFlag extraFlag = PROPERTY_UPDATE_NORMAL) override
    {
        MarkDirtyNode(extraFlag);
    }

    void OnMountToParentDone();

    void UpdateLayoutConstraint(const MeasureProperty& calcLayoutConstraint);

    RefPtr<LayoutWrapper> CreateLayoutWrapper(bool forceMeasure = false, bool forceLayout = false) override;

    std::optional<UITask> CreateLayoutTask(bool forceUseMainThread = false);

    std::optional<UITask> CreateRenderTask(bool forceUseMainThread = false);

    void SwapDirtyLayoutWrapperOnMainThread(const RefPtr<LayoutWrapper>& dirty);

    void SetOnAreaChangeCallback(OnAreaChangedFunc&& callback);
    void TriggerOnAreaChangeCallback();

    void TriggerVisibleAreaChangeCallback(std::list<VisibleCallbackInfo>& callbackInfoList);

    const RefPtr<GeometryNode>& GetGeometryNode() const
    {
        return geometryNode_;
    }
    void SetGeometryNode(const RefPtr<GeometryNode>& node);

    const RefPtr<RenderContext>& GetRenderContext() const
    {
        return renderContext_;
    }

    const RefPtr<Pattern>& GetPattern() const;

    template<typename T>
    RefPtr<T> GetPattern() const
    {
        return DynamicCast<T>(pattern_);
    }

    template<typename T>
    RefPtr<T> GetAccessibilityProperty() const
    {
        return DynamicCast<T>(accessibilityProperty_);
    }

    template<typename T>
    RefPtr<T> GetLayoutProperty() const
    {
        return DynamicCast<T>(layoutProperty_);
    }

    template<typename T>
    RefPtr<T> GetPaintProperty() const
    {
        return DynamicCast<T>(paintProperty_);
    }

    template<typename T>
    RefPtr<T> GetEventHub() const
    {
        return DynamicCast<T>(eventHub_);
    }

    RefPtr<GestureEventHub> GetOrCreateGestureEventHub() const
    {
        return eventHub_->GetOrCreateGestureEventHub();
    }

    RefPtr<InputEventHub> GetOrCreateInputEventHub() const
    {
        return eventHub_->GetOrCreateInputEventHub();
    }

    RefPtr<FocusHub> GetOrCreateFocusHub() const;

    RefPtr<FocusHub> GetFocusHub() const
    {
        return eventHub_->GetFocusHub();
    }

    FocusType GetFocusType() const
    {
        FocusType type = FocusType::DISABLE;
        auto focusHub = GetFocusHub();
        if (focusHub) {
            type = focusHub->GetFocusType();
        }
        return type;
    }

    const RefPtr<LayoutProperty>& GetLayoutProperty() const
    {
        return layoutProperty_;
    }

    static void PostTask(std::function<void()>&& task, TaskExecutor::TaskType taskType = TaskExecutor::TaskType::UI);

    // If return true, will prevent TouchTest Bubbling to parent and brother nodes.
    HitTestResult TouchTest(const PointF& globalPoint, const PointF& parentLocalPoint,
        const TouchRestrict& touchRestrict, TouchTestResult& result, int32_t touchId) override;

    HitTestResult MouseTest(const PointF& globalPoint, const PointF& parentLocalPoint, MouseTestResult& onMouseResult,
        MouseTestResult& onHoverResult, RefPtr<FrameNode>& hoverNode) override;

    HitTestResult AxisTest(
        const PointF& globalPoint, const PointF& parentLocalPoint, AxisTestResult& onAxisResult) override;

    void AnimateHoverEffect(bool isHovered) const;

    bool IsAtomicNode() const override;

    void MarkNeedSyncRenderTree() override
    {
        needSyncRenderTree_ = true;
    }

    void RebuildRenderContextTree() override;

    bool IsVisible() const
    {
        return layoutProperty_->GetVisibility().value_or(VisibleType::VISIBLE) == VisibleType::VISIBLE;
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override;

    RefPtr<FrameNode> GetAncestorNodeOfFrame() const;

    std::string& GetNodeName()
    {
        return nodeName_;
    }

    void SetNodeName(std::string& nodeName)
    {
        nodeName_ = nodeName;
    }
    bool IsResponseRegion() const;
    void MarkResponseRegion(bool isResponseRegion);

    void OnWindowShow() override;

    void OnWindowHide() override;

    void OnWindowFocused() override;

    void OnWindowUnfocused() override;

    void OnNotifyMemoryLevel(int32_t level) override;

    OffsetF GetOffsetRelativeToWindow() const;

    OffsetF GetTransformRelativeOffset() const;

    RectF GetTransformRectRelativeToWindow() const;

    OffsetF GetPaintRectOffset() const;

    OffsetF GetPaintRectOffsetToPage() const;

    void AdjustGridOffset();

    void SetActive(bool active) override;

    bool IsActive() const
    {
        return isActive_;
    }

    bool IsInternal() const
    {
        return isInternal_;
    }

    void SetInternal()
    {
        isInternal_ = true;
    }

    int32_t GetAllDepthChildrenCount();

    void OnAccessibilityEvent(AccessibilityEventType eventType) const;
    void MarkNeedRenderOnly();

    void OnDetachFromMainTree() override;
    void OnAttachToMainTree() override;

    void OnVisibleChange(bool isVisible) override;

    void PushDestroyCallback(std::function<void()>&& callback)
    {
        destroyCallbacks_.emplace_back(callback);
    }

    void AddHotZoneRect(const DimensionRect& hotZoneRect) const;
    
    void RemoveLastHotZoneRect() const;

    bool IsOutOfTouchTestRegion(const PointF& parentLocalPoint);

    bool IsLayoutDirtyMarked() const
    {
        return isLayoutDirtyMarked_;
    }

    bool HasPositionProp() const
    {
        CHECK_NULL_RETURN_NOLOG(renderContext_, false);
        return renderContext_->HasPosition() || renderContext_->HasOffset() || renderContext_->HasAnchor();
    }

    // The function is only used for fast preview.
    void FastPreviewUpdateChildDone() override
    {
        OnMountToParentDone();
    }

private:
    void MarkNeedRender(bool isRenderBoundary);
    bool IsNeedRequestParentMeasure() const;
    void UpdateLayoutPropertyFlag() override;
    void AdjustParentLayoutFlag(PropertyChangeFlag& flag) override;

    void UpdateChildrenLayoutWrapper(const RefPtr<LayoutWrapper>& self, bool forceMeasure, bool forceLayout);
    void AdjustLayoutWrapperTree(const RefPtr<LayoutWrapper>& parent, bool forceMeasure, bool forceLayout) override;

    LayoutConstraintF GetLayoutConstraint() const;
    OffsetF GetParentGlobalOffset() const;

    RefPtr<PaintWrapper> CreatePaintWrapper();

    void OnGenerateOneDepthVisibleFrame(std::list<RefPtr<FrameNode>>& visibleList) override;
    void OnGenerateOneDepthAllFrame(std::list<RefPtr<FrameNode>>& allList) override;

    bool IsMeasureBoundary();
    bool IsRenderBoundary();

    // dump self info.
    void DumpInfo() override;

    void FocusToJsonValue(std::unique_ptr<JsonValue>& json) const;
    void MouseToJsonValue(std::unique_ptr<JsonValue>& json) const;
    void TouchToJsonValue(std::unique_ptr<JsonValue>& json) const;
    void GeometryNodeToJsonValue(std::unique_ptr<JsonValue>& json) const;

    HitTestMode GetHitTestMode() const override;
    bool GetTouchable() const;
    std::vector<RectF> GetResponseRegionList(const RectF& rect);
    bool InResponseRegionList(const PointF& parentLocalPoint, const std::vector<RectF>& responseRegionList) const;

    void ProcessAllVisibleCallback(std::list<VisibleCallbackInfo>& callbackInfoList, double currentVisibleRatio);
    void OnVisibleAreaChangeCallback(
        VisibleCallbackInfo& callbackInfo, bool visibleType, double currentVisibleRatio);
    double CalculateCurrentVisibleRatio(const RectF& visibleRect, const RectF& renderRect);

    struct ZIndexComparator {
        bool operator()(const RefPtr<FrameNode>& left, const RefPtr<FrameNode>& right) const
        {
            if (left && right) {
                return left->GetRenderContext()->GetZIndexValue(1) < right->GetRenderContext()->GetZIndexValue(1);
            }
            return false;
        }
    };
    // sort in ZIndex.
    std::multiset<RefPtr<FrameNode>, ZIndexComparator> frameChildren_;
    RefPtr<GeometryNode> geometryNode_ = MakeRefPtr<GeometryNode>();

    std::list<std::function<void()>> destroyCallbacks_;

    RefPtr<AccessibilityProperty> accessibilityProperty_;
    RefPtr<LayoutProperty> layoutProperty_;
    RefPtr<PaintProperty> paintProperty_;
    RefPtr<RenderContext> renderContext_ = RenderContext::Create();
    RefPtr<EventHub> eventHub_;
    RefPtr<Pattern> pattern_;

    std::unique_ptr<RectF> lastFrameRect_;
    std::unique_ptr<OffsetF> lastParentOffsetToWindow_;

    bool needSyncRenderTree_ = false;

    bool isLayoutDirtyMarked_ = false;
    bool isRenderDirtyMarked_ = false;
    bool isMeasureBoundary_ = false;
    bool hasPendingRequest_ = false;

    bool isActive_ = false;
    bool isResponseRegion_ = false;

    double lastVisibleRatio_ = 0.0;

    // internal node such as Text in Button CreateWithLabel
    // should not seen by preview inspector or accessibility
    bool isInternal_ = false;

    std::string nodeName_;

    friend class RosenRenderContext;
    friend class RenderContext;
    friend class Pattern;

    ACE_DISALLOW_COPY_AND_MOVE(FrameNode);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_FRAME_NODE_H
