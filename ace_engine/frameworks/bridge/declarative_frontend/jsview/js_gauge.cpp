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

#include "bridge/declarative_frontend/jsview/js_gauge.h"

#include <string>

#include "bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "bridge/declarative_frontend/jsview/models/gauge_model_impl.h"
#include "core/components_ng/pattern/gauge/gauge_model_ng.h"

namespace OHOS::Ace {

std::unique_ptr<GaugeModel> GaugeModel::instance_ = nullptr;

GaugeModel* GaugeModel::GetInstance()
{
    if (!instance_) {
#ifdef NG_BUILD
        instance_.reset(new NG::GaugeModelNG());
#else
        if (Container::IsCurrentUseNewPipeline()) {
            instance_.reset(new NG::GaugeModelNG());
        } else {
            instance_.reset(new Framework::GaugeModelImpl());
        }
#endif
    }
    return instance_.get();
}

} // namespace OHOS::Ace
namespace OHOS::Ace::Framework {

void JSGauge::JSBind(BindingTarget globalObj)
{
    JSClass<JSGauge>::Declare("Gauge");
    JSClass<JSGauge>::StaticMethod("create", &JSGauge::Create);

    JSClass<JSGauge>::StaticMethod("value", &JSGauge::SetValue);
    JSClass<JSGauge>::StaticMethod("startAngle", &JSGauge::SetStartAngle);
    JSClass<JSGauge>::StaticMethod("endAngle", &JSGauge::SetEndAngle);
    JSClass<JSGauge>::StaticMethod("colors", &JSGauge::SetColors);
    JSClass<JSGauge>::StaticMethod("strokeWidth", &JSGauge::SetStrokeWidth);
    JSClass<JSGauge>::StaticMethod("labelConfig", &JSGauge::SetLabelConfig);
    JSClass<JSGauge>::StaticMethod("onClick", &JSInteractableView::JsOnClick);
    JSClass<JSGauge>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSGauge>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSGauge>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSGauge>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSGauge>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);

    JSClass<JSGauge>::Inherit<JSViewAbstract>();
    JSClass<JSGauge>::Bind(globalObj);
}

void JSGauge::Create(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsObject()) {
        LOGE("gauge create error, info is non-valid");
        return;
    }

    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    auto value = paramObject->GetProperty("value");
    auto min = paramObject->GetProperty("min");
    auto max = paramObject->GetProperty("max");

    double gaugeMin = min->IsNumber() ? min->ToNumber<double>() : 0;
    double gaugeMax = max->IsNumber() ? max->ToNumber<double>() : 100;
    double gaugeValue = value->IsNumber() ? value->ToNumber<double>() : 0;
    GaugeModel::GetInstance()->Create(gaugeValue, gaugeMin, gaugeMax);
}

void JSGauge::SetValue(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsNumber()) {
        LOGE("JSGauge::SetValue::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    GaugeModel::GetInstance()->SetValue(info[0]->ToNumber<float>());
}

void JSGauge::SetStartAngle(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsNumber()) {
        LOGE("JSGauge::SetStartAngle::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    GaugeModel::GetInstance()->SetStartAngle(info[0]->ToNumber<float>());
}

void JSGauge::SetEndAngle(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsNumber()) {
        LOGE("JSGauge::SetEndAngle::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    GaugeModel::GetInstance()->SetEndAngle(info[0]->ToNumber<float>());
}

void JSGauge::SetColors(const JSCallbackInfo& info)
{
    if (info.Length() < 1 || !info[0]->IsArray()) {
        LOGE("The number of argument is less than 1, or the argument is not array.");
        return;
    }
    std::vector<Color> colors;
    std::vector<double> values;
    std::vector<float> weights;
    auto theme = GetTheme<ProgressTheme>();
    auto jsColor = JSRef<JSArray>::Cast(info[0]);
    for (size_t i = 0; i < jsColor->Length(); ++i) {
        JSRef<JSVal> jsValue = jsColor->GetValueAt(i);
        if (!jsValue->IsArray()) {
            return;
        }
        JSRef<JSArray> tempColors = jsColor->GetValueAt(i);
        double value = tempColors->GetValueAt(1)->ToNumber<double>();
        float weight = tempColors->GetValueAt(1)->ToNumber<float>();
        Color selectedColor;
        if (!ParseJsColor(tempColors->GetValueAt(0), selectedColor)) {
            selectedColor = Color::BLACK;
        }
        colors.push_back(selectedColor);
        values.push_back(value);
        if (weight > 0) {
            weights.push_back(weight);
        } else {
            weights.push_back(0.0f);
        }
    }
    GaugeModel::GetInstance()->SetColors(colors, weights);
}

void JSGauge::SetStrokeWidth(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE(" JSGauge::SetStrokeWidth::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    Dimension strokeWidth;
    if (!ParseJsDimensionVp(info[0], strokeWidth)) {
        strokeWidth = Dimension(0);
    }
    GaugeModel::GetInstance()->SetStrokeWidth(strokeWidth);
}

void JSGauge::SetLabelConfig(const JSCallbackInfo& info)
{
    if (info.Length() < 1 && !info[0]->IsObject()) {
        LOGE("JSGauge::SetLabelTextConfig::The info is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    auto paramObject = JSRef<JSObject>::Cast(info[0]);
    auto labelText = paramObject->GetProperty("text");
    auto labelColor = paramObject->GetProperty("color");
    Color currentColor;
    ParseJsColor(labelColor, currentColor);
    if (labelText->IsString()) {
        GaugeModel::GetInstance()->SetLabelMarkedText(labelText->ToString());
    }
    if (ParseJsColor(labelColor, currentColor)) {
        GaugeModel::GetInstance()->SetMarkedTextColor(currentColor);
    }
}

} // namespace OHOS::Ace::Framework
