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

#include "frameworks/bridge/declarative_frontend/jsview/js_rect.h"

#include "base/log/ace_trace.h"
#include "bridge/declarative_frontend/jsview/models/rect_model_impl.h"
#include "bridge/declarative_frontend/view_stack_processor.h"
#include "core/components_ng/pattern/shape/rect_model.h"
#include "core/components_ng/pattern/shape/rect_model_ng.h"

namespace OHOS::Ace {

std::unique_ptr<RectModel> RectModel::instance_ = nullptr;

RectModel* RectModel::GetInstance()
{
    if (!instance_) {
#ifdef NG_BUILD
        instance_.reset(new NG::RectModelNG());
#else
        if (Container::IsCurrentUseNewPipeline()) {
            instance_.reset(new NG::RectModelNG());
        } else {
            instance_.reset(new Framework::RectModelImpl());
        }
#endif
    }
    return instance_.get();
}

} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {

void JSRect::Create(const JSCallbackInfo& info)
{
    RectModel::GetInstance()->Create();
    JSShapeAbstract::SetSize(info);
    if (info.Length() > 0 && info[0]->IsObject()) {
        JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[0]);
        JSRef<JSVal> radiusWidth = obj->GetProperty("radiusWidth");
        Dimension widthValue;
        if (ParseJsDimensionVp(radiusWidth, widthValue)) {
            RectModel::GetInstance()->SetRadiusWidth(widthValue);
        }

        JSRef<JSVal> radiusHeight = obj->GetProperty("radiusHeight");
        Dimension heightValue;
        if (ParseJsDimensionVp(radiusHeight, heightValue)) {
            RectModel::GetInstance()->SetRadiusHeight(heightValue);
        }

        JSRef<JSVal> radius = obj->GetProperty("radius");
        if (radius->IsNumber() || radius->IsString()) {
            SetRadiusWithJsVal<ShapeComponent>(nullptr, radius);
        }
        if (radius->IsArray()) {
            SetRadiusWithArrayValue<ShapeComponent>(nullptr, radius);
        }
        info.SetReturnValue(info.This());
    }
}

void JSRect::SetRadiusWidth(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (!info[0]->IsNumber() && !info[0]->IsString()) {
        LOGE("arg is not Number or String.");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    RectModel::GetInstance()->SetRadiusWidth(value);
}

void JSRect::SetRadiusHeight(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    RectModel::GetInstance()->SetRadiusHeight(value);
}

void JSRect::SetRadius(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
    if (info[0]->IsArray()) {
        SetRadiusWithArrayValue<ShapeComponent>(nullptr, info[0]);
        info.SetReturnValue(info.This());
        return;
    }
    if (info[0]->IsNumber() || info[0]->IsString() || info[0]->IsObject()) {
        SetRadiusWithJsVal<ShapeComponent>(nullptr, info[0]);
        info.SetReturnValue(info.This());
    }
}

template<class T>
void JSRect::SetRadiusWithJsVal(const RefPtr<T>& component, const JSRef<JSVal>& jsVal)
{
    Dimension value;
    if (!ParseJsDimensionVp(jsVal, value)) {
        return;
    }
    if (component) {
        AnimationOption option = ViewStackProcessor::GetInstance()->GetImplicitAnimationOption();
        component->SetRadiusWidth(value, option);
        component->SetRadiusHeight(value, option);
        return;
    }
    RectModel::GetInstance()->SetRadiusWidth(value);
    RectModel::GetInstance()->SetRadiusHeight(value);
}

template<class T>
void JSRect::SetRadiusWithArrayValue(const RefPtr<T>& component, const JSRef<JSVal>& jsVal)
{
    JSRef<JSArray> array = JSRef<JSArray>::Cast(jsVal);
    int32_t length = static_cast<int32_t>(array->Length());
    if (length <= 0) {
        LOGE("info is invalid");
        return;
    }
    length = std::min(length, 4);
    for (int32_t i = 0; i < length; i++) {
        JSRef<JSVal> radiusItem = array->GetValueAt(i);
        if (!radiusItem->IsArray()) {
            break;
        }
        JSRef<JSArray> radiusArray = JSRef<JSArray>::Cast(radiusItem);
        if (radiusArray->Length() != 2) {
            break;
        }
        JSRef<JSVal> radiusX = radiusArray->GetValueAt(0);
        JSRef<JSVal> radiusY = radiusArray->GetValueAt(1);
        Dimension radiusXValue;
        Dimension radiusYValue;
        if (ParseJsDimensionVp(radiusX, radiusXValue) && ParseJsDimensionVp(radiusY, radiusYValue)) {
            SetRadiusValue<T>(component, radiusXValue, radiusYValue, i);
        }
    }
}

template<class T>
void JSRect::SetRadiusValue(
    const RefPtr<T>& component, const Dimension& radiusX, const Dimension& radiusY, int32_t index)
{
    if (component) {
        RectModel::GetInstance()->SetCallbackRadius(component, radiusX, radiusY, index);
    } else {
        RectModel::GetInstance()->SetRadiusValue(radiusX, radiusY, index);
    }
}

void JSRect::ObjectRadiusWidth(const JSCallbackInfo& info)
{
    info.ReturnSelf();
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    if (LessNotEqual(value.Value(), 0.0)) {
        LOGE("Value is less than zero");
        return;
    }
    auto rect = AceType::DynamicCast<ShapeRect>(basicShape_);
    if (rect) {
        rect->SetRadiusWidth(value);
    }
}

void JSRect::ObjectRadiusHeight(const JSCallbackInfo& info)
{
    info.ReturnSelf();
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }
    if (LessNotEqual(value.Value(), 0.0)) {
        LOGE("Value is less than zero");
        return;
    }
    auto rect = AceType::DynamicCast<ShapeRect>(basicShape_);
    if (rect) {
        rect->SetRadiusHeight(value);
    }
}

void JSRect::ObjectRadius(const JSCallbackInfo& info)
{
    info.ReturnSelf();
    if (info.Length() < 1) {
        LOGE("arg is invalid");
        return;
    }
    auto rect = AceType::DynamicCast<ShapeRect>(basicShape_);
    if (!rect) {
        LOGE("rect is null");
        return;
    }
    if (info[0]->IsNumber() || info[0]->IsString()) {
        SetRadiusWithJsVal<ShapeRect>(rect, info[0]);
    }
    if (info[0]->IsArray()) {
        SetRadiusWithArrayValue<ShapeRect>(rect, info[0]);
    }
}

void JSRect::ConstructorCallback(const JSCallbackInfo& info)
{
    auto jsRect = AceType::MakeRefPtr<JSRect>();
    auto rect = AceType::MakeRefPtr<ShapeRect>();
    if (info.Length() > 0 && info[0]->IsObject()) {
        JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[0]);
        Dimension width;
        if (ParseJsDimensionVp(obj->GetProperty("width"), width) && width.IsValid()) {
            rect->SetWidth(width);
        }
        Dimension height;
        if (ParseJsDimensionVp(obj->GetProperty("height"), height) && height.IsValid()) {
            rect->SetHeight(height);
        }
        Dimension radiusWidth;
        if (ParseJsDimensionVp(obj->GetProperty("radiusWidth"), radiusWidth) && radiusWidth.IsValid()) {
            rect->SetRadiusWidth(radiusWidth);
        }
        Dimension radiusHeight;
        if (ParseJsDimensionVp(obj->GetProperty("radiusHeight"), radiusHeight) && radiusHeight.IsValid()) {
            rect->SetRadiusHeight(radiusHeight);
        }
        JSRef<JSVal> radius = obj->GetProperty("radius");
        if (radius->IsNumber() || radius->IsString()) {
            SetRadiusWithJsVal<ShapeRect>(rect, radius);
        }
        if (radius->IsArray()) {
            SetRadiusWithArrayValue<ShapeRect>(rect, radius);
        }
        info.SetReturnValue(info.This());
    }
    jsRect->SetBasicShape(rect);
    jsRect->IncRefCount();
    info.SetReturnValue(AceType::RawPtr(jsRect));
}

void JSRect::DestructorCallback(JSRect* jsRect)
{
    if (jsRect != nullptr) {
        jsRect->DecRefCount();
    }
}

void JSRect::JSBind(BindingTarget globalObj)
{
    JSClass<JSRect>::Declare("Rect");
    JSClass<JSRect>::StaticMethod("create", &JSRect::Create);
    JSClass<JSRect>::StaticMethod("radiusWidth", &JSRect::SetRadiusWidth);
    JSClass<JSRect>::StaticMethod("radiusHeight", &JSRect::SetRadiusHeight);
    JSClass<JSRect>::StaticMethod("radius", &JSRect::SetRadius);

    JSClass<JSRect>::CustomMethod("width", &JSShapeAbstract::ObjectWidth);
    JSClass<JSRect>::CustomMethod("height", &JSShapeAbstract::ObjectHeight);
    JSClass<JSRect>::CustomMethod("size", &JSShapeAbstract::ObjectSize);
    JSClass<JSRect>::CustomMethod("offset", &JSShapeAbstract::ObjectOffset);
    JSClass<JSRect>::CustomMethod("radiusWidth", &JSRect::ObjectRadiusWidth);
    JSClass<JSRect>::CustomMethod("radiusHeight", &JSRect::ObjectRadiusHeight);
    JSClass<JSRect>::CustomMethod("radius", &JSRect::ObjectRadius);
    JSClass<JSRect>::CustomMethod("fill", &JSShapeAbstract::ObjectFill);

    JSClass<JSRect>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSRect>::StaticMethod("onHover", &JSInteractableView::JsOnHover);
    JSClass<JSRect>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSRect>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSRect>::StaticMethod("onClick", &JSInteractableView::JsOnClick);

    JSClass<JSRect>::Inherit<JSShapeAbstract>();
    JSClass<JSRect>::Bind(globalObj, JSRect::ConstructorCallback, JSRect::DestructorCallback);
}

} // namespace OHOS::Ace::Framework
