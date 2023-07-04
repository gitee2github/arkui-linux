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

#include "bridge/declarative_frontend/jsview/models/radio_model_impl.h"

#include <utility>

#include "bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "bridge/declarative_frontend/jsview/js_view_common_def.h"
#include "bridge/declarative_frontend/view_stack_processor.h"
#include "core/components/checkable/checkable_component.h"
#include "core/components/checkable/checkable_theme.h"

namespace OHOS::Ace::Framework {

void RadioModelImpl::Create(const std::optional<std::string>& value, const std::optional<std::string>& group)
{
    RefPtr<RadioTheme> radioTheme = JSViewAbstract::GetTheme<RadioTheme>();
    auto radioComponent = AceType::MakeRefPtr<OHOS::Ace::RadioComponent<std::string>>(radioTheme);

    if (value.has_value()) {
        const auto& radioValue = value.value();
        radioComponent->SetValue(radioValue);
    }
    if (group.has_value()) {
        const auto& radioGroupName = group.value();
        radioComponent->SetGroupName(radioGroupName);
        auto radioGroupComponent = ViewStackProcessor::GetInstance()->GetRadioGroupComponent();
        auto& radioGroup = (*radioGroupComponent)[radioGroupName];
        radioGroup.SetIsDeclarative(true);
        radioGroup.AddRadio(radioComponent);
    }

    radioComponent->SetMouseAnimationType(HoverAnimationType::NONE);
    ViewStackProcessor::GetInstance()->Push(radioComponent);
    auto box = ViewStackProcessor::GetInstance()->GetBoxComponent();
    auto horizontalPadding = radioTheme->GetHotZoneHorizontalPadding();
    auto verticalPadding = radioTheme->GetHotZoneVerticalPadding();
    radioComponent->SetWidth(radioTheme->GetWidth() - horizontalPadding * 2);
    radioComponent->SetHeight(radioTheme->GetHeight() - verticalPadding * 2);
    box->SetDeliverMinToChild(true);
    box->SetWidth(radioTheme->GetWidth());
    box->SetHeight(radioTheme->GetHeight());
}

void RadioModelImpl::SetChecked(bool isChecked)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto radioComponent = AceType::DynamicCast<RadioComponent<std::string>>(stack->GetMainComponent());
    if (isChecked) {
        radioComponent->SetGroupValue(radioComponent->GetValue());
        radioComponent->SetOriginChecked(isChecked);
    } else {
        radioComponent->SetGroupValue("");
    }
}

void RadioModelImpl::SetOnChange(NG::ChangeEvent&& onChange)
{
    JSViewSetProperty(&CheckableComponent::SetOnChange, std::move(onChange));
}

void RadioModelImpl::SetWidth(const Dimension& width)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto box = stack->GetBoxComponent();
    auto radioComponent = AceType::DynamicCast<RadioComponent<std::string>>(stack->GetMainComponent());
    if (radioComponent) {
        auto padding = radioComponent->GetHotZoneHorizontalPadding();
        radioComponent->SetWidth(width);
        box->SetWidth(width + padding * 2);
    }
}

void RadioModelImpl::SetHeight(const Dimension& height)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto box = stack->GetBoxComponent();
    auto radioComponent = AceType::DynamicCast<RadioComponent<std::string>>(stack->GetMainComponent());
    if (radioComponent) {
        auto padding = radioComponent->GetHotZoneVerticalPadding();
        radioComponent->SetHeight(height);
        box->SetHeight(height + padding * 2);
    }
}

void RadioModelImpl::SetPadding(const NG::PaddingPropertyF& args)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto radioComponent = AceType::DynamicCast<RadioComponent<std::string>>(stack->GetMainComponent());
    auto box = ViewStackProcessor::GetInstance()->GetBoxComponent();

    if (radioComponent) {
        auto width = radioComponent->GetWidth();
        auto height = radioComponent->GetHeight();
        radioComponent->SetHeight(height);
        radioComponent->SetWidth(width);
        box->SetHeight(height + Dimension(args.top.value(), DimensionUnit::VP) * 2);
        box->SetWidth(width + Dimension(args.left.value(), DimensionUnit::VP) * 2);
        radioComponent->SetHotZoneVerticalPadding(Dimension(args.top.value(), DimensionUnit::VP));
        radioComponent->SetHorizontalPadding(Dimension(args.left.value(), DimensionUnit::VP));
    }
}

} // namespace OHOS::Ace::Framework
