/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "frameworks/bridge/declarative_frontend/jsview/js_button.h"

#include "base/geometry/dimension.h"
#include "base/log/ace_scoring_log.h"
#include "base/log/ace_trace.h"
#include "base/log/log_wrapper.h"
#include "bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "core/common/ace_page.h"
#include "core/components/box/box_component_helper.h"
#include "core/components/button/button_component.h"
#include "core/components/button/button_theme.h"
#include "core/components/padding/padding_component.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_abstract_model.h"
#include "core/components_ng/pattern/button/button_view.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "frameworks/bridge/common/utils/utils.h"
#include "frameworks/bridge/declarative_frontend/engine/bindings.h"
#include "frameworks/bridge/declarative_frontend/engine/functions/js_click_function.h"
#include "frameworks/bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "frameworks/bridge/declarative_frontend/view_stack_processor.h"

namespace OHOS::Ace::Framework {

void JSButton::SetFontSize(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension fontSize;
    if (!ParseJsDimensionFp(info[0], fontSize)) {
        return;
    }
    if (Container::IsCurrentUseNewPipeline()) {
        NG::ButtonView::SetFontSize(fontSize);
        return;
    }
    auto textComponent = GetTextComponent();
    if (textComponent) {
        auto textStyle = textComponent->GetTextStyle();
        textStyle.SetFontSize(fontSize);
        textStyle.SetAdaptTextSize(fontSize, fontSize);
        textComponent->SetTextStyle(textStyle);
    }

    auto stack = ViewStackProcessor::GetInstance();
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
    if (!buttonComponent) {
        return;
    }
    if (buttonComponent->NeedResetHeight()) {
        ResetButtonHeight();
    }
}

void JSButton::SetFontWeight(const std::string& value)
{
    if (Container::IsCurrentUseNewPipeline()) {
        NG::ButtonView::SetFontWeight(ConvertStrToFontWeight(value));
        return;
    }
    auto textComponent = GetTextComponent();
    if (textComponent) {
        auto textStyle = textComponent->GetTextStyle();
        textStyle.SetFontWeight(ConvertStrToFontWeight(value));
        textComponent->SetTextStyle(textStyle);
    }
}

void JSButton::SetFontStyle(int32_t value)
{
    const std::vector<FontStyle> fontStyles = { FontStyle::NORMAL, FontStyle::ITALIC };
    if (value < 0 || value >= static_cast<int32_t>(fontStyles.size())) {
        LOGE("Text fontStyle(%d) is invalid value", value);
        return;
    }
    if (Container::IsCurrentUseNewPipeline()) {
        NG::ButtonView::SetFontStyle(fontStyles[value]);
        return;
    }
    auto textComponent = GetTextComponent();
    if (textComponent) {
        auto textStyle = textComponent->GetTextStyle();
        textStyle.SetFontStyle(fontStyles[value]);
        textComponent->SetTextStyle(textStyle);
    }
}

void JSButton::SetFontFamily(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    std::vector<std::string> fontFamilies;
    if (!ParseJsFontFamilies(info[0], fontFamilies)) {
        LOGE("Parse FontFamilies failed");
        return;
    }
    if (Container::IsCurrentUseNewPipeline()) {
        NG::ButtonView::SetFontFamily(fontFamilies);
        return;
    }
    auto textComponent = GetTextComponent();
    if (textComponent) {
        auto textStyle = textComponent->GetTextStyle();
        textStyle.SetFontFamilies(fontFamilies);
        textComponent->SetTextStyle(textStyle);
    }
}

void JSButton::SetTextColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Color textColor;
    if (!ParseJsColor(info[0], textColor)) {
        LOGI("Set text color is invalid, use default text color.");
        auto buttonTheme = PipelineBase::GetCurrentContext()->GetTheme<ButtonTheme>();
        textColor = buttonTheme->GetTextStyle().GetTextColor();
    }
    if (Container::IsCurrentUseNewPipeline()) {
        NG::ButtonView::SetFontColor(textColor);
        return;
    }
    auto textComponent = GetTextComponent();
    if (textComponent) {
        auto textStyle = textComponent->GetTextStyle();
        textStyle.SetTextColor(textColor);
        textComponent->SetTextStyle(textStyle);
    }
}

RefPtr<TextComponent> JSButton::GetTextComponent()
{
    auto stack = ViewStackProcessor::GetInstance();
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
    if (buttonComponent == nullptr) {
        LOGE("Button component create failed");
        return nullptr;
    }
    auto paddingComponent = AceType::DynamicCast<PaddingComponent>(buttonComponent->GetChildren().front());
    if (!paddingComponent) {
        LOGE("Padding component create failed");
        return nullptr;
    }
    auto textComponent = AceType::DynamicCast<TextComponent>(paddingComponent->GetChild());
    return textComponent;
}

void JSButton::SetType(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        return;
    }
    int32_t value = 1;
    if (info[0]->IsNumber()) {
        value = info[0]->ToNumber<int32_t>();
    }
    if ((ButtonType)value == ButtonType::CAPSULE || (ButtonType)value == ButtonType::CIRCLE ||
        (ButtonType)value == ButtonType::ARC || (ButtonType)value == ButtonType::NORMAL) {
        if (Container::IsCurrentUseNewPipeline()) {
            NG::ButtonView::SetType(static_cast<ButtonType>(value));
            return;
        }
        auto stack = ViewStackProcessor::GetInstance();
        auto buttonComponent = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
        if (buttonComponent) {
            buttonComponent->SetType((ButtonType)value);
        }
    } else {
        LOGE("Setting button to non valid ButtonType %d", value);
    }
}

void JSButton::SetStateEffect(bool stateEffect)
{
    if (Container::IsCurrentUseNewPipeline()) {
        NG::ButtonView::SetStateEffect(stateEffect);
        return;
    }
    auto stack = ViewStackProcessor::GetInstance();
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
    if (buttonComponent) {
        buttonComponent->SetStateEffect(stateEffect);
    }
}

void JSButton::JsRemoteMessage(const JSCallbackInfo& info)
{
    RemoteCallback remoteCallback;
    JSInteractableView::JsRemoteMessage(info, remoteCallback);
    EventMarker remoteMessageEventId(std::move(remoteCallback));
    auto stack = ViewStackProcessor::GetInstance();
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
    if (buttonComponent) {
        buttonComponent->SetRemoteMessageEventId(remoteMessageEventId);
    }
}

void JSButton::JSBind(BindingTarget globalObj)
{
    JSClass<JSButton>::Declare("Button");
    JSClass<JSButton>::StaticMethod("fontColor", &JSButton::SetTextColor, MethodOptions::NONE);
    JSClass<JSButton>::StaticMethod("fontSize", &JSButton::SetFontSize, MethodOptions::NONE);
    JSClass<JSButton>::StaticMethod("fontWeight", &JSButton::SetFontWeight, MethodOptions::NONE);
    JSClass<JSButton>::StaticMethod("fontStyle", &JSButton::SetFontStyle, MethodOptions::NONE);
    JSClass<JSButton>::StaticMethod("fontFamily", &JSButton::SetFontFamily, MethodOptions::NONE);
    JSClass<JSButton>::StaticMethod("type", &JSButton::SetType, MethodOptions::NONE);
    JSClass<JSButton>::StaticMethod("stateEffect", &JSButton::SetStateEffect, MethodOptions::NONE);
    JSClass<JSButton>::StaticMethod("onClick", &JSButton::JsOnClick);
    JSClass<JSButton>::StaticMethod("remoteMessage", &JSButton::JsRemoteMessage);
    JSClass<JSButton>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSButton>::StaticMethod("onHover", &JSInteractableView::JsOnHover);
    JSClass<JSButton>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSButton>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSButton>::StaticMethod("backgroundColor", &JSButton::JsBackgroundColor);
    JSClass<JSButton>::StaticMethod("width", &JSButton::JsWidth);
    JSClass<JSButton>::StaticMethod("height", &JSButton::JsHeight);
    JSClass<JSButton>::StaticMethod("aspectRatio", &JSButton::JsAspectRatio);
    JSClass<JSButton>::StaticMethod("borderRadius", &JSButton::JsRadius);
    JSClass<JSButton>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSButton>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSButton>::StaticMethod("size", &JSButton::JsSize);
    JSClass<JSButton>::StaticMethod("padding", &JSButton::JsPadding);
    JSClass<JSButton>::StaticMethod("hoverEffect", &JSButton::JsHoverEffect);

    JSClass<JSButton>::StaticMethod("createWithLabel", &JSButton::CreateWithLabel, MethodOptions::NONE);
    JSClass<JSButton>::StaticMethod("createWithChild", &JSButton::CreateWithChild, MethodOptions::NONE);
    JSClass<JSButton>::Inherit<JSContainerBase>();
    JSClass<JSButton>::Inherit<JSViewAbstract>();
    JSClass<JSButton>::Bind<>(globalObj);
}

void JSButton::CreateWithLabel(const JSCallbackInfo& info)
{
    std::list<RefPtr<Component>> buttonChildren;
    std::string label;
    bool labelSet = false;
    if (ParseJsString(info[0], label)) {
        labelSet = true;
        if (Container::IsCurrentUseNewPipeline()) {
            NG::ButtonView::CreateWithLabel(label);
            if (!labelSet && info[0]->IsObject()) {
                SetTypeAndStateEffect(JSRef<JSObject>::Cast(info[0]));
            }
            if ((info.Length() > 1) && info[1]->IsObject()) {
                SetTypeAndStateEffect(JSRef<JSObject>::Cast(info[1]));
            }
            NG::ViewAbstract::SetHoverEffectAuto(HoverEffectType::SCALE);
            return;
        }
        auto textComponent = AceType::MakeRefPtr<TextComponent>(label);
        auto buttonTheme = GetTheme<ButtonTheme>();
        auto textStyle = buttonTheme ? buttonTheme->GetTextStyle() : textComponent->GetTextStyle();
        textStyle.SetMaxLines(buttonTheme ? buttonTheme->GetTextMaxLines() : 1);
        textStyle.SetTextOverflow(TextOverflow::ELLIPSIS);
        textComponent->SetTextStyle(textStyle);
        auto padding = AceType::MakeRefPtr<PaddingComponent>();
        padding->SetPadding(buttonTheme ? buttonTheme->GetPadding() : Edge());
        padding->SetChild(textComponent);
        Component::MergeRSNode(padding, textComponent);
        buttonChildren.emplace_back(padding);
    }

    if (Container::IsCurrentUseNewPipeline()) {
        NG::ButtonView::Create(V2::BUTTON_ETS_TAG);
        if (!labelSet && info[0]->IsObject()) {
            SetTypeAndStateEffect(JSRef<JSObject>::Cast(info[0]));
        }
        if ((info.Length() > 1) && info[1]->IsObject()) {
            SetTypeAndStateEffect(JSRef<JSObject>::Cast(info[1]));
        }
        NG::ViewAbstract::SetHoverEffectAuto(HoverEffectType::SCALE);
        return;
    }
    auto buttonComponent = AceType::MakeRefPtr<ButtonComponent>(buttonChildren);
    ViewStackProcessor::GetInstance()->ClaimElementId(buttonComponent);
    buttonComponent->SetHasCustomChild(false);
    buttonComponent->SetCatchMode(false);
    SetDefaultAttributes(buttonComponent);
    if (!labelSet && info[0]->IsObject()) {
        SetTypeAndStateEffect(JSRef<JSObject>::Cast(info[0]), buttonComponent);
    }
    if ((info.Length() > 1) && info[1]->IsObject()) {
        SetTypeAndStateEffect(JSRef<JSObject>::Cast(info[1]), buttonComponent);
    }
    ViewStackProcessor::GetInstance()->Push(buttonComponent);
    JSInteractableView::SetFocusable(true);
    JSInteractableView::SetFocusNode(true);

    buttonComponent->SetMouseAnimationType(HoverAnimationType::SCALE);
}

void JSButton::CreateWithChild(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        NG::ButtonView::Create(V2::BUTTON_ETS_TAG);
        if (info[0]->IsObject()) {
            auto obj = JSRef<JSObject>::Cast(info[0]);
            SetTypeAndStateEffect(obj);
        }
        NG::ViewAbstract::SetHoverEffectAuto(HoverEffectType::SCALE);
        return;
    }
    std::list<RefPtr<Component>> buttonChildren;
    auto buttonComponent = AceType::MakeRefPtr<ButtonComponent>(buttonChildren);
    ViewStackProcessor::GetInstance()->ClaimElementId(buttonComponent);
    buttonComponent->SetHasCustomChild(true);
    buttonComponent->SetCatchMode(false);
    SetDefaultAttributes(buttonComponent);
    if (info[0]->IsObject()) {
        auto obj = JSRef<JSObject>::Cast(info[0]);
        SetTypeAndStateEffect(obj, buttonComponent);
    }
    if ((info.Length() > 1) && info[1]->IsObject()) {
        SetTypeAndStateEffect(JSRef<JSObject>::Cast(info[1]), buttonComponent);
    }
    ViewStackProcessor::GetInstance()->Push(buttonComponent);
    JSInteractableView::SetFocusable(true);
    JSInteractableView::SetFocusNode(true);
    buttonComponent->SetMouseAnimationType(HoverAnimationType::SCALE);
    if (buttonComponent->NeedResetHeight()) {
        ResetButtonHeight();
    }
}

void JSButton::SetDefaultAttributes(const RefPtr<ButtonComponent>& buttonComponent)
{
    buttonComponent->SetType(ButtonType::CAPSULE);
    buttonComponent->SetDeclarativeFlag(true);
    auto buttonTheme = GetTheme<ButtonTheme>();
    if (!buttonTheme) {
        return;
    }
    buttonComponent->SetHeight(buttonTheme->GetHeight());
    buttonComponent->SetBackgroundColor(buttonTheme->GetBgColor());
    buttonComponent->SetClickedColor(buttonComponent->GetBackgroundColor().BlendColor(buttonTheme->GetClickedColor()));
    buttonComponent->SetHoverColor(buttonTheme->GetHoverColor());
}

void JSButton::SetTypeAndStateEffect(const JSRef<JSObject>& obj)
{
    auto type = obj->GetProperty("type");
    if (type->IsNumber()) {
        auto buttonType = static_cast<ButtonType>(type->ToNumber<int32_t>());
        NG::ButtonView::SetType(buttonType);
    }
    auto stateEffect = obj->GetProperty("stateEffect");
    if (stateEffect->IsBoolean()) {
        NG::ButtonView::SetStateEffect(stateEffect->ToBoolean());
    }
}

void JSButton::SetTypeAndStateEffect(const JSRef<JSObject>& obj, const RefPtr<ButtonComponent>& buttonComponent)
{
    auto type = obj->GetProperty("type");
    if (type->IsNumber()) {
        auto buttonType = (ButtonType)type->ToNumber<int32_t>();
        buttonComponent->SetType(buttonType);
    }
    auto stateEffect = obj->GetProperty("stateEffect");
    if (stateEffect->IsBoolean()) {
        buttonComponent->SetStateEffect(stateEffect->ToBoolean());
    }
}

void JSButton::ResetButtonHeight()
{
    auto stack = ViewStackProcessor::GetInstance();
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
    if (buttonComponent) {
        if (buttonComponent->GetType() == ButtonType::CIRCLE) {
            return;
        }
        const Dimension initialHeight = Dimension(-1.0, DimensionUnit::VP);
        buttonComponent->SetHeight(initialHeight);
    }
}

void JSButton::JsPadding(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsPadding(info);
        return;
    }
    if (!info[0]->IsString() && !info[0]->IsNumber() && !info[0]->IsObject()) {
        LOGE("arg is not a string, number or object.");
        return;
    }
    Edge padding;
    if (info[0]->IsNumber()) {
        Dimension edgeValue;
        if (ParseJsDimensionVp(info[0], edgeValue)) {
            padding = Edge(edgeValue);
        }
    }
    if (info[0]->IsObject()) {
        auto object = JsonUtil::ParseJsonString(info[0]->ToString());
        if (!object) {
            LOGE("Js Parse object failed. argsPtr is null.");
            return;
        }
        Dimension left = Dimension(0.0, DimensionUnit::VP);
        Dimension top = Dimension(0.0, DimensionUnit::VP);
        Dimension right = Dimension(0.0, DimensionUnit::VP);
        Dimension bottom = Dimension(0.0, DimensionUnit::VP);
        if (object->Contains("top") || object->Contains("bottom") || object->Contains("left") ||
            object->Contains("right")) {
            ParseJsonDimensionVp(object->GetValue("left"), left);
            ParseJsonDimensionVp(object->GetValue("top"), top);
            ParseJsonDimensionVp(object->GetValue("right"), right);
            ParseJsonDimensionVp(object->GetValue("bottom"), bottom);
        }
        padding = Edge(left, top, right, bottom);
    }
    auto stack = ViewStackProcessor::GetInstance();
    auto component = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
    if (component) {
        auto paddingChild = AceType::DynamicCast<PaddingComponent>(component->GetChildren().front());
        if (paddingChild) {
            paddingChild->SetPadding(padding);
        }
        if (component->NeedResetHeight()) {
            ResetButtonHeight();
        }
    }
}

void JSButton::JsOnClick(const JSCallbackInfo& info)
{
    LOGD("JSButton JsOnClick");
    if (!info[0]->IsFunction()) {
        LOGE("OnClick parameter need a function.");
        return;
    }
    if (Container::IsCurrentUseNewPipeline()) {
        JSInteractableView::JsOnClick(info);
        return;
    }
    auto inspector = ViewStackProcessor::GetInstance()->GetInspectorComposedComponent();
    if (!inspector) {
        LOGE("fail to get inspector for on click event");
        return;
    }
    auto impl = inspector->GetInspectorFunctionImpl();

    RefPtr<JsClickFunction> jsOnClickFunc = AceType::MakeRefPtr<JsClickFunction>(JSRef<JSFunc>::Cast(info[0]));
    auto clickId = [execCtx = info.GetExecutionContext(), func = std::move(jsOnClickFunc), impl](GestureEvent& info) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        if (impl) {
            impl->UpdateEventInfo(info);
        }
        ACE_SCORING_EVENT("onClick");
        func->Execute(info);
    };
    RefPtr<Gesture> tapGesture = AceType::MakeRefPtr<TapGesture>(DEFAULT_TAP_COUNTS, DEFAULT_TAP_FINGERS);
    if (!tapGesture) {
        LOGE("tapGesture is null");
        return;
    }
    tapGesture->SetOnActionId(clickId);
    auto box = ViewStackProcessor::GetInstance()->GetBoxComponent();
    if (tapGesture) {
        box->SetOnClick(tapGesture);
    }

    RefPtr<JsClickFunction> jsClickEventFunc = AceType::MakeRefPtr<JsClickFunction>(JSRef<JSFunc>::Cast(info[0]));
    EventMarker clickEventId(
        [execCtx = info.GetExecutionContext(), func = std::move(jsClickEventFunc), impl](const BaseEventInfo* info) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            auto clickInfo = TypeInfoHelper::DynamicCast<ClickInfo>(info);
            auto newInfo = *clickInfo;
            if (impl) {
                impl->UpdateEventInfo(newInfo);
            }
            ACE_SCORING_EVENT("Button.onClick");
            func->Execute(newInfo);
        });
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (buttonComponent) {
        buttonComponent->SetKeyEnterEventId(clickEventId);
    }
    auto focusableComponent = ViewStackProcessor::GetInstance()->GetFocusableComponent(false);
    if (focusableComponent) {
        focusableComponent->SetOnClickId(clickEventId);
    }
}

void JSButton::JsBackgroundColor(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsBackgroundColor(info);
        return;
    }
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Color backgroundColor;
    if (!ParseJsColor(info[0], backgroundColor)) {
        return;
    }
    auto stack = ViewStackProcessor::GetInstance();
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
    AnimationOption option = ViewStackProcessor::GetInstance()->GetImplicitAnimationOption();
    if (buttonComponent == nullptr) {
        LOGE("Button component create failed");
        return;
    }
    if (!stack->IsVisualStateSet()) {
        buttonComponent->SetBackgroundColor(backgroundColor);
        auto buttonTheme = GetTheme<ButtonTheme>();
        if (buttonTheme) {
            Color blendColor = buttonTheme->GetClickedColor();
            buttonComponent->SetClickedColor(buttonComponent->GetBackgroundColor().BlendColor(blendColor));
        }
    } else {
        buttonComponent->GetStateAttributes()->AddAttribute<AnimatableColor>(
            ButtonStateAttribute::COLOR, AnimatableColor(backgroundColor, option), stack->GetVisualState());
        if (!buttonComponent->GetStateAttributes()->HasAttribute(ButtonStateAttribute::COLOR, VisualState::NORMAL)) {
            buttonComponent->GetStateAttributes()->AddAttribute<AnimatableColor>(ButtonStateAttribute::COLOR,
                AnimatableColor(buttonComponent->GetBackgroundColor(), option), VisualState::NORMAL);
        }
    }
    info.ReturnSelf();
}

void JSButton::JsWidth(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsWidth(info);
        return;
    }
    JSViewAbstract::JsWidth(info);
    Dimension value = GetSizeValue(info);
    if (LessNotEqual(value.Value(), 0.0)) {
        return;
    }
    auto stack = ViewStackProcessor::GetInstance();
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
    AnimationOption option = stack->GetImplicitAnimationOption();
    if (!buttonComponent) {
        return;
    }
    if (!stack->IsVisualStateSet()) {
        buttonComponent->SetWidth(value, stack->GetImplicitAnimationOption());
    } else {
        buttonComponent->GetStateAttributes()->AddAttribute<AnimatableDimension>(
            ButtonStateAttribute::WIDTH, AnimatableDimension(value, option), stack->GetVisualState());
        if (!buttonComponent->GetStateAttributes()->HasAttribute(ButtonStateAttribute::WIDTH, VisualState::NORMAL)) {
            buttonComponent->GetStateAttributes()->AddAttribute<AnimatableDimension>(ButtonStateAttribute::WIDTH,
                AnimatableDimension(buttonComponent->GetWidth(), option), VisualState::NORMAL);
        }
    }
}

void JSButton::JsHeight(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsHeight(info);
        return;
    }
    JSViewAbstract::JsHeight(info);
    Dimension value = GetSizeValue(info);
    if (LessNotEqual(value.Value(), 0.0)) {
        return;
    }
    auto stack = ViewStackProcessor::GetInstance();
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
    auto option = stack->GetImplicitAnimationOption();
    if (!buttonComponent) {
        return;
    }
    buttonComponent->IsNeedResetHeight(false);
    if (!stack->IsVisualStateSet()) {
        buttonComponent->SetHeight(value, option);
        buttonComponent->SetDeclareHeight(true);
    } else {
        buttonComponent->GetStateAttributes()->AddAttribute<AnimatableDimension>(
            ButtonStateAttribute::HEIGHT, AnimatableDimension(value, option), stack->GetVisualState());
        if (!buttonComponent->GetStateAttributes()->HasAttribute(ButtonStateAttribute::HEIGHT, VisualState::NORMAL)) {
            buttonComponent->GetStateAttributes()->AddAttribute<AnimatableDimension>(ButtonStateAttribute::HEIGHT,
                AnimatableDimension(buttonComponent->GetHeight(), option), VisualState::NORMAL);
        }
    }
}

void JSButton::JsAspectRatio(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsAspectRatio(info);
        return;
    }
    JSViewAbstract::JsAspectRatio(info);
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }
    double value = 0.0;
    if (!ParseJsDouble(info[0], value)) {
        return;
    }
    auto stack = ViewStackProcessor::GetInstance();
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
    if (!buttonComponent) {
        return;
    }
    buttonComponent->SetAspectRatio(value);
}

void JSButton::JsSize(const JSCallbackInfo& info)
{
    if (info.Length() < 0) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    if (!info[0]->IsObject()) {
        LOGE("arg is not Object or String.");
        return;
    }
    
    if (Container::IsCurrentUseNewPipeline()) {
        JSRef<JSObject> sizeObj = JSRef<JSObject>::Cast(info[0]);
        Dimension width;
        if (ParseJsDimensionVp(sizeObj->GetProperty("width"), width)) {
            NG::ViewAbstract::SetWidth(NG::CalcLength(width));
        }
        Dimension height;
        if (ParseJsDimensionVp(sizeObj->GetProperty("height"), height)) {
            NG::ViewAbstract::SetHeight(NG::CalcLength(height));
        }
        return;
    }
    auto stack = ViewStackProcessor::GetInstance();
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
    auto option = stack->GetImplicitAnimationOption();
    if (buttonComponent == nullptr) {
        LOGE("Button component create failed");
        return;
    }
    JSRef<JSObject> sizeObj = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> widthValue = sizeObj->GetProperty("width");
    Dimension width;
    if (ParseJsDimensionVp(widthValue, width)) {
        if (!stack->IsVisualStateSet()) {
            buttonComponent->SetWidth(width, stack->GetImplicitAnimationOption());
        } else {
            buttonComponent->GetStateAttributes()->AddAttribute<AnimatableDimension>(
                ButtonStateAttribute::WIDTH, AnimatableDimension(width, option), stack->GetVisualState());
            if (!buttonComponent->GetStateAttributes()->HasAttribute(
                    ButtonStateAttribute::WIDTH, VisualState::NORMAL)) {
                buttonComponent->GetStateAttributes()->AddAttribute<AnimatableDimension>(ButtonStateAttribute::WIDTH,
                    AnimatableDimension(buttonComponent->GetWidth(), option), VisualState::NORMAL);
            }
        }
    }
    JSRef<JSVal> heightValue = sizeObj->GetProperty("height");
    Dimension height;
    if (ParseJsDimensionVp(heightValue, height)) {
        buttonComponent->IsNeedResetHeight(false);
        if (!stack->IsVisualStateSet()) {
            buttonComponent->SetHeight(height, stack->GetImplicitAnimationOption());
            buttonComponent->SetDeclareHeight(true);
        } else {
            buttonComponent->GetStateAttributes()->AddAttribute<AnimatableDimension>(
                ButtonStateAttribute::HEIGHT, AnimatableDimension(height, option), stack->GetVisualState());
            if (!buttonComponent->GetStateAttributes()->HasAttribute(
                    ButtonStateAttribute::HEIGHT, VisualState::NORMAL)) {
                buttonComponent->GetStateAttributes()->AddAttribute<AnimatableDimension>(ButtonStateAttribute::HEIGHT,
                    AnimatableDimension(buttonComponent->GetHeight(), option), VisualState::NORMAL);
            }
        }
    }
}

void JSButton::JsRadius(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension radius;
    if (!ParseJsDimensionVp(info[0], radius)) {
        return;
    }
    if (Container::IsCurrentUseNewPipeline()) {
        NG::ButtonView::SetBorderRadius(radius);
        return;
    }
    auto stack = ViewStackProcessor::GetInstance();
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(stack->GetMainComponent());
    auto option = stack->GetImplicitAnimationOption();
    if (buttonComponent == nullptr) {
        LOGE("Button component create failed");
        return;
    }
    buttonComponent->SetRadiusState(true);
    if (!stack->IsVisualStateSet()) {
        buttonComponent->SetRectRadius(radius);
        ViewAbstractModel::GetInstance()->SetBorderRadius(radius);
    } else {
        buttonComponent->GetStateAttributes()->AddAttribute<AnimatableDimension>(
            ButtonStateAttribute::RADIUS, AnimatableDimension(radius, option), stack->GetVisualState());
        auto boxComponent = stack->GetBoxComponent();
        boxComponent->GetStateAttributes()->AddAttribute<AnimatableDimension>(
            BoxStateAttribute::BORDER_RADIUS, AnimatableDimension(radius, option), stack->GetVisualState());

        if (!buttonComponent->GetStateAttributes()->HasAttribute(ButtonStateAttribute::RADIUS, VisualState::NORMAL)) {
            buttonComponent->GetStateAttributes()->AddAttribute<AnimatableDimension>(ButtonStateAttribute::RADIUS,
                AnimatableDimension(buttonComponent->GetRectRadius(), option), VisualState::NORMAL);
            auto defaultRadius = BoxComponentHelper::GetBorderRadius(boxComponent->GetBackDecoration());
            boxComponent->GetStateAttributes()->AddAttribute<AnimatableDimension>(BoxStateAttribute::BORDER_RADIUS,
                AnimatableDimension(defaultRadius.GetX(), option), VisualState::NORMAL);
        }
    }
}

Dimension JSButton::GetSizeValue(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return Dimension(-1.0);
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return Dimension(-1.0);
    }
    return value;
}

void JSButton::JsHoverEffect(const JSCallbackInfo& info)
{
    if (!info[0]->IsNumber()) {
        LOGE("The arg is not a number");
        return;
    }
    if (Container::IsCurrentUseNewPipeline()) {
        NG::ViewAbstract::SetHoverEffect(static_cast<HoverEffectType>(info[0]->ToNumber<int32_t>()));
        return;
    }
    auto buttonComponent = AceType::DynamicCast<ButtonComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (buttonComponent) {
        buttonComponent->SetMouseAnimationType(static_cast<HoverAnimationType>(info[0]->ToNumber<int32_t>()));
    }
}

} // namespace OHOS::Ace::Framework
