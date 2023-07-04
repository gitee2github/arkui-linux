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

#include "frameworks/bridge/declarative_frontend/jsview/js_plugin.h"

#include "base/geometry/dimension.h"
#include "base/log/ace_scoring_log.h"
#include "base/log/log_wrapper.h"
#include "core/components/common/properties/clip_path.h"
#include "core/components_ng/pattern/plugin/plugin_pattern.h"
#include "core/components_ng/pattern/plugin/plugin_view.h"
#include "frameworks/bridge/declarative_frontend/view_stack_processor.h"
#include "frameworks/core/components/box/box_component.h"
#include "frameworks/core/components/plugin/plugin_component.h"

namespace OHOS::Ace::Framework {
void JSPlugin::Create(const JSCallbackInfo& info)
{
    if (info.Length() == 0 || !info[0]->IsObject()) {
        LOGE("plugin create fail due to PluginComponent construct param is empty or type is not Object");
        return;
    }

    // Parse template
    RequestPluginInfo pluginInfo;
    auto obj = JSRef<JSObject>::Cast(info[0]);
    auto templateObj = obj->GetProperty("template");
    if (templateObj->IsObject()) {
        auto jstemplateObj = JSRef<JSObject>::Cast(templateObj);
        auto sourceVal = jstemplateObj->GetProperty("source");
        if (sourceVal->IsString()) {
            pluginInfo.pluginName = sourceVal->ToString();
        }
        auto abilityVal = jstemplateObj->GetProperty("ability");
        if (!abilityVal->IsEmpty() && abilityVal->IsString()) {
            pluginInfo.bundleName = abilityVal->ToString();
        }

        auto bundleValue = jstemplateObj->GetProperty("bundleName");
        if (!bundleValue->IsEmpty() && bundleValue->IsString()) {
            pluginInfo.bundleName = bundleValue->ToString();
        }
        LOGD("JSPlugin::Create: source=%{public}s bundleName=%{public}s", pluginInfo.pluginName.c_str(),
            pluginInfo.bundleName.c_str());
    }
    if (pluginInfo.bundleName.size() > PATH_MAX || pluginInfo.pluginName.size() > PATH_MAX) {
        LOGE("the source path or the bundleName is too long");
        return;
    }
    // Parse data
    auto dataValue = obj->GetProperty("data");

    if (Container::IsCurrentUseNewPipeline()) {
        NG::PluginView::Create(pluginInfo);
        if (dataValue->IsObject()) {
            NG::PluginView::SetData(dataValue->ToString());
        }
        return;
    }

    RefPtr<PluginComponent> plugin = AceType::MakeRefPtr<OHOS::Ace::PluginComponent>();
    if (dataValue->IsObject()) {
        plugin->SetData(dataValue->ToString());
    }
    plugin->SetPluginRequestInfo(pluginInfo);
    ViewStackProcessor::GetInstance()->Push(plugin, false);
}

void JSPlugin::JsSize(const JSCallbackInfo& info)
{
    if (Container::IsCurrentUseNewPipeline()) {
        JSViewAbstract::JsSize(info);
        return;
    }
    if (info.Length() == 0) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    if (!info[0]->IsObject()) {
        LOGE("The arg is not Object or String.");
        return;
    }

    JSRef<JSObject> sizeObj = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> widthValue = sizeObj->GetProperty("width");
    Dimension width = 0.0_vp;
    if (!ParseJsDimensionVp(widthValue, width)) {
        LOGE("ParseJsDimensionVp width is error.");
        return;
    }
    JSRef<JSVal> heightValue = sizeObj->GetProperty("height");
    Dimension height = 0.0_vp;
    if (!ParseJsDimensionVp(heightValue, height)) {
        LOGE("ParseJsDimensionVp height is error.");
        return;
    }

    auto plugin = AceType::DynamicCast<PluginComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (plugin) {
        plugin->SetPluginSize(width.IsValid() ? width : 0.0_vp, height.IsValid() ? height : 0.0_vp);
    }
}

void JSPlugin::JsOnComplete(const JSCallbackInfo& info)
{
#if defined(PLUGIN_COMPONENT_SUPPORTED)
    if (Container::IsCurrentUseNewPipeline()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
        auto OnComplete = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](const std::string& param) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("Plugin.OnComplete");
            func->Execute();
        };
        NG::PluginView::SetOnComplete(std::move(OnComplete));
        return;
    }

    if (info[0]->IsFunction()) {
        RefPtr<JsFunction> jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
        auto plugin = AceType::DynamicCast<PluginComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());

        auto onCompleteId =
            EventMarker([execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](const std::string& param) {
                JAVASCRIPT_EXECUTION_SCOPE(execCtx);
                LOGD("onComplete send");
                ACE_SCORING_EVENT("Plugin.onComplete");
                func->Execute();
            });
        plugin->SetOnCompleteEventId(onCompleteId);
    }
#endif
}

void JSPlugin::JsOnError(const JSCallbackInfo& info)
{
#if defined(PLUGIN_COMPONENT_SUPPORTED)
    if (Container::IsCurrentUseNewPipeline()) {
        auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
        auto onError = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](const std::string& param) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("Plugin.OnComplete");
            std::vector<std::string> keys = { "errcode", "msg" };
            func->Execute(keys, param);
        };
        NG::PluginView::SetOnError(std::move(onError));
        return;
    }

    if (info[0]->IsFunction()) {
        RefPtr<JsFunction> jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
        auto plugin = AceType::DynamicCast<PluginComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());

        auto onErrorId =
            EventMarker([execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](const std::string& param) {
                JAVASCRIPT_EXECUTION_SCOPE(execCtx);
                std::vector<std::string> keys = { "errcode", "msg" };
                LOGD("onError send");
                ACE_SCORING_EVENT("Plugin.onError");
                func->Execute(keys, param);
            });

        plugin->SetOnErrorEventId(onErrorId);
    }
#endif
}

void JSPlugin::JSBind(BindingTarget globalObj)
{
    JSClass<JSPlugin>::Declare("PluginComponent");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSPlugin>::StaticMethod("create", &JSPlugin::Create, opt);
    JSClass<JSPlugin>::StaticMethod("size", &JSPlugin::JsSize, opt);

    JSClass<JSPlugin>::StaticMethod("onComplete", &JSPlugin::JsOnComplete);
    JSClass<JSPlugin>::StaticMethod("onError", &JSPlugin::JsOnError);
    JSClass<JSPlugin>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSPlugin>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSPlugin>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSPlugin>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSPlugin>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSPlugin>::StaticMethod("onClick", &JSInteractableView::JsOnClick);

    JSClass<JSPlugin>::Inherit<JSViewAbstract>();
    JSClass<JSPlugin>::Bind<>(globalObj);
}
} // namespace OHOS::Ace::Framework
