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

#include "core/components_ng/pattern/xcomponent/xcomponent_model_ng.h"

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_event_hub.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_layout_property.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
void XComponentModelNG::Create(const std::string& id, const std::string& type, const std::string& libraryname,
    const RefPtr<XComponentController>& xcomponentController)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto xcomponentType = type == "component" ? XComponentType::COMPONENT : XComponentType::SURFACE;
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::XCOMPONENT_ETS_TAG, nodeId, [id, xcomponentType, libraryname, xcomponentController]() {
            return AceType::MakeRefPtr<XComponentPattern>(id, xcomponentType, libraryname, xcomponentController);
        });
    stack->Push(frameNode);
    ACE_UPDATE_LAYOUT_PROPERTY(XComponentLayoutProperty, XComponentType, xcomponentType);
}
void XComponentModelNG::SetSoPath(const std::string& soPath)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty<XComponentLayoutProperty>();
    if (layoutProperty->GetXComponentTypeValue() == XComponentType::COMPONENT) {
        return;
    }
    auto xcPattern = AceType::DynamicCast<XComponentPattern>(frameNode->GetPattern());
    xcPattern->SetSoPath(soPath);
}
void XComponentModelNG::SetOnLoad(LoadEvent&& onLoad)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty<XComponentLayoutProperty>();
    if (layoutProperty->GetXComponentTypeValue() == XComponentType::COMPONENT) {
        return;
    }
    auto eventHub = frameNode->GetEventHub<XComponentEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnLoad(std::move(onLoad));
}
void XComponentModelNG::SetOnDestroy(DestroyEvent&& onDestroy)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty<XComponentLayoutProperty>();
    if (layoutProperty->GetXComponentTypeValue() == XComponentType::COMPONENT) {
        return;
    }
    auto eventHub = frameNode->GetEventHub<XComponentEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnDestroy(std::move(onDestroy));
}
} // namespace OHOS::Ace::NG
