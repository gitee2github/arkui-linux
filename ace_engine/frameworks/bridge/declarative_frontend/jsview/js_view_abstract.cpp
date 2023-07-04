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

#include "bridge/declarative_frontend/jsview/js_view_abstract.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <vector>

#include "base/geometry/dimension.h"
#include "base/geometry/matrix4.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/vector.h"
#include "base/json/json_util.h"
#include "base/log/ace_scoring_log.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "bridge/common/utils/utils.h"
#include "bridge/declarative_frontend/engine/functions/js_click_function.h"
#include "bridge/declarative_frontend/engine/functions/js_drag_function.h"
#include "bridge/declarative_frontend/engine/functions/js_focus_function.h"
#include "bridge/declarative_frontend/engine/functions/js_function.h"
#include "bridge/declarative_frontend/engine/functions/js_hover_function.h"
#include "bridge/declarative_frontend/engine/functions/js_key_function.h"
#include "bridge/declarative_frontend/engine/functions/js_on_area_change_function.h"
#include "bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "bridge/declarative_frontend/jsview/js_grid_container.h"
#include "bridge/declarative_frontend/jsview/js_shape_abstract.h"
#include "bridge/declarative_frontend/jsview/js_utils.h"
#include "bridge/declarative_frontend/jsview/models/view_abstract_model_impl.h"
#include "core/components/common/properties/border_image.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/decoration.h"
#include "core/components/common/properties/shadow.h"
#include "core/components_ng/base/view_abstract_model.h"
#include "core/gestures/gesture_info.h"
#ifdef PLUGIN_COMPONENT_SUPPORTED
#include "core/common/plugin_manager.h"
#endif
#include "core/common/card_scope.h"
#include "core/common/container.h"
#include "core/components_ng/base/view_abstract_model_ng.h"
#include "core/components_ng/base/view_stack_model.h"

namespace OHOS::Ace {

std::unique_ptr<ViewAbstractModel> ViewAbstractModel::instance_ = nullptr;

ViewAbstractModel* ViewAbstractModel::GetInstance()
{
    if (!instance_) {
#ifdef NG_BUILD
        instance_.reset(new NG::ViewAbstractModelNG());
#else
        if (Container::IsCurrentUseNewPipeline()) {
            instance_.reset(new NG::ViewAbstractModelNG());
        } else {
            instance_.reset(new Framework::ViewAbstractModelImpl());
        }
#endif
    }
    return instance_.get();
}

} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {
namespace {

constexpr uint32_t DEFAULT_DURATION = 1000; // ms
constexpr uint32_t COLOR_ALPHA_OFFSET = 24;
constexpr uint32_t COLOR_ALPHA_VALUE = 0xFF000000;
constexpr int32_t MAX_ALIGN_VALUE = 8;
const std::regex RESOURCE_APP_STRING_PLACEHOLDER(R"(\%((\d+)(\$)){0,1}([dsf]))", std::regex::icase);
constexpr double FULL_DIMENSION = 100.0;
constexpr double HALF_DIMENSION = 50.0;
constexpr double ROUND_UNIT = 360.0;
constexpr double VISIBLE_RATIO_MIN = 0.0;
constexpr double VISIBLE_RATIO_MAX = 1.0;
constexpr int32_t MIN_ROTATE_VECTOR_Z = 9;

bool CheckJSCallbackInfo(
    const std::string& callerName, const JSCallbackInfo& info, std::vector<JSCallbackInfoType>& infoTypes)
{
    if (info.Length() < 1) {
        LOGE("%{public}s: The arg is supposed to have at least one argument", callerName.c_str());
        return false;
    }
    bool typeVerified = false;
    std::string unrecognizedType;
    for (const auto& infoType : infoTypes) {
        switch (infoType) {
            case JSCallbackInfoType::STRING:
                if (info[0]->IsString()) {
                    typeVerified = true;
                } else {
                    unrecognizedType += "string|";
                }
                break;
            case JSCallbackInfoType::NUMBER:
                if (info[0]->IsNumber()) {
                    typeVerified = true;
                } else {
                    unrecognizedType += "number|";
                }
                break;
            case JSCallbackInfoType::OBJECT:
                if (info[0]->IsObject()) {
                    typeVerified = true;
                } else {
                    unrecognizedType += "object|";
                }
                break;
            case JSCallbackInfoType::FUNCTION:
                if (info[0]->IsFunction()) {
                    typeVerified = true;
                } else {
                    unrecognizedType += "Function|";
                }
                break;
            default:
                break;
        }
    }
    if (!typeVerified) {
        LOGE("%{public}s: info[0] is not a [%{public}s]", callerName.c_str(),
            unrecognizedType.substr(0, unrecognizedType.size() - 1).c_str());
    }
    return typeVerified || infoTypes.size() == 0;
}

void ParseJsScale(std::unique_ptr<JsonValue>& argsPtrItem, float& scaleX, float& scaleY, float& scaleZ,
    Dimension& centerX, Dimension& centerY)
{
    double xVal = 1.0;
    double yVal = 1.0;
    double zVal = 1.0;
    JSViewAbstract::ParseJsonDouble(argsPtrItem->GetValue("x"), xVal);
    JSViewAbstract::ParseJsonDouble(argsPtrItem->GetValue("y"), yVal);
    JSViewAbstract::ParseJsonDouble(argsPtrItem->GetValue("z"), zVal);
    scaleX = static_cast<float>(xVal);
    scaleY = static_cast<float>(yVal);
    scaleZ = static_cast<float>(zVal);
    // if specify centerX
    Dimension length;
    if (JSViewAbstract::ParseJsonDimensionVp(argsPtrItem->GetValue("centerX"), length)) {
        centerX = length;
    }
    // if specify centerY
    if (JSViewAbstract::ParseJsonDimensionVp(argsPtrItem->GetValue("centerY"), length)) {
        centerY = length;
    }
}

void ParseJsTranslate(
    std::unique_ptr<JsonValue>& argsPtrItem, Dimension& translateX, Dimension& translateY, Dimension& translateZ)
{
    Dimension length;
    if (JSViewAbstract::ParseJsonDimensionVp(argsPtrItem->GetValue("x"), length)) {
        translateX = length;
    }
    if (JSViewAbstract::ParseJsonDimensionVp(argsPtrItem->GetValue("y"), length)) {
        translateY = length;
    }
    if (JSViewAbstract::ParseJsonDimensionVp(argsPtrItem->GetValue("z"), length)) {
        translateZ = length;
    }
}

void GetDefaultRotateVector(double& dx, double& dy, double& dz)
{
    dx = 0.0;
    dy = 0.0;
    dz = 0.0;
    auto container = Container::Current();
    if (!container) {
        LOGW("container is null");
        return;
    }
    auto pipelineContext = container->GetPipelineContext();
    if (!pipelineContext) {
        LOGE("pipelineContext is null!");
        return;
    }
    if (pipelineContext->GetMinPlatformVersion() >= MIN_ROTATE_VECTOR_Z) {
        dz = 1.0;
    }
}

void ParseJsRotate(std::unique_ptr<JsonValue>& argsPtrItem, float& dx, float& dy, float& dz, Dimension& centerX,
    Dimension& centerY, std::optional<float>& angle)
{
    // default: dx, dy, dz (0.0, 0.0, 0.0)
    double dxVal = 0.0;
    double dyVal = 0.0;
    double dzVal = 0.0;
    if (!argsPtrItem->Contains("x") && !argsPtrItem->Contains("y") && !argsPtrItem->Contains("z")) {
        GetDefaultRotateVector(dxVal, dyVal, dzVal);
    }
    JSViewAbstract::ParseJsonDouble(argsPtrItem->GetValue("x"), dxVal);
    JSViewAbstract::ParseJsonDouble(argsPtrItem->GetValue("y"), dyVal);
    JSViewAbstract::ParseJsonDouble(argsPtrItem->GetValue("z"), dzVal);
    dx = static_cast<float>(dxVal);
    dy = static_cast<float>(dyVal);
    dz = static_cast<float>(dzVal);
    // if specify centerX
    Dimension length;
    if (JSViewAbstract::ParseJsonDimensionVp(argsPtrItem->GetValue("centerX"), length)) {
        centerX = length;
    }
    // if specify centerY
    if (JSViewAbstract::ParseJsonDimensionVp(argsPtrItem->GetValue("centerY"), length)) {
        centerY = length;
    }
    // if specify angle
    JSViewAbstract::GetAngle("angle", argsPtrItem, angle);
}

bool ParseMotionPath(const std::unique_ptr<JsonValue>& argsPtrItem, MotionPathOption& option)
{
    if (argsPtrItem && !argsPtrItem->IsNull()) {
        auto path = argsPtrItem->GetString("path", "");
        if (!path.empty()) {
            option.SetPath(path);
            double from = 0.0;
            double to = 1.0;
            JSViewAbstract::ParseJsonDouble(argsPtrItem->GetValue("from"), from);
            JSViewAbstract::ParseJsonDouble(argsPtrItem->GetValue("to"), to);
            option.SetBegin(static_cast<float>(from));
            option.SetEnd(static_cast<float>(to));
            option.SetRotate(argsPtrItem->GetBool("rotatable", false));
            return true;
        }
    }
    return false;
}

void SetBgImgPosition(const DimensionUnit& typeX, const DimensionUnit& typeY, const double valueX, const double valueY,
    BackgroundImagePosition& bgImgPosition)
{
    AnimationOption option = ViewStackModel::GetInstance()->GetImplicitAnimationOption();
    bgImgPosition.SetSizeX(AnimatableDimension(valueX, typeX, option));
    bgImgPosition.SetSizeY(AnimatableDimension(valueY, typeY, option));
}

std::string GetReplaceContentStr(int pos, const std::string& type, JSRef<JSArray> params, int32_t containCount)
{
    JSRef<JSVal> item = params->GetValueAt(pos + containCount);
    if (type == "d") {
        if (item->IsNumber()) {
            return std::to_string(item->ToNumber<uint32_t>());
        }
    } else if (type == "s") {
        if (item->IsString()) {
            return item->ToString();
        }
    } else if (type == "f") {
        if (item->IsNumber()) {
            return std::to_string(item->ToNumber<float>());
        }
    }
    return std::string();
}

void ReplaceHolder(std::string& originStr, JSRef<JSArray> params, int32_t containCount)
{
    auto size = static_cast<int32_t>(params->Length());
    if (containCount == size) {
        return;
    }
    std::string::const_iterator start = originStr.begin();
    std::string::const_iterator end = originStr.end();
    std::smatch matches;
    bool shortHolderType = false;
    bool firstMatch = true;
    int searchTime = 0;
    while (std::regex_search(start, end, matches, RESOURCE_APP_STRING_PLACEHOLDER)) {
        std::string pos = matches[2];
        std::string type = matches[4];
        if (firstMatch) {
            firstMatch = false;
            shortHolderType = pos.length() == 0;
        } else {
            if (shortHolderType ^ (pos.length() == 0)) {
                LOGE("wrong place holder,stop parse string");
                return;
            }
        }

        std::string replaceContentStr;
        if (shortHolderType) {
            replaceContentStr = GetReplaceContentStr(searchTime, type, params, containCount);
        } else {
            replaceContentStr = GetReplaceContentStr(StringToInt(pos) - 1, type, params, containCount);
        }

        originStr.replace(matches[0].first - originStr.begin(), matches[0].length(), replaceContentStr);
        start = originStr.begin() + matches.prefix().length() + replaceContentStr.length();
        end = originStr.end();
        searchTime++;
    }
}

bool ParseLocationProps(const JSCallbackInfo& info, Dimension& x, Dimension& y)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("ParseLocationProps", info, checkList)) {
        return false;
    }
    JSRef<JSObject> sizeObj = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> xVal = sizeObj->GetProperty("x");
    JSRef<JSVal> yVal = sizeObj->GetProperty("y");
    bool hasX = JSViewAbstract::ParseJsDimensionVp(xVal, x);
    bool hasY = JSViewAbstract::ParseJsDimensionVp(yVal, y);
    return hasX || hasY;
}

RefPtr<JsFunction> ParseDragStartBuilderFunc(const JSRef<JSVal>& info)
{
    JSRef<JSVal> builder;
    if (info->IsObject()) {
        auto builderObj = JSRef<JSObject>::Cast(info);
        builder = builderObj->GetProperty("builder");
    } else if (info->IsFunction()) {
        builder = info;
    } else {
        return nullptr;
    }

    if (!builder->IsFunction()) {
        return nullptr;
    }

    return AceType::MakeRefPtr<JsFunction>(JSRef<JSFunc>::Cast(builder));
}

#ifndef WEARABLE_PRODUCT
const std::vector<Placement> PLACEMENT = { Placement::LEFT, Placement::RIGHT, Placement::TOP, Placement::BOTTOM,
    Placement::TOP_LEFT, Placement::TOP_RIGHT, Placement::BOTTOM_LEFT, Placement::BOTTOM_RIGHT, Placement::LEFT_TOP,
    Placement::LEFT_BOTTOM, Placement::RIGHT_TOP, Placement::RIGHT_BOTTOM };

void ParseShowObject(const JSCallbackInfo& info, const JSRef<JSObject>& showObj, const RefPtr<PopupParam>& popupParam)
{
    JSRef<JSVal> changeEventVal = showObj->GetProperty("changeEvent");
    if (!changeEventVal->IsFunction()) {
        return;
    }
    RefPtr<JsFunction> jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(changeEventVal));

    if (popupParam) {
        auto onStateChangeCallback = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](
                                         const std::string& param) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);

            if (param != "true" && param != "false") {
                LOGE("param is not equal true or false, invalid.");
                return;
            }

            bool newValue = StringToBool(param);
            JSRef<JSVal> newJSVal = JSRef<JSVal>::Make(ToJSValue(newValue));
            func->ExecuteJS(1, &newJSVal);
        };
        popupParam->SetOnStateChange(onStateChangeCallback);
        return;
    }
}

void ParsePopupParam(const JSCallbackInfo& info, const JSRef<JSObject>& popupObj, const RefPtr<PopupParam>& popupParam)
{
    JSRef<JSVal> messageVal = popupObj->GetProperty("message");
    if (popupParam) {
        popupParam->SetMessage(messageVal->ToString());
    } else {
        LOGI("Empty popup.");
    }

    auto arrowOffset = popupObj->GetProperty("arrowOffset");
    Dimension offset;
    if (JSViewAbstract::ParseJsDimensionVp(arrowOffset, offset)) {
        if (popupParam) {
            popupParam->SetArrowOffset(offset);
        } else {
            LOGI("Empty popup.");
        }
    }

#if defined(PREVIEW)
    LOGW("[Engine Log] Unable to use the SubWindow in the Previewer. Perform this operation on the "
         "emulator or a real device instead.");
#else
    JSRef<JSVal> showInSubWindowValue = popupObj->GetProperty("showInSubWindow");
    if (showInSubWindowValue->IsBoolean()) {
        if (popupParam) {
            popupParam->SetShowInSubWindow(showInSubWindowValue->ToBoolean());
        } else {
            LOGI("Empty popup.");
        }
    }
#endif

    JSRef<JSVal> placementOnTopVal = popupObj->GetProperty("placementOnTop");
    if (placementOnTopVal->IsBoolean()) {
        if (popupParam) {
            popupParam->SetPlacement(placementOnTopVal->ToBoolean() ? Placement::TOP : Placement::BOTTOM);
        } else {
            LOGI("Empty popup.");
        }
    }

    JSRef<JSVal> maskValue = popupObj->GetProperty("mask");
    if (maskValue->IsBoolean()) {
        if (popupParam) {
            popupParam->SetBlockEvent(maskValue->ToBoolean());
        } else {
            LOGI("Empty popup.");
        }
    }
    if (maskValue->IsObject()) {
        auto maskObj = JSRef<JSObject>::Cast(maskValue);
        auto colorValue = maskObj->GetProperty("color");
        Color maskColor;
        if (JSViewAbstract::ParseJsColor(colorValue, maskColor)) {
            popupParam->SetMaskColor(maskColor);
        }
    }
    JSRef<JSVal> onStateChangeVal = popupObj->GetProperty("onStateChange");
    if (onStateChangeVal->IsFunction()) {
        std::vector<std::string> keys = { "isVisible" };
        RefPtr<JsFunction> jsFunc =
            AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onStateChangeVal));
        if (popupParam) {
            auto onStateChangeCallback = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), keys](
                                             const std::string& param) {
                JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
                ACE_SCORING_EVENT("Popup.onStateChange");
                func->Execute(keys, param);
            };
            popupParam->SetOnStateChange(onStateChangeCallback);
        } else {
            LOGI("Empty popup.");
        }
    }

    JSRef<JSVal> primaryButtonVal = popupObj->GetProperty("primaryButton");
    if (primaryButtonVal->IsObject()) {
        ButtonProperties properties;
        JSRef<JSObject> obj = JSRef<JSObject>::Cast(primaryButtonVal);
        JSRef<JSVal> value = obj->GetProperty("value");
        if (value->IsString()) {
            properties.value = value->ToString();
        }

        JSRef<JSVal> actionValue = obj->GetProperty("action");
        if (actionValue->IsFunction()) {
            auto jsOnClickFunc = AceType::MakeRefPtr<JsClickFunction>(JSRef<JSFunc>::Cast(actionValue));
            if (popupParam) {
                auto clickCallback = [execCtx = info.GetExecutionContext(), func = std::move(jsOnClickFunc)](
                                         GestureEvent& info) {
                    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
                    ACE_SCORING_EVENT("primaryButton.action");
                    LOGI("Call primary click");
                    func->Execute(info);
                };
                properties.action = AceType::MakeRefPtr<NG::ClickEvent>(clickCallback);
            }
        }
        properties.showButton = true;
        if (popupParam) {
            popupParam->SetPrimaryButtonProperties(properties);
        } else {
            LOGI("Empty.");
        }
    }

    JSRef<JSVal> secondaryButtonVal = popupObj->GetProperty("secondaryButton");
    if (secondaryButtonVal->IsObject()) {
        ButtonProperties properties;
        JSRef<JSObject> obj = JSRef<JSObject>::Cast(secondaryButtonVal);
        JSRef<JSVal> value = obj->GetProperty("value");
        if (value->IsString()) {
            properties.value = value->ToString();
        }

        JSRef<JSVal> actionValue = obj->GetProperty("action");
        if (actionValue->IsFunction()) {
            auto jsOnClickFunc = AceType::MakeRefPtr<JsClickFunction>(JSRef<JSFunc>::Cast(actionValue));
            if (popupParam) {
                auto clickCallback = [execCtx = info.GetExecutionContext(), func = std::move(jsOnClickFunc)](
                                         GestureEvent& info) {
                    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
                    ACE_SCORING_EVENT("primaryButton.action");
                    LOGI("Call primary click");
                    func->Execute(info);
                };
                properties.action = AceType::MakeRefPtr<NG::ClickEvent>(clickCallback);
            }
        }
        properties.showButton = true;
        if (popupParam) {
            popupParam->SetSecondaryButtonProperties(properties);
        } else {
            LOGI("Empty.");
        }
    }
}

void ParseCustomPopupParam(
    const JSCallbackInfo& info, const JSRef<JSObject>& popupObj, const RefPtr<PopupParam>& popupParam)
{
    auto builderValue = popupObj->GetProperty("builder");
    if (!builderValue->IsObject()) {
        LOGE("builder param is not an object.");
        return;
    }

    JSRef<JSObject> builderObj;
    builderObj = JSRef<JSObject>::Cast(builderValue);
    auto builder = builderObj->GetProperty("builder");
    if (!builder->IsFunction()) {
        LOGE("builder param is not a function.");
        return;
    }
    auto builderFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSFunc>::Cast(builder));
    if (!builderFunc) {
        LOGE("builder function is null.");
        return;
    }

    if (popupParam) {
        popupParam->SetUseCustomComponent(true);
    }

    auto placementValue = popupObj->GetProperty("placement");
    if (placementValue->IsNumber()) {
        auto placement = placementValue->ToNumber<int32_t>();
        if (placement >= 0 && placement <= static_cast<int32_t>(PLACEMENT.size())) {
            popupParam->SetPlacement(PLACEMENT[placement]);
        }
    }

    auto arrowOffset = popupObj->GetProperty("arrowOffset");
    Dimension offset;
    if (JSViewAbstract::ParseJsDimensionVp(arrowOffset, offset)) {
        popupParam->SetArrowOffset(offset);
    }

    auto maskValue = popupObj->GetProperty("mask");
    if (maskValue->IsBoolean()) {
        popupParam->SetBlockEvent(maskValue->ToBoolean());
    }
    if (maskValue->IsObject()) {
        auto maskObj = JSRef<JSObject>::Cast(maskValue);
        auto colorValue = maskObj->GetProperty("color");
        Color maskColor;
        if (JSViewAbstract::ParseJsColor(colorValue, maskColor)) {
            popupParam->SetMaskColor(maskColor);
        }
    }

    auto popupColorValue = popupObj->GetProperty("popupColor");
    Color backgroundColor;
    if (JSViewAbstract::ParseJsColor(popupColorValue, backgroundColor)) {
        popupParam->SetBackgroundColor(backgroundColor);
    }

    auto enableArrowValue = popupObj->GetProperty("enableArrow");
    if (enableArrowValue->IsBoolean()) {
        popupParam->SetEnableArrow(enableArrowValue->ToBoolean());
    }

#if defined(PREVIEW)
    LOGW("[Engine Log] Unable to use the SubWindow in the Previewer. Perform this operation on the "
         "emulator or a real device instead.");
#else
    auto showInSubWindowValue = popupObj->GetProperty("showInSubWindow");
    if (showInSubWindowValue->IsBoolean()) {
        popupParam->SetShowInSubWindow(showInSubWindowValue->ToBoolean());
    }
#endif

    auto autoCancelValue = popupObj->GetProperty("autoCancel");
    if (autoCancelValue->IsBoolean()) {
        popupParam->SetHasAction(!autoCancelValue->ToBoolean());
    }

    JSRef<JSVal> onStateChangeVal = popupObj->GetProperty("onStateChange");
    if (onStateChangeVal->IsFunction()) {
        std::vector<std::string> keys = { "isVisible" };
        RefPtr<JsFunction> jsFunc =
            AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(onStateChangeVal));
        if (popupParam) {
            auto onStateChangeCallback = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), keys](
                                             const std::string& param) {
                JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
                ACE_SCORING_EVENT("popup.onStateChange");
                func->Execute(keys, param);
            };
            popupParam->SetOnStateChange(onStateChangeCallback);
        }
    }
}
#endif

} // namespace

uint32_t ColorAlphaAdapt(uint32_t origin)
{
    uint32_t result = origin;
    if (origin >> COLOR_ALPHA_OFFSET == 0) {
        result = origin | COLOR_ALPHA_VALUE;
    }
    return result;
}

void JSViewAbstract::JsScale(const JSCallbackInfo& info)
{
    LOGD("JsScale");
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::NUMBER, JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsScale", info, checkList)) {
        return;
    }

    if (info[0]->IsObject()) {
        auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
        if (!argsPtrItem || argsPtrItem->IsNull()) {
            LOGE("Js Parse object failed. argsPtr is null. %s", info[0]->ToString().c_str());
            return;
        }
        if (argsPtrItem->Contains("x") || argsPtrItem->Contains("y") || argsPtrItem->Contains("z")) {
            // default: x, y, z (1.0, 1.0, 1.0)
            auto scaleX = 1.0f;
            auto scaleY = 1.0f;
            auto scaleZ = 1.0f;
            // default centerX, centerY 50% 50%;
            Dimension centerX = 0.5_pct;
            Dimension centerY = 0.5_pct;
            ParseJsScale(argsPtrItem, scaleX, scaleY, scaleZ, centerX, centerY);
            ViewAbstractModel::GetInstance()->SetScale(scaleX, scaleY, scaleZ);
            ViewAbstractModel::GetInstance()->SetPivot(centerX, centerY);
            return;
        }
    }
    double scale = 0.0;
    if (ParseJsDouble(info[0], scale)) {
        ViewAbstractModel::GetInstance()->SetScale(scale, scale, 1.0f);
    }
}

void JSViewAbstract::JsScaleX(const JSCallbackInfo& info)
{
    LOGD("JsScaleX");
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    double scaleVal = 0.0;
    if (!ParseJsDouble(info[0], scaleVal)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetScale(scaleVal, 1.0f, 1.0f);
}

void JSViewAbstract::JsScaleY(const JSCallbackInfo& info)
{
    LOGD("JsScaleY");
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    double scaleVal = 0.0;
    if (!ParseJsDouble(info[0], scaleVal)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetScale(1.0f, scaleVal, 1.0f);
}

void JSViewAbstract::JsOpacity(const JSCallbackInfo& info)
{
    LOGD("js_opacity");
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    double opacity = 0.0;
    if (!ParseJsDouble(info[0], opacity)) {
        return;
    }

    if ((LessNotEqual(opacity, 0.0)) || opacity > 1) {
        LOGW("set opacity to %{public}f, over range, set to default opacity", opacity);
        opacity = 1.0;
    }

    ViewAbstractModel::GetInstance()->SetOpacity(opacity);
}

void JSViewAbstract::JsTranslate(const JSCallbackInfo& info)
{
    LOGD("JsTranslate");
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::STRING, JSCallbackInfoType::NUMBER,
        JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsTranslate", info, checkList)) {
        return;
    }

    Dimension value;

    if (info[0]->IsObject()) {
        auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
        if (!argsPtrItem || argsPtrItem->IsNull()) {
            LOGE("Js Parse object failed. argsPtr is null. %s", info[0]->ToString().c_str());
            return;
        }
        if (argsPtrItem->Contains("x") || argsPtrItem->Contains("y") || argsPtrItem->Contains("z")) {
            // default: x, y, z (0.0, 0.0, 0.0)
            auto translateX = Dimension(0.0);
            auto translateY = Dimension(0.0);
            auto translateZ = Dimension(0.0);
            ParseJsTranslate(argsPtrItem, translateX, translateY, translateZ);
            ViewAbstractModel::GetInstance()->SetTranslate(translateX, translateY, translateZ);
            return;
        }
    }
    if (ParseJsDimensionVp(info[0], value)) {
        ViewAbstractModel::GetInstance()->SetTranslate(value, value, value);
    }
}

void JSViewAbstract::JsTranslateX(const JSCallbackInfo& info)
{
    LOGD("JsTranslateX");
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetTranslate(value, 0.0_px, 0.0_px);
}

void JSViewAbstract::JsTranslateY(const JSCallbackInfo& info)
{
    LOGD("JsTranslateY");
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetTranslate(0.0_px, value, 0.0_px);
}

void JSViewAbstract::JsRotate(const JSCallbackInfo& info)
{
    LOGD("JsRotate");
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::NUMBER, JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsRotate", info, checkList)) {
        return;
    }

    if (info[0]->IsObject()) {
        auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
        if (!argsPtrItem || argsPtrItem->IsNull()) {
            LOGE("Js Parse object failed. argsPtr is null. %s", info[0]->ToString().c_str());
            return;
        }
        float dx = 0.0f;
        float dy = 0.0f;
        float dz = 0.0f;
        // default centerX, centerY 50% 50%;
        Dimension centerX = 0.5_pct;
        Dimension centerY = 0.5_pct;
        std::optional<float> angle;
        ParseJsRotate(argsPtrItem, dx, dy, dz, centerX, centerY, angle);
        if (angle) {
            ViewAbstractModel::GetInstance()->SetRotate(dx, dy, dz, angle.value());
            ViewAbstractModel::GetInstance()->SetPivot(centerX, centerY);
        } else {
            LOGE("Js JsRotate failed, not specify angle");
        }
        return;
    }
    double rotateZ;
    if (ParseJsDouble(info[0], rotateZ)) {
        ViewAbstractModel::GetInstance()->SetRotate(0.0f, 0.0f, 1.0f, rotateZ);
    }
}

void JSViewAbstract::JsRotateX(const JSCallbackInfo& info)
{
    LOGD("JsRotateX");
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    double rotateVal = 0.0;
    if (!ParseJsDouble(info[0], rotateVal)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetRotate(1.0f, 0.0f, 0.0f, rotateVal);
}

void JSViewAbstract::JsRotateY(const JSCallbackInfo& info)
{
    LOGD("JsRotateX");
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    double rotateVal = 0.0;
    if (!ParseJsDouble(info[0], rotateVal)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetRotate(0.0f, 1.0f, 0.0f, rotateVal);
}

void JSViewAbstract::JsTransform(const JSCallbackInfo& info)
{
    LOGD("JsTransform");
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsTransform", info, checkList)) {
        return;
    }
    auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
    if (!argsPtrItem || argsPtrItem->IsNull()) {
        LOGE("Js Parse object failed. argsPtr is null. %{public}s", info[0]->ToString().c_str());
        return;
    }
    auto array = argsPtrItem->GetValue("matrix4x4");
    const auto matrix4Len = Matrix4::DIMENSION * Matrix4::DIMENSION;
    if (!array || array->IsNull() || !array->IsArray() || array->GetArraySize() != matrix4Len) {
        LOGE("Js Parse object failed, matrix4x4 is null or not Array");
        return;
    }
    std::vector<float> matrix(matrix4Len);
    for (int32_t i = 0; i < matrix4Len; i++) {
        double value = 0.0;
        ParseJsonDouble(array->GetArrayItem(i), value);
        matrix[i] = static_cast<float>(value);
    }
    ViewAbstractModel::GetInstance()->SetTransformMatrix(matrix);
}

NG::TransitionOptions JSViewAbstract::ParseTransition(std::unique_ptr<JsonValue>& transitionArgs)
{
    bool hasEffect = false;
    NG::TransitionOptions transitionOption;
    transitionOption.Type = ParseTransitionType(transitionArgs->GetString("type", "All"));
    if (transitionArgs->Contains("opacity")) {
        double opacity = 1.0;
        ParseJsonDouble(transitionArgs->GetValue("opacity"), opacity);
        if (opacity > 1.0 || LessNotEqual(opacity, 0.0)) {
            LOGW("set opacity in transition to %{public}lf, over range, use default opacity 1", opacity);
            opacity = 1.0;
        }
        transitionOption.UpdateOpacity(static_cast<float>(opacity));
        hasEffect = true;
    }
    if (transitionArgs->Contains("translate")) {
        auto translateArgs = transitionArgs->GetObject("translate");
        // default: x, y, z (0.0, 0.0, 0.0)
        NG::TranslateOptions translate;
        ParseJsTranslate(translateArgs, translate.x, translate.y, translate.z);
        transitionOption.UpdateTranslate(translate);
        hasEffect = true;
    }
    if (transitionArgs->Contains("scale")) {
        auto scaleArgs = transitionArgs->GetObject("scale");
        // default: x, y, z (1.0, 1.0, 1.0), centerX, centerY 50% 50%;
        NG::ScaleOptions scale(1.0f, 1.0f, 1.0f, 0.5_pct, 0.5_pct);
        ParseJsScale(scaleArgs, scale.xScale, scale.yScale, scale.zScale, scale.centerX, scale.centerY);
        transitionOption.UpdateScale(scale);
        hasEffect = true;
    }
    if (transitionArgs->Contains("rotate")) {
        auto rotateArgs = transitionArgs->GetObject("rotate");
        // default: dx, dy, dz (0.0, 0.0, 0.0), angle 0, centerX, centerY 50% 50%;
        NG::RotateOptions rotate(0.0f, 0.0f, 0.0f, 0.0f, 0.5_pct, 0.5_pct);
        std::optional<float> angle;
        ParseJsRotate(
            rotateArgs, rotate.xDirection, rotate.yDirection, rotate.zDirection, rotate.centerX, rotate.centerY, angle);
        if (angle.has_value()) {
            rotate.angle = angle.value();
            transitionOption.UpdateRotate(rotate);
            hasEffect = true;
        }
    }
    if (!hasEffect) {
        // default transition
        transitionOption = NG::TransitionOptions::GetDefaultTransition(transitionOption.Type);
    }
    return transitionOption;
}

void JSViewAbstract::JsTransition(const JSCallbackInfo& info)
{
    LOGD("JsTransition");
    if (info.Length() > 1) {
        LOGE("Too many arguments");
        return;
    }
    if (info.Length() == 0) {
        ViewAbstractModel::GetInstance()->SetTransition(
            NG::TransitionOptions::GetDefaultTransition(TransitionType::ALL));
        return;
    }
    if (!info[0]->IsObject()) {
        LOGE("arg is not Object.");
        return;
    }
    auto transitionArgs = JsonUtil::ParseJsonString(info[0]->ToString());
    auto options = ParseTransition(transitionArgs);
    ViewAbstractModel::GetInstance()->SetTransition(options);
}

void JSViewAbstract::JsWidth(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    JsWidth(info[0]);
}

bool JSViewAbstract::JsWidth(const JSRef<JSVal>& jsValue)
{
    Dimension value;
    if (!ParseJsDimensionVp(jsValue, value)) {
        return false;
    }

    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }

    ViewAbstractModel::GetInstance()->SetWidth(value);
    return true;
}

void JSViewAbstract::JsHeight(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    JsHeight(info[0]);
}

bool JSViewAbstract::JsHeight(const JSRef<JSVal>& jsValue)
{
    Dimension value;
    if (!ParseJsDimensionVp(jsValue, value)) {
        return false;
    }

    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }

    ViewAbstractModel::GetInstance()->SetHeight(value);
    return true;
}

void JSViewAbstract::JsResponseRegion(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    std::vector<DimensionRect> result;
    if (!JSViewAbstract::ParseJsResponseRegionArray(info[0], result)) {
        return;
    }

    ViewAbstractModel::GetInstance()->SetResponseRegion(result);
}

bool JSViewAbstract::ParseJsDimensionRect(const JSRef<JSVal>& jsValue, DimensionRect& result)
{
    if (!jsValue->IsObject()) {
        LOGE("arg is not Object.");
        return false;
    }

    JSRef<JSObject> obj = JSRef<JSObject>::Cast(jsValue);
    JSRef<JSVal> x = obj->GetProperty("x");
    JSRef<JSVal> y = obj->GetProperty("y");
    JSRef<JSVal> width = obj->GetProperty("width");
    JSRef<JSVal> height = obj->GetProperty("height");
    Dimension xDimen = result.GetOffset().GetX();
    Dimension yDimen = result.GetOffset().GetY();
    Dimension widthDimen = result.GetWidth();
    Dimension heightDimen = result.GetHeight();
    auto s1 = width->ToString();
    auto s2 = height->ToString();
    if (s1.find('-') != std::string::npos) {
        width = JSRef<JSVal>::Make(ToJSValue("100%"));
    }
    if (s2.find('-') != std::string::npos) {
        height = JSRef<JSVal>::Make(ToJSValue("100%"));
    }
    if (ParseJsDimension(x, xDimen, DimensionUnit::VP)) {
        auto offset = result.GetOffset();
        offset.SetX(xDimen);
        result.SetOffset(offset);
    }
    if (ParseJsDimension(y, yDimen, DimensionUnit::VP)) {
        auto offset = result.GetOffset();
        offset.SetY(yDimen);
        result.SetOffset(offset);
    }
    if (ParseJsDimension(width, widthDimen, DimensionUnit::VP)) {
        if (widthDimen.Unit() == DimensionUnit::PERCENT && widthDimen.Value() < 0) {
            return true;
        }
        result.SetWidth(widthDimen);
    }
    if (ParseJsDimension(height, heightDimen, DimensionUnit::VP)) {
        if (heightDimen.Unit() == DimensionUnit::PERCENT && heightDimen.Value() < 0) {
            return true;
        }
        result.SetHeight(heightDimen);
    }
    return true;
}

bool JSViewAbstract::ParseJsResponseRegionArray(const JSRef<JSVal>& jsValue, std::vector<DimensionRect>& result)
{
    if (!jsValue->IsArray() && !jsValue->IsObject()) {
        LOGE("arg is not array or Object.");
        return false;
    }

    if (jsValue->IsArray()) {
        JSRef<JSArray> array = JSRef<JSArray>::Cast(jsValue);
        for (size_t i = 0; i < array->Length(); i++) {
            Dimension xDimen = Dimension(0.0, DimensionUnit::VP);
            Dimension yDimen = Dimension(0.0, DimensionUnit::VP);
            Dimension widthDimen = Dimension(1, DimensionUnit::PERCENT);
            Dimension heightDimen = Dimension(1, DimensionUnit::PERCENT);
            DimensionOffset offsetDimen(xDimen, yDimen);
            DimensionRect dimenRect(widthDimen, heightDimen, offsetDimen);
            if (ParseJsDimensionRect(array->GetValueAt(i), dimenRect)) {
                result.emplace_back(dimenRect);
            } else {
                LOGE("Array element is not Object.");
                return false;
            }
        }
        return true;
    }

    Dimension xDimen = Dimension(0.0, DimensionUnit::VP);
    Dimension yDimen = Dimension(0.0, DimensionUnit::VP);
    Dimension widthDimen = Dimension(1, DimensionUnit::PERCENT);
    Dimension heightDimen = Dimension(1, DimensionUnit::PERCENT);
    DimensionOffset offsetDimen(xDimen, yDimen);
    DimensionRect dimenRect(widthDimen, heightDimen, offsetDimen);
    if (ParseJsDimensionRect(jsValue, dimenRect)) {
        result.emplace_back(dimenRect);
        return true;
    } else {
        LOGE("Array element is not Object.");
        return false;
    }
}

void JSViewAbstract::JsSize(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsSize", info, checkList)) {
        return;
    }

    JSRef<JSObject> sizeObj = JSRef<JSObject>::Cast(info[0]);
    JsWidth(sizeObj->GetProperty("width"));
    JsHeight(sizeObj->GetProperty("height"));
}

void JSViewAbstract::JsConstraintSize(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsConstraintSize", info, checkList)) {
        return;
    }

    JSRef<JSObject> sizeObj = JSRef<JSObject>::Cast(info[0]);

    JSRef<JSVal> minWidthValue = sizeObj->GetProperty("minWidth");
    Dimension minWidth;
    JSRef<JSVal> maxWidthValue = sizeObj->GetProperty("maxWidth");
    Dimension maxWidth;
    JSRef<JSVal> minHeightValue = sizeObj->GetProperty("minHeight");
    Dimension minHeight;
    JSRef<JSVal> maxHeightValue = sizeObj->GetProperty("maxHeight");
    Dimension maxHeight;

    if (ParseJsDimensionVp(minWidthValue, minWidth)) {
        ViewAbstractModel::GetInstance()->SetMinWidth(minWidth);
    }

    if (ParseJsDimensionVp(maxWidthValue, maxWidth)) {
        ViewAbstractModel::GetInstance()->SetMaxWidth(maxWidth);
    }

    if (ParseJsDimensionVp(minHeightValue, minHeight)) {
        ViewAbstractModel::GetInstance()->SetMinHeight(minHeight);
    }

    if (ParseJsDimensionVp(maxHeightValue, maxHeight)) {
        ViewAbstractModel::GetInstance()->SetMaxHeight(maxHeight);
    }
}

void JSViewAbstract::JsLayoutPriority(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::STRING, JSCallbackInfoType::NUMBER };
    if (!CheckJSCallbackInfo("JsLayoutPriority", info, checkList)) {
        return;
    }

    int32_t priority;
    if (info[0]->IsNumber()) {
        priority = info[0]->ToNumber<int32_t>();
    } else {
        priority = static_cast<int32_t>(StringUtils::StringToUint(info[0]->ToString()));
    }
    ViewAbstractModel::GetInstance()->SetLayoutPriority(priority);
}

void JSViewAbstract::JsLayoutWeight(const JSCallbackInfo& info)
{
    int32_t value = 0.0;
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::STRING, JSCallbackInfoType::NUMBER };
    if (!CheckJSCallbackInfo("JsLayoutWeight", info, checkList)) {
        if (!info[0]->IsUndefined()) {
            return;
        }
    }

    if (info[0]->IsNumber()) {
        value = info[0]->ToNumber<int32_t>();
    } else {
        value = static_cast<int32_t>(StringUtils::StringToUint(info[0]->ToString()));
    }

    ViewAbstractModel::GetInstance()->SetLayoutWeight(value);
}

void JSViewAbstract::JsAlign(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::NUMBER };
    if (!CheckJSCallbackInfo("JsAlign", info, checkList)) {
        return;
    }
    auto value = info[0]->ToNumber<int32_t>();
    Alignment alignment = ParseAlignment(value);
    ViewAbstractModel::GetInstance()->SetAlign(alignment);
}

void JSViewAbstract::JsPosition(const JSCallbackInfo& info)
{
    Dimension x;
    Dimension y;
    if (ParseLocationProps(info, x, y)) {
        ViewAbstractModel::GetInstance()->SetPosition(x, y);
    } else {
        ViewAbstractModel::GetInstance()->SetPosition(0.0_vp, 0.0_vp);
    }
}

void JSViewAbstract::JsMarkAnchor(const JSCallbackInfo& info)
{
    Dimension x;
    Dimension y;
    if (ParseLocationProps(info, x, y)) {
        ViewAbstractModel::GetInstance()->MarkAnchor(x, y);
    } else {
        ViewAbstractModel::GetInstance()->MarkAnchor(0.0_vp, 0.0_vp);
    }
}

void JSViewAbstract::JsOffset(const JSCallbackInfo& info)
{
    Dimension x;
    Dimension y;
    if (ParseLocationProps(info, x, y)) {
        ViewAbstractModel::GetInstance()->SetOffset(x, y);
    } else {
        ViewAbstractModel::GetInstance()->SetOffset(0.0_vp, 0.0_vp);
    }
}

void JSViewAbstract::JsEnabled(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    if (!info[0]->IsBoolean()) {
        LOGE("arg is not bool.");
        return;
    }

    bool enabled = info[0]->ToBoolean();

    ViewAbstractModel::GetInstance()->SetEnabled(enabled);
}

void JSViewAbstract::JsAspectRatio(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    double value = 0.0;
    if (!ParseJsDouble(info[0], value)) {
        return;
    }

    ViewAbstractModel::GetInstance()->SetAspectRatio(static_cast<float>(value));
}

void JSViewAbstract::JsOverlay(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::STRING };
    if (!CheckJSCallbackInfo("JsOverlay", info, checkList)) {
        return;
    }

    std::string text = info[0]->ToString();
    std::optional<Alignment> align;
    std::optional<Dimension> offsetX;
    std::optional<Dimension> offsetY;

    if (info.Length() > 1 && !info[1]->IsNull()) {
        JSRef<JSObject> optionObj = JSRef<JSObject>::Cast(info[1]);
        JSRef<JSVal> alignVal = optionObj->GetProperty("align");
        auto value = alignVal->ToNumber<int32_t>();
        Alignment alignment = ParseAlignment(value);
        align = alignment;

        JSRef<JSVal> val = optionObj->GetProperty("offset");
        if (val->IsObject()) {
            JSRef<JSObject> offsetObj = JSRef<JSObject>::Cast(val);
            JSRef<JSVal> xVal = offsetObj->GetProperty("x");
            Dimension x;
            if (ParseJsDimensionVp(xVal, x)) {
                offsetX = x;
            }
            JSRef<JSVal> yVal = offsetObj->GetProperty("y");
            Dimension y;
            if (ParseJsDimensionVp(yVal, y)) {
                offsetY = y;
            }
        }
    }

    ViewAbstractModel::GetInstance()->SetOverlay(text, align, offsetX, offsetY);
}

Alignment JSViewAbstract::ParseAlignment(int32_t align)
{
    Alignment alignment = Alignment::CENTER;
    switch (align) {
        case 0:
            alignment = Alignment::TOP_LEFT;
            break;
        case 1:
            alignment = Alignment::TOP_CENTER;
            break;
        case 2:
            alignment = Alignment::TOP_RIGHT;
            break;
        case 3:
            alignment = Alignment::CENTER_LEFT;
            break;
        case 4:
            alignment = Alignment::CENTER;
            break;
        case 5:
            alignment = Alignment::CENTER_RIGHT;
            break;
        case 6:
            alignment = Alignment::BOTTOM_LEFT;
            break;
        case 7:
            alignment = Alignment::BOTTOM_CENTER;
            break;
        case 8:
            alignment = Alignment::BOTTOM_RIGHT;
            break;
        default:
            LOGE("Invalid value for alignment");
    }
    return alignment;
}

void JSViewAbstract::SetVisibility(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("SetVisibility: The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }
    int32_t visible = 0;
    if (info[0]->IsNull() || info[0]->IsUndefined()) {
        // undefined value use default value.
        visible = 0;
    } else if (!info[0]->IsNumber()) {
        LOGD("SetVisibility: The first param type is not number, invalid.");
        return;
    } else {
        visible = info[0]->ToNumber<int32_t>();
    }

    if (info.Length() > 1 && info[1]->IsFunction()) {
        RefPtr<JsFunction> jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[1]));

        auto onVisibilityChange = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](int32_t visible) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("onVisibilityChange");

            JSRef<JSVal> newJSVal = JSRef<JSVal>::Make(ToJSValue(visible));
            func->ExecuteJS(1, &newJSVal);
        };
        ViewAbstractModel::GetInstance()->SetVisibility(
            static_cast<VisibleType>(visible), std::move(onVisibilityChange));
    } else {
        ViewAbstractModel::GetInstance()->SetVisibility(static_cast<VisibleType>(visible), [](int32_t visible) {});
    }
}

void JSViewAbstract::JsFlexBasis(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("JsFlexBasis: The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    // flexbasis don't support percent case.
    if (value.Unit() == DimensionUnit::PERCENT) {
        value.SetUnit(DimensionUnit::AUTO);
    }
    ViewAbstractModel::GetInstance()->SetFlexBasis(value);
}

void JSViewAbstract::JsFlexGrow(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("JsFlexGrow: The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }
    double value = 0.0;
    if (!ParseJsDouble(info[0], value)) {
        if (info[0]->IsNull() || info[0]->IsUndefined()) {
            // undefined use default value.
            value = 0.0;
        } else {
            return;
        }
    }
    // negative use default value.
    if (value < 0.0) {
        value = 0.0;
    }
    ViewAbstractModel::GetInstance()->SetFlexGrow(static_cast<float>(value));
}

void JSViewAbstract::JsFlexShrink(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("JsFlexShrink: The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }
    double value = 0.0;
    if (!ParseJsDouble(info[0], value)) {
        if (info[0]->IsNull() || info[0]->IsUndefined()) {
            // undefined use default value.
            value = 1.0;
        } else {
            return;
        }
    }
    // negative use default value.
    if (value < 0.0) {
        value = 1.0;
    }
    ViewAbstractModel::GetInstance()->SetFlexShrink(static_cast<float>(value));
}

void JSViewAbstract::JsDisplayPriority(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("JsDisplayPriority: The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }
    double value = 0.0;
    if (!ParseJsDouble(info[0], value)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetDisplayIndex(static_cast<int32_t>(value));
}

void JSViewAbstract::JsSharedTransition(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::STRING };
    if (!CheckJSCallbackInfo("JsSharedTransition", info, checkList)) {
        return;
    }
    // id
    auto id = info[0]->ToString();
    if (id.empty()) {
        LOGE("JsSharedTransition: id is empty.");
        return;
    }
    std::shared_ptr<SharedTransitionOption> sharedOption;

    // options
    if (info.Length() > 1 && info[1]->IsObject()) {
        auto optionsArgs = JsonUtil::ParseJsonString(info[1]->ToString());
        sharedOption = std::make_shared<SharedTransitionOption>();
        // default: duration: 1000; if not specify: duration: 0
        int32_t duration = 0;
        auto durationValue = optionsArgs->GetValue("duration");
        if (durationValue && durationValue->IsNumber()) {
            duration = durationValue->GetInt();
            if (duration < 0) {
                duration = DEFAULT_DURATION;
            }
        }
        sharedOption->duration = duration;
        // default: delay: 0
        auto delay = optionsArgs->GetInt("delay", 0);
        if (delay < 0) {
            delay = 0;
        }
        sharedOption->delay = delay;
        // default: LinearCurve
        RefPtr<Curve> curve;
        auto curveArgs = optionsArgs->GetValue("curve");
        if (curveArgs->IsString()) {
            curve = CreateCurve(optionsArgs->GetString("curve", "linear"));
        } else if (curveArgs->IsObject()) {
            auto curveString = curveArgs->GetValue("__curveString");
            if (!curveString) {
                return;
            }
            curve = CreateCurve(curveString->GetString());
        } else {
            curve = AceType::MakeRefPtr<LinearCurve>();
        }
        sharedOption->curve = curve;
        // motionPath
        if (optionsArgs->Contains("motionPath")) {
            MotionPathOption motionPathOption;
            if (ParseMotionPath(optionsArgs->GetValue("motionPath"), motionPathOption)) {
                sharedOption->motionPathOption = motionPathOption;
            }
        }
        // zIndex
        int32_t zIndex = 0;
        if (optionsArgs->Contains("zIndex")) {
            zIndex = optionsArgs->GetInt("zIndex", 0);
        }
        sharedOption->zIndex = zIndex;
        // type
        SharedTransitionEffectType type = SharedTransitionEffectType::SHARED_EFFECT_EXCHANGE;
        if (optionsArgs->Contains("type")) {
            type = static_cast<SharedTransitionEffectType>(
                optionsArgs->GetInt("type", static_cast<int32_t>(SharedTransitionEffectType::SHARED_EFFECT_EXCHANGE)));
        }
        sharedOption->type = type;
    }
    ViewAbstractModel::GetInstance()->SetSharedTransition(id, sharedOption);
}

void JSViewAbstract::JsGeometryTransition(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::STRING };
    if (!CheckJSCallbackInfo("JsGeometryTransition", info, checkList)) {
        return;
    }
    // id
    auto id = info[0]->ToString();
    if (id.empty()) {
        LOGE("JsGeometryTransition: id is empty.");
        return;
    }
    ViewAbstractModel::GetInstance()->SetGeometryTransition(id);
}

void JSViewAbstract::JsAlignSelf(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::NUMBER };
    if (!CheckJSCallbackInfo("JsAlignSelf", info, checkList)) {
        return;
    }
    auto alignVal = info[0]->ToNumber<int32_t>();

    if (alignVal >= 0 && alignVal <= MAX_ALIGN_VALUE) {
        ViewAbstractModel::GetInstance()->SetAlignSelf(static_cast<FlexAlign>(alignVal));
    }
}

void JSViewAbstract::JsBackgroundColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Color backgroundColor;
    if (!ParseJsColor(info[0], backgroundColor)) {
        backgroundColor = Color::TRANSPARENT;
    }

    ViewAbstractModel::GetInstance()->SetBackgroundColor(backgroundColor);
}

void JSViewAbstract::JsBackgroundImage(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    std::string src;
    if (info[0]->IsString()) {
        src = info[0]->ToString();
    } else if (!ParseJsMedia(info[0], src)) {
        LOGE("can not parse image src.");
        return;
    }

    int32_t repeatIndex = 0;
    if (info.Length() == 2 && info[1]->IsNumber()) {
        repeatIndex = info[1]->ToNumber<int32_t>();
    }
    auto repeat = static_cast<ImageRepeat>(repeatIndex);
    if (info[0]->IsString()) {
        ViewAbstractModel::GetInstance()->SetBackgroundImage(src, GetThemeConstants());
    } else {
        ViewAbstractModel::GetInstance()->SetBackgroundImage(src, nullptr);
    }
    ViewAbstractModel::GetInstance()->SetBackgroundImageRepeat(repeat);
}

void JSViewAbstract::JsBackgroundBlurStyle(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::NUMBER };
    if (!CheckJSCallbackInfo("JsBackgroundBlurStyle", info, checkList)) {
        return;
    }
    BlurStyleOption styleOption;
    auto blurStyle = info[0]->ToNumber<int32_t>();
    if (blurStyle >= static_cast<int>(BlurStyle::THIN) &&
        blurStyle <= static_cast<int>(BlurStyle::BACKGROUND_ULTRA_THICK)) {
        styleOption.blurStyle = static_cast<BlurStyle>(blurStyle);
    }
    if (info.Length() > 1 && info[1]->IsObject()) {
        JSRef<JSObject> jsOption = JSRef<JSObject>::Cast(info[1]);
        auto colorMode = static_cast<int32_t>(ThemeColorMode::SYSTEM);
        ParseJsInt32(jsOption->GetProperty("colorMode"), colorMode);
        if (colorMode >= static_cast<int32_t>(ThemeColorMode::SYSTEM) &&
            colorMode <= static_cast<int32_t>(ThemeColorMode::DARK)) {
            styleOption.colorMode = static_cast<ThemeColorMode>(colorMode);
        }
        auto adaptiveColor = static_cast<int32_t>(AdaptiveColor::DEFAULT);
        ParseJsInt32(jsOption->GetProperty("adaptiveColor"), adaptiveColor);
        if (adaptiveColor >= static_cast<int32_t>(AdaptiveColor::DEFAULT) &&
            adaptiveColor <= static_cast<int32_t>(AdaptiveColor::AVERAGE)) {
            styleOption.adaptiveColor = static_cast<AdaptiveColor>(adaptiveColor);
        }
    }
    ViewAbstractModel::GetInstance()->SetBackgroundBlurStyle(styleOption);
}

void JSViewAbstract::JsBackgroundImageSize(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::NUMBER, JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsBackgroundImageSize", info, checkList)) {
        return;
    }
    BackgroundImageSize bgImgSize;
    if (info[0]->IsNumber()) {
        auto sizeType = static_cast<BackgroundImageSizeType>(info[0]->ToNumber<int32_t>());
        bgImgSize.SetSizeTypeX(sizeType);
        bgImgSize.SetSizeTypeY(sizeType);
    } else {
        auto imageArgs = JsonUtil::ParseJsonString(info[0]->ToString());
        if (imageArgs->IsNull()) {
            LOGE("Js Parse failed. imageArgs is null.");
            return;
        }
        Dimension width;
        Dimension height;
        ParseJsonDimensionVp(imageArgs->GetValue("width"), width);
        ParseJsonDimensionVp(imageArgs->GetValue("height"), height);
        double valueWidth = width.ConvertToPx();
        double valueHeight = height.ConvertToPx();
        BackgroundImageSizeType typeWidth = BackgroundImageSizeType::LENGTH;
        BackgroundImageSizeType typeHeight = BackgroundImageSizeType::LENGTH;
        if (width.Unit() == DimensionUnit::PERCENT) {
            typeWidth = BackgroundImageSizeType::PERCENT;
            valueWidth = width.Value() * FULL_DIMENSION;
        }
        if (height.Unit() == DimensionUnit::PERCENT) {
            typeHeight = BackgroundImageSizeType::PERCENT;
            valueHeight = height.Value() * FULL_DIMENSION;
        }
        bgImgSize.SetSizeTypeX(typeWidth);
        bgImgSize.SetSizeValueX(valueWidth);
        bgImgSize.SetSizeTypeY(typeHeight);
        bgImgSize.SetSizeValueY(valueHeight);
    }

    ViewAbstractModel::GetInstance()->SetBackgroundImageSize(bgImgSize);
}

void JSViewAbstract::JsBackgroundImagePosition(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::NUMBER, JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsBackgroundImagePosition", info, checkList)) {
        return;
    }
    BackgroundImagePosition bgImgPosition;
    if (info[0]->IsNumber()) {
        int32_t align = info[0]->ToNumber<int32_t>();
        switch (align) {
            case 0:
                SetBgImgPosition(DimensionUnit::PERCENT, DimensionUnit::PERCENT, 0.0, 0.0, bgImgPosition);
                break;
            case 1:
                SetBgImgPosition(DimensionUnit::PERCENT, DimensionUnit::PERCENT, HALF_DIMENSION, 0.0, bgImgPosition);
                break;
            case 2:
                SetBgImgPosition(DimensionUnit::PERCENT, DimensionUnit::PERCENT, FULL_DIMENSION, 0.0, bgImgPosition);
                break;
            case 3:
                SetBgImgPosition(DimensionUnit::PERCENT, DimensionUnit::PERCENT, 0.0, HALF_DIMENSION, bgImgPosition);
                break;
            case 4:
                SetBgImgPosition(
                    DimensionUnit::PERCENT, DimensionUnit::PERCENT, HALF_DIMENSION, HALF_DIMENSION, bgImgPosition);
                break;
            case 5:
                SetBgImgPosition(
                    DimensionUnit::PERCENT, DimensionUnit::PERCENT, FULL_DIMENSION, HALF_DIMENSION, bgImgPosition);
                break;
            case 6:
                SetBgImgPosition(DimensionUnit::PERCENT, DimensionUnit::PERCENT, 0.0, FULL_DIMENSION, bgImgPosition);
                break;
            case 7:
                SetBgImgPosition(
                    DimensionUnit::PERCENT, DimensionUnit::PERCENT, HALF_DIMENSION, FULL_DIMENSION, bgImgPosition);
                break;
            case 8:
                SetBgImgPosition(
                    DimensionUnit::PERCENT, DimensionUnit::PERCENT, FULL_DIMENSION, FULL_DIMENSION, bgImgPosition);
                break;
            default:
                break;
        }
    } else {
        auto imageArgs = JsonUtil::ParseJsonString(info[0]->ToString());
        if (imageArgs->IsNull()) {
            LOGE("Js Parse failed. imageArgs is null.");
            return;
        }
        Dimension x;
        Dimension y;
        ParseJsonDimensionVp(imageArgs->GetValue("x"), x);
        ParseJsonDimensionVp(imageArgs->GetValue("y"), y);
        double valueX = x.Value();
        double valueY = y.Value();
        DimensionUnit typeX = DimensionUnit::PX;
        DimensionUnit typeY = DimensionUnit::PX;
        if (x.Unit() == DimensionUnit::PERCENT) {
            valueX = x.Value() * FULL_DIMENSION;
            typeX = DimensionUnit::PERCENT;
        }
        if (y.Unit() == DimensionUnit::PERCENT) {
            valueY = y.Value() * FULL_DIMENSION;
            typeY = DimensionUnit::PERCENT;
        }
        SetBgImgPosition(typeX, typeY, valueX, valueY, bgImgPosition);
    }

    ViewAbstractModel::GetInstance()->SetBackgroundImagePosition(bgImgPosition);
}

std::vector<NG::OptionParam> ParseBindOptionParam(const JSCallbackInfo& info)
{
    auto paramArray = JSRef<JSArray>::Cast(info[0]);
    std::vector<NG::OptionParam> params(paramArray->Length());
    // parse paramArray
    LOGD("parsing paramArray size = %{public}d", static_cast<int>(paramArray->Length()));
    for (size_t i = 0; i < paramArray->Length(); ++i) {
        auto indexObject = JSRef<JSObject>::Cast(paramArray->GetValueAt(i));
        JSViewAbstract::ParseJsString(indexObject->GetProperty("value"), params[i].first);
        LOGD("option #%{public}d is %{public}s", static_cast<int>(i), params[i].first.c_str());
        auto action = AceType::MakeRefPtr<JsClickFunction>(JSRef<JSFunc>::Cast(indexObject->GetProperty("action")));
        // set onClick function
        params[i].second = [func = std::move(action), context = info.GetExecutionContext()]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context);
            ACE_SCORING_EVENT("menu.action");
            if (func) {
                func->Execute();
            }
        };
    }
    return params;
}

void JSViewAbstract::JsBindMenu(const JSCallbackInfo& info)
{
    if (info[0]->IsArray()) {
        std::vector<NG::OptionParam> optionsParam = ParseBindOptionParam(info);
        ViewAbstractModel::GetInstance()->BindMenu(std::move(optionsParam), nullptr);
    } else if (info[0]->IsObject()) {
        // CustomBuilder
        JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[0]);
        auto builder = obj->GetProperty("builder");
        if (!builder->IsFunction()) {
            LOGE("builder param is not a function.");
            return;
        }
        auto builderFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSFunc>::Cast(builder));
        CHECK_NULL_VOID(builderFunc);
        auto buildFunc = [execCtx = info.GetExecutionContext(), func = std::move(builderFunc)]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("BuildMenu");
            func->Execute();
        };
        ViewAbstractModel::GetInstance()->BindMenu({}, std::move(buildFunc));
    } else {
        LOGE("bindMenu info is invalid");
    }
}

void JSViewAbstract::JsPadding(const JSCallbackInfo& info)
{
    ParseMarginOrPadding(info, false);
}

void JSViewAbstract::JsMargin(const JSCallbackInfo& info)
{
    ParseMarginOrPadding(info, true);
}

void JSViewAbstract::ParseMarginOrPadding(const JSCallbackInfo& info, bool isMargin)
{
    if (info[0]->IsObject()) {
        std::optional<Dimension> left;
        std::optional<Dimension> right;
        std::optional<Dimension> top;
        std::optional<Dimension> bottom;
        JSRef<JSObject> paddingObj = JSRef<JSObject>::Cast(info[0]);

        Dimension leftDimen;
        if (ParseJsDimensionVp(paddingObj->GetProperty("left"), leftDimen)) {
            left = leftDimen;
        }
        Dimension rightDimen;
        if (ParseJsDimensionVp(paddingObj->GetProperty("right"), rightDimen)) {
            right = rightDimen;
        }
        Dimension topDimen;
        if (ParseJsDimensionVp(paddingObj->GetProperty("top"), topDimen)) {
            top = topDimen;
        }
        Dimension bottomDimen;
        if (ParseJsDimensionVp(paddingObj->GetProperty("bottom"), bottomDimen)) {
            bottom = bottomDimen;
        }
        if (left.has_value() || right.has_value() || top.has_value() || bottom.has_value()) {
            if (isMargin) {
                ViewAbstractModel::GetInstance()->SetMargins(top, bottom, left, right);
            } else {
                ViewAbstractModel::GetInstance()->SetPaddings(top, bottom, left, right);
            }
            return;
        }
    }

    Dimension length;
    if (!ParseJsDimensionVp(info[0], length)) {
        // use default value.
        length.Reset();
    }
    if (isMargin) {
        ViewAbstractModel::GetInstance()->SetMargin(length);
    } else {
        ViewAbstractModel::GetInstance()->SetPadding(length);
    }
}

void JSViewAbstract::JsBorder(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsBorder", info, checkList)) {
        LOGE("args is not a object. %s", info[0]->ToString().c_str());
        return;
    }
    JSRef<JSObject> object = JSRef<JSObject>::Cast(info[0]);
    auto valueWidth = object->GetProperty("width");
    if (!valueWidth->IsUndefined()) {
        ParseBorderWidth(valueWidth);
    }

    // use default value when undefined.
    ParseBorderColor(object->GetProperty("color"));

    auto valueRadius = object->GetProperty("radius");
    if (!valueRadius->IsUndefined()) {
        ParseBorderRadius(valueRadius);
    }
    // use default value when undefined.
    ParseBorderStyle(object->GetProperty("style"));

    info.ReturnSelf();
}

void JSViewAbstract::JsBorderWidth(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::STRING, JSCallbackInfoType::NUMBER,
        JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsBorderWidth", info, checkList)) {
        LOGE("args need a string or number or object");
        return;
    }
    ParseBorderWidth(info[0]);
}

void JSViewAbstract::ParseBorderWidth(const JSRef<JSVal>& args)
{
    if (!args->IsObject() && !args->IsNumber() && !args->IsString()) {
        LOGE("args need a object or number or string. %{public}s", args->ToString().c_str());
        return;
    }
    std::optional<Dimension> leftDimen;
    std::optional<Dimension> rightDimen;
    std::optional<Dimension> topDimen;
    std::optional<Dimension> bottomDimen;
    Dimension borderWidth;
    if (ParseJsDimensionVp(args, borderWidth)) {
        if (borderWidth.IsNegative()) {
            borderWidth.Reset();
        }
        ViewAbstractModel::GetInstance()->SetBorderWidth(borderWidth);
    } else if (args->IsObject()) {
        JSRef<JSObject> object = JSRef<JSObject>::Cast(args);
        Dimension left;
        if (ParseJsDimensionVp(object->GetProperty("left"), left) && left.IsNonNegative()) {
            leftDimen = left;
        }
        Dimension right;
        if (ParseJsDimensionVp(object->GetProperty("right"), right) && right.IsNonNegative()) {
            rightDimen = right;
        }
        Dimension top;
        if (ParseJsDimensionVp(object->GetProperty("top"), top) && top.IsNonNegative()) {
            topDimen = top;
        }
        Dimension bottom;
        if (ParseJsDimensionVp(object->GetProperty("bottom"), bottom) && bottom.IsNonNegative()) {
            bottomDimen = bottom;
        }
        ViewAbstractModel::GetInstance()->SetBorderWidth(leftDimen, rightDimen, topDimen, bottomDimen);
    } else {
        LOGE("args format error. %{public}s", args->ToString().c_str());
        return;
    }
}

void JSViewAbstract::JsBorderImage(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsBorderImage", info, checkList)) {
        LOGE("args is not a object. %{public}s", info[0]->ToString().c_str());
        return;
    }
    JSRef<JSObject> object = JSRef<JSObject>::Cast(info[0]);
    if (object->IsEmpty()) {
        return;
    }

    RefPtr<BorderImage> borderImage = AceType::MakeRefPtr<BorderImage>();
    uint8_t imageBorderBitsets = 0;

    auto valueSource = object->GetProperty("source");
    if (!valueSource->IsString() && !valueSource->IsObject()) {
        LOGE("Border image source type not recognized");
        return;
    }
    std::string srcResult;
    if (valueSource->IsString()) {
        srcResult = valueSource->ToString();
        if (!srcResult.empty()) {
            borderImage->SetSrc(srcResult);
            imageBorderBitsets |= BorderImage::SOURCE_BIT;
        }
    } else if (valueSource->IsObject()) {
        if (ParseJsMedia(valueSource, srcResult)) {
            borderImage->SetSrc(srcResult);
            imageBorderBitsets |= BorderImage::SOURCE_BIT;
        } else {
            ParseBorderImageLinearGradient(valueSource, imageBorderBitsets);
        }
    }
    auto valueOutset = object->GetProperty("outset");
    if (valueOutset->IsNumber() || valueOutset->IsString() || valueOutset->IsObject()) {
        imageBorderBitsets |= BorderImage::OUTSET_BIT;
        ParseBorderImageOutset(valueOutset, borderImage);
    }
    auto valueRepeat = object->GetProperty("repeat");
    if (!valueRepeat->IsNull()) {
        imageBorderBitsets |= BorderImage::REPEAT_BIT;
        ParseBorderImageRepeat(valueRepeat, borderImage);
    }
    auto valueSlice = object->GetProperty("slice");
    if (valueSlice->IsNumber() || valueSlice->IsString() || valueSlice->IsObject()) {
        imageBorderBitsets |= BorderImage::SLICE_BIT;
        ParseBorderImageSlice(valueSlice, borderImage);
    }
    auto valueWidth = object->GetProperty("width");
    if (valueWidth->IsNumber() || valueWidth->IsString() || valueWidth->IsObject()) {
        imageBorderBitsets |= BorderImage::WIDTH_BIT;
        ParseBorderImageWidth(valueWidth, borderImage);
    }
    auto needFill = object->GetProperty("fill");
    if (needFill->IsBoolean()) {
        borderImage->SetNeedFillCenter(needFill->ToBoolean());
    }
    ViewAbstractModel::GetInstance()->SetBorderImage(borderImage, imageBorderBitsets);
    info.ReturnSelf();
}

void JSViewAbstract::ParseBorderImageDimension(
    const JSRef<JSVal>& args, BorderImage::BorderImageOption& borderImageDimension)
{
    JSRef<JSObject> object = JSRef<JSObject>::Cast(args);
    static std::array<std::string, 4> keys = { "left", "right", "top", "bottom" };
    for (uint32_t i = 0; i < keys.size(); i++) {
        Dimension currentDimension;
        auto dimensionValue = object->GetProperty(keys.at(i).c_str());
        if (ParseJsDimensionVp(dimensionValue, currentDimension)) {
            auto direction = static_cast<BorderImageDirection>(i);
            switch (direction) {
                case BorderImageDirection::LEFT:
                    borderImageDimension.leftDimension = currentDimension;
                    break;
                case BorderImageDirection::RIGHT:
                    borderImageDimension.rightDimension = currentDimension;
                    break;
                case BorderImageDirection::TOP:
                    borderImageDimension.topDimension = currentDimension;
                    break;
                case BorderImageDirection::BOTTOM:
                    borderImageDimension.bottomDimension = currentDimension;
                    break;
                default:
                    LOGE("Unsupported border image direction");
                    break;
            }
        }
    }
}

void JSViewAbstract::ParseBorderImageLinearGradient(const JSRef<JSVal>& args, uint8_t& bitset)
{
    auto argsPtrItem = JsonUtil::ParseJsonString(args->ToString());
    if (!argsPtrItem || argsPtrItem->IsNull()) {
        LOGE("Parse border image linear gradient failed. argsPtr is null. %{public}s", args->ToString().c_str());
        return;
    }
    NG::Gradient lineGradient;
    lineGradient.CreateGradientWithType(NG::GradientType::LINEAR);
    // angle
    std::optional<float> degree;
    GetAngle("angle", argsPtrItem, degree);
    if (degree) {
        lineGradient.GetLinearGradient()->angle = Dimension(degree.value(), DimensionUnit::PX);
        degree.reset();
    }
    // direction
    auto direction = static_cast<NG::GradientDirection>(
        argsPtrItem->GetInt("direction", static_cast<int32_t>(NG::GradientDirection::NONE)));
    switch (direction) {
        case NG::GradientDirection::LEFT:
            lineGradient.GetLinearGradient()->linearX = NG::GradientDirection::LEFT;
            break;
        case NG::GradientDirection::RIGHT:
            lineGradient.GetLinearGradient()->linearX = NG::GradientDirection::RIGHT;
            break;
        case NG::GradientDirection::TOP:
            lineGradient.GetLinearGradient()->linearY = NG::GradientDirection::TOP;
            break;
        case NG::GradientDirection::BOTTOM:
            lineGradient.GetLinearGradient()->linearY = NG::GradientDirection::BOTTOM;
            break;
        case NG::GradientDirection::LEFT_TOP:
            lineGradient.GetLinearGradient()->linearX = NG::GradientDirection::LEFT;
            lineGradient.GetLinearGradient()->linearY = NG::GradientDirection::TOP;
            break;
        case NG::GradientDirection::LEFT_BOTTOM:
            lineGradient.GetLinearGradient()->linearX = NG::GradientDirection::LEFT;
            lineGradient.GetLinearGradient()->linearY = NG::GradientDirection::BOTTOM;
            break;
        case NG::GradientDirection::RIGHT_TOP:
            lineGradient.GetLinearGradient()->linearX = NG::GradientDirection::RIGHT;
            lineGradient.GetLinearGradient()->linearY = NG::GradientDirection::TOP;
            break;
        case NG::GradientDirection::RIGHT_BOTTOM:
            lineGradient.GetLinearGradient()->linearX = NG::GradientDirection::RIGHT;
            lineGradient.GetLinearGradient()->linearY = NG::GradientDirection::BOTTOM;
            break;
        case NG::GradientDirection::NONE:
        case NG::GradientDirection::START_TO_END:
        case NG::GradientDirection::END_TO_START:
        default:
            break;
    }
    auto repeating = argsPtrItem->GetBool("repeating", false);
    lineGradient.SetRepeat(repeating);
    NewGetGradientColorStops(lineGradient, argsPtrItem->GetValue("colors"));
    ViewAbstractModel::GetInstance()->SetBorderImageGradient(lineGradient);
    bitset |= BorderImage::GRADIENT_BIT;
}

void JSViewAbstract::ParseBorderImageRepeat(const JSRef<JSVal>& args, RefPtr<BorderImage>& borderImage)
{
    auto repeatString = args->ToString();
    if (repeatString == "Repeat") {
        borderImage->SetRepeatMode(BorderImageRepeat::REPEAT);
    } else if (repeatString == "Round") {
        borderImage->SetRepeatMode(BorderImageRepeat::ROUND);
    } else if (repeatString == "Space") {
        borderImage->SetRepeatMode(BorderImageRepeat::SPACE);
    } else {
        borderImage->SetRepeatMode(BorderImageRepeat::STRETCH);
    }
}

void JSViewAbstract::ParseBorderImageOutset(const JSRef<JSVal>& args, RefPtr<BorderImage>& borderImage)
{
    Dimension outsetDimension;
    if (ParseJsDimensionVp(args, outsetDimension)) {
        borderImage->SetEdgeOutset(BorderImageDirection::LEFT, outsetDimension);
        borderImage->SetEdgeOutset(BorderImageDirection::RIGHT, outsetDimension);
        borderImage->SetEdgeOutset(BorderImageDirection::TOP, outsetDimension);
        borderImage->SetEdgeOutset(BorderImageDirection::BOTTOM, outsetDimension);
        return;
    }
    BorderImage::BorderImageOption option;
    ParseBorderImageDimension(args, option);
    if (option.leftDimension.has_value()) {
        borderImage->SetEdgeOutset(BorderImageDirection::LEFT, option.leftDimension.value());
    }
    if (option.rightDimension.has_value()) {
        borderImage->SetEdgeOutset(BorderImageDirection::RIGHT, option.rightDimension.value());
    }
    if (option.topDimension.has_value()) {
        borderImage->SetEdgeOutset(BorderImageDirection::TOP, option.topDimension.value());
    }
    if (option.bottomDimension.has_value()) {
        borderImage->SetEdgeOutset(BorderImageDirection::BOTTOM, option.bottomDimension.value());
    }
}

void JSViewAbstract::ParseBorderImageSlice(const JSRef<JSVal>& args, RefPtr<BorderImage>& borderImage)
{
    Dimension sliceDimension;
    if (ParseJsDimensionVp(args, sliceDimension)) {
        borderImage->SetEdgeSlice(BorderImageDirection::LEFT, sliceDimension);
        borderImage->SetEdgeSlice(BorderImageDirection::RIGHT, sliceDimension);
        borderImage->SetEdgeSlice(BorderImageDirection::TOP, sliceDimension);
        borderImage->SetEdgeSlice(BorderImageDirection::BOTTOM, sliceDimension);
        return;
    }

    JSRef<JSObject> object = JSRef<JSObject>::Cast(args);
    static std::array<std::string, 4> keys = { "left", "right", "top", "bottom" };
    for (uint32_t i = 0; i < keys.size(); i++) {
        auto dimensionValue = object->GetProperty(keys.at(i).c_str());
        if (ParseJsDimensionVp(dimensionValue, sliceDimension)) {
            borderImage->SetEdgeSlice(static_cast<BorderImageDirection>(i), sliceDimension);
        }
    }
}

void JSViewAbstract::ParseBorderImageWidth(const JSRef<JSVal>& args, RefPtr<BorderImage>& borderImage)
{
    Dimension widthDimension;
    if (ParseJsDimensionVp(args, widthDimension)) {
        borderImage->SetEdgeWidth(BorderImageDirection::LEFT, widthDimension);
        borderImage->SetEdgeWidth(BorderImageDirection::RIGHT, widthDimension);
        borderImage->SetEdgeWidth(BorderImageDirection::TOP, widthDimension);
        borderImage->SetEdgeWidth(BorderImageDirection::BOTTOM, widthDimension);
        return;
    }

    BorderImage::BorderImageOption option;
    ParseBorderImageDimension(args, option);
    if (option.leftDimension.has_value()) {
        borderImage->SetEdgeWidth(BorderImageDirection::LEFT, option.leftDimension.value());
    }
    if (option.rightDimension.has_value()) {
        borderImage->SetEdgeWidth(BorderImageDirection::RIGHT, option.rightDimension.value());
    }
    if (option.topDimension.has_value()) {
        borderImage->SetEdgeWidth(BorderImageDirection::TOP, option.topDimension.value());
    }
    if (option.bottomDimension.has_value()) {
        borderImage->SetEdgeWidth(BorderImageDirection::BOTTOM, option.bottomDimension.value());
    }
}

void JSViewAbstract::JsBorderColor(const JSCallbackInfo& info)
{
    ParseBorderColor(info[0]);
}

void JSViewAbstract::ParseBorderColor(const JSRef<JSVal>& args)
{
    if (!args->IsObject() && !args->IsNumber() && !args->IsString()) {
        LOGI("args(%{public}s) is invalid, use default value.", args->ToString().c_str());
        ViewAbstractModel::GetInstance()->SetBorderColor(Color::BLACK);
        return;
    }
    std::optional<Color> leftColor;
    std::optional<Color> rightColor;
    std::optional<Color> topColor;
    std::optional<Color> bottomColor;
    Color borderColor;
    if (ParseJsColor(args, borderColor)) {
        ViewAbstractModel::GetInstance()->SetBorderColor(borderColor);
    } else if (args->IsObject()) {
        JSRef<JSObject> object = JSRef<JSObject>::Cast(args);
        Color left;
        if (ParseJsColor(object->GetProperty("left"), left)) {
            leftColor = left;
        }
        Color right;
        if (ParseJsColor(object->GetProperty("right"), right)) {
            rightColor = right;
        }
        Color top;
        if (ParseJsColor(object->GetProperty("top"), top)) {
            topColor = top;
        }
        Color bottom;
        if (ParseJsColor(object->GetProperty("bottom"), bottom)) {
            bottomColor = bottom;
        }

        ViewAbstractModel::GetInstance()->SetBorderColor(leftColor, rightColor, topColor, bottomColor);
    } else {
        LOGE("args format error. %{public}s", args->ToString().c_str());
        return;
    }
}

void JSViewAbstract::JsBorderRadius(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::STRING, JSCallbackInfoType::NUMBER,
        JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsBorderRadius", info, checkList)) {
        LOGE("args need a string or number or object");
        return;
    }
    ParseBorderRadius(info[0]);
}

void JSViewAbstract::ParseBorderRadius(const JSRef<JSVal>& args)
{
    if (!args->IsObject() && !args->IsNumber() && !args->IsString()) {
        LOGE("args need a object or number or string. %{public}s", args->ToString().c_str());
        return;
    }
    std::optional<Dimension> radiusTopLeft;
    std::optional<Dimension> radiusTopRight;
    std::optional<Dimension> radiusBottomLeft;
    std::optional<Dimension> radiusBottomRight;
    Dimension borderRadius;
    if (ParseJsDimensionVp(args, borderRadius)) {
        ViewAbstractModel::GetInstance()->SetBorderRadius(borderRadius);
    } else if (args->IsObject()) {
        JSRef<JSObject> object = JSRef<JSObject>::Cast(args);
        Dimension topLeft;
        if (ParseJsDimensionVp(object->GetProperty("topLeft"), topLeft)) {
            radiusTopLeft = topLeft;
        }
        Dimension topRight;
        if (ParseJsDimensionVp(object->GetProperty("topRight"), topRight)) {
            radiusTopRight = topRight;
        }
        Dimension bottomLeft;
        if (ParseJsDimensionVp(object->GetProperty("bottomLeft"), bottomLeft)) {
            radiusBottomLeft = bottomLeft;
        }
        Dimension bottomRight;
        if (ParseJsDimensionVp(object->GetProperty("bottomRight"), bottomRight)) {
            radiusBottomRight = bottomRight;
        }
        ViewAbstractModel::GetInstance()->SetBorderRadius(
            radiusTopLeft, radiusTopRight, radiusBottomLeft, radiusBottomRight);
    } else {
        LOGE("args format error. %{public}s", args->ToString().c_str());
        return;
    }
}

void JSViewAbstract::JsBorderStyle(const JSCallbackInfo& info)
{
    ParseBorderStyle(info[0]);
}
namespace {
BorderStyle ConvertBorderStyle(int32_t value)
{
    auto style = static_cast<BorderStyle>(value);
    if (style < BorderStyle::SOLID || style > BorderStyle::NONE) {
        LOGW("border style(%{public}d) is invalid, use default value.", value);
        style = BorderStyle::SOLID;
    }
    return style;
}
} // namespace

void JSViewAbstract::ParseBorderStyle(const JSRef<JSVal>& args)
{
    if (!args->IsObject() && !args->IsNumber()) {
        LOGI("args(%{public}s) is invalid, use default value.", args->ToString().c_str());
        ViewAbstractModel::GetInstance()->SetBorderStyle(BorderStyle::SOLID);
        return;
    }
    if (args->IsObject()) {
        std::optional<BorderStyle> styleLeft;
        std::optional<BorderStyle> styleRight;
        std::optional<BorderStyle> styleTop;
        std::optional<BorderStyle> styleBottom;
        JSRef<JSObject> object = JSRef<JSObject>::Cast(args);
        auto leftValue = object->GetProperty("left");
        if (!leftValue->IsUndefined() && leftValue->IsNumber()) {
            styleLeft = ConvertBorderStyle(leftValue->ToNumber<int32_t>());
        }
        auto rightValue = object->GetProperty("right");
        if (!rightValue->IsUndefined() && rightValue->IsNumber()) {
            styleRight = ConvertBorderStyle(rightValue->ToNumber<int32_t>());
        }
        auto topValue = object->GetProperty("top");
        if (!topValue->IsUndefined() && topValue->IsNumber()) {
            styleTop = ConvertBorderStyle(topValue->ToNumber<int32_t>());
        }
        auto bottomValue = object->GetProperty("bottom");
        if (!bottomValue->IsUndefined() && bottomValue->IsNumber()) {
            styleBottom = ConvertBorderStyle(bottomValue->ToNumber<int32_t>());
        }
        ViewAbstractModel::GetInstance()->SetBorderStyle(styleLeft, styleRight, styleTop, styleBottom);
        return;
    }
    auto borderStyle = ConvertBorderStyle(args->ToNumber<int32_t>());
    ViewAbstractModel::GetInstance()->SetBorderStyle(borderStyle);
}

void JSViewAbstract::JsBlur(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }

    double blur = 0.0;
    if (!ParseJsDouble(info[0], blur)) {
        return;
    }
    SetBlur(blur);
    info.SetReturnValue(info.This());
}

void JSViewAbstract::JsColorBlend(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Color colorBlend;
    if (!ParseJsColor(info[0], colorBlend)) {
        return;
    }
    SetColorBlend(colorBlend);
    info.SetReturnValue(info.This());
}

void JSViewAbstract::JsBackdropBlur(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }

    double blur = 0.0;
    if (!ParseJsDouble(info[0], blur)) {
        return;
    }
    SetBackdropBlur(blur);
    info.SetReturnValue(info.This());
}

void JSViewAbstract::JsWindowBlur(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsWindowBlur", info, checkList)) {
        return;
    }

    auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
    if (!argsPtrItem || argsPtrItem->IsNull()) {
        LOGE("Js Parse object failed. argsPtr is null. %s", info[0]->ToString().c_str());
        return;
    }
    double progress = 0.0;
    ParseJsonDouble(argsPtrItem->GetValue("percent"), progress);
    auto style = argsPtrItem->GetInt("style", static_cast<int32_t>(WindowBlurStyle::STYLE_BACKGROUND_SMALL_LIGHT));

    progress = std::clamp(progress, 0.0, 1.0);
    style = std::clamp(style, static_cast<int32_t>(WindowBlurStyle::STYLE_BACKGROUND_SMALL_LIGHT),
        static_cast<int32_t>(WindowBlurStyle::STYLE_BACKGROUND_XLARGE_DARK));

    SetWindowBlur(static_cast<float>(progress), static_cast<WindowBlurStyle>(style));
    info.SetReturnValue(info.This());
}

bool JSViewAbstract::ParseJsDimension(const JSRef<JSVal>& jsValue, Dimension& result, DimensionUnit defaultUnit)
{
    if (!jsValue->IsNumber() && !jsValue->IsString() && !jsValue->IsObject()) {
        return false;
    }

    if (jsValue->IsNumber()) {
        result = Dimension(jsValue->ToNumber<double>(), defaultUnit);
        return true;
    }
    if (jsValue->IsString()) {
        result = StringUtils::StringToDimensionWithUnit(jsValue->ToString(), defaultUnit);
        return true;
    }
    JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(jsValue);
    JSRef<JSVal> resId = jsObj->GetProperty("id");
    if (!resId->IsNumber()) {
        LOGW("resId is not number");
        return false;
    }
    auto themeConstants = GetThemeConstants(jsObj);
    if (!themeConstants) {
        LOGE("themeConstants is nullptr");
        return false;
    }
    auto resIdNum = resId->ToNumber<int32_t>();
    if (resIdNum == -1) {
        if (!IsGetResourceByName(jsObj)) {
            return false;
        }
        JSRef<JSVal> args = jsObj->GetProperty("params");
        JSRef<JSArray> params = JSRef<JSArray>::Cast(args);
        auto param = params->GetValueAt(0);
        result = themeConstants->GetDimensionByName(param->ToString());
        return true;
    }

    JSRef<JSVal> type = jsObj->GetProperty("type");
    if (!type->IsNull() && type->IsNumber() &&
        type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::STRING)) {
        auto value = themeConstants->GetString(resId->ToNumber<uint32_t>());
        result = StringUtils::StringToDimensionWithUnit(value, defaultUnit);
        return true;
    }
    if (!type->IsNull() && type->IsNumber() &&
        type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::INTEGER)) {
        auto value = std::to_string(themeConstants->GetInt(resId->ToNumber<uint32_t>()));
        result = StringUtils::StringToDimensionWithUnit(value, defaultUnit);
        return true;
    }
    result = themeConstants->GetDimension(resId->ToNumber<uint32_t>());
    return true;
}

bool JSViewAbstract::ParseJsDimensionVp(const JSRef<JSVal>& jsValue, Dimension& result)
{
    // 'vp' -> the value varies with pixel density of device.
    return ParseJsDimension(jsValue, result, DimensionUnit::VP);
}

bool JSViewAbstract::ParseJsDimensionFp(const JSRef<JSVal>& jsValue, Dimension& result)
{
    // the 'fp' unit is used for text scenes.
    return ParseJsDimension(jsValue, result, DimensionUnit::FP);
}

bool JSViewAbstract::ParseJsDimensionPx(const JSRef<JSVal>& jsValue, Dimension& result)
{
    return ParseJsDimension(jsValue, result, DimensionUnit::PX);
}

bool JSViewAbstract::ParseResourceToDouble(const JSRef<JSVal>& jsValue, double& result)
{
    JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(jsValue);
    if (jsObj->IsEmpty()) {
        LOGW("jsObj is nullptr");
        return false;
    }
    JSRef<JSVal> id = jsObj->GetProperty("id");
    JSRef<JSVal> type = jsObj->GetProperty("type");
    if (!id->IsNumber() || !type->IsNumber()) {
        LOGW("at least one of id and type is not number");
        return false;
    }
    auto themeConstants = GetThemeConstants(jsObj);
    auto resId = id->ToNumber<int32_t>();
    auto resType = type->ToNumber<uint32_t>();
    if (!themeConstants) {
        LOGW("themeConstants is nullptr");
        return false;
    }
    if (resId == -1) {
        if (!IsGetResourceByName(jsObj)) {
            return false;
        }
        JSRef<JSVal> args = jsObj->GetProperty("params");
        JSRef<JSArray> params = JSRef<JSArray>::Cast(args);
        auto param = params->GetValueAt(0);
        if (resType == static_cast<uint32_t>(ResourceType::STRING)) {
            auto numberString = themeConstants->GetStringByName(param->ToString());
            return StringUtils::StringToDouble(numberString, result);
        }
        if (resType == static_cast<uint32_t>(ResourceType::INTEGER)) {
            result = themeConstants->GetIntByName(param->ToString());
            return true;
        }
        if (resType == static_cast<uint32_t>(ResourceType::FLOAT)) {
            result = themeConstants->GetDoubleByName(param->ToString());
            return true;
        }
        return false;
    }
    if (resType == static_cast<uint32_t>(ResourceType::STRING)) {
        auto numberString = themeConstants->GetString(resId);
        return StringUtils::StringToDouble(numberString, result);
    }
    if (resType == static_cast<uint32_t>(ResourceType::INTEGER)) {
        result = themeConstants->GetInt(resId);
        return true;
    }
    if (resType == static_cast<uint32_t>(ResourceType::FLOAT)) {
        result = themeConstants->GetDouble(resId);
        return true;
    }
    return false;
}

bool JSViewAbstract::ParseJsDouble(const JSRef<JSVal>& jsValue, double& result)
{
    if (jsValue->IsNumber()) {
        result = jsValue->ToNumber<double>();
        return true;
    }
    if (jsValue->IsString()) {
        return StringUtils::StringToDouble(jsValue->ToString(), result);
    }
    if (jsValue->IsObject()) {
        return ParseResourceToDouble(jsValue, result);
    }
    return false;
}

bool JSViewAbstract::ParseJsInt32(const JSRef<JSVal>& jsValue, int32_t& result)
{
    if (!jsValue->IsNumber() && !jsValue->IsString() && !jsValue->IsObject()) {
        return false;
    }
    if (jsValue->IsNumber()) {
        result = jsValue->ToNumber<int32_t>();
        return true;
    }
    if (jsValue->IsString()) {
        result = StringUtils::StringToInt(jsValue->ToString());
        return true;
    }
    JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(jsValue);
    JSRef<JSVal> resId = jsObj->GetProperty("id");
    if (!resId->IsNumber()) {
        LOGW("resId is not number");
        return false;
    }

    auto themeConstants = GetThemeConstants(jsObj);
    if (!themeConstants) {
        LOGW("themeConstants is nullptr");
        return false;
    }
    auto resIdNum = resId->ToNumber<int32_t>();
    if (resIdNum == -1) {
        if (!IsGetResourceByName(jsObj)) {
            return false;
        }
        JSRef<JSVal> args = jsObj->GetProperty("params");
        JSRef<JSArray> params = JSRef<JSArray>::Cast(args);
        auto param = params->GetValueAt(0);
        result = themeConstants->GetIntByName(param->ToString());
        return true;
    }
    result = themeConstants->GetInt(resId->ToNumber<uint32_t>());
    return true;
}

bool JSViewAbstract::ParseJsColor(const JSRef<JSVal>& jsValue, Color& result)
{
    if (!jsValue->IsNumber() && !jsValue->IsString() && !jsValue->IsObject()) {
        return false;
    }
    if (jsValue->IsNumber()) {
        result = Color(ColorAlphaAdapt(jsValue->ToNumber<uint32_t>()));
        return true;
    }
    if (jsValue->IsString()) {
        return Color::ParseColorString(jsValue->ToString(), result);
    }
    JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(jsValue);
    JSRef<JSVal> resId = jsObj->GetProperty("id");
    if (!resId->IsNumber()) {
        LOGW("resId is not number");
        return false;
    }

    auto themeConstants = GetThemeConstants(jsObj);
    if (!themeConstants) {
        LOGW("themeConstants is nullptr");
        return false;
    }
    auto resIdNum = resId->ToNumber<int32_t>();
    if (resIdNum == -1) {
        if (!IsGetResourceByName(jsObj)) {
            return false;
        }
        JSRef<JSVal> args = jsObj->GetProperty("params");
        JSRef<JSArray> params = JSRef<JSArray>::Cast(args);
        auto param = params->GetValueAt(0);
        result = themeConstants->GetColorByName(param->ToString());
        return true;
    }
    result = themeConstants->GetColor(resId->ToNumber<uint32_t>());
    return true;
}

bool JSViewAbstract::ParseJsFontFamilies(const JSRef<JSVal>& jsValue, std::vector<std::string>& result)
{
    result.clear();
    if (!jsValue->IsString() && !jsValue->IsObject()) {
        LOGE("arg is not String or Object.");
        return false;
    }
    if (jsValue->IsString()) {
        result = ConvertStrToFontFamilies(jsValue->ToString());
        return true;
    }
    JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(jsValue);
    JSRef<JSVal> resId = jsObj->GetProperty("id");
    if (!resId->IsNumber()) {
        LOGW("resId is not number");
        return false;
    }

    auto themeConstants = GetThemeConstants(jsObj);
    if (!themeConstants) {
        LOGW("themeConstants is nullptr");
        return false;
    }
    auto resIdNum = resId->ToNumber<int32_t>();
    if (resIdNum == -1) {
        if (!IsGetResourceByName(jsObj)) {
            return false;
        }
        JSRef<JSVal> args = jsObj->GetProperty("params");
        JSRef<JSArray> params = JSRef<JSArray>::Cast(args);
        auto param = params->GetValueAt(0);
        result.emplace_back(themeConstants->GetStringByName(param->ToString()));
        return true;
    }
    result.emplace_back(themeConstants->GetString(resId->ToNumber<uint32_t>()));
    return true;
}

bool JSViewAbstract::ParseJsString(const JSRef<JSVal>& jsValue, std::string& result)
{
    if (!jsValue->IsString() && !jsValue->IsObject()) {
        LOGE("arg is not String or Object.");
        return false;
    }

    if (jsValue->IsString()) {
        LOGD("jsValue->IsString()");
        result = jsValue->ToString();
        return true;
    }

    JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(jsValue);
    JSRef<JSVal> type = jsObj->GetProperty("type");
    if (!type->IsNumber()) {
        LOGW("type is not number");
        return false;
    }

    JSRef<JSVal> resId = jsObj->GetProperty("id");
    if (!resId->IsNumber()) {
        LOGW("resId is not number");
        return false;
    }

    auto themeConstants = GetThemeConstants(jsObj);
    if (!themeConstants) {
        LOGW("themeConstants is nullptr");
        return false;
    }

    JSRef<JSVal> args = jsObj->GetProperty("params");
    if (!args->IsArray()) {
        LOGW("args is not array");
        return false;
    }

    JSRef<JSArray> params = JSRef<JSArray>::Cast(args);
    auto resIdNum = resId->ToNumber<int32_t>();
    if (resIdNum == -1) {
        if (!IsGetResourceByName(jsObj)) {
            return false;
        }
        auto param = params->GetValueAt(0);
        if (type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::STRING)) {
            auto originStr = themeConstants->GetStringByName(param->ToString());
            ReplaceHolder(originStr, params, 0);
            result = originStr;
        } else if (type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::PLURAL)) {
            auto countJsVal = params->GetValueAt(1);
            int count = 0;
            if (!countJsVal->IsNumber()) {
                LOGW("pluralString, pluralnumber is not number");
                return false;
            }
            count = countJsVal->ToNumber<int>();
            auto pluralStr = themeConstants->GetPluralStringByName(param->ToString(), count);
            ReplaceHolder(pluralStr, params, 2);
            result = pluralStr;
        } else {
            return false;
        }
        return true;
    }
    if (type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::STRING)) {
        auto originStr = themeConstants->GetString(resId->ToNumber<uint32_t>());
        ReplaceHolder(originStr, params, 0);
        result = originStr;
    } else if (type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::PLURAL)) {
        auto countJsVal = params->GetValueAt(0);
        int count = 0;
        if (!countJsVal->IsNumber()) {
            LOGW("pluralString, pluralnumber is not number");
            return false;
        }
        count = countJsVal->ToNumber<int>();
        auto pluralStr = themeConstants->GetPluralString(resId->ToNumber<uint32_t>(), count);
        ReplaceHolder(pluralStr, params, 1);
        result = pluralStr;
    } else if (type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::FLOAT)) {
        result = std::to_string(themeConstants->GetDouble(resId->ToNumber<uint32_t>()));
    } else if (type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::INTEGER)) {
        result = std::to_string(themeConstants->GetInt(resId->ToNumber<uint32_t>()));
    } else {
        return false;
    }
    return true;
}

bool JSViewAbstract::ParseJsMedia(const JSRef<JSVal>& jsValue, std::string& result)
{
    if (!jsValue->IsObject() && !jsValue->IsString()) {
        LOGE("arg is not Object and String.");
        return false;
    }
    if (jsValue->IsString()) {
        result = jsValue->ToString();
        return true;
    }
    JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(jsValue);
    JSRef<JSVal> type = jsObj->GetProperty("type");
    JSRef<JSVal> resId = jsObj->GetProperty("id");
    if (!resId->IsNull() && !type->IsNull() && type->IsNumber() && resId->IsNumber()) {
        auto themeConstants = GetThemeConstants(jsObj);
        if (!themeConstants) {
            LOGW("themeConstants is nullptr");
            return false;
        }
        if (type->ToNumber<int32_t>() == static_cast<int32_t>(ResourceType::RAWFILE)) {
            JSRef<JSVal> args = jsObj->GetProperty("params");
            if (!args->IsArray()) {
                LOGW("args is not Array");
                return false;
            }
            JSRef<JSArray> params = JSRef<JSArray>::Cast(args);
            auto fileName = params->GetValueAt(0);
            if (!fileName->IsString()) {
                LOGW("fileName is not String");
                return false;
            }
            result = themeConstants->GetRawfile(fileName->ToString());
            return true;
        }
        auto resIdNum = resId->ToNumber<int32_t>();
        if (resIdNum == -1) {
            if (!IsGetResourceByName(jsObj)) {
                return false;
            }
            JSRef<JSVal> args = jsObj->GetProperty("params");
            JSRef<JSArray> params = JSRef<JSArray>::Cast(args);
            auto param = params->GetValueAt(0);
            if (type->ToNumber<int32_t>() == static_cast<int32_t>(ResourceType::MEDIA)) {
                result = themeConstants->GetMediaPathByName(param->ToString());
                return true;
            }
            LOGE("JSImage::Create ParseJsMedia type is wrong");
            return false;
        }
        if (type->ToNumber<int32_t>() == static_cast<int32_t>(ResourceType::MEDIA)) {
            result = themeConstants->GetMediaPath(resId->ToNumber<uint32_t>());
            return true;
        }
        LOGE("JSImage::Create ParseJsMedia type is wrong");
        return false;
    }
    LOGI("input value is not string or number, using PixelMap");
    return false;
}

bool JSViewAbstract::ParseJsBool(const JSRef<JSVal>& jsValue, bool& result)
{
    if (!jsValue->IsBoolean() && !jsValue->IsObject()) {
        LOGE("arg is not bool or Object.");
        return false;
    }

    if (jsValue->IsBoolean()) {
        LOGD("jsValue->IsBoolean()");
        result = jsValue->ToBoolean();
        return true;
    }

    JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(jsValue);
    JSRef<JSVal> type = jsObj->GetProperty("type");
    if (!type->IsNumber()) {
        LOGW("type is not number");
        return false;
    }

    JSRef<JSVal> resId = jsObj->GetProperty("id");
    if (!resId->IsNumber()) {
        LOGW("resId is not number");
        return false;
    }

    auto themeConstants = GetThemeConstants(jsObj);
    if (!themeConstants) {
        LOGW("themeConstants is nullptr");
        return false;
    }

    auto resIdNum = resId->ToNumber<int32_t>();
    if (resIdNum == -1) {
        if (!IsGetResourceByName(jsObj)) {
            return false;
        }
        JSRef<JSVal> args = jsObj->GetProperty("params");
        JSRef<JSArray> params = JSRef<JSArray>::Cast(args);
        auto param = params->GetValueAt(0);
        if (type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::BOOLEAN)) {
            result = themeConstants->GetBooleanByName(param->ToString());
            return true;
        }
        return false;
    }

    if (type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::BOOLEAN)) {
        result = themeConstants->GetBoolean(resId->ToNumber<uint32_t>());
        return true;
    }
    return false;
}

bool JSViewAbstract::ParseJsInteger(const JSRef<JSVal>& jsValue, uint32_t& result)
{
    return ParseJsInteger<uint32_t>(jsValue, result);
}

bool JSViewAbstract::ParseJsInteger(const JSRef<JSVal>& jsValue, int32_t& result)
{
    return ParseJsInteger<int32_t>(jsValue, result);
}

bool JSViewAbstract::ParseJsIntegerArray(const JSRef<JSVal>& jsValue, std::vector<uint32_t>& result)
{
    if (!jsValue->IsArray() && !jsValue->IsObject()) {
        LOGE("arg is not array or Object.");
        return false;
    }

    if (jsValue->IsArray()) {
        JSRef<JSArray> array = JSRef<JSArray>::Cast(jsValue);
        for (size_t i = 0; i < array->Length(); i++) {
            JSRef<JSVal> value = array->GetValueAt(i);
            if (value->IsNumber()) {
                result.emplace_back(value->ToNumber<uint32_t>());
            } else if (value->IsObject()) {
                uint32_t singleResInt;
                if (ParseJsInteger(value, singleResInt)) {
                    result.emplace_back(singleResInt);
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
        return true;
    }

    JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(jsValue);
    JSRef<JSVal> type = jsObj->GetProperty("type");
    if (!type->IsNumber()) {
        LOGW("type is not number");
        return false;
    }

    JSRef<JSVal> resId = jsObj->GetProperty("id");
    if (!resId->IsNumber()) {
        LOGW("resId is not number");
        return false;
    }

    auto themeConstants = GetThemeConstants(jsObj);
    if (!themeConstants) {
        LOGW("themeConstants is nullptr");
        return false;
    }

    auto resIdNum = resId->ToNumber<int32_t>();
    if (resIdNum == -1) {
        if (!IsGetResourceByName(jsObj)) {
            return false;
        }
        JSRef<JSVal> args = jsObj->GetProperty("params");
        JSRef<JSArray> params = JSRef<JSArray>::Cast(args);
        auto param = params->GetValueAt(0);
        if (type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::INTARRAY)) {
            result = themeConstants->GetIntArrayByName(param->ToString());
            return true;
        }
        return false;
    }

    if (type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::INTARRAY)) {
        result = themeConstants->GetIntArray(resId->ToNumber<uint32_t>());
        return true;
    }
    return false;
}

bool JSViewAbstract::ParseJsStrArray(const JSRef<JSVal>& jsValue, std::vector<std::string>& result)
{
    if (!jsValue->IsArray() && !jsValue->IsObject()) {
        LOGE("arg is not array or Object.");
        return false;
    }

    if (jsValue->IsArray()) {
        JSRef<JSArray> array = JSRef<JSArray>::Cast(jsValue);
        for (size_t i = 0; i < array->Length(); i++) {
            JSRef<JSVal> value = array->GetValueAt(i);
            if (value->IsString()) {
                result.emplace_back(value->ToString());
            } else if (value->IsObject()) {
                std::string singleResStr;
                if (ParseJsString(value, singleResStr)) {
                    result.emplace_back(singleResStr);
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
        return true;
    }

    JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(jsValue);
    JSRef<JSVal> type = jsObj->GetProperty("type");
    if (!type->IsNumber()) {
        LOGW("type is not number");
        return false;
    }

    JSRef<JSVal> resId = jsObj->GetProperty("id");
    if (!resId->IsNumber()) {
        LOGW("resId is not number");
        return false;
    }

    auto themeConstants = GetThemeConstants(jsObj);
    if (!themeConstants) {
        LOGW("themeConstants is nullptr");
        return false;
    }

    auto resIdNum = resId->ToNumber<int32_t>();
    if (resIdNum == -1) {
        if (!IsGetResourceByName(jsObj)) {
            return false;
        }
        JSRef<JSVal> args = jsObj->GetProperty("params");
        JSRef<JSArray> params = JSRef<JSArray>::Cast(args);
        auto param = params->GetValueAt(0);
        if (type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::STRARRAY)) {
            result = themeConstants->GetStringArrayByName(param->ToString());
            return true;
        }
        return false;
    }

    if (type->ToNumber<uint32_t>() == static_cast<uint32_t>(ResourceType::STRARRAY)) {
        result = themeConstants->GetStringArray(resId->ToNumber<uint32_t>());
        return true;
    }
    return false;
}

bool JSViewAbstract::IsGetResourceByName(const JSRef<JSObject>& jsObj)
{
    JSRef<JSVal> args = jsObj->GetProperty("params");
    if (!args->IsArray()) {
        LOGW("args is not array");
        return false;
    }
    JSRef<JSVal> bundleName = jsObj->GetProperty("bundleName");
    JSRef<JSVal> moduleName = jsObj->GetProperty("moduleName");
    if (!bundleName->IsString() || !moduleName->IsString()) {
        LOGW("bundleName or moduleName is not string");
        return false;
    }
    if (!bundleName->ToString().empty() || !moduleName->ToString().empty()) {
        LOGW("bundleName or moduleName is not empty");
        return false;
    }
    JSRef<JSArray> params = JSRef<JSArray>::Cast(args);
    if (params->IsEmpty()) {
        LOGW("params is empty");
        return false;
    }
    return true;
}

std::pair<Dimension, Dimension> JSViewAbstract::ParseSize(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("ParseSize", info, checkList)) {
        return std::pair<Dimension, Dimension>();
    }
    auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
    if (!argsPtrItem || argsPtrItem->IsNull()) {
        LOGE("Js Parse object failed. argsPtr is null. %s", info[0]->ToString().c_str());
        info.SetReturnValue(info.This());
        return std::pair<Dimension, Dimension>();
    }
    Dimension width;
    Dimension height;
    if (!ParseJsonDimensionVp(argsPtrItem->GetValue("width"), width) ||
        !ParseJsonDimensionVp(argsPtrItem->GetValue("height"), height)) {
        return std::pair<Dimension, Dimension>();
    }
    LOGD("JsSize width = %lf unit = %d, height = %lf unit = %d", width.Value(), width.Unit(), height.Value(),
        height.Unit());
    info.SetReturnValue(info.This());
    return std::pair<Dimension, Dimension>(width, height);
}

void JSViewAbstract::JsUseAlign(const JSCallbackInfo& info)
{
    if (info.Length() < 2) {
        LOGE("The arg is wrong, it is supposed to have atleast 2 arguments");
        return;
    }

    if (!info[0]->IsObject() && !info[1]->IsObject()) {
        LOGE("arg is not IsObject.");
        return;
    }

    AlignDeclaration* declaration = JSRef<JSObject>::Cast(info[0])->Unwrap<AlignDeclaration>();
    if (declaration == nullptr) {
        LOGE("declaration is nullptr");
        return;
    }

    JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[1]);
    JSRef<JSVal> side = obj->GetProperty("side");
    JSRef<JSVal> offset = obj->GetProperty("offset");

    if (!side->IsNumber()) {
        LOGE("side is not Number [%s]", side->ToString().c_str());
        return;
    }

    auto sideValue = side->ToNumber<int32_t>();

    if (declaration->GetDeclarationType() == AlignDeclaration::DeclarationType::HORIZONTAL) {
        if (sideValue < static_cast<int32_t>(AlignDeclaration::Edge::START) ||
            sideValue > static_cast<int32_t>(AlignDeclaration::Edge::END)) {
            LOGE("side should be Edge.Start Edge.Middle or Edge.End with HorizontalAlignDeclaration");
            return;
        }
    } else if (declaration->GetDeclarationType() == AlignDeclaration::DeclarationType::VERTICAL) {
        if (sideValue < static_cast<int32_t>(AlignDeclaration::Edge::TOP) ||
            sideValue > static_cast<int32_t>(AlignDeclaration::Edge::BASELINE)) {
            LOGE("side should be Edge.Top Edge.Center Edge.Bottom or Edge.Baseline with VerticalAlignDeclaration");
            return;
        }
    }

    std::optional<Dimension> optOffset;
    Dimension offsetDimension;
    if (ParseJsDimensionVp(offset, offsetDimension)) {
        optOffset = offsetDimension;
    }
    ViewAbstractModel::GetInstance()->SetUseAlign(
        declaration, static_cast<AlignDeclaration::Edge>(sideValue), optOffset);
}

void JSViewAbstract::JsGridSpan(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::NUMBER };
    if (!CheckJSCallbackInfo("JsGridSpan", info, checkList)) {
        return;
    }
    auto span = info[0]->ToNumber<int32_t>();
    ViewAbstractModel::GetInstance()->SetGrid(span, std::nullopt);
}

void JSViewAbstract::JsGridOffset(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::NUMBER };
    if (!CheckJSCallbackInfo("JsGridOffset", info, checkList)) {
        return;
    }
    auto offset = info[0]->ToNumber<int32_t>();
    ViewAbstractModel::GetInstance()->SetGrid(std::nullopt, offset);
}

static bool ParseSpanAndOffset(const JSRef<JSVal>& val, uint32_t& span, int32_t& offset)
{
    // {lg: 4}
    if (val->IsNumber()) {
        span = val->ToNumber<uint32_t>();
        return true;
    }

    if (!val->IsObject()) {
        LOGE("The argument is not object or number.");
        return false;
    }

    // {lg: {span: 1, offset: 2}}
    JSRef<JSObject> obj = JSRef<JSObject>::Cast(val);
    span = obj->GetProperty("span")->ToNumber<uint32_t>();
    offset = obj->GetProperty("offset")->ToNumber<int32_t>();
    return true;
}

void JSViewAbstract::JsUseSizeType(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsUseSizeType", info, checkList)) {
        return;
    }
    JSRef<JSObject> sizeObj = JSRef<JSObject>::Cast(info[0]);
    // keys order must be strictly refer to GridSizeType
    const char* keys[] = { "", "xs", "sm", "md", "lg" };

    for (uint32_t i = 1; i < sizeof(keys) / sizeof(const char*); i++) {
        JSRef<JSVal> val = sizeObj->GetProperty(keys[i]);
        if (val->IsNull() || val->IsEmpty()) {
            continue;
        }
        uint32_t span = 0;
        int32_t offset = 0;
        if (ParseSpanAndOffset(val, span, offset)) {
            ViewAbstractModel::GetInstance()->SetGrid(span, offset, static_cast<GridSizeType>(i));
        }
    }
}

void JSViewAbstract::JsZIndex(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    int zIndex = 0;
    if (info[0]->IsNumber()) {
        zIndex = info[0]->ToNumber<int>();
    }

    ViewAbstractModel::GetInstance()->SetZIndex(zIndex);
}

void JSViewAbstract::Pop()
{
    ViewStackModel::GetInstance()->Pop();
}

void JSViewAbstract::JsOnDragStart(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::FUNCTION };
    if (!CheckJSCallbackInfo("JsOnDragStart", info, checkList)) {
        return;
    }

    RefPtr<JsDragFunction> jsOnDragStartFunc = AceType::MakeRefPtr<JsDragFunction>(JSRef<JSFunc>::Cast(info[0]));

    auto onDragStart = [execCtx = info.GetExecutionContext(), func = std::move(jsOnDragStartFunc)](
                           const RefPtr<DragEvent>& info, const std::string& extraParams) -> NG::DragDropBaseInfo {
        NG::DragDropBaseInfo dragDropInfo;
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx, dragDropInfo);

        auto ret = func->Execute(info, extraParams);
        if (!ret->IsObject()) {
            LOGE("NG: builder param is not an object.");
            return dragDropInfo;
        }

        auto node = ParseDragNode(ret);
        if (node) {
            LOGI("use custom builder param.");
            dragDropInfo.node = node;
            return dragDropInfo;
        }

        auto builderObj = JSRef<JSObject>::Cast(ret);
#if defined(PIXEL_MAP_SUPPORTED)
        auto pixmap = builderObj->GetProperty("pixelMap");
        dragDropInfo.pixelMap = CreatePixelMapFromNapiValue(pixmap);
#endif
        auto extraInfo = builderObj->GetProperty("extraInfo");
        ParseJsString(extraInfo, dragDropInfo.extraInfo);
        node = ParseDragNode(builderObj->GetProperty("builder"));
        dragDropInfo.node = node;
        return dragDropInfo;
    };
    ViewAbstractModel::GetInstance()->SetOnDragStart(std::move(onDragStart));
}

bool JSViewAbstract::ParseAndUpdateDragItemInfo(const JSRef<JSVal>& info, NG::DragDropBaseInfo& dragInfo)
{
    auto node = ParseDragNode(info);
    if (!node) {
        return false;
    }
    dragInfo.node = node;
    return true;
}

RefPtr<AceType> JSViewAbstract::ParseDragNode(const JSRef<JSVal>& info)
{
    auto builderFunc = ParseDragStartBuilderFunc(info);
    if (!builderFunc) {
        return nullptr;
    }
    // use another VSP instance while executing the builder function
    ViewStackModel::GetInstance()->NewScope();
    {
        ACE_SCORING_EVENT("onDragStart.builder");
        builderFunc->Execute();
    }

    return ViewStackModel::GetInstance()->Finish();
}

void JSViewAbstract::JsOnDragEnter(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::FUNCTION };
    if (!CheckJSCallbackInfo("JsOnDragEnter", info, checkList)) {
        return;
    }
    RefPtr<JsDragFunction> jsOnDragEnterFunc = AceType::MakeRefPtr<JsDragFunction>(JSRef<JSFunc>::Cast(info[0]));

    auto onDragEnter = [execCtx = info.GetExecutionContext(), func = std::move(jsOnDragEnterFunc)](
                           const RefPtr<OHOS::Ace::DragEvent>& info, const std::string& extraParams) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onDragEnter");
        func->Execute(info, extraParams);
    };

    ViewAbstractModel::GetInstance()->SetOnDragEnter(std::move(onDragEnter));
}

void JSViewAbstract::JsOnDragMove(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::FUNCTION };
    if (!CheckJSCallbackInfo("JsOnDragMove", info, checkList)) {
        return;
    }
    RefPtr<JsDragFunction> jsOnDragMoveFunc = AceType::MakeRefPtr<JsDragFunction>(JSRef<JSFunc>::Cast(info[0]));

    auto onDragMove = [execCtx = info.GetExecutionContext(), func = std::move(jsOnDragMoveFunc)](
                          const RefPtr<OHOS::Ace::DragEvent>& info, const std::string& extraParams) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onDragMove");
        func->Execute(info, extraParams);
    };

    ViewAbstractModel::GetInstance()->SetOnDragMove(std::move(onDragMove));
}

void JSViewAbstract::JsOnDragLeave(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::FUNCTION };
    if (!CheckJSCallbackInfo("JsOnDragLeave", info, checkList)) {
        return;
    }
    RefPtr<JsDragFunction> jsOnDragLeaveFunc = AceType::MakeRefPtr<JsDragFunction>(JSRef<JSFunc>::Cast(info[0]));

    auto onDragLeave = [execCtx = info.GetExecutionContext(), func = std::move(jsOnDragLeaveFunc)](
                           const RefPtr<OHOS::Ace::DragEvent>& info, const std::string& extraParams) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onDragLeave");
        func->Execute(info, extraParams);
    };

    ViewAbstractModel::GetInstance()->SetOnDragLeave(std::move(onDragLeave));
}

void JSViewAbstract::JsOnDrop(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::FUNCTION };
    if (!CheckJSCallbackInfo("JsOnDrop", info, checkList)) {
        return;
    }
    RefPtr<JsDragFunction> jsOnDropFunc = AceType::MakeRefPtr<JsDragFunction>(JSRef<JSFunc>::Cast(info[0]));

    auto onDrop = [execCtx = info.GetExecutionContext(), func = std::move(jsOnDropFunc)](
                      const RefPtr<OHOS::Ace::DragEvent>& info, const std::string& extraParams) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onDrop");
        func->Execute(info, extraParams);
    };

    ViewAbstractModel::GetInstance()->SetOnDrop(std::move(onDrop));
}

void JSViewAbstract::JsOnAreaChange(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::FUNCTION };
    if (!CheckJSCallbackInfo("JsOnAreaChange", info, checkList)) {
        return;
    }
    auto jsOnAreaChangeFunction = AceType::MakeRefPtr<JsOnAreaChangeFunction>(JSRef<JSFunc>::Cast(info[0]));

    auto onAreaChanged = [execCtx = info.GetExecutionContext(), func = std::move(jsOnAreaChangeFunction)](
                             const Rect& oldRect, const Offset& oldOrigin, const Rect& rect, const Offset& origin) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onAreaChange");
        func->Execute(oldRect, oldOrigin, rect, origin);
    };
    ViewAbstractModel::GetInstance()->SetOnAreaChanged(std::move(onAreaChanged));
}

#ifndef WEARABLE_PRODUCT
void JSViewAbstract::JsBindPopup(const JSCallbackInfo& info)
{
    if (info.Length() < 2) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }

    if (!info[0]->IsBoolean() && !info[0]->IsObject()) {
        LOGE("The first param type is not bool or object, invalid.");
        return;
    }

    if (!info[1]->IsObject()) {
        LOGE("The second param type is not object, invalid.");
        return;
    }

    auto popupParam = AceType::MakeRefPtr<PopupParam>();
    // Set IsShow to popupParam
    if (info[0]->IsBoolean()) {
        popupParam->SetIsShow(info[0]->ToBoolean());
    } else {
        JSRef<JSObject> showObj = JSRef<JSObject>::Cast(info[0]);
        ParseShowObject(info, showObj, popupParam);
        popupParam->SetIsShow(showObj->GetProperty("value")->ToBoolean());
    }

    // Set popup to popupParam
    auto popupObj = JSRef<JSObject>::Cast(info[1]);

    if (popupObj->GetProperty("message")->IsString()) {
        ParsePopupParam(info, popupObj, popupParam); // Parse PopupOptions param
        ViewAbstractModel::GetInstance()->BindPopup(popupParam, nullptr);
    } else if (!popupObj->GetProperty("builder").IsEmpty()) {
        ParseCustomPopupParam(info, popupObj, popupParam); // Parse CustomPopupOptions param
        auto builderValue = popupObj->GetProperty("builder");
        if (!builderValue->IsObject()) {
            LOGE("builder param is not an object.");
            return;
        }

        JSRef<JSObject> builderObj;
        builderObj = JSRef<JSObject>::Cast(builderValue);
        auto builder = builderObj->GetProperty("builder");
        if (!builder->IsFunction()) {
            LOGE("builder param is not a function.");
            return;
        }
        auto builderFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSFunc>::Cast(builder));
        CHECK_NULL_VOID(builderFunc);
        ViewStackModel::GetInstance()->NewScope();
        builderFunc->Execute();
        auto customNode = ViewStackModel::GetInstance()->Finish();
        ViewAbstractModel::GetInstance()->BindPopup(popupParam, customNode);
    } else {
        LOGE("BindPop info is invalid");
        return;
    }
}
#endif

void JSViewAbstract::JsLinearGradient(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
    if (!info[0]->IsObject()) {
        LOGE("arg is not a object.");
        return;
    }
    auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
    if (!argsPtrItem || argsPtrItem->IsNull()) {
        LOGE("Js Parse object failed. argsPtr is null. %s", info[0]->ToString().c_str());
        info.ReturnSelf();
        return;
    }
    NG::Gradient newGradient;
    NewJsLinearGradient(info, newGradient);
    ViewAbstractModel::GetInstance()->SetLinearGradient(newGradient);
}

void JSViewAbstract::NewJsLinearGradient(const JSCallbackInfo& info, NG::Gradient& newGradient)
{
    auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
    newGradient.CreateGradientWithType(NG::GradientType::LINEAR);
    // angle
    std::optional<float> degree;
    GetAngle("angle", argsPtrItem, degree);
    if (degree) {
        newGradient.GetLinearGradient()->angle = Dimension(degree.value(), DimensionUnit::PX);
        degree.reset();
    }
    // direction
    auto direction =
        static_cast<GradientDirection>(argsPtrItem->GetInt("direction", static_cast<int32_t>(GradientDirection::NONE)));
    switch (direction) {
        case GradientDirection::LEFT:
            newGradient.GetLinearGradient()->linearX = NG::GradientDirection::LEFT;
            break;
        case GradientDirection::RIGHT:
            newGradient.GetLinearGradient()->linearX = NG::GradientDirection::RIGHT;
            break;
        case GradientDirection::TOP:
            newGradient.GetLinearGradient()->linearY = NG::GradientDirection::TOP;
            break;
        case GradientDirection::BOTTOM:
            newGradient.GetLinearGradient()->linearY = NG::GradientDirection::BOTTOM;
            break;
        case GradientDirection::LEFT_TOP:
            newGradient.GetLinearGradient()->linearX = NG::GradientDirection::LEFT;
            newGradient.GetLinearGradient()->linearY = NG::GradientDirection::TOP;
            break;
        case GradientDirection::LEFT_BOTTOM:
            newGradient.GetLinearGradient()->linearX = NG::GradientDirection::LEFT;
            newGradient.GetLinearGradient()->linearY = NG::GradientDirection::BOTTOM;
            break;
        case GradientDirection::RIGHT_TOP:
            newGradient.GetLinearGradient()->linearX = NG::GradientDirection::RIGHT;
            newGradient.GetLinearGradient()->linearY = NG::GradientDirection::TOP;
            break;
        case GradientDirection::RIGHT_BOTTOM:
            newGradient.GetLinearGradient()->linearX = NG::GradientDirection::RIGHT;
            newGradient.GetLinearGradient()->linearY = NG::GradientDirection::BOTTOM;
            break;
        case GradientDirection::NONE:
        case GradientDirection::START_TO_END:
        case GradientDirection::END_TO_START:
        default:
            break;
    }
    auto repeating = argsPtrItem->GetBool("repeating", false);
    newGradient.SetRepeat(repeating);
    NewGetGradientColorStops(newGradient, argsPtrItem->GetValue("colors"));
}

void JSViewAbstract::JsRadialGradient(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsRadialGradient", info, checkList)) {
        return;
    }

    auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
    if (!argsPtrItem || argsPtrItem->IsNull()) {
        LOGE("Js Parse object failed. argsPtr is null. %s", info[0]->ToString().c_str());
        info.ReturnSelf();
        return;
    }
    NG::Gradient newGradient;
    NewJsRadialGradient(info, newGradient);
    ViewAbstractModel::GetInstance()->SetRadialGradient(newGradient);
}

void JSViewAbstract::NewJsRadialGradient(const JSCallbackInfo& info, NG::Gradient& newGradient)
{
    auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
    newGradient.CreateGradientWithType(NG::GradientType::RADIAL);
    // center
    auto center = argsPtrItem->GetValue("center");
    if (center && !center->IsNull() && center->IsArray() && center->GetArraySize() == 2) {
        Dimension value;
        if (ParseJsonDimensionVp(center->GetArrayItem(0), value)) {
            newGradient.GetRadialGradient()->radialCenterX = Dimension(value);
            if (value.Unit() == DimensionUnit::PERCENT) {
                // [0,1] -> [0, 100]
                newGradient.GetRadialGradient()->radialCenterX =
                    Dimension(value.Value() * 100.0, DimensionUnit::PERCENT);
            }
        }
        if (ParseJsonDimensionVp(center->GetArrayItem(1), value)) {
            newGradient.GetRadialGradient()->radialCenterY = Dimension(value);
            if (value.Unit() == DimensionUnit::PERCENT) {
                // [0,1] -> [0, 100]
                newGradient.GetRadialGradient()->radialCenterY =
                    Dimension(value.Value() * 100.0, DimensionUnit::PERCENT);
            }
        }
    }
    // radius
    Dimension radius;
    if (ParseJsonDimensionVp(argsPtrItem->GetValue("radius"), radius)) {
        newGradient.GetRadialGradient()->radialVerticalSize = Dimension(radius);
        newGradient.GetRadialGradient()->radialHorizontalSize = Dimension(radius);
    }
    // repeating
    auto repeating = argsPtrItem->GetBool("repeating", false);
    newGradient.SetRepeat(repeating);
    // color stops
    NewGetGradientColorStops(newGradient, argsPtrItem->GetValue("colors"));
}

void JSViewAbstract::JsSweepGradient(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsSweepGradient", info, checkList)) {
        return;
    }

    auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
    if (!argsPtrItem || argsPtrItem->IsNull()) {
        LOGE("Js Parse object failed. argsPtr is null. %s", info[0]->ToString().c_str());
        info.ReturnSelf();
        return;
    }

    NG::Gradient newGradient;
    NewJsSweepGradient(info, newGradient);
    ViewAbstractModel::GetInstance()->SetSweepGradient(newGradient);
}

void JSViewAbstract::NewJsSweepGradient(const JSCallbackInfo& info, NG::Gradient& newGradient)
{
    auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
    newGradient.CreateGradientWithType(NG::GradientType::SWEEP);
    // center
    auto center = argsPtrItem->GetValue("center");
    if (center && !center->IsNull() && center->IsArray() && center->GetArraySize() == 2) {
        Dimension value;
        if (ParseJsonDimensionVp(center->GetArrayItem(0), value)) {
            newGradient.GetSweepGradient()->centerX = Dimension(value);
            if (value.Unit() == DimensionUnit::PERCENT) {
                // [0,1] -> [0, 100]
                newGradient.GetSweepGradient()->centerX = Dimension(value.Value() * 100.0, DimensionUnit::PERCENT);
            }
        }
        if (ParseJsonDimensionVp(center->GetArrayItem(1), value)) {
            newGradient.GetSweepGradient()->centerY = Dimension(value);
            if (value.Unit() == DimensionUnit::PERCENT) {
                // [0,1] -> [0, 100]
                newGradient.GetSweepGradient()->centerY = Dimension(value.Value() * 100.0, DimensionUnit::PERCENT);
            }
        }
    }
    std::optional<float> degree;
    // start
    GetAngle("start", argsPtrItem, degree);
    if (degree) {
        newGradient.GetSweepGradient()->startAngle = Dimension(degree.value(), DimensionUnit::PX);
        degree.reset();
    }
    // end
    GetAngle("end", argsPtrItem, degree);
    if (degree) {
        newGradient.GetSweepGradient()->endAngle = Dimension(degree.value(), DimensionUnit::PX);
        degree.reset();
    }
    // rotation
    GetAngle("rotation", argsPtrItem, degree);
    if (degree) {
        newGradient.GetSweepGradient()->rotation = Dimension(degree.value(), DimensionUnit::PX);
        degree.reset();
    }
    // repeating
    auto repeating = argsPtrItem->GetBool("repeating", false);
    newGradient.SetRepeat(repeating);
    // color stops
    NewGetGradientColorStops(newGradient, argsPtrItem->GetValue("colors"));
}

void JSViewAbstract::JsMotionPath(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsMotionPath", info, checkList)) {
        return;
    }
    auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
    MotionPathOption motionPathOption;
    if (ParseMotionPath(argsPtrItem, motionPathOption)) {
        ViewAbstractModel::GetInstance()->SetMotionPath(motionPathOption);
    } else {
        LOGE("parse motionPath failed. %{public}s", info[0]->ToString().c_str());
    }
}

void JSViewAbstract::JsShadow(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::OBJECT };
    if (!CheckJSCallbackInfo("JsShadow", info, checkList)) {
        return;
    }
    auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
    if (!argsPtrItem || argsPtrItem->IsNull()) {
        LOGE("Js Parse object failed. argsPtr is null. %s", info[0]->ToString().c_str());
        info.ReturnSelf();
        return;
    }
    double radius = 0.0;
    ParseJsonDouble(argsPtrItem->GetValue("radius"), radius);
    if (LessNotEqual(radius, 0.0)) {
        radius = 0.0;
    }
    std::vector<Shadow> shadows(1);
    shadows.begin()->SetBlurRadius(radius);
    Dimension offsetX;
    if (ParseJsonDimensionVp(argsPtrItem->GetValue("offsetX"), offsetX)) {
        shadows.begin()->SetOffsetX(offsetX.Value());
    }
    Dimension offsetY;
    if (ParseJsonDimensionVp(argsPtrItem->GetValue("offsetY"), offsetY)) {
        shadows.begin()->SetOffsetY(offsetY.Value());
    }
    Color color;
    if (ParseJsonColor(argsPtrItem->GetValue("color"), color)) {
        shadows.begin()->SetColor(color);
    }
    ViewAbstractModel::GetInstance()->SetBackShadow(shadows);
}

void JSViewAbstract::JsGrayScale(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }

    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }

    if (GreatNotEqual(value.Value(), 1.0)) {
        value.SetValue(1.0);
    }

    ViewAbstractModel::GetInstance()->SetGrayScale(value);
}

void JSViewAbstract::JsBrightness(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }

    ViewAbstractModel::GetInstance()->SetBrightness(value);
}

void JSViewAbstract::JsContrast(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }

    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }

    ViewAbstractModel::GetInstance()->SetContrast(value);
}

void JSViewAbstract::JsSaturate(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }

    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }

    ViewAbstractModel::GetInstance()->SetSaturate(value);
}

void JSViewAbstract::JsSepia(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }

    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }

    ViewAbstractModel::GetInstance()->SetSepia(value);
}

void JSViewAbstract::JsInvert(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    if (LessNotEqual(value.Value(), 0.0)) {
        value.SetValue(0.0);
    }

    ViewAbstractModel::GetInstance()->SetInvert(value);
}

void JSViewAbstract::JsHueRotate(const JSCallbackInfo& info)
{
    std::optional<float> degree;
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 arguments");
        return;
    }
    if (info[0]->IsString()) {
        degree = static_cast<float>(StringUtils::StringToDegree(info[0]->ToString()));
    } else if (info[0]->IsNumber()) {
        degree = static_cast<float>(info[0]->ToNumber<int32_t>());
    } else {
        LOGE("Invalid value type");
    }
    float deg = 0.0f;
    if (degree) {
        deg = degree.value();
        degree.reset();
    }
    deg = std::fmod(deg, ROUND_UNIT);
    if (deg < 0.0f) {
        deg += ROUND_UNIT;
    }
    ViewAbstractModel::GetInstance()->SetHueRotate(deg);
}

void JSViewAbstract::JsClip(const JSCallbackInfo& info)
{
    if (info[0]->IsObject()) {
        JSShapeAbstract* clipShape = JSRef<JSObject>::Cast(info[0])->Unwrap<JSShapeAbstract>();
        if (clipShape == nullptr) {
            LOGE("clipShape is null");
            return;
        }
        ViewAbstractModel::GetInstance()->SetClipShape(clipShape->GetBasicShape());
    } else if (info[0]->IsBoolean()) {
        ViewAbstractModel::GetInstance()->SetClipEdge(info[0]->ToBoolean());
    }
}

void JSViewAbstract::JsMask(const JSCallbackInfo& info)
{
    if (info.Length() > 0 && info[0]->IsObject()) {
        JSShapeAbstract* maskShape = JSRef<JSObject>::Cast(info[0])->Unwrap<JSShapeAbstract>();
        if (maskShape == nullptr) {
            return;
        };
        ViewAbstractModel::GetInstance()->SetMask(maskShape->GetBasicShape());
    }
}

void JSViewAbstract::JsFocusable(const JSCallbackInfo& info)
{
    if (!info[0]->IsBoolean()) {
        LOGE("The info is wrong, it is supposed to be an boolean");
        return;
    }
    ViewAbstractModel::GetInstance()->SetFocusable(info[0]->ToBoolean());
}

void JSViewAbstract::JsOnFocusMove(const JSCallbackInfo& args)
{
    if (args[0]->IsFunction()) {
        RefPtr<JsFocusFunction> jsOnFocusMove = AceType::MakeRefPtr<JsFocusFunction>(JSRef<JSFunc>::Cast(args[0]));
        auto onFocusMove = [execCtx = args.GetExecutionContext(), func = std::move(jsOnFocusMove)](int info) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("onFocusMove");
            func->Execute(info);
        };
        ViewAbstractModel::GetInstance()->SetOnFocusMove(std::move(onFocusMove));
    }
}

void JSViewAbstract::JsOnKeyEvent(const JSCallbackInfo& args)
{
    if (!args[0]->IsFunction()) {
        LOGE("OnKeyEvent args need a function.");
        return;
    }
    RefPtr<JsKeyFunction> JsOnKeyEvent = AceType::MakeRefPtr<JsKeyFunction>(JSRef<JSFunc>::Cast(args[0]));
    auto onKeyEvent = [execCtx = args.GetExecutionContext(), func = std::move(JsOnKeyEvent)](KeyEventInfo& info) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onKey");
        func->Execute(info);
    };
    ViewAbstractModel::GetInstance()->SetOnKeyEvent(std::move(onKeyEvent));
}

void JSViewAbstract::JsOnFocus(const JSCallbackInfo& args)
{
    if (!args[0]->IsFunction()) {
        LOGE("OnFocus args need a function.");
        return;
    }
    RefPtr<JsFocusFunction> jsOnFocus = AceType::MakeRefPtr<JsFocusFunction>(JSRef<JSFunc>::Cast(args[0]));
    auto onFocus = [execCtx = args.GetExecutionContext(), func = std::move(jsOnFocus)]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onFocus");
        func->Execute();
    };

    ViewAbstractModel::GetInstance()->SetOnFocus(std::move(onFocus));
}

void JSViewAbstract::JsOnBlur(const JSCallbackInfo& args)
{
    if (!args[0]->IsFunction()) {
        LOGE("OnBlur args need a function.");
        return;
    }
    RefPtr<JsFocusFunction> jsOnBlur = AceType::MakeRefPtr<JsFocusFunction>(JSRef<JSFunc>::Cast(args[0]));
    auto onBlur = [execCtx = args.GetExecutionContext(), func = std::move(jsOnBlur)]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onBlur");
        func->Execute();
    };

    ViewAbstractModel::GetInstance()->SetOnBlur(std::move(onBlur));
}

void JSViewAbstract::JsTabIndex(const JSCallbackInfo& info)
{
    if (!info[0]->IsNumber()) {
        LOGE("Param is wrong, it is supposed to be a number");
        return;
    }
    ViewAbstractModel::GetInstance()->SetTabIndex(info[0]->ToNumber<int32_t>());
}

void JSViewAbstract::JsFocusOnTouch(const JSCallbackInfo& info)
{
    if (!info[0]->IsBoolean()) {
        LOGE("Param is wrong, it is supposed to be a boolean");
        return;
    }
    auto isFocusOnTouch = info[0]->ToBoolean();
    ViewAbstractModel::GetInstance()->SetFocusOnTouch(isFocusOnTouch);
}

void JSViewAbstract::JsDefaultFocus(const JSCallbackInfo& info)
{
    if (!info[0]->IsBoolean()) {
        LOGE("Param is wrong, it is supposed to be a boolean");
        return;
    }
    auto isDefaultFocus = info[0]->ToBoolean();
    ViewAbstractModel::GetInstance()->SetDefaultFocus(isDefaultFocus);
}

void JSViewAbstract::JsGroupDefaultFocus(const JSCallbackInfo& info)
{
    if (!info[0]->IsBoolean()) {
        LOGE("Param is wrong, it is supposed to be a boolean");
        return;
    }
    auto isGroupDefaultFocus = info[0]->ToBoolean();
    ViewAbstractModel::GetInstance()->SetGroupDefaultFocus(isGroupDefaultFocus);
}

void JSViewAbstract::JsKey(const std::string& key)
{
    ViewAbstractModel::GetInstance()->SetInspectorId(key);
}

void JSViewAbstract::JsId(const std::string& id)
{
    JsKey(id);
}

void JSViewAbstract::JsRestoreId(int32_t restoreId)
{
    ViewAbstractModel::GetInstance()->SetRestoreId(restoreId);
}

#if defined(PREVIEW)
void JSViewAbstract::JsDebugLine(const JSCallbackInfo& info)
{
    std::vector<JSCallbackInfoType> checkList { JSCallbackInfoType::STRING };
    if (!CheckJSCallbackInfo("JsDebugLine", info, checkList)) {
        return;
    }

    ViewAbstractModel::GetInstance()->SetDebugLine(info[0]->ToString());
}
#endif

void JSViewAbstract::JsOpacityPassThrough(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    double opacity = 0.0;
    if (!ParseJsDouble(info[0], opacity)) {
        return;
    }

    if ((LessNotEqual(opacity, 0.0)) || opacity > 1) {
        LOGW("set opacity to %{public}f, over range, set to default opacity", opacity);
        opacity = 1.0;
    }

    ViewAbstractModel::GetInstance()->SetOpacity(opacity, true);
}

void JSViewAbstract::JsTransitionPassThrough(const JSCallbackInfo& info)
{
    if (info.Length() > 1) {
        LOGE("Too many arguments");
        return;
    }
    if (info.Length() == 0) {
        ViewAbstractModel::GetInstance()->SetTransition(
            NG::TransitionOptions::GetDefaultTransition(TransitionType::ALL));
        return;
    }
    if (!info[0]->IsObject()) {
        LOGE("arg is not Object.");
        return;
    }
    auto transitionArgs = JsonUtil::ParseJsonString(info[0]->ToString());
    auto options = ParseTransition(transitionArgs);
    ViewAbstractModel::GetInstance()->SetTransition(options, true);
}

void JSViewAbstract::JsAccessibilityGroup(bool accessible)
{
    ViewAbstractModel::GetInstance()->SetAccessibilityGroup(accessible);
}

void JSViewAbstract::JsAccessibilityText(const std::string& text)
{
    ViewAbstractModel::GetInstance()->SetAccessibilityText(text);
}

void JSViewAbstract::JsAccessibilityDescription(const std::string& description)
{
    ViewAbstractModel::GetInstance()->SetAccessibilityDescription(description);
}

void JSViewAbstract::JsAccessibilityImportance(const std::string& importance)
{
    ViewAbstractModel::GetInstance()->SetAccessibilityImportance(importance);
}

void JSViewAbstract::JsBindContextMenu(const JSCallbackInfo& info)
{
    // Check the parameters
    if (info.Length() <= 0 || !info[0]->IsObject()) {
        LOGE("Builder param is invalid, not an object.");
        return;
    }
    JSRef<JSObject> menuObj = JSRef<JSObject>::Cast(info[0]);
    auto builder = menuObj->GetProperty("builder");
    if (!builder->IsFunction()) {
        LOGE("builder param is not a function.");
        return;
    }
    auto builderFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSFunc>::Cast(builder));
    CHECK_NULL_VOID(builderFunc);

    ResponseType responseType = ResponseType::LONG_PRESS;
    if (info.Length() == 2 && info[1]->IsNumber()) {
        auto response = info[1]->ToNumber<int32_t>();
        LOGI("Set the responseType is %{public}d.", response);
        responseType = static_cast<ResponseType>(response);
    }
    auto buildFunc = [execCtx = info.GetExecutionContext(), func = std::move(builderFunc)]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("BuildContextMenu");
        func->Execute();
    };
    ViewAbstractModel::GetInstance()->BindContextMenu(responseType, std::move(buildFunc));
}

void JSViewAbstract::JSBind()
{
    JSClass<JSViewAbstract>::Declare("JSViewAbstract");

    // static methods
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSViewAbstract>::StaticMethod("pop", &JSViewAbstract::Pop, opt);

    JSClass<JSViewAbstract>::StaticMethod("width", &JSViewAbstract::JsWidth);
    JSClass<JSViewAbstract>::StaticMethod("height", &JSViewAbstract::JsHeight);
    JSClass<JSViewAbstract>::StaticMethod("responseRegion", &JSViewAbstract::JsResponseRegion);
    JSClass<JSViewAbstract>::StaticMethod("size", &JSViewAbstract::JsSize);
    JSClass<JSViewAbstract>::StaticMethod("constraintSize", &JSViewAbstract::JsConstraintSize);
    JSClass<JSViewAbstract>::StaticMethod("layoutPriority", &JSViewAbstract::JsLayoutPriority);
    JSClass<JSViewAbstract>::StaticMethod("layoutWeight", &JSViewAbstract::JsLayoutWeight);

    JSClass<JSViewAbstract>::StaticMethod("margin", &JSViewAbstract::JsMargin);
    JSClass<JSViewAbstract>::StaticMethod("marginTop", &JSViewAbstract::SetMarginTop, opt);
    JSClass<JSViewAbstract>::StaticMethod("marginBottom", &JSViewAbstract::SetMarginBottom, opt);
    JSClass<JSViewAbstract>::StaticMethod("marginLeft", &JSViewAbstract::SetMarginLeft, opt);
    JSClass<JSViewAbstract>::StaticMethod("marginRight", &JSViewAbstract::SetMarginRight, opt);

    JSClass<JSViewAbstract>::StaticMethod("padding", &JSViewAbstract::JsPadding);
    JSClass<JSViewAbstract>::StaticMethod("paddingTop", &JSViewAbstract::SetPaddingTop, opt);
    JSClass<JSViewAbstract>::StaticMethod("paddingBottom", &JSViewAbstract::SetPaddingBottom, opt);
    JSClass<JSViewAbstract>::StaticMethod("paddingLeft", &JSViewAbstract::SetPaddingLeft, opt);
    JSClass<JSViewAbstract>::StaticMethod("paddingRight", &JSViewAbstract::SetPaddingRight, opt);

    JSClass<JSViewAbstract>::StaticMethod("backgroundColor", &JSViewAbstract::JsBackgroundColor);
    JSClass<JSViewAbstract>::StaticMethod("backgroundImage", &JSViewAbstract::JsBackgroundImage);
    JSClass<JSViewAbstract>::StaticMethod("backgroundImageSize", &JSViewAbstract::JsBackgroundImageSize);
    JSClass<JSViewAbstract>::StaticMethod("backgroundImagePosition", &JSViewAbstract::JsBackgroundImagePosition);
    JSClass<JSViewAbstract>::StaticMethod("backgroundBlurStyle", &JSViewAbstract::JsBackgroundBlurStyle);
    JSClass<JSViewAbstract>::StaticMethod("border", &JSViewAbstract::JsBorder);
    JSClass<JSViewAbstract>::StaticMethod("borderWidth", &JSViewAbstract::JsBorderWidth);
    JSClass<JSViewAbstract>::StaticMethod("borderColor", &JSViewAbstract::JsBorderColor);
    JSClass<JSViewAbstract>::StaticMethod("borderRadius", &JSViewAbstract::JsBorderRadius);
    JSClass<JSViewAbstract>::StaticMethod("borderStyle", &JSViewAbstract::JsBorderStyle);
    JSClass<JSViewAbstract>::StaticMethod("borderImage", &JSViewAbstract::JsBorderImage);

    JSClass<JSViewAbstract>::StaticMethod("scale", &JSViewAbstract::JsScale);
    JSClass<JSViewAbstract>::StaticMethod("scaleX", &JSViewAbstract::JsScaleX);
    JSClass<JSViewAbstract>::StaticMethod("scaleY", &JSViewAbstract::JsScaleY);
    JSClass<JSViewAbstract>::StaticMethod("opacity", &JSViewAbstract::JsOpacity);
    JSClass<JSViewAbstract>::StaticMethod("rotate", &JSViewAbstract::JsRotate);
    JSClass<JSViewAbstract>::StaticMethod("rotateX", &JSViewAbstract::JsRotateX);
    JSClass<JSViewAbstract>::StaticMethod("rotateY", &JSViewAbstract::JsRotateY);
    JSClass<JSViewAbstract>::StaticMethod("translate", &JSViewAbstract::JsTranslate);
    JSClass<JSViewAbstract>::StaticMethod("translateX", &JSViewAbstract::JsTranslateX);
    JSClass<JSViewAbstract>::StaticMethod("translateY", &JSViewAbstract::JsTranslateY);
    JSClass<JSViewAbstract>::StaticMethod("transform", &JSViewAbstract::JsTransform);
    JSClass<JSViewAbstract>::StaticMethod("transition", &JSViewAbstract::JsTransition);

    JSClass<JSViewAbstract>::StaticMethod("align", &JSViewAbstract::JsAlign);
    JSClass<JSViewAbstract>::StaticMethod("position", &JSViewAbstract::JsPosition);
    JSClass<JSViewAbstract>::StaticMethod("markAnchor", &JSViewAbstract::JsMarkAnchor);
    JSClass<JSViewAbstract>::StaticMethod("offset", &JSViewAbstract::JsOffset);
    JSClass<JSViewAbstract>::StaticMethod("enabled", &JSViewAbstract::JsEnabled);
    JSClass<JSViewAbstract>::StaticMethod("aspectRatio", &JSViewAbstract::JsAspectRatio);
    JSClass<JSViewAbstract>::StaticMethod("overlay", &JSViewAbstract::JsOverlay);

    JSClass<JSViewAbstract>::StaticMethod("blur", &JSViewAbstract::JsBlur);
    JSClass<JSViewAbstract>::StaticMethod("colorBlend", &JSViewAbstract::JsColorBlend);
    JSClass<JSViewAbstract>::StaticMethod("backdropBlur", &JSViewAbstract::JsBackdropBlur);
    JSClass<JSViewAbstract>::StaticMethod("windowBlur", &JSViewAbstract::JsWindowBlur);
    JSClass<JSViewAbstract>::StaticMethod("visibility", &JSViewAbstract::SetVisibility);
    JSClass<JSViewAbstract>::StaticMethod("flexBasis", &JSViewAbstract::JsFlexBasis);
    JSClass<JSViewAbstract>::StaticMethod("flexGrow", &JSViewAbstract::JsFlexGrow);
    JSClass<JSViewAbstract>::StaticMethod("flexShrink", &JSViewAbstract::JsFlexShrink);
    JSClass<JSViewAbstract>::StaticMethod("alignSelf", &JSViewAbstract::JsAlignSelf);
    JSClass<JSViewAbstract>::StaticMethod("displayPriority", &JSViewAbstract::JsDisplayPriority);
    JSClass<JSViewAbstract>::StaticMethod("useAlign", &JSViewAbstract::JsUseAlign);
    JSClass<JSViewAbstract>::StaticMethod("zIndex", &JSViewAbstract::JsZIndex);
    JSClass<JSViewAbstract>::StaticMethod("sharedTransition", &JSViewAbstract::JsSharedTransition);
    JSClass<JSViewAbstract>::StaticMethod("direction", &JSViewAbstract::SetDirection, opt);
#ifndef WEARABLE_PRODUCT
    JSClass<JSViewAbstract>::StaticMethod("bindPopup", &JSViewAbstract::JsBindPopup);
#endif

    JSClass<JSViewAbstract>::StaticMethod("bindMenu", &JSViewAbstract::JsBindMenu);
    JSClass<JSViewAbstract>::StaticMethod("bindContextMenu", &JSViewAbstract::JsBindContextMenu);
    JSClass<JSViewAbstract>::StaticMethod("onDragStart", &JSViewAbstract::JsOnDragStart);
    JSClass<JSViewAbstract>::StaticMethod("onDragEnter", &JSViewAbstract::JsOnDragEnter);
    JSClass<JSViewAbstract>::StaticMethod("onDragMove", &JSViewAbstract::JsOnDragMove);
    JSClass<JSViewAbstract>::StaticMethod("onDragLeave", &JSViewAbstract::JsOnDragLeave);
    JSClass<JSViewAbstract>::StaticMethod("onDrop", &JSViewAbstract::JsOnDrop);

    JSClass<JSViewAbstract>::StaticMethod("linearGradient", &JSViewAbstract::JsLinearGradient);
    JSClass<JSViewAbstract>::StaticMethod("sweepGradient", &JSViewAbstract::JsSweepGradient);
    JSClass<JSViewAbstract>::StaticMethod("radialGradient", &JSViewAbstract::JsRadialGradient);
    JSClass<JSViewAbstract>::StaticMethod("motionPath", &JSViewAbstract::JsMotionPath);
    JSClass<JSViewAbstract>::StaticMethod("gridSpan", &JSViewAbstract::JsGridSpan);
    JSClass<JSViewAbstract>::StaticMethod("gridOffset", &JSViewAbstract::JsGridOffset);
    JSClass<JSViewAbstract>::StaticMethod("useSizeType", &JSViewAbstract::JsUseSizeType);
    JSClass<JSViewAbstract>::StaticMethod("shadow", &JSViewAbstract::JsShadow);
    JSClass<JSViewAbstract>::StaticMethod("grayscale", &JSViewAbstract::JsGrayScale);
    JSClass<JSViewAbstract>::StaticMethod("focusable", &JSViewAbstract::JsFocusable);
    JSClass<JSViewAbstract>::StaticMethod("onKeyEvent", &JSViewAbstract::JsOnKeyEvent);
    JSClass<JSViewAbstract>::StaticMethod("onFocusMove", &JSViewAbstract::JsOnFocusMove);
    JSClass<JSViewAbstract>::StaticMethod("onFocus", &JSViewAbstract::JsOnFocus);
    JSClass<JSViewAbstract>::StaticMethod("onBlur", &JSViewAbstract::JsOnBlur);
    JSClass<JSViewAbstract>::StaticMethod("tabIndex", &JSViewAbstract::JsTabIndex);
    JSClass<JSViewAbstract>::StaticMethod("focusOnTouch", &JSViewAbstract::JsFocusOnTouch);
    JSClass<JSViewAbstract>::StaticMethod("defaultFocus", &JSViewAbstract::JsDefaultFocus);
    JSClass<JSViewAbstract>::StaticMethod("groupDefaultFocus", &JSViewAbstract::JsGroupDefaultFocus);
    JSClass<JSViewAbstract>::StaticMethod("brightness", &JSViewAbstract::JsBrightness);
    JSClass<JSViewAbstract>::StaticMethod("contrast", &JSViewAbstract::JsContrast);
    JSClass<JSViewAbstract>::StaticMethod("saturate", &JSViewAbstract::JsSaturate);
    JSClass<JSViewAbstract>::StaticMethod("sepia", &JSViewAbstract::JsSepia);
    JSClass<JSViewAbstract>::StaticMethod("invert", &JSViewAbstract::JsInvert);
    JSClass<JSViewAbstract>::StaticMethod("hueRotate", &JSViewAbstract::JsHueRotate);
    JSClass<JSViewAbstract>::StaticMethod("clip", &JSViewAbstract::JsClip);
    JSClass<JSViewAbstract>::StaticMethod("mask", &JSViewAbstract::JsMask);
    JSClass<JSViewAbstract>::StaticMethod("key", &JSViewAbstract::JsKey);
    JSClass<JSViewAbstract>::StaticMethod("id", &JSViewAbstract::JsId);
    JSClass<JSViewAbstract>::StaticMethod("restoreId", &JSViewAbstract::JsRestoreId);
    JSClass<JSViewAbstract>::StaticMethod("hoverEffect", &JSViewAbstract::JsHoverEffect);
    JSClass<JSViewAbstract>::StaticMethod("onMouse", &JSViewAbstract::JsOnMouse);
    JSClass<JSViewAbstract>::StaticMethod("onHover", &JSViewAbstract::JsOnHover);
    JSClass<JSViewAbstract>::StaticMethod("onClick", &JSViewAbstract::JsOnClick);
#if defined(PREVIEW)
    JSClass<JSViewAbstract>::StaticMethod("debugLine", &JSViewAbstract::JsDebugLine);
#endif
    JSClass<JSViewAbstract>::StaticMethod("geometryTransition", &JSViewAbstract::JsGeometryTransition);
    JSClass<JSViewAbstract>::StaticMethod("onAreaChange", &JSViewAbstract::JsOnAreaChange);
    JSClass<JSViewAbstract>::StaticMethod("touchable", &JSInteractableView::JsTouchable);

    JSClass<JSViewAbstract>::StaticMethod("accessibilityGroup", &JSViewAbstract::JsAccessibilityGroup);
    JSClass<JSViewAbstract>::StaticMethod("accessibilityText", &JSViewAbstract::JsAccessibilityText);
    JSClass<JSViewAbstract>::StaticMethod("accessibilityDescription", &JSViewAbstract::JsAccessibilityDescription);
    JSClass<JSViewAbstract>::StaticMethod("accessibilityImportance", &JSViewAbstract::JsAccessibilityImportance);
    JSClass<JSViewAbstract>::StaticMethod("onAccessibility", &JSInteractableView::JsOnAccessibility);
    JSClass<JSViewAbstract>::StaticMethod("alignRules", &JSViewAbstract::JsAlignRules);
    JSClass<JSViewAbstract>::StaticMethod("onVisibleAreaChange", &JSViewAbstract::JsOnVisibleAreaChange);
    JSClass<JSViewAbstract>::StaticMethod("hitTestBehavior", &JSViewAbstract::JsHitTestBehavior);
}

void JSViewAbstract::JsAlignRules(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    if (!info[0]->IsObject()) {
        LOGE("arg is not Object or String.");
        return;
    }

    JSRef<JSObject> valueObj = JSRef<JSObject>::Cast(info[0]);
    if (valueObj->IsEmpty()) {
        LOGE("Align rule is empty");
        return;
    }
    const char* keys[] = { "left", "middle", "right", "top", "center", "bottom" };
    std::map<AlignDirection, AlignRule> alignRules;
    for (uint32_t i = 0; i < sizeof(keys) / sizeof(const char*); i++) {
        auto rule = valueObj->GetProperty(keys[i]);
        if (rule->IsObject()) {
            JSRef<JSObject> val = JSRef<JSObject>::Cast(rule);
            JSRef<JSVal> align = val->GetProperty("align");
            AlignRule alignRule;
            alignRule.anchor = val->GetProperty("anchor")->ToString();
            if (i < DIRECTION_RANGE) {
                alignRule.horizontal = static_cast<HorizontalAlign>(val->GetProperty("align")->ToNumber<int32_t>());
            } else {
                alignRule.vertical = static_cast<VerticalAlign>(val->GetProperty("align")->ToNumber<int32_t>());
            }
            alignRules[static_cast<AlignDirection>(i)] = alignRule;
        }
    }

    ViewAbstractModel::GetInstance()->SetAlignRules(alignRules);
}

void JSViewAbstract::SetMarginTop(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetMargins(value, std::nullopt, std::nullopt, std::nullopt);
}

void JSViewAbstract::SetMarginBottom(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetMargins(std::nullopt, value, std::nullopt, std::nullopt);
}

void JSViewAbstract::SetMarginLeft(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetMargins(std::nullopt, std::nullopt, value, std::nullopt);
}

void JSViewAbstract::SetMarginRight(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetMargins(std::nullopt, std::nullopt, std::nullopt, value);
}

void JSViewAbstract::SetPaddingTop(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetPaddings(value, std::nullopt, std::nullopt, std::nullopt);
}

void JSViewAbstract::SetPaddingBottom(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetPaddings(std::nullopt, value, std::nullopt, std::nullopt);
}

void JSViewAbstract::SetPaddingLeft(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetPaddings(std::nullopt, std::nullopt, value, std::nullopt);
}

void JSViewAbstract::SetPaddingRight(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetPaddings(std::nullopt, std::nullopt, std::nullopt, value);
}

void JSViewAbstract::SetBlur(float radius)
{
    Dimension dimensionRadius(radius, DimensionUnit::PX);
    ViewAbstractModel::GetInstance()->SetFrontBlur(dimensionRadius);
}

void JSViewAbstract::SetColorBlend(Color color)
{
    ViewAbstractModel::GetInstance()->SetColorBlend(color);
}

void JSViewAbstract::SetBackdropBlur(float radius)
{
    Dimension dimensionRadius(radius, DimensionUnit::PX);
    ViewAbstractModel::GetInstance()->SetBackdropBlur(dimensionRadius);
}

void JSViewAbstract::SetWindowBlur(float progress, WindowBlurStyle blurStyle)
{
    ViewAbstractModel::GetInstance()->SetWindowBlur(progress, blurStyle);
}

bool JSViewAbstract::ParseJsonDimension(
    const std::unique_ptr<JsonValue>& jsonValue, Dimension& result, DimensionUnit defaultUnit)
{
    if (!jsonValue || jsonValue->IsNull()) {
        LOGD("invalid json value");
        return false;
    }
    if (!jsonValue->IsNumber() && !jsonValue->IsString() && !jsonValue->IsObject()) {
        LOGE("json value is not number, string or object");
        return false;
    }
    if (jsonValue->IsNumber()) {
        result = Dimension(jsonValue->GetDouble(), defaultUnit);
        return true;
    }
    if (jsonValue->IsString()) {
        result = StringUtils::StringToDimensionWithUnit(jsonValue->GetString(), defaultUnit);
        return true;
    }
    auto resVal = JsonUtil::ParseJsonString(jsonValue->ToString());
    auto resId = resVal->GetValue("id");
    if (!resId || !resId->IsNumber()) {
        LOGE("invalid resource id");
        return false;
    }
    auto themeConstants = GetThemeConstants();
    if (!themeConstants) {
        LOGE("themeConstants is nullptr");
        return false;
    }
    result = themeConstants->GetDimension(resId->GetUInt());
    return true;
}

bool JSViewAbstract::ParseJsonDimensionVp(const std::unique_ptr<JsonValue>& jsonValue, Dimension& result)
{
    return ParseJsonDimension(jsonValue, result, DimensionUnit::VP);
}

bool JSViewAbstract::ParseJsonDouble(const std::unique_ptr<JsonValue>& jsonValue, double& result)
{
    if (!jsonValue || jsonValue->IsNull()) {
        LOGD("invalid json value");
        return false;
    }
    if (!jsonValue->IsNumber() && !jsonValue->IsString() && !jsonValue->IsObject()) {
        LOGE("json value is not number, string or object");
        return false;
    }
    if (jsonValue->IsNumber()) {
        result = jsonValue->GetDouble();
        return true;
    }
    if (jsonValue->IsString()) {
        result = StringUtils::StringToDouble(jsonValue->GetString());
        return true;
    }
    auto resVal = JsonUtil::ParseJsonString(jsonValue->ToString());
    auto resId = resVal->GetValue("id");
    if (!resId || !resId->IsNumber()) {
        LOGE("invalid resource id");
        return false;
    }
    auto themeConstants = GetThemeConstants();
    if (!themeConstants) {
        LOGW("themeConstants is nullptr");
        return false;
    }
    result = themeConstants->GetDouble(resId->GetUInt());
    return true;
}

bool JSViewAbstract::ParseJsonColor(const std::unique_ptr<JsonValue>& jsonValue, Color& result)
{
    if (!jsonValue || jsonValue->IsNull()) {
        LOGD("invalid json value");
        return false;
    }
    if (!jsonValue->IsNumber() && !jsonValue->IsString() && !jsonValue->IsObject()) {
        LOGE("json value is not number, string or object");
        return false;
    }
    if (jsonValue->IsNumber()) {
        result = Color(ColorAlphaAdapt(jsonValue->GetUInt()));
        return true;
    }
    if (jsonValue->IsString()) {
        result = Color::FromString(jsonValue->GetString());
        return true;
    }
    auto resVal = JsonUtil::ParseJsonString(jsonValue->ToString());
    auto resId = resVal->GetValue("id");
    if (!resId || !resId->IsNumber()) {
        LOGE("invalid resource id");
        return false;
    }
    auto themeConstants = GetThemeConstants();
    if (!themeConstants) {
        LOGW("themeConstants is nullptr");
        return false;
    }
    result = themeConstants->GetColor(resId->GetUInt());
    return true;
}

void JSViewAbstract::GetAngle(
    const std::string& key, const std::unique_ptr<JsonValue>& jsonValue, std::optional<float>& angle)
{
    auto value = jsonValue->GetValue(key);
    if (value && value->IsString()) {
        angle = static_cast<float>(StringUtils::StringToDegree(value->GetString()));
    } else if (value && value->IsNumber()) {
        angle = static_cast<float>(value->GetDouble());
    } else {
        LOGE("Invalid value type");
    }
}

void JSViewAbstract::GetGradientColorStops(Gradient& gradient, const std::unique_ptr<JsonValue>& colorStops)
{
    if (!colorStops || colorStops->IsNull() || !colorStops->IsArray()) {
        return;
    }

    for (int32_t i = 0; i < colorStops->GetArraySize(); i++) {
        GradientColor gradientColor;
        auto item = colorStops->GetArrayItem(i);
        if (item && !item->IsNull() && item->IsArray() && item->GetArraySize() >= 1) {
            auto colorParams = item->GetArrayItem(0);
            // color
            Color color;
            if (!ParseJsonColor(colorParams, color)) {
                LOGE("parse colorParams failed");
                continue;
            }
            gradientColor.SetColor(color);
            gradientColor.SetHasValue(false);
            // stop value
            if (item->GetArraySize() <= 1) {
                continue;
            }
            auto stopValue = item->GetArrayItem(1);
            double value = 0.0;
            if (ParseJsonDouble(stopValue, value)) {
                value = std::clamp(value, 0.0, 1.0);
                gradientColor.SetHasValue(true);
                gradientColor.SetDimension(Dimension(value * 100.0, DimensionUnit::PERCENT));
            }
            gradient.AddColor(gradientColor);
        }
    }
}

void JSViewAbstract::NewGetGradientColorStops(NG::Gradient& gradient, const std::unique_ptr<JsonValue>& colorStops)
{
    if (!colorStops || colorStops->IsNull() || !colorStops->IsArray()) {
        return;
    }

    for (int32_t i = 0; i < colorStops->GetArraySize(); i++) {
        NG::GradientColor gradientColor;
        auto item = colorStops->GetArrayItem(i);
        if (item && !item->IsNull() && item->IsArray() && item->GetArraySize() >= 1) {
            auto colorParams = item->GetArrayItem(0);
            // color
            Color color;
            if (!ParseJsonColor(colorParams, color)) {
                LOGE("parse colorParams failed");
                continue;
            }
            gradientColor.SetColor(color);
            gradientColor.SetHasValue(false);
            // stop value
            if (item->GetArraySize() <= 1) {
                continue;
            }
            auto stopValue = item->GetArrayItem(1);
            double value = 0.0;
            if (ParseJsonDouble(stopValue, value)) {
                value = std::clamp(value, 0.0, 1.0);
                gradientColor.SetHasValue(true);
                //  [0, 1] -> [0, 100.0];
                gradientColor.SetDimension(Dimension(value * 100.0, DimensionUnit::PERCENT));
            }
            gradient.AddColor(gradientColor);
        }
    }
}

void JSViewAbstract::SetDirection(const std::string& dir)
{
    TextDirection direction = TextDirection::AUTO;
    if (dir == "Ltr") {
        direction = TextDirection::LTR;
    } else if (dir == "Rtl") {
        direction = TextDirection::RTL;
    } else if (dir == "Auto") {
        direction = TextDirection::AUTO;
    }
    ViewAbstractModel::GetInstance()->SetLayoutDirection(direction);
}

RefPtr<ThemeConstants> JSViewAbstract::GetThemeConstants(const JSRef<JSObject>& jsObj)
{
    std::string bundleName;
    std::string moduleName;
    if (!jsObj->IsUndefined()) {
        JSRef<JSVal> bundle = jsObj->GetProperty("bundleName");
        JSRef<JSVal> module = jsObj->GetProperty("moduleName");
        if (bundle->IsString() && module->IsString()) {
            bundleName = bundle->ToString();
            moduleName = module->ToString();
        }
    }

    auto cardId = CardScope::CurrentId();
    if (cardId != INVALID_CARD_ID) {
        auto container = Container::Current();
        auto weak = container->GetCardPipeline(cardId);
        auto cardPipelineContext = weak.Upgrade();
        if (!cardPipelineContext) {
            LOGE("card pipelineContext is null!");
            return nullptr;
        }
        auto cardThemeManager = cardPipelineContext->GetThemeManager();
        if (!cardThemeManager) {
            LOGE("card themeManager is null!");
            return nullptr;
        }
        return cardThemeManager->GetThemeConstants(bundleName, moduleName);
    }

#ifdef PLUGIN_COMPONENT_SUPPORTED
    if (Container::CurrentId() >= MIN_PLUGIN_SUBCONTAINER_ID) {
        auto pluginContainer = PluginManager::GetInstance().GetPluginSubContainer(Container::CurrentId());
        if (!pluginContainer) {
            LOGW("pluginContainer is null");
            return nullptr;
        }
        auto pluginPipelineContext = pluginContainer->GetPipelineContext();
        if (!pluginPipelineContext) {
            LOGE("pluginPipelineContext is null!");
            return nullptr;
        }
        auto pluginThemeManager = pluginPipelineContext->GetThemeManager();
        if (!pluginThemeManager) {
            LOGE("pluginThemeManager is null!");
            return nullptr;
        }
        return pluginThemeManager->GetThemeConstants(bundleName, moduleName);
    }
#endif
    auto container = Container::Current();
    if (!container) {
        LOGW("container is null");
        return nullptr;
    }
    auto pipelineContext = container->GetPipelineContext();
    if (!pipelineContext) {
        LOGE("pipelineContext is null!");
        return nullptr;
    }
    auto themeManager = pipelineContext->GetThemeManager();
    if (!themeManager) {
        LOGE("themeManager is null!");
        return nullptr;
    }
    return themeManager->GetThemeConstants(bundleName, moduleName);
}

void JSViewAbstract::JsHoverEffect(const JSCallbackInfo& info)
{
    if (!info[0]->IsNumber()) {
        LOGE("info[0] is not a number");
        return;
    }
    ViewAbstractModel::GetInstance()->SetHoverEffect(static_cast<HoverEffectType>(info[0]->ToNumber<int32_t>()));
}

void JSViewAbstract::JsOnMouse(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        LOGE("the param is not a function");
        return;
    }

    RefPtr<JsClickFunction> jsOnMouseFunc = AceType::MakeRefPtr<JsClickFunction>(JSRef<JSFunc>::Cast(info[0]));
    auto onMouse = [execCtx = info.GetExecutionContext(), func = std::move(jsOnMouseFunc)](MouseInfo& mouseInfo) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onMouse");
        func->Execute(mouseInfo);
    };
    ViewAbstractModel::GetInstance()->SetOnMouse(std::move(onMouse));
}

void JSViewAbstract::JsOnHover(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        LOGE("the param is not a function");
        return;
    }

    RefPtr<JsHoverFunction> jsOnHoverFunc = AceType::MakeRefPtr<JsHoverFunction>(JSRef<JSFunc>::Cast(info[0]));
    auto onHover = [execCtx = info.GetExecutionContext(), func = std::move(jsOnHoverFunc)](bool param) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onHover");
        func->Execute(param);
    };
    ViewAbstractModel::GetInstance()->SetOnHover(std::move(onHover));
}

void JSViewAbstract::JsOnClick(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        LOGW("the info is not click function");
        return;
    }

    auto jsOnClickFunc = AceType::MakeRefPtr<JsClickFunction>(JSRef<JSFunc>::Cast(info[0]));
    auto onTap = [execCtx = info.GetExecutionContext(), func = std::move(jsOnClickFunc)](GestureEvent& info) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onClick");
        func->Execute(info);
    };
    auto onClick = [execCtx = info.GetExecutionContext(), func = jsOnClickFunc](const ClickInfo* info) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onClick");
        func->Execute(*info);
    };
    ViewAbstractModel::GetInstance()->SetOnClick(std::move(onTap), std::move(onClick));
}

void JSViewAbstract::JsOnVisibleAreaChange(const JSCallbackInfo& info)
{
    if (info.Length() != 2) {
        LOGE("JsOnVisibleAreaChange: The arg is wrong, it is supposed to have 2 arguments");
        return;
    }

    if (!info[0]->IsArray() || !info[1]->IsFunction()) {
        LOGE("JsOnVisibleAreaChange: The param type is invalid.");
        return;
    }

    auto ratioArray = JSRef<JSArray>::Cast(info[0]);
    size_t size = ratioArray->Length();
    std::vector<double> ratioVec(size);
    ratioVec.clear();
    for (size_t i = 0; i < size; i++) {
        double ratio = 0.0;
        ParseJsDouble(ratioArray->GetValueAt(i), ratio);
        if (LessOrEqual(ratio, VISIBLE_RATIO_MIN)) {
            ratio = VISIBLE_RATIO_MIN;
        }

        if (GreatOrEqual(ratio, VISIBLE_RATIO_MAX)) {
            ratio = VISIBLE_RATIO_MAX;
        }
        ratioVec.push_back(ratio);
    }

    RefPtr<JsFunction> jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[1]));
    auto onVisibleChange = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](
                               bool visible, double ratio) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onVisibleAreaChange");

        JSRef<JSVal> params[2];
        params[0] = JSRef<JSVal>::Make(ToJSValue(visible));
        params[1] = JSRef<JSVal>::Make(ToJSValue(ratio));
        func->ExecuteJS(2, params);
    };
    ViewAbstractModel::GetInstance()->SetOnVisibleChange(std::move(onVisibleChange), ratioVec);
}

void JSViewAbstract::JsHitTestBehavior(const JSCallbackInfo& info)
{
    if (info.Length() != 1) {
        LOGE("JsHitTestBehavior: The arg is wrong, it is supposed to have 1 arguments");
        return;
    }

    NG::HitTestMode hitTestModeNG = NG::HitTestMode::HTMDEFAULT;
    hitTestModeNG = static_cast<NG::HitTestMode>(info[0]->ToNumber<int32_t>());
    ViewAbstractModel::GetInstance()->SetHitTestMode(hitTestModeNG);
}

} // namespace OHOS::Ace::Framework
