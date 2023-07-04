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

#include "frameworks/bridge/declarative_frontend/jsview/js_texttimer.h"

#include <regex>

#include "base/log/ace_scoring_log.h"
#include "bridge/declarative_frontend/engine/js_types.h"
#include "bridge/declarative_frontend/jsview/js_view_common_def.h"
#include "bridge/declarative_frontend/jsview/models/text_timer_model_impl.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/texttimer/text_timer_model.h"
#include "core/components_ng/pattern/texttimer/text_timer_model_ng.h"

namespace OHOS::Ace {

std::unique_ptr<TextTimerModel> TextTimerModel::instance_ = nullptr;

TextTimerModel* TextTimerModel::GetInstance()
{
    if (!instance_) {
#ifdef NG_BUILD
        instance_.reset(new NG::TextTimerModelNG());
#else
        if (Container::IsCurrentUseNewPipeline()) {
            instance_.reset(new NG::TextTimerModelNG());
        } else {
            instance_.reset(new Framework::TextTimerModelImpl());
        }
#endif
    }
    return instance_.get();
}
} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {
namespace {
const std::vector<FontStyle> FONT_STYLES = { FontStyle::NORMAL, FontStyle::ITALIC };
const std::string DEFAULT_FORMAT = "HH:mm:ss.SS";
constexpr double MAX_COUNT_DOWN = 86400000.0;
} // namespace

void JSTextTimer::Create(const JSCallbackInfo& info)
{
    if (info.Length() < 1 || !info[0]->IsObject()) {
        LOGE("TextTimer create error, info is non-valid");
        return;
    }
    auto controller = TextTimerModel::GetInstance()->Create();
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    auto tempIsCountDown = paramObject->GetProperty("isCountDown");
    if (tempIsCountDown->IsBoolean()) {
        bool isCountDown = tempIsCountDown->ToBoolean();
        TextTimerModel::GetInstance()->SetIsCountDown(isCountDown);
        if (isCountDown) {
            auto count = paramObject->GetProperty("count");
            if (count->IsNumber()) {
                auto inputCount = count->ToNumber<double>();
                if (inputCount > 0 && inputCount < MAX_COUNT_DOWN) {
                    TextTimerModel::GetInstance()->SetInputCount(inputCount);
                } else {
                    LOGE("Parameter out of range, use default value.");
                }
            }
        }
    }

    auto controllerObj = paramObject->GetProperty("controller");
    if (controllerObj->IsObject()) {
        auto* jsController = JSRef<JSObject>::Cast(controllerObj)->Unwrap<JSTextTimerController>();
        if (jsController) {
            jsController->SetController(controller);
        }
    }
}

void JSTextTimer::JSBind(BindingTarget globalObj)
{
    JSClass<JSTextTimer>::Declare("TextTimer");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSTextTimer>::StaticMethod("create", &JSTextTimer::Create, opt);
    JSClass<JSTextTimer>::StaticMethod("format", &JSTextTimer::SetFormat);
    JSClass<JSTextTimer>::StaticMethod("fontColor", &JSTextTimer::SetTextColor);
    JSClass<JSTextTimer>::StaticMethod("fontSize", &JSTextTimer::SetFontSize);
    JSClass<JSTextTimer>::StaticMethod("fontWeight", &JSTextTimer::SetFontWeight, opt);
    JSClass<JSTextTimer>::StaticMethod("fontStyle", &JSTextTimer::SetFontStyle, opt);
    JSClass<JSTextTimer>::StaticMethod("fontFamily", &JSTextTimer::SetFontFamily, opt);
    JSClass<JSTextTimer>::StaticMethod("onTimer", &JSTextTimer::OnTimer);
    JSClass<JSTextTimer>::StaticMethod("onClick", &JSInteractableView::JsOnClick);
    JSClass<JSTextTimer>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSTextTimer>::Inherit<JSViewAbstract>();
    JSClass<JSTextTimer>::Bind<>(globalObj);
}

void JSTextTimer::SetFormat(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    if (!info[0]->IsString()) {
        LOGE("arg is not string.");
        return;
    }

    auto format = info[0]->ToString();
    std::smatch result;
    std::regex pattern("(([YyMdD]+))");
    if (std::regex_search(format, result, pattern)) {
        if (!result.empty()) {
            format = DEFAULT_FORMAT;
        }
    }

    auto pos = format.find("hh");
    if (pos != std::string::npos) {
        format.replace(pos, sizeof("hh") - 1, "HH");
    }

    TextTimerModel::GetInstance()->SetFormat(format);
}

void JSTextTimer::SetFontSize(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("JSTextInput::SetFontSize The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension fontSize;
    if (!ParseJsDimensionFp(info[0], fontSize)) {
        return;
    }

    TextTimerModel::GetInstance()->SetFontSize(fontSize);
}

void JSTextTimer::SetTextColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Color textColor;
    if (!ParseJsColor(info[0], textColor)) {
        return;
    }

    TextTimerModel::GetInstance()->SetTextColor(textColor);
}

void JSTextTimer::SetFontWeight(const std::string& value)
{
    TextTimerModel::GetInstance()->SetFontWeight(ConvertStrToFontWeight(value));
}

void JSTextTimer::SetFontStyle(int32_t value)
{
    if (value < 0 || value >= static_cast<int32_t>(FONT_STYLES.size())) {
        LOGE("TextTimer fontStyle(%{public}d) illegal value", value);
        return;
    }
    TextTimerModel::GetInstance()->SetItalicFontStyle(FONT_STYLES[value]);
}

void JSTextTimer::SetFontFamily(const JSCallbackInfo& info)
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
    TextTimerModel::GetInstance()->SetFontFamily(fontFamilies);
}

void JSTextTimer::OnTimer(const JSCallbackInfo& info)
{
    CHECK_NULL_VOID(info[0]->IsFunction());
    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onChange = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](
                        const std::string& utc, const std::string& elapsedTime) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("TextTimer.onTimer");
        JSRef<JSVal> newJSVal[2];
        newJSVal[0] = JSRef<JSVal>::Make(ToJSValue(utc));
        newJSVal[1] = JSRef<JSVal>::Make(ToJSValue(elapsedTime));
        func->ExecuteJS(2, newJSVal);
    };
    TextTimerModel::GetInstance()->SetOnTimer(std::move(onChange));
}

void JSTextTimerController::JSBind(BindingTarget globalObj)
{
    JSClass<JSTextTimerController>::Declare("TextTimerController");
    JSClass<JSTextTimerController>::CustomMethod("start", &JSTextTimerController::Start);
    JSClass<JSTextTimerController>::CustomMethod("pause", &JSTextTimerController::Pause);
    JSClass<JSTextTimerController>::CustomMethod("reset", &JSTextTimerController::Reset);
    JSClass<JSTextTimerController>::Bind(
        globalObj, JSTextTimerController::Constructor, JSTextTimerController::Destructor);
}

void JSTextTimerController::Constructor(const JSCallbackInfo& info)
{
    auto timerController = Referenced::MakeRefPtr<JSTextTimerController>();
    timerController->IncRefCount();
    info.SetReturnValue(Referenced::RawPtr(timerController));
}

void JSTextTimerController::Destructor(JSTextTimerController* timerController)
{
    if (timerController != nullptr) {
        timerController->DecRefCount();
    }
}

void JSTextTimerController::Start(const JSCallbackInfo& info)
{
    if (controller_) {
        controller_->Start();
    }
}

void JSTextTimerController::Pause(const JSCallbackInfo& info)
{
    if (controller_) {
        controller_->Pause();
    }
}

void JSTextTimerController::Reset(const JSCallbackInfo& info)
{
    if (controller_) {
        controller_->Reset();
    }
}
} // namespace OHOS::Ace::Framework