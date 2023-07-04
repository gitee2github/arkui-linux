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

#include "frameworks/bridge/declarative_frontend/jsview/js_textinput.h"

#include <optional>
#include <string>
#include <vector>

#include "core/common/container.h"
#include "frameworks/bridge/common/utils/utils.h"
#include "frameworks/bridge/declarative_frontend/engine/functions/js_clipboard_function.h"
#include "frameworks/bridge/declarative_frontend/engine/functions/js_function.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_textfield.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_common_def.h"
#include "frameworks/bridge/declarative_frontend/view_stack_processor.h"
#include "frameworks/core/common/ime/text_input_action.h"
#include "frameworks/core/common/ime/text_input_type.h"
#include "frameworks/core/components/text_field/text_field_component.h"
#include "frameworks/core/components/text_field/textfield_theme.h"

namespace OHOS::Ace::Framework {

void JSTextInput::JSBind(BindingTarget globalObj)
{
    JSClass<JSTextInput>::Declare("TextInput");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSTextInput>::StaticMethod("create", &JSTextInput::Create, opt);
    JSClass<JSTextInput>::StaticMethod("type", &JSTextField::SetType);
    JSClass<JSTextInput>::StaticMethod("placeholderColor", &JSTextField::SetPlaceholderColor);
    JSClass<JSTextInput>::StaticMethod("placeholderFont", &JSTextField::SetPlaceholderFont);
    JSClass<JSTextInput>::StaticMethod("backgroundColor", &JSTextField::SetBackgroundColor);
    JSClass<JSTextInput>::StaticMethod("enterKeyType", &JSTextField::SetEnterKeyType);
    JSClass<JSTextInput>::StaticMethod("caretColor", &JSTextField::SetCaretColor);
    JSClass<JSTextInput>::StaticMethod("maxLength", &JSTextField::SetMaxLength);
    JSClass<JSTextInput>::StaticMethod("width", &JSTextField::JsWidth);
    JSClass<JSTextInput>::StaticMethod("height", &JSTextField::JsHeight);
    JSClass<JSTextInput>::StaticMethod("padding", &JSTextField::JsPadding);
    JSClass<JSTextInput>::StaticMethod("border", &JSTextField::JsBorder);
    JSClass<JSTextInput>::StaticMethod("borderWidth", &JSTextField::JsBorderWidth);
    JSClass<JSTextInput>::StaticMethod("borderColor", &JSTextField::JsBorderColor);
    JSClass<JSTextInput>::StaticMethod("borderStyle", &JSTextField::JsBorderStyle);
    JSClass<JSTextInput>::StaticMethod("borderRadius", &JSTextField::JsBorderRadius);
    JSClass<JSTextInput>::StaticMethod("fontSize", &JSTextField::SetFontSize);
    JSClass<JSTextInput>::StaticMethod("fontColor", &JSTextField::SetTextColor);
    JSClass<JSTextInput>::StaticMethod("fontWeight", &JSTextField::SetFontWeight);
    JSClass<JSTextInput>::StaticMethod("fontStyle", &JSTextField::SetFontStyle);
    JSClass<JSTextInput>::StaticMethod("fontFamily", &JSTextField::SetFontFamily);
    JSClass<JSTextInput>::StaticMethod("inputFilter", &JSTextField::SetInputFilter);
    JSClass<JSTextInput>::StaticMethod("showPasswordIcon", &JSTextField::SetShowPasswordIcon);
    JSClass<JSTextInput>::StaticMethod("textAlign", &JSTextField::SetTextAlign);
    JSClass<JSTextInput>::StaticMethod("style", &JSTextField::SetInputStyle);
    JSClass<JSTextInput>::StaticMethod("hoverEffect", &JSTextField::JsHoverEffect);
    JSClass<JSTextInput>::StaticMethod("copyOption", &JSTextField::SetCopyOption);
    // API7 onEditChanged deprecated
    JSClass<JSTextInput>::StaticMethod("onEditChanged", &JSTextField::SetOnEditChanged);
    JSClass<JSTextInput>::StaticMethod("onEditChange", &JSTextField::SetOnEditChanged);
    JSClass<JSTextInput>::StaticMethod("onSubmit", &JSTextField::SetOnSubmit);
    JSClass<JSTextInput>::StaticMethod("onChange", &JSTextField::SetOnChange);
    JSClass<JSTextInput>::StaticMethod("onCopy", &JSTextField::SetOnCopy);
    JSClass<JSTextInput>::StaticMethod("onCut", &JSTextField::SetOnCut);
    JSClass<JSTextInput>::StaticMethod("onPaste", &JSTextField::SetOnPaste);
    JSClass<JSTextInput>::StaticMethod("onClick", &JSTextField::SetOnClick);
    JSClass<JSTextInput>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSTextInput>::StaticMethod("onHover", &JSInteractableView::JsOnHover);
    JSClass<JSTextInput>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSTextInput>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSTextInput>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSTextInput>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSTextInput>::Inherit<JSViewAbstract>();
    JSClass<JSTextInput>::Bind(globalObj);
}

void JSTextInput::Create(const JSCallbackInfo& info)
{
    JSTextField::CreateTextInput(info);
}

void JSTextInputController::JSBind(BindingTarget globalObj)
{
    JSClass<JSTextInputController>::Declare("TextInputController");
    JSClass<JSTextInputController>::Method("caretPosition", &JSTextInputController::CaretPosition);
    JSClass<JSTextInputController>::Bind(
        globalObj, JSTextInputController::Constructor, JSTextInputController::Destructor);
}

void JSTextInputController::Constructor(const JSCallbackInfo& args)
{
    auto scroller = Referenced::MakeRefPtr<JSTextInputController>();
    scroller->IncRefCount();
    args.SetReturnValue(Referenced::RawPtr(scroller));
}

void JSTextInputController::Destructor(JSTextInputController* scroller)
{
    if (scroller != nullptr) {
        scroller->DecRefCount();
    }
}

void JSTextInputController::CaretPosition(int32_t caretPosition)
{
    auto controller = controllerWeak_.Upgrade();
    if (controller) {
        controller->CaretPosition(caretPosition);
    }
}

} // namespace OHOS::Ace::Framework