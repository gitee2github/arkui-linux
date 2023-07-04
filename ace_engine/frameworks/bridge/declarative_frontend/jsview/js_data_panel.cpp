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

#include "bridge/declarative_frontend/jsview/js_data_panel.h"

#include <vector>

#include "bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "bridge/declarative_frontend/jsview/models/data_panel_model_impl.h"
#include "core/components_ng/pattern/data_panel/data_panel_model_ng.h"

namespace OHOS::Ace {

std::unique_ptr<DataPanelModel> DataPanelModel::instance_ = nullptr;

DataPanelModel* DataPanelModel::GetInstance()
{
    if (!instance_) {
#ifdef NG_BUILD
        instance_.reset(new NG::DataPanelModelNG());
#else
        if (Container::IsCurrentUseNewPipeline()) {
            instance_.reset(new NG::DataPanelModelNG());
        } else {
            instance_.reset(new Framework::DataPanelModelImpl());
        }
#endif
    }
    return instance_.get();
}

} // namespace OHOS::Ace
namespace OHOS::Ace::Framework {

constexpr size_t MAX_COUNT = 9;
void JSDataPanel::JSBind(BindingTarget globalObj)
{
    JSClass<JSDataPanel>::Declare("DataPanel");
    JSClass<JSDataPanel>::StaticMethod("create", &JSDataPanel::Create);
    JSClass<JSDataPanel>::StaticMethod("closeEffect", &JSDataPanel::CloseEffect);
    JSClass<JSDataPanel>::StaticMethod("onClick", &JSInteractableView::JsOnClick);
    JSClass<JSDataPanel>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSDataPanel>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSDataPanel>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSDataPanel>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSDataPanel>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSDataPanel>::StaticMethod("remoteMessage", &JSInteractableView::JsCommonRemoteMessage);

    JSClass<JSDataPanel>::Inherit<JSViewAbstract>();
    JSClass<JSDataPanel>::Bind(globalObj);
}

void JSDataPanel::Create(const JSCallbackInfo& info)
{
    if (info.Length() < 1 || !info[0]->IsObject()) {
        LOGE("toggle create error, info is non-valid");
        return;
    }
    auto param = JsonUtil::ParseJsonString(info[0]->ToString());
    if (!param || param->IsNull()) {
        LOGE("JSDataPanel::Create param is null");
        return;
    }
    // max
    auto max = param->GetDouble("max", 100.0);
    // values
    auto values = param->GetValue("values");
    if (!values || !values->IsArray()) {
        LOGE("JSDataPanel::Create values is not array");
        return;
    }
    auto type = param->GetValue("type");
    size_t length = static_cast<size_t>(values->GetArraySize());
    std::vector<double> dateValues;
    double dataSum = 0.0;
    for (size_t i = 0; i < length && i < MAX_COUNT; i++) {
        auto item = values->GetArrayItem(i);
        if (!item || !item->IsNumber()) {
            LOGE("JSDataPanel::Create value is not number");
            continue;
        }
        auto value = item->GetDouble();
        if (LessOrEqual(value, 0.0)) {
            value = 0.0;
        }
        dataSum += value;
        if (GreatOrEqual(dataSum, max) && max > 0) {
            value = max - (dataSum - value);
            if (NearEqual(value, 0.0)) {
                break;
            }
        }
        dateValues.push_back(value);
    }
    if (LessOrEqual(max, 0.0)) {
        max = dataSum;
    }
    size_t dataPanelType = 0;
    if (type->IsNumber()) {
        if (type->GetInt() == static_cast<int32_t>(ChartType::LINE)) {
            dataPanelType = 1;
        } else if (type->GetInt() == static_cast<int32_t>(ChartType::RAINBOW)) {
            dataPanelType = 0;
        }
    }
    DataPanelModel::GetInstance()->Create(dateValues, max, dataPanelType);
}

void JSDataPanel::CloseEffect(const JSCallbackInfo& info)
{
    bool isCloseEffect = true;
    if (info[0]->IsBoolean()) {
        isCloseEffect = info[0]->ToBoolean();
    }
    DataPanelModel::GetInstance()->SetEffect(isCloseEffect);
}

} // namespace OHOS::Ace::Framework
