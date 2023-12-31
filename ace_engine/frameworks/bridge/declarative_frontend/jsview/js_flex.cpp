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

#include "frameworks/bridge/declarative_frontend/jsview/js_flex.h"

#include "core/components_ng/pattern/flex/flex_model.h"
#include "frameworks/bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_common_def.h"
#include "frameworks/bridge/declarative_frontend/jsview/models/flex_model_impl.h"
#include "frameworks/bridge/declarative_frontend/view_stack_processor.h"
#include "frameworks/core/components_ng/pattern/flex/flex_model_ng.h"

namespace OHOS::Ace {

std::unique_ptr<FlexModel> FlexModel::instance_ = nullptr;

FlexModel* FlexModel::GetInstance()
{
    if (!instance_) {
#ifdef NG_BUILD
        instance_.reset(new NG::FlexModelNG());
#else
        if (Container::IsCurrentUseNewPipeline()) {
            instance_.reset(new NG::FlexModelNG());
        } else {
            instance_.reset(new Framework::FlexModelImpl());
        }
#endif
    }
    return instance_.get();
}

} // namespace OHOS::Ace
namespace OHOS::Ace::Framework {

void JSFlex::SetFillParent()
{
    /* Deprecated */
}

void JSFlex::SetWrapContent()
{
    /* Deprecated */
}

void JSFlex::SetJustifyContent(int32_t value)
{
    if ((value == static_cast<int32_t>(FlexAlign::FLEX_START)) ||
        (value == static_cast<int32_t>(FlexAlign::FLEX_END)) || (value == static_cast<int32_t>(FlexAlign::CENTER)) ||
        (value == static_cast<int32_t>(FlexAlign::SPACE_AROUND)) ||
        (value == static_cast<int32_t>(FlexAlign::SPACE_BETWEEN)) ||
        (value == static_cast<int32_t>(FlexAlign::SPACE_EVENLY))) {
        FlexModel::GetInstance()->SetJustifyContent(value);
    } else {
        LOGE("invalid value for justifyContent");
    }
}

void JSFlex::SetAlignItems(int32_t value)
{
    if ((value == static_cast<int32_t>(FlexAlign::FLEX_START)) ||
        (value == static_cast<int32_t>(FlexAlign::FLEX_END)) || (value == static_cast<int32_t>(FlexAlign::CENTER)) ||
        (value == static_cast<int32_t>(FlexAlign::STRETCH))) {
        FlexModel::GetInstance()->SetAlignItems(value);
    } else {
        LOGE("invalid value for justifyContent");
    }
}

void JSFlex::SetAlignContent(int32_t value)
{
    if ((value == static_cast<int32_t>(WrapAlignment::START)) ||
        (value == static_cast<int32_t>(WrapAlignment::CENTER)) || (value == static_cast<int32_t>(WrapAlignment::END)) ||
        (value == static_cast<int32_t>(WrapAlignment::SPACE_AROUND)) ||
        (value == static_cast<int32_t>(WrapAlignment::SPACE_BETWEEN)) ||
        (value == static_cast<int32_t>(WrapAlignment::STRETCH))) {
        FlexModel::GetInstance()->SetAlignContent(value);
    } else {
        LOGE("invalid value for justifyContent");
    }
}

void JSFlex::JsHeight(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    SetHeight(info[0]);
}

void JSFlex::SetHeight(const JSRef<JSVal>& jsValue)
{
    if (!JSViewAbstract::JsHeight(jsValue)) {
        // JsHeight return false, just return.
        return;
    }
    FlexModel::GetInstance()->SetHasHeight();
}

void JSFlex::JsWidth(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    SetWidth(info[0]);
}

void JSFlex::SetWidth(const JSRef<JSVal>& jsValue)
{
    if (!JSViewAbstract::JsWidth(jsValue)) {
        // JsWidth return false, just return.
        return;
    }
    FlexModel::GetInstance()->SetHasWidth();
}

void JSFlex::JsSize(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    if (!info[0]->IsObject()) {
        LOGE("arg is not Object or String.");
        return;
    }

    JSRef<JSObject> sizeObj = JSRef<JSObject>::Cast(info[0]);
    SetWidth(sizeObj->GetProperty("width"));
    SetHeight(sizeObj->GetProperty("height"));
}

}; // namespace OHOS::Ace::Framework
