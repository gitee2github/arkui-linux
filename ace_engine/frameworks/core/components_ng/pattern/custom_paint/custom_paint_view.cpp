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

#include "core/components_ng/pattern/custom_paint/custom_paint_view.h"

#include "drawing/engine_adapter/skia_adapter/skia_canvas.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/custom_paint/canvas_paint_method.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
RefPtr<CustomPaintPattern> CustomPaintView::Create()
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::CANVAS_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<CustomPaintPattern>(); });
    stack->Push(frameNode);
    auto pattern = frameNode->GetPattern<CustomPaintPattern>();
    return pattern;
}

void CustomPaintView::SetOnReady(ReadyEvent&& onReady)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<CustomPaintEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnReady(std::move(onReady));
}
} // namespace OHOS::Ace::NG
