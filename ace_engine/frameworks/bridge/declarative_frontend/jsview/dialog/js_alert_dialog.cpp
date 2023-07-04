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

#include "frameworks/bridge/declarative_frontend/jsview/dialog/js_alert_dialog.h"

#include <sstream>
#include <string>
#include <vector>

#include "base/log/ace_scoring_log.h"
#include "core/common/container.h"
#include "core/components/dialog/dialog_component.h"
#include "core/components_ng/event/click_event.h"
#include "core/components_ng/pattern/dialog/dialog_event_hub.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/bridge/common/utils/engine_helper.h"
#include "frameworks/bridge/declarative_frontend/engine/functions/js_function.h"
#include "frameworks/bridge/declarative_frontend/view_stack_processor.h"

namespace OHOS::Ace::Framework {
namespace {
const std::vector<DialogAlignment> DIALOG_ALIGNMENT = { DialogAlignment::TOP, DialogAlignment::CENTER,
    DialogAlignment::BOTTOM, DialogAlignment::DEFAULT, DialogAlignment::TOP_START, DialogAlignment::TOP_END,
    DialogAlignment::CENTER_START, DialogAlignment::CENTER_END, DialogAlignment::BOTTOM_START,
    DialogAlignment::BOTTOM_END };
} // namespace

void ParseButtonObj(
    const JSCallbackInfo& args, DialogProperties& properties, JSRef<JSObject> obj, const std::string& property)
{
    auto jsVal = obj->GetProperty(property.c_str());
    if (!jsVal->IsObject()) {
        return;
    }
    auto objInner = JSRef<JSObject>::Cast(jsVal);
    auto value = objInner->GetProperty("value");
    std::string buttonValue;
    ButtonInfo buttonInfo;
    if (JSAlertDialog::ParseJsString(value, buttonValue)) {
        buttonInfo.text = buttonValue;
    }

    auto fontColorValue = objInner->GetProperty("fontColor");
    Color textColor;
    if (JSAlertDialog::ParseJsColor(fontColorValue, textColor)) {
        buttonInfo.textColor = textColor.ColorToString();
    }

    auto backgroundColorValue = objInner->GetProperty("backgroundColor");
    Color backgroundColor;
    if (JSAlertDialog::ParseJsColor(backgroundColorValue, backgroundColor)) {
        buttonInfo.isBgColorSetted = true;
        buttonInfo.bgColor = backgroundColor;
    }

    auto actionValue = objInner->GetProperty("action");
    if (actionValue->IsFunction()) {
        auto actionFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(actionValue));
        // NG
        if (Container::IsCurrentUseNewPipeline()) {
            auto callback = [execCtx = args.GetExecutionContext(), func = std::move(actionFunc), property](
                                GestureEvent& /*info*/) {
                JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
                ACE_SCORING_EVENT("AlertDialog.[" + property + "].onAction");
                func->ExecuteJS();
            };
            buttonInfo.action = AceType::MakeRefPtr<NG::ClickEvent>(std::move(callback));
        } else {
            EventMarker actionId([execCtx = args.GetExecutionContext(), func = std::move(actionFunc), property]() {
                JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
                ACE_SCORING_EVENT("AlertDialog.[" + property + "].onAction");
                func->Execute();
            });

            if (property == "confirm" || property == "primaryButton") {
                properties.primaryId = actionId;
            } else if (property == "secondaryButton") {
                properties.secondaryId = actionId;
            }
        }
    }

    if (buttonInfo.IsValid()) {
        properties.buttons.emplace_back(buttonInfo);
    }
}

void JSAlertDialog::Show(const JSCallbackInfo& args)
{
    auto scopedDelegate = EngineHelper::GetCurrentDelegate();
    if (!scopedDelegate) {
        // this case usually means there is no foreground container, need to figure out the reason.
        LOGE("scopedDelegate is null, please check");
        return;
    }

    DialogProperties properties { .type = DialogType::ALERT_DIALOG };
    if (args[0]->IsObject()) {
        auto obj = JSRef<JSObject>::Cast(args[0]);

        // Parse title.
        auto titleValue = obj->GetProperty("title");
        std::string title;
        if (ParseJsString(titleValue, title)) {
            properties.title = title;
        }

        // Parses message.
        auto messageValue = obj->GetProperty("message");
        std::string message;
        if (ParseJsString(messageValue, message)) {
            properties.content = message;
        }

        // Parses gridCount.
        auto gridCountValue = obj->GetProperty("gridCount");
        if (gridCountValue->IsNumber()) {
            properties.gridCount = gridCountValue->ToNumber<int32_t>();
        }

        // Parse auto autoCancel.
        auto autoCancelValue = obj->GetProperty("autoCancel");
        if (autoCancelValue->IsBoolean()) {
            properties.autoCancel = autoCancelValue->ToBoolean();
        }

        // Parse cancel.
        auto cancelValue = obj->GetProperty("cancel");
        if (cancelValue->IsFunction()) {
            auto cancelFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(cancelValue));
            // NG set onCancel
            if (Container::IsCurrentUseNewPipeline()) {
                properties.onCancel = [execCtx = args.GetExecutionContext(), func = std::move(cancelFunc)] {
                    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
                    ACE_SCORING_EVENT("AlertDialog.property.cancel");
                    LOGD("dialog onCancel triggered");
                    func->ExecuteJS();
                };
            } else {
                EventMarker cancelId([execCtx = args.GetExecutionContext(), func = std::move(cancelFunc)]() {
                    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
                    ACE_SCORING_EVENT("AlertDialog.property.cancel");
                    func->Execute();
                });
                properties.callbacks.try_emplace("cancel", cancelId);
            }
        }

        if (obj->GetProperty("confirm")->IsObject()) {
            // Parse confirm.
            ParseButtonObj(args, properties, obj, "confirm");
        } else {
            // Parse primaryButton and secondaryButton.
            ParseButtonObj(args, properties, obj, "primaryButton");
            ParseButtonObj(args, properties, obj, "secondaryButton");
        }

        // Parse alignment
        auto alignmentValue = obj->GetProperty("alignment");
        if (alignmentValue->IsNumber()) {
            auto alignment = alignmentValue->ToNumber<int32_t>();
            if (alignment >= 0 && alignment <= static_cast<int32_t>(DIALOG_ALIGNMENT.size())) {
                properties.alignment = DIALOG_ALIGNMENT[alignment];
            }
        }

        // Parse offset
        auto offsetValue = obj->GetProperty("offset");
        if (offsetValue->IsObject()) {
            auto offsetObj = JSRef<JSObject>::Cast(offsetValue);
            Dimension dx;
            auto dxValue = offsetObj->GetProperty("dx");
            ParseJsDimensionVp(dxValue, dx);
            Dimension dy;
            auto dyValue = offsetObj->GetProperty("dy");
            ParseJsDimensionVp(dyValue, dy);
            properties.offset = DimensionOffset(dx, dy);
        }

        if (Container::IsCurrentUseNewPipeline()) {
            auto container = Container::Current();
            CHECK_NULL_VOID(container);
            auto pipelineContext = container->GetPipelineContext();
            CHECK_NULL_VOID(pipelineContext);
            auto context = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
            CHECK_NULL_VOID(context);
            auto overlayManager = context->GetOverlayManager();
            CHECK_NULL_VOID(overlayManager);

            auto dialog = overlayManager->ShowDialog(properties, nullptr, false);
            CHECK_NULL_VOID(dialog);
            auto hub = dialog->GetEventHub<NG::DialogEventHub>();
            hub->SetOnCancel(std::move(properties.onCancel));
            return;
        }

        // Show dialog.
        auto container = Container::Current();
        if (container) {
            auto context = AceType::DynamicCast<PipelineContext>(container->GetPipelineContext());
            auto executor = container->GetTaskExecutor();
            if (executor) {
                executor->PostTask(
                    [context, properties]() {
                        if (context) {
                            context->ShowDialog(properties, false, "AlertDialog");
                        }
                    },
                    TaskExecutor::TaskType::UI);
            }
        }
        args.SetReturnValue(args.This());
    }
}

void JSAlertDialog::JSBind(BindingTarget globalObj)
{
    JSClass<JSAlertDialog>::Declare("AlertDialog");
    JSClass<JSAlertDialog>::StaticMethod("show", &JSAlertDialog::Show);

    JSClass<JSAlertDialog>::Inherit<JSViewAbstract>();
    JSClass<JSAlertDialog>::Bind<>(globalObj);
}

} // namespace OHOS::Ace::Framework
