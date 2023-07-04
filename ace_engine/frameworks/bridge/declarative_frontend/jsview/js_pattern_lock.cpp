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

#include "bridge/declarative_frontend/jsview/js_pattern_lock.h"

#include "base/log/ace_trace.h"
#include "bridge/declarative_frontend/jsview/models/patternlock_model_impl.h"
#include "bridge/declarative_frontend/view_stack_processor.h"
#include "core/components_ng/pattern/patternlock/patternlock_model_ng.h"
#include "core/components_v2/pattern_lock/pattern_lock_component.h"

namespace OHOS::Ace {

std::unique_ptr<PatternLockModel> PatternLockModel::instance_ = nullptr;

PatternLockModel* PatternLockModel::GetInstance()
{
    if (!instance_) {
#ifdef NG_BUILD
        instance_.reset(new NG::PatternLockModelNG());
#else
        if (Container::IsCurrentUseNewPipeline()) {
            instance_.reset(new NG::PatternLockModelNG());
        } else {
            instance_.reset(new Framework::PatternLockModelImpl());
        }
#endif
    }
    return instance_.get();
}

} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {
JSRef<JSArray> JSPatternLock::ChoosePointToJSValue(std::vector<int> input)
{
    JSRef<JSArray> arr = JSRef<JSArray>::New();
    for (size_t i = 0; i < input.size(); i++) {
        arr->SetValueAt(i, ConvertToJSValue(input[i]));
    }
    return arr;
}
void JSPatternLock::Create(const JSCallbackInfo& info)
{
    auto controller = PatternLockModel::GetInstance()->Create();

    if (info.Length() > 0 && info[0]->IsObject()) {
        auto* jsController = JSRef<JSObject>::Cast(info[0])->Unwrap<JSPatternLockController>();
        if (jsController) {
            jsController->SetController(controller);
        }
    }
}
void JSPatternLock::JSBind(BindingTarget globalObj)
{
    JSClass<JSPatternLock>::Declare("PatternLock");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSPatternLock>::StaticMethod("create", &JSPatternLock::Create, opt);
    JSClass<JSPatternLock>::StaticMethod("onPatternComplete", &JSPatternLock::SetPatternComplete);
    JSClass<JSPatternLock>::StaticMethod("regularColor", &JSPatternLock::SetRegularColor, MethodOptions::NONE);
    JSClass<JSPatternLock>::StaticMethod("selectedColor", &JSPatternLock::SetSelectedColor, MethodOptions::NONE);
    JSClass<JSPatternLock>::StaticMethod("activeColor", &JSPatternLock::SetActiveColor, MethodOptions::NONE);
    JSClass<JSPatternLock>::StaticMethod("pathColor", &JSPatternLock::SetPathColor, MethodOptions::NONE);
    JSClass<JSPatternLock>::StaticMethod("pathStrokeWidth", &JSPatternLock::SetPathStrokeWidth);
    JSClass<JSPatternLock>::StaticMethod("circleRadius", &JSPatternLock::SetCircleRadius, MethodOptions::NONE);
    JSClass<JSPatternLock>::StaticMethod("sideLength", &JSPatternLock::SetSideLength, MethodOptions::NONE);
    JSClass<JSPatternLock>::StaticMethod("autoReset", &JSPatternLock::SetAutoReset, MethodOptions::NONE);
    JSClass<JSPatternLock>::Inherit<JSViewAbstract>();
    JSClass<JSPatternLock>::Bind(globalObj);
}
void JSPatternLock::SetDefaultTheme(OHOS::Ace::RefPtr<OHOS::Ace::V2::PatternLockComponent>& patternLock)
{
    auto lockTheme = GetTheme<V2::PatternLockTheme>();
    if (!lockTheme) {
        LOGE("Get PatternLock theme error");
        return;
    }
    patternLock->SetRegularColor(lockTheme->GetRegularColor());
    patternLock->SetSelectedColor(lockTheme->GetSelectedColor());
    patternLock->SetActiveColor(lockTheme->GetActiveColor());
    patternLock->SetPathColor(lockTheme->GetPathColor());
}

void JSPatternLock::SetPatternComplete(const JSCallbackInfo& args)
{
    if (!args[0]->IsFunction()) {
        return;
    }

    auto onComplete = [execCtx = args.GetExecutionContext(), func = JSRef<JSFunc>::Cast(args[0])](
                          const BaseEventInfo* event) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        const auto* eventInfo = TypeInfoHelper::DynamicCast<V2::PatternCompleteEvent>(event);
        CHECK_NULL_VOID(eventInfo);
        JSRef<JSVal> params[] = { ChoosePointToJSValue(eventInfo->GetInput()) };
        func->Call(JSRef<JSObject>(), ArraySize(params), params);
    };

    PatternLockModel::GetInstance()->SetPatternComplete(std::move(onComplete));
}
void JSPatternLock::SetSelectedColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Color selectedColor;
    if (!ParseJsColor(info[0], selectedColor)) {
        return;
    }

    PatternLockModel::GetInstance()->SetSelectedColor(selectedColor);
}
void JSPatternLock::SetAutoReset(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    if (!info[0]->IsBoolean()) {
        LOGE("arg is not bool.");
        PatternLockModel::GetInstance()->SetAutoReset(true);
        return;
    }
    PatternLockModel::GetInstance()->SetAutoReset(info[0]->ToBoolean());
}

void JSPatternLock::SetPathColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Color pathColor;
    if (!ParseJsColor(info[0], pathColor)) {
        return;
    }

    PatternLockModel::GetInstance()->SetPathColor(pathColor);
}
void JSPatternLock::SetActiveColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Color activeColor;
    if (!ParseJsColor(info[0], activeColor)) {
        return;
    }

    PatternLockModel::GetInstance()->SetActiveColor(activeColor);
}
void JSPatternLock::SetRegularColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Color regularColor;
    if (!ParseJsColor(info[0], regularColor)) {
        return;
    }

    PatternLockModel::GetInstance()->SetRegularColor(regularColor);
}
void JSPatternLock::SetCircleRadius(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension radius;
    if (!ParseJsDimensionVp(info[0], radius)) {
        return;
    }

    PatternLockModel::GetInstance()->SetCircleRadius(radius);
}
void JSPatternLock::SetSideLength(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension sideLength;
    if (!ParseJsDimensionVp(info[0], sideLength)) {
        return;
    }
    JSViewAbstract::JsWidth(info);
    JSViewAbstract::JsHeight(info);

    PatternLockModel::GetInstance()->SetSideLength(sideLength);
}
void JSPatternLock::SetPathStrokeWidth(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension lineWidth;
    if (!ParseJsDimensionVp(info[0], lineWidth)) {
        return;
    }

    PatternLockModel::GetInstance()->SetStrokeWidth(lineWidth);
}
void JSPatternLockController::JSBind(BindingTarget globalObj)
{
    JSClass<JSPatternLockController>::Declare("PatternLockController");
    JSClass<JSPatternLockController>::CustomMethod("reset", &JSPatternLockController::Reset);
    JSClass<JSPatternLockController>::Bind(
        globalObj, JSPatternLockController::Constructor, JSPatternLockController::Destructor);
}
void JSPatternLockController::Reset(const JSCallbackInfo& args)
{
    if (controller_) {
        controller_->Reset();
    }
}
void JSPatternLockController::Constructor(const JSCallbackInfo& args)
{
    auto controller = Referenced::MakeRefPtr<JSPatternLockController>();
    controller->IncRefCount();
    args.SetReturnValue(Referenced::RawPtr(controller));
}
void JSPatternLockController::Destructor(JSPatternLockController* controller)
{
    if (controller != nullptr) {
        controller->DecRefCount();
    }
}
} // namespace OHOS::Ace::Framework