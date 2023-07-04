/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/menu/menu_item_group/menu_item_group_view.h"

#include "core/components/select/select_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/menu/menu_item_group/menu_item_group_pattern.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_theme.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {
void MenuItemGroupView::Create()
{
    LOGI("MenuItemGroupView::Create");
    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = stack->ClaimNodeId();
    auto menuItemGroup = FrameNode::GetOrCreateFrameNode(
        V2::MENU_ITEM_GROUP_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<MenuItemGroupPattern>(); });
    CHECK_NULL_VOID(menuItemGroup);

    stack->Push(menuItemGroup);
}

void MenuItemGroupView::SetHeader(const RefPtr<UINode>& header)
{
    LOGI("MenuItemGroupView::SetHeader");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<MenuItemGroupPattern>();
    CHECK_NULL_VOID(pattern);
    auto row = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    header->MountToParent(row);
    pattern->AddHeader(row);
}

void MenuItemGroupView::SetHeader(const std::string& headerStr)
{
    LOGI("MenuItemGroupView::SetHeader content is string");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<MenuItemGroupPattern>();
    CHECK_NULL_VOID(pattern);
    auto row = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    auto content = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_VOID(row && content);
    content->MountToParent(row);
    auto layoutProps = content->GetLayoutProperty<TextLayoutProperty>();
    layoutProps->UpdateContent(headerStr);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    layoutProps->UpdateTextColor(theme->GetSecondaryFontColor());
    layoutProps->UpdateFontSize(theme->GetMenuFontSize());
    pattern->AddHeaderContent(content);
    pattern->AddHeader(row);
}

void MenuItemGroupView::SetFooter(const RefPtr<UINode>& footer)
{
    LOGI("MenuItemGroupView::SetFooter");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<MenuItemGroupPattern>();
    CHECK_NULL_VOID(pattern);
    auto row = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    footer->MountToParent(row);
    pattern->AddFooter(row);
}

void MenuItemGroupView::SetFooter(const std::string& footerStr)
{
    LOGI("MenuItemGroupView::SetFooter content is string");
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<MenuItemGroupPattern>();
    CHECK_NULL_VOID(pattern);
    auto row = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    auto content = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_VOID(row && content);
    content->MountToParent(row);
    auto layoutProps = content->GetLayoutProperty<TextLayoutProperty>();
    layoutProps->UpdateContent(footerStr);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    layoutProps->UpdateTextColor(theme->GetSecondaryFontColor());
    layoutProps->UpdateFontSize(theme->GetMenuFontSize());
    pattern->AddFooterContent(content);
    pattern->AddFooter(row);
}
} // namespace OHOS::Ace::NG
