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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_TEXT_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_TEXT_PATTERN_H

#include <string>

#include "base/geometry/dimension.h"
#include "base/memory/referenced.h"
#include "base/utils/noncopyable.h"
#include "core/components_ng/event/long_press_event.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_ng/pattern/text/text_accessibility_property.h"
#include "core/components_ng/pattern/text/text_layout_algorithm.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_paint_method.h"
#include "core/components_ng/pattern/text_field/text_selector.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/paragraph.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {
// TextPattern is the base class for text render node to perform paint text.
class TextPattern : public Pattern {
    DECLARE_ACE_TYPE(TextPattern, Pattern);

public:
    TextPattern() = default;
    ~TextPattern() override = default;

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        return MakeRefPtr<TextPaintMethod>(WeakClaim(this), paragraph_, baselineOffset_);
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<TextLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<TextLayoutAlgorithm>(spanItemChildren_, paragraph_);
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<TextAccessibilityProperty>();
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    void OnModifyDone() override;

    void BeforeCreateLayoutWrapper() override;

    void AddChildSpanItem(const RefPtr<SpanNode>& child)
    {
        spanItemChildren_.emplace_back(child->GetSpanItem());
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, false };
    }

    void DumpInfo() override;

    TextSelector GetTextSelector() const
    {
        return textSelector_;
    }

    std::string GetTextForDisplay() const
    {
        return textForDisplay_;
    }

private:
    void OnDetachFromFrameNode(FrameNode* node) override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    void HandleLongPress(GestureEvent& info);
    void HandleOnSelectAll();
    void HandleOnCopy();
    void OnHandleMove(const RectF& handleRect, bool isFirstHandle);
    void OnHandleMoveDone(const RectF& handleRect, bool isFirstHandle);
    void InitLongPressEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitMouseEvent();
    void HandleMouseEvent(const MouseInfo& info);
    void OnHandleTouchUp();
    void InitClickEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleClickEvent(GestureEvent& info);

    void ShowSelectOverlay(const RectF& firstHandle, const RectF& secondHandle);
    void InitSelection(const Offset& pos);
    void CalcuateHandleOffsetAndShowOverlay(bool isUsingMouse = false);

    int32_t GetGraphemeClusterLength(int32_t extend) const;
    OffsetF CalcCursorOffsetByPosition(int32_t position, float& selectLineHeight);
    std::string GetSelectedText(int32_t start, int32_t end) const;
    std::wstring GetWideText() const;

    std::list<RefPtr<SpanItem>> spanItemChildren_;
    std::string textForDisplay_;
    RefPtr<Paragraph> paragraph_;
    RefPtr<LongPressEvent> longPressEvent_;
    RefPtr<SelectOverlayProxy> selectOverlayProxy_;
    RefPtr<Clipboard> clipboard_;
    CopyOptions copyOption_ = CopyOptions::None;
    TextSelector textSelector_;
    OffsetF contentOffset_;
    RectF contentRect_;
    float baselineOffset_ = 0.0f;
    bool clickEventInitialized_ = false;
    bool mouseEventInitialized_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(TextPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TEXT_TEXT_PATTERN_H
