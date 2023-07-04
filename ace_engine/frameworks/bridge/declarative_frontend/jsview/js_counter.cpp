/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "frameworks/bridge/declarative_frontend/jsview/js_counter.h"

#include "bridge/declarative_frontend/jsview/models/counter_model_impl.h"
#include "core/components_ng/pattern/counter/counter_model_ng.h"
#include "frameworks/bridge/common/utils/utils.h"
#include "frameworks/bridge/declarative_frontend/engine/bindings.h"
#include "frameworks/bridge/declarative_frontend/engine/functions/js_click_function.h"
#include "frameworks/bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_common_def.h"
#include "frameworks/core/components/counter/counter_theme.h"

namespace OHOS::Ace {

std::unique_ptr<CounterModel> CounterModel::instance_ = nullptr;

CounterModel* CounterModel::GetInstance()
{
    if (!instance_) {
#ifdef NG_BUILD
        instance_.reset(new NG::CounterModelNG());
#else
        if (Container::IsCurrentUseNewPipeline()) {
            instance_.reset(new NG::CounterModelNG());
        } else {
            instance_.reset(new Framework::CounterModelImpl());
        }
#endif
    }
    return instance_.get();
}

} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {

void JSCounter::JSBind(BindingTarget globalObj)
{
    JSClass<JSCounter>::Declare("Counter");
    JSClass<JSCounter>::StaticMethod("create", &JSCounter::Create, MethodOptions::NONE);
    JSClass<JSCounter>::StaticMethod("onInc", &JSCounter::JsOnInc);
    JSClass<JSCounter>::StaticMethod("onDec", &JSCounter::JsOnDec);
    JSClass<JSCounter>::StaticMethod("height", &JSCounter::JSHeight);
    JSClass<JSCounter>::StaticMethod("width", &JSCounter::JSWidth);
    JSClass<JSCounter>::StaticMethod("size", &JSCounter::SetSize);
    JSClass<JSCounter>::StaticMethod("controlWidth", &JSCounter::JSControlWidth);
    JSClass<JSCounter>::StaticMethod("state", &JSCounter::JSStateChange);
    JSClass<JSCounter>::StaticMethod("backgroundColor", &JSCounter::JsBackgroundColor);
    JSClass<JSCounter>::Inherit<JSContainerBase>();
    JSClass<JSCounter>::Bind(globalObj);
}

void JSCounter::JsOnInc(const JSCallbackInfo& args)
{
    if (args.Length() < 1) {
        LOGW("Must contain at least 1 argument");
        return;
    }
    if (!args[0]->IsFunction()) {
        LOGW("Argument is not a function object");
        return;
    }
    CounterModel::GetInstance()->SetOnInc(
        JsEventCallback<void()>(args.GetExecutionContext(), JSRef<JSFunc>::Cast(args[0])));
    args.ReturnSelf();
}

void JSCounter::JsOnDec(const JSCallbackInfo& args)
{
    if (args.Length() < 1) {
        LOGW("Must contain at least 1 argument");
        return;
    }
    if (!args[0]->IsFunction()) {
        LOGW("Argument is not a function object");
        return;
    }
    CounterModel::GetInstance()->SetOnDec(
        JsEventCallback<void()>(args.GetExecutionContext(), JSRef<JSFunc>::Cast(args[0])));
    args.ReturnSelf();
}

void JSCounter::JSHeight(const JSCallbackInfo& args)
{
    if (args.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    Dimension value;
    if (!ConvertFromJSValue(args[0], value)) {
        LOGE("args can not set height");
        return;
    }

    if (LessNotEqual(value.Value(), 0.0)) {
        return;
    }
    CounterModel::GetInstance()->SetHeight(value);
    args.ReturnSelf();
}

void JSCounter::JSWidth(const JSCallbackInfo& args)
{
    if (args.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    Dimension value;
    if (!ConvertFromJSValue(args[0], value)) {
        LOGE("args can not set width");
        return;
    }

    if (LessNotEqual(value.Value(), 0.0)) {
        return;
    }
    CounterModel::GetInstance()->SetWidth(value);
    args.ReturnSelf();
}

void JSCounter::SetSize(const JSCallbackInfo& args)
{
    if (args.Length() >= 1 && args[0]->IsObject()) {
        JSRef<JSObject> obj = JSRef<JSObject>::Cast(args[0]);

        Dimension height;
        if (ConvertFromJSValue(obj->GetProperty("height"), height) && height.IsValid()) {
            if (GreatOrEqual(height.Value(), 0.0)) {
                CounterModel::GetInstance()->SetWidth(height);
            }
        }

        Dimension width;
        if (ConvertFromJSValue(obj->GetProperty("width"), width) && width.IsValid()) {
            if (GreatOrEqual(width.Value(), 0.0)) {
                CounterModel::GetInstance()->SetWidth(width);
            }
        }
    }
    args.ReturnSelf();
}

void JSCounter::JSControlWidth(const JSCallbackInfo& args)
{
    if (args.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    Dimension value;
    if (!ConvertFromJSValue(args[0], value)) {
        LOGE("args can not set control width");
        return;
    }
    CounterModel::GetInstance()->SetControlWidth(value);
    args.ReturnSelf();
}

void JSCounter::JSStateChange(bool state)
{
    CounterModel::GetInstance()->SetStateChange(state);
}

void JSCounter::JsBackgroundColor(const JSCallbackInfo& args)
{
    if (args.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    Color color;
    if (!ParseJsColor(args[0], color)) {
        LOGE("args can not set backgroundColor");
        return;
    }
    CounterModel::GetInstance()->SetBackgroundColor(color);
    args.ReturnSelf();
}

void JSCounter::Create()
{
    CounterModel::GetInstance()->Create();
}

} // namespace OHOS::Ace::Framework
