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

#include "core/components_ng/pattern/select/select_view.h"

#include "base/utils/utils.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/select/select_pattern.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

void SelectView::Create(const std::vector<SelectParam>& params)
{
    LOGI("creating select component");
    auto* stack = ViewStackProcessor::GetInstance();
    int32_t nodeId = (stack == nullptr ? 0 : stack->ClaimNodeId());
    auto select = FrameNode::GetOrCreateFrameNode(
        V2::SELECT_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<SelectPattern>(); });
    ViewStackProcessor::GetInstance()->Push(select);

    auto pattern = select->GetPattern<SelectPattern>();
    pattern->BuildChild();

    // create menu node
    auto menuWrapper = MenuView::Create(params, nodeId);
    pattern->SetMenuNode(menuWrapper);
    pattern->InitSelected();

    // store option pointers in select
    auto menuContainer = pattern->GetMenuNode();
    CHECK_NULL_VOID(menuContainer);
    pattern->ClearOptions();
    auto menuPattern = menuContainer->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    auto options = menuPattern->GetOptions();
    for (auto&& option : options) {
        pattern->AddOptionNode(option);
    }

    // delete menu when select node is deleted
    auto eventHub = select->GetEventHub<NG::EventHub>();
    auto destructor = [id = select->GetId()]() {
        auto pipeline = NG::PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto overlayManager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(overlayManager);
        overlayManager->DeleteMenu(id);
    };
    eventHub->SetOnDisappear(destructor);
}

void SelectView::SetSelected(int32_t idx)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelected(idx);
}

void SelectView::SetValue(const std::string& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetValue(value);
}

void SelectView::SetFontSize(const Dimension& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetFontSize(value);
}

void SelectView::SetFontColor(const Color& color)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetFontColor(color);
}

void SelectView::SetItalicFontStyle(const Ace::FontStyle& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetItalicFontStyle(value);
}

void SelectView::SetFontWeight(const FontWeight& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetFontWeight(value);
}

void SelectView::SetFontFamily(const std::vector<std::string>& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetFontFamily(value);
}

void SelectView::SetSelectedOptionBgColor(const Color& color)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionBgColor(color);
}

void SelectView::SetSelectedOptionFontSize(const Dimension& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionFontSize(value);
}

void SelectView::SetSelectedOptionFontColor(const Color& color)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionFontColor(color);
}

void SelectView::SetSelectedOptionItalicFontStyle(const Ace::FontStyle& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionItalicFontStyle(value);
}

void SelectView::SetSelectedOptionFontWeight(const FontWeight& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionFontWeight(value);
}

void SelectView::SetSelectedOptionFontFamily(const std::vector<std::string>& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSelectedOptionFontFamily(value);
}

void SelectView::SetOptionBgColor(const Color& color)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionBgColor(color);
}

void SelectView::SetOptionFontSize(const Dimension& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionFontSize(value);
}

void SelectView::SetOptionFontColor(const Color& color)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionFontColor(color);
}

void SelectView::SetOptionItalicFontStyle(const Ace::FontStyle& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionItalicFontStyle(value);
}

void SelectView::SetOptionFontWeight(const FontWeight& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionFontWeight(value);
}

void SelectView::SetOptionFontFamily(const std::vector<std::string>& value)
{
    auto pattern = ViewStackProcessor::GetInstance()->GetMainFrameNodePattern<SelectPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetOptionFontFamily(value);
}

void SelectView::SetOnSelect(SelectEvent&& onSelect)
{
    auto hub = ViewStackProcessor::GetInstance()->GetMainFrameNodeEventHub<SelectEventHub>();
    CHECK_NULL_VOID(hub);
    hub->SetSelectEvent(std::move(onSelect));
}

} // namespace OHOS::Ace::NG
