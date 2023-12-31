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

#include "core/components_ng/pattern/texttimer/text_timer_model_ng.h"

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/texttimer/text_timer_pattern.h"

namespace OHOS::Ace::NG {
RefPtr<TextTimerController> TextTimerModelNG::Create()
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::TEXTTIMER_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<TextTimerPattern>(); });
    stack->Push(frameNode);
    auto pattern = frameNode->GetPattern<TextTimerPattern>();
    return pattern ? pattern->GetTextTimerController() : nullptr;
}

void TextTimerModelNG::SetFormat(const std::string& format)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TextTimerLayoutProperty, Format, format);
}

void TextTimerModelNG::SetIsCountDown(bool isCountDown)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TextTimerLayoutProperty, IsCountDown, isCountDown);
}

void TextTimerModelNG::SetInputCount(double count)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TextTimerLayoutProperty, InputCount, count);
}

void TextTimerModelNG::SetFontSize(const Dimension& value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TextLayoutProperty, FontSize, value);
}

void TextTimerModelNG::SetTextColor(const Color& value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TextLayoutProperty, TextColor, value);
}

void TextTimerModelNG::SetItalicFontStyle(Ace::FontStyle value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TextLayoutProperty, ItalicFontStyle, value);
}

void TextTimerModelNG::SetFontWeight(FontWeight value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TextLayoutProperty, FontWeight, value);
}

void TextTimerModelNG::SetFontFamily(const std::vector<std::string>& value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(TextLayoutProperty, FontFamily, value);
}

void TextTimerModelNG::SetOnTimer(std::function<void(const std::string, const std::string)>&& onChange)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<TextTimerEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnTimer(std::move(onChange));
}
} // namespace OHOS::Ace::NG
