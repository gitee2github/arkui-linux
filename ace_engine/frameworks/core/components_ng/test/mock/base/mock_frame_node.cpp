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

#include "base/memory/ace_type.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace::NG {
FrameNode::~FrameNode() {}
void FrameNode::OnWindowShow() {}
void FrameNode::OnWindowHide() {}
void FrameNode::OnWindowFocused() {}
void FrameNode::OnWindowUnfocused() {}
void FrameNode::OnNotifyMemoryLevel(int32_t level) {}
void FrameNode::OnAccessibilityEvent(AccessibilityEventType eventType) const {}
void FrameNode::DumpInfo() {}
void FrameNode::FocusToJsonValue(std::unique_ptr<JsonValue>& json) const {}
void FrameNode::MouseToJsonValue(std::unique_ptr<JsonValue>& json) const {}
void FrameNode::TouchToJsonValue(std::unique_ptr<JsonValue>& json) const {}
void FrameNode::ToJsonValue(std::unique_ptr<JsonValue>& json) const {}
void FrameNode::OnAttachToMainTree() {}
void FrameNode::OnDetachFromMainTree() {}
void FrameNode::SwapDirtyLayoutWrapperOnMainThread(const RefPtr<LayoutWrapper>& dirty) {}
void FrameNode::SetActive(bool active)
{
    isActive_ = active;
}
void FrameNode::OnVisibleChange(bool isVisible) {}
void FrameNode::PostTask(std::function<void()>&& task, TaskExecutor::TaskType taskType) {}
void FrameNode::RebuildRenderContextTree() {}
void FrameNode::MarkModifyDone()
{
    pattern_->OnModifyDone();
}
void FrameNode::OnMountToParentDone() {}
void FrameNode::FlushUpdateAndMarkDirty() {}
void FrameNode::MarkDirtyNode(PropertyChangeFlag extraFlag) {}
void FrameNode::MarkNeedRenderOnly() {}
void FrameNode::MarkNeedRender(bool isRenderBoundary) {}
void FrameNode::MarkDirtyNode(bool isMeasureBoundary, bool isRenderBoundary, PropertyChangeFlag extraFlag) {}
void FrameNode::OnGenerateOneDepthVisibleFrame(std::list<RefPtr<FrameNode>>& visibleList) {}
void FrameNode::OnGenerateOneDepthAllFrame(std::list<RefPtr<FrameNode>>& allList) {}
void FrameNode::UpdateChildrenLayoutWrapper(const RefPtr<LayoutWrapper>& self, bool forceMeasure, bool forceLayout) {}
void FrameNode::AdjustLayoutWrapperTree(const RefPtr<LayoutWrapper>& parent, bool forceMeasure, bool forceLayout) {}
void FrameNode::UpdateLayoutPropertyFlag() {}
void FrameNode::AdjustParentLayoutFlag(PropertyChangeFlag& flag) {}
void FrameNode::MarkResponseRegion(bool isResponseRegion) {}

FrameNode::FrameNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern, bool isRoot)
    : UINode(tag, nodeId, isRoot), pattern_(pattern)
{
    paintProperty_ = pattern->CreatePaintProperty();
    layoutProperty_ = pattern->CreateLayoutProperty();
    eventHub_ = pattern->CreateEventHub();
    accessibilityProperty_ = pattern->CreateAccessibilityProperty();
    layoutProperty_->SetHost(WeakClaim(this));
}

void FrameNode::SetGeometryNode(const RefPtr<GeometryNode>& node)
{
    geometryNode_ = node;
}

void FrameNode::UpdateLayoutConstraint(const MeasureProperty& calcLayoutConstraint)
{
    layoutProperty_->UpdateCalcLayoutProperty(calcLayoutConstraint);
}

RefPtr<FrameNode> FrameNode::CreateFrameNodeWithTree(
    const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
{
    auto frameNode = AceType::MakeRefPtr<FrameNode>(tag, nodeId, pattern);
    return frameNode;
}

RefPtr<FrameNode> FrameNode::GetOrCreateFrameNode(
    const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator)
{
    auto pattern = patternCreator ? patternCreator() : MakeRefPtr<Pattern>();
    return CreateFrameNode(tag, nodeId, pattern);
}

RefPtr<FrameNode> FrameNode::GetFrameNode(const std::string& tag, int32_t nodeId)
{
    return nullptr;
}

RefPtr<FrameNode> FrameNode::CreateFrameNode(
    const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern, bool isRoot)
{
    auto frameNode = MakeRefPtr<FrameNode>(tag, nodeId, pattern, isRoot);
    frameNode->InitializePatternAndContext();
    return frameNode;
}

void FrameNode::InitializePatternAndContext()
{
    pattern_->AttachToFrameNode(WeakClaim(this));
}

std::optional<UITask> FrameNode::CreateLayoutTask(bool forceUseMainThread)
{
    return std::nullopt;
}

std::optional<UITask> FrameNode::CreateRenderTask(bool forceUseMainThread)
{
    return std::nullopt;
}

LayoutConstraintF FrameNode::GetLayoutConstraint() const
{
    return LayoutConstraintF();
}

OffsetF FrameNode::GetParentGlobalOffset() const
{
    return { 1.0f, 1.0f };
}

OffsetF FrameNode::GetPaintRectOffsetToPage() const
{
    return { 1.0f, 1.0f };
}

RefPtr<LayoutWrapper> FrameNode::CreateLayoutWrapper(bool forceMeasure, bool forceLayout)
{
    return nullptr;
}

RefPtr<PaintWrapper> FrameNode::CreatePaintWrapper()
{
    return nullptr;
}

RefPtr<FrameNode> FrameNode::GetAncestorNodeOfFrame() const
{
    return nullptr;
}

bool FrameNode::IsMeasureBoundary()
{
    return false;
}

bool FrameNode::IsRenderBoundary()
{
    return false;
}

const RefPtr<Pattern>& FrameNode::GetPattern() const
{
    return pattern_;
}

bool FrameNode::IsAtomicNode() const
{
    return true;
}

HitTestMode FrameNode::GetHitTestMode() const
{
    return HitTestMode::HTMDEFAULT;
}

bool FrameNode::GetTouchable() const
{
    return false;
}

bool FrameNode::IsResponseRegion() const
{
    return false;
}

HitTestResult FrameNode::TouchTest(const PointF& globalPoint, const PointF& parentLocalPoint,
    const TouchRestrict& touchRestrict, TouchTestResult& result, int32_t touchId)
{
    return HitTestResult::BUBBLING;
}

std::vector<RectF> FrameNode::GetResponseRegionList(const RectF& rect)
{
    return std::vector<RectF>();
}

bool FrameNode::InResponseRegionList(const PointF& parentLocalPoint, const std::vector<RectF>& responseRegionList) const
{
    return false;
}

HitTestResult FrameNode::MouseTest(const PointF& globalPoint, const PointF& parentLocalPoint,
    MouseTestResult& onMouseResult, MouseTestResult& onHoverResult, RefPtr<FrameNode>& hoverNode)
{
    return HitTestResult::BUBBLING;
}

HitTestResult FrameNode::AxisTest(
    const PointF& globalPoint, const PointF& parentLocalPoint, AxisTestResult& onAxisResult)
{
    return HitTestResult::BUBBLING;
}

RefPtr<FocusHub> FrameNode::GetOrCreateFocusHub() const
{
    return nullptr;
}

OffsetF FrameNode::GetOffsetRelativeToWindow() const
{
    return { 1.0f, 1.0f };
}

OffsetF FrameNode::GetTransformRelativeOffset() const
{
    return { 1.0f, 1.0f };
}
} // namespace OHOS::Ace::NG
