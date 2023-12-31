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

#include "bridge/declarative_frontend/jsview/js_canvas_renderer.h"

#include "bridge/common/utils/engine_helper.h"
#include "bridge/declarative_frontend/engine/bindings.h"
#include "bridge/declarative_frontend/engine/js_converter.h"
#include "bridge/declarative_frontend/jsview/js_offscreen_rendering_context.h"
#include "bridge/declarative_frontend/jsview/js_utils.h"

#ifdef PIXEL_MAP_SUPPORTED
#include "pixel_map.h"
#include "pixel_map_napi.h"
#endif
namespace OHOS::Ace::Framework {
std::unordered_map<int32_t, Pattern> JSCanvasRenderer::pattern_;
int32_t JSCanvasRenderer::patternCount_ = 0;
namespace {

const std::set<std::string> FONT_WEIGHTS = {
    "normal", "bold", "lighter", "bolder",
    "100", "200", "300", "400", "500", "600", "700", "800", "900"
};
const std::set<std::string> FONT_STYLES = { "italic", "oblique", "normal" };
const std::set<std::string> FONT_FAMILIES = { "sans-serif", "serif", "monospace" };
const std::set<std::string> QUALITY_TYPE = { "low", "medium", "high" }; // Default value is low.
constexpr double DEFAULT_QUALITY = 0.92;
template<typename T>
inline T ConvertStrToEnum(const char* key, const LinearMapNode<T>* map, size_t length, T defaultValue)
{
    int64_t index = BinarySearchFindIndex(map, length, key);
    return index != -1 ? map[index].value : defaultValue;
}

inline Rect GetJsRectParam(const JSCallbackInfo& info)
{
    // 4 parameters: rect(x, y, width, height)
    if (info.Length() != 4) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return Rect();
    }
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
    JSViewAbstract::ParseJsDouble(info[0], x);
    JSViewAbstract::ParseJsDouble(info[1], y);
    JSViewAbstract::ParseJsDouble(info[2], width);
    JSViewAbstract::ParseJsDouble(info[3], height);
    x = SystemProperties::Vp2Px(x);
    y = SystemProperties::Vp2Px(y);
    width = SystemProperties::Vp2Px(width);
    height = SystemProperties::Vp2Px(height);

    Rect rect = Rect(x, y, width, height);
    return rect;
}

inline bool ParseJsDoubleArray(const JSRef<JSVal>& jsValue, std::vector<double>& result)
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
                result.emplace_back(value->ToNumber<double>());
            } else if (value->IsObject()) {
                double singleResInt;
                if (JSViewAbstract::ParseJsDouble(value, singleResInt)) {
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
    return false;
}

inline bool ParseJsInt(const JSRef<JSVal>& jsValue, int32_t& result)
{
    if (!jsValue->IsNumber() && !jsValue->IsObject()) {
        LOGE("arg is not number or Object.");
        return false;
    }

    if (jsValue->IsNumber()) {
        LOGD("jsValue->IsNumber()");
        result = jsValue->ToNumber<int32_t>();
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
    return false;
}

const LinearMapNode<TextBaseline> BASELINE_TABLE[] = {
    { "alphabetic", TextBaseline::ALPHABETIC },
    { "bottom", TextBaseline::BOTTOM },
    { "hanging", TextBaseline::HANGING },
    { "ideographic", TextBaseline::IDEOGRAPHIC },
    { "middle", TextBaseline::MIDDLE },
    { "top", TextBaseline::TOP },
};

} // namespace

JSCanvasRenderer::JSCanvasRenderer()
{
}

void JSCanvasRenderer::JsCreateLinearGradient(const JSCallbackInfo& info)
{
    JSRef<JSObject> pasteObj = JSClass<JSCanvasGradient>::NewInstance();
    pasteObj->SetProperty("__type", "gradient");

    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()
        && info[3]->IsNumber()) {
        double x0 = 0.0;
        double y0 = 0.0;
        double x1 = 0.0;
        double y1 = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], x0);
        JSViewAbstract::ParseJsDouble(info[1], y0);
        JSViewAbstract::ParseJsDouble(info[2], x1);
        JSViewAbstract::ParseJsDouble(info[3], y1);
        x0 = SystemProperties::Vp2Px(x0);
        y0 = SystemProperties::Vp2Px(y0);
        x1 = SystemProperties::Vp2Px(x1);
        y1 = SystemProperties::Vp2Px(y1);
        Offset beginOffset = Offset(x0, y0);
        Offset endOffset = Offset(x1, y1);

        Gradient* gradient = new Gradient();
        gradient->SetType(GradientType::LINEAR);
        gradient->SetBeginOffset(beginOffset);
        gradient->SetEndOffset(endOffset);

        auto pasteData = Referenced::Claim(pasteObj->Unwrap<JSCanvasGradient>());
        pasteData->SetGradient(gradient);
        info.SetReturnValue(pasteObj);
    }
}

void JSCanvasRenderer::JsCreateRadialGradient(const JSCallbackInfo& info)
{
    JSRef<JSObject> pasteObj = JSClass<JSCanvasGradient>::NewInstance();
    pasteObj->SetProperty("__type", "gradient");

    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()
        && info[3]->IsNumber()  && info[4]->IsNumber() && info[5]->IsNumber()) {
        double startX = 0.0;
        double startY = 0.0;
        double startRadial = 0.0;
        double endX = 0.0;
        double endY = 0.0;
        double endRadial = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], startX);
        JSViewAbstract::ParseJsDouble(info[1], startY);
        JSViewAbstract::ParseJsDouble(info[2], startRadial);
        JSViewAbstract::ParseJsDouble(info[3], endX);
        JSViewAbstract::ParseJsDouble(info[4], endY);
        JSViewAbstract::ParseJsDouble(info[5], endRadial);
        startX = SystemProperties::Vp2Px(startX);
        startY = SystemProperties::Vp2Px(startY);
        startRadial = SystemProperties::Vp2Px(startRadial);
        endX = SystemProperties::Vp2Px(endX);
        endY = SystemProperties::Vp2Px(endY);
        endRadial = SystemProperties::Vp2Px(endRadial);
        Offset innerCenter = Offset(startX, startY);
        Offset outerCenter = Offset(endX, endY);

        Gradient* gradient = new Gradient();
        gradient->SetType(GradientType::RADIAL);
        gradient->SetBeginOffset(innerCenter);
        gradient->SetEndOffset(outerCenter);
        gradient->SetInnerRadius(startRadial);
        gradient->SetOuterRadius(endRadial);

        auto pasteData = Referenced::Claim(pasteObj->Unwrap<JSCanvasGradient>());
        pasteData->SetGradient(gradient);
        info.SetReturnValue(pasteObj);
    }
}

void JSCanvasRenderer::JsFillText(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (info[0]->IsString() && info[1]->IsNumber() && info[2]->IsNumber()) {
        double x = 0.0;
        double y = 0.0;
        std::string text = "";
        JSViewAbstract::ParseJsString(info[0], text);
        JSViewAbstract::ParseJsDouble(info[1], x);
        JSViewAbstract::ParseJsDouble(info[2], y);
        x = SystemProperties::Vp2Px(x);
        y = SystemProperties::Vp2Px(y);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->FillText(text, x, y, paintState_);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->FillText(text, x, y);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->FillText(text, x, y, paintState_);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->FillText(text, Offset(x, y));
        }
    }
}

void JSCanvasRenderer::JsStrokeText(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (info[0]->IsString() && info[1]->IsNumber() && info[2]->IsNumber()) {
        double x = 0.0;
        double y = 0.0;
        std::string text = "";
        JSViewAbstract::ParseJsString(info[0], text);
        JSViewAbstract::ParseJsDouble(info[1], x);
        JSViewAbstract::ParseJsDouble(info[2], y);
        x = SystemProperties::Vp2Px(x);
        y = SystemProperties::Vp2Px(y);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->StrokeText(text, x, y, paintState_);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->StrokeText(text, x, y);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->StrokeText(text, x, y, paintState_);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->StrokeText(text, Offset(x, y));
        }
    }
}

void JSCanvasRenderer::SetAntiAlias()
{
    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            offscreenCanvasPattern_->SetAntiAlias(anti_);
            return;
        }
        if (!isOffscreen_ && customPaintPattern_) {
            customPaintPattern_->SetAntiAlias(anti_);
        }
        return;
    }
    if (isOffscreen_ && offscreenCanvas_) {
        offscreenCanvas_->SetAntiAlias(anti_);
        return;
    }
    if (!isOffscreen_ && pool_) {
        pool_->SetAntiAlias(anti_);
    }
}

void JSCanvasRenderer::JsSetFont(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    std::string fontStr = "";
    JSViewAbstract::ParseJsString(info[0], fontStr);

    std::vector<std::string> fontProps;
    StringUtils::StringSplitter(fontStr.c_str(), ' ', fontProps);
    bool updateFontStyle = false;
    for (const auto& fontProp : fontProps) {
        if (FONT_WEIGHTS.find(fontProp) != FONT_WEIGHTS.end()) {
            auto weight = ConvertStrToFontWeight(fontProp);
            style_.SetFontWeight(weight);
            if (Container::IsCurrentUseNewPipeline()) {
                if (isOffscreen_ && offscreenCanvasPattern_) {
                    offscreenCanvasPattern_->SetFontWeight(weight);
                    continue;
                }
                if (!isOffscreen_ && customPaintPattern_) {
                    customPaintPattern_->UpdateFontWeight(weight);
                }
                continue;
            }
            if (isOffscreen_ && offscreenCanvas_) {
                offscreenCanvas_->SetFontWeight(weight);
                continue;
            }
            if (!isOffscreen_ && pool_) {
                pool_->UpdateFontWeight(weight);
            }
        } else if (FONT_STYLES.find(fontProp) != FONT_STYLES.end()) {
            updateFontStyle = true;
            auto fontStyle = ConvertStrToFontStyle(fontProp);
            style_.SetFontStyle(fontStyle);
            if (Container::IsCurrentUseNewPipeline()) {
                if (isOffscreen_ && offscreenCanvasPattern_) {
                    offscreenCanvasPattern_->SetFontStyle(fontStyle);
                    continue;
                }
                if (!isOffscreen_ && customPaintPattern_) {
                    customPaintPattern_->UpdateFontStyle(fontStyle);
                }
                continue;
            }
            if (isOffscreen_ && offscreenCanvas_) {
                offscreenCanvas_->SetFontStyle(fontStyle);
                continue;
            }
            if (!isOffscreen_ && pool_) {
                pool_->UpdateFontStyle(fontStyle);
            }
        } else if (FONT_FAMILIES.find(fontProp) != FONT_FAMILIES.end()) {
            auto families = ConvertStrToFontFamilies(fontProp);
            style_.SetFontFamilies(families);
            if (Container::IsCurrentUseNewPipeline()) {
                if (isOffscreen_ && offscreenCanvasPattern_) {
                    offscreenCanvasPattern_->SetFontFamilies(families);
                    continue;
                }
                if (!isOffscreen_ && customPaintPattern_) {
                    customPaintPattern_->UpdateFontFamilies(families);
                }
                continue;
            }
            if (isOffscreen_ && offscreenCanvas_) {
                offscreenCanvas_->SetFontFamilies(families);
                continue;
            }
            if (!isOffscreen_ && pool_) {
                pool_->UpdateFontFamilies(families);
            }
        } else if (fontProp.find("px") != std::string::npos) {
            std::string fontSize = fontProp.substr(0, fontProp.size() - 2);
            auto size = Dimension(StringToDouble(fontProp));
            style_.SetFontSize(size);
            if (Container::IsCurrentUseNewPipeline()) {
                if (isOffscreen_ && offscreenCanvasPattern_) {
                    offscreenCanvasPattern_->SetFontSize(size);
                    continue;
                }
                if (!isOffscreen_ && customPaintPattern_) {
                    customPaintPattern_->UpdateFontSize(size);
                }
                continue;
            }
            if (isOffscreen_ && offscreenCanvas_) {
                offscreenCanvas_->SetFontSize(size);
                continue;
            }
            if (!isOffscreen_ && pool_) {
                pool_->UpdateFontSize(size);
            }
        } else {
            LOGW("parse text error");
        }
    }
    if (!updateFontStyle) {
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetFontStyle(FontStyle::NORMAL);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateFontStyle(FontStyle::NORMAL);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetFontStyle(FontStyle::NORMAL);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateFontStyle(FontStyle::NORMAL);
        }
    }
}

void JSCanvasRenderer::JsGetFont(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetFillStyle(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetStrokeStyle(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetLineCap(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetLineDash(const JSCallbackInfo& info)
{
    std::vector<double> lineDash{};
    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            lineDash = offscreenCanvasPattern_->GetLineDash().lineDash;
        } else if (!isOffscreen_ && customPaintPattern_) {
            lineDash = customPaintPattern_->GetLineDash().lineDash;
        }
    } else {
        if (isOffscreen_ && offscreenCanvas_) {
            lineDash = offscreenCanvas_->GetLineDash().lineDash;
        } else if (!isOffscreen_ && pool_) {
            lineDash = pool_->GetLineDash().lineDash;
        }
    }
    JSRef<JSObject> lineDashObj = JSRef<JSObject>::New();
    for (size_t i = 0; i < lineDash.size(); i++) {
        lineDashObj->SetProperty<double>(std::to_string(i).c_str(), lineDash[i]);
    }
    info.SetReturnValue(lineDashObj);
}

void JSCanvasRenderer::JsGetLineJoin(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetMiterLimit(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetLineWidth(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetTextAlign(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetTextBaseline(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetGlobalAlpha(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetGlobalCompositeOperation(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetLineDashOffset(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetShadowBlur(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetShadowColor(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetShadowOffsetX(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetShadowOffsetY(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetImageSmoothingEnabled(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetImageSmoothingQuality(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsSetFillStyle(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    if (info[0]->IsString()) {
        std::string colorStr = "";
        JSViewAbstract::ParseJsString(info[0], colorStr);
        auto color = Color::FromString(colorStr);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetFillColor(color);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateFillColor(color);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetFillColor(color);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateFillColor(color);
        }
        return;
    }
    JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> typeValue = obj->GetProperty("__type");
    std::string type = "";
    JSViewAbstract::ParseJsString(typeValue, type);
    if (type == "gradient") {
        JSCanvasGradient* jSCanvasGradient = JSRef<JSObject>::Cast(info[0])->Unwrap<JSCanvasGradient>();
        if (!jSCanvasGradient) {
            return;
        }
        Gradient* gradient = jSCanvasGradient->GetGradient();
        if (!gradient) {
            return;
        }
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetFillGradient(*gradient);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateFillGradient(*gradient);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetFillGradient(*gradient);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateFillGradient(*gradient);
        }
    } else if (type == "pattern") {
        JSRef<JSVal> typeValue = obj->GetProperty("__id");
        int32_t id;
        ParseJsInt(typeValue, id);
        auto pattern = GetPattern(id);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetFillPattern(pattern);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateFillPattern(pattern);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetFillPattern(pattern);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateFillPattern(pattern);
        }
    } else {
        LOGW("unsupported function for fill style.");
    }
}

void JSCanvasRenderer::JsSetStrokeStyle(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    if (info[0]->IsString()) {
        std::string colorStr = "";
        JSViewAbstract::ParseJsString(info[0], colorStr);
        auto color = Color::FromString(colorStr);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetStrokeColor(color);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateStrokeColor(color);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetStrokeColor(color);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateStrokeColor(color);
        }
        return;
    }
    JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> typeValue = obj->GetProperty("__type");
    std::string type = "";
    JSViewAbstract::ParseJsString(typeValue, type);
    if (type == "gradient") {
        JSCanvasGradient* jSCanvasGradient = JSRef<JSObject>::Cast(info[0])->Unwrap<JSCanvasGradient>();
        if (!jSCanvasGradient) {
            return;
        }
        Gradient* gradient = jSCanvasGradient->GetGradient();
        if (!gradient) {
            return;
        }
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetStrokeGradient(*gradient);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateStrokeGradient(*gradient);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetStrokeGradient(*gradient);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateStrokeGradient(*gradient);
        }
    } else if (type == "pattern") {
        JSRef<JSVal> typeValue = obj->GetProperty("__id");
        int32_t id;
        ParseJsInt(typeValue, id);
        auto pattern = GetPattern(id);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetStrokePattern(pattern);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateStrokePattern(pattern);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetStrokePattern(pattern);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateStrokePattern(pattern);
        }
    } else {
        LOGW("unsupported function for stroke style.");
    }
}

RefPtr<CanvasPath2D> JSCanvasRenderer::JsMakePath2D(const JSCallbackInfo& info)
{
    if (info.Length() == 1) {
        if (info[0]->IsString()) {
            std::string capStr = "";
            JSViewAbstract::ParseJsString(info[0], capStr);
            return AceType::MakeRefPtr<CanvasPath2D>(capStr);
        }
    }
    // Example: ctx.createPath2D()
    return AceType::MakeRefPtr<CanvasPath2D>();
}

void JSCanvasRenderer::JsDrawImage(const JSCallbackInfo& info)
{
    CanvasImage image;
    double imgWidth;
    double imgHeight;
    RefPtr<PixelMap> pixelMap = nullptr;
    bool isImage = false;
    if (!info[0]->IsObject()) {
        LOGE("The arg is not Object or String.");
        return;
    }
    JSRenderImage* jsImage = JSRef<JSObject>::Cast(info[0])->Unwrap<JSRenderImage>();
    if (jsImage) {
        isImage = true;
        std::string imageValue = jsImage->GetSrc();
        image.src = imageValue;
        imgWidth = jsImage->GetWidth();
        imgHeight = jsImage->GetHeight();
    } else {
#if !defined(PREVIEW)
        pixelMap = CreatePixelMapFromNapiValue(info[0]);
#endif
        if (!pixelMap) {
            LOGE("pixelMap is null");
            return;
        }
    }
    ExtractInfoToImage(image, info, isImage);
    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            if (isImage) {
                offscreenCanvasPattern_->DrawImage(image, imgWidth, imgHeight);
                return;
            }
            offscreenCanvasPattern_->DrawPixelMap(pixelMap, image);
            return;
        }
        if (!isOffscreen_ && customPaintPattern_) {
            if (isImage) {
                customPaintPattern_->DrawImage(image, imgWidth, imgHeight);
                return;
            }
            customPaintPattern_->DrawPixelMap(pixelMap, image);
        }
        return;
    }
    if (isOffscreen_ && offscreenCanvas_) {
        if (isImage) {
            offscreenCanvas_->DrawImage(image, imgWidth, imgHeight);
            return;
        }
        offscreenCanvas_->DrawPixelMap(pixelMap, image);
        return;
    }
    if (!isOffscreen_ && pool_) {
        if (isImage) {
            pool_->DrawImage(image, imgWidth, imgHeight);
            return;
        }
        pool_->DrawPixelMap(pixelMap, image);
    }
}

void JSCanvasRenderer::ExtractInfoToImage(CanvasImage& image, const JSCallbackInfo& info, bool isImage)
{
    switch (info.Length()) {
        case 3:
            image.flag = 0;
            JSViewAbstract::ParseJsDouble(info[1], image.dx);
            JSViewAbstract::ParseJsDouble(info[2], image.dy);
            image.dx = SystemProperties::Vp2Px(image.dx);
            image.dy = SystemProperties::Vp2Px(image.dy);
            break;
        // 5 parameters: drawImage(image, dx, dy, dWidth, dHeight)
        case 5:
            image.flag = 1;
            JSViewAbstract::ParseJsDouble(info[1], image.dx);
            JSViewAbstract::ParseJsDouble(info[2], image.dy);
            JSViewAbstract::ParseJsDouble(info[3], image.dWidth);
            JSViewAbstract::ParseJsDouble(info[4], image.dHeight);
            image.dx = SystemProperties::Vp2Px(image.dx);
            image.dy = SystemProperties::Vp2Px(image.dy);
            image.dWidth = SystemProperties::Vp2Px(image.dWidth);
            image.dHeight = SystemProperties::Vp2Px(image.dHeight);
            break;
        // 9 parameters: drawImage(image, sx, sy, sWidth, sHeight, dx, dy, dWidth, dHeight)
        case 9:
            image.flag = 2;
            JSViewAbstract::ParseJsDouble(info[1], image.sx);
            JSViewAbstract::ParseJsDouble(info[2], image.sy);
            JSViewAbstract::ParseJsDouble(info[3], image.sWidth);
            JSViewAbstract::ParseJsDouble(info[4], image.sHeight);
            JSViewAbstract::ParseJsDouble(info[5], image.dx);
            JSViewAbstract::ParseJsDouble(info[6], image.dy);
            JSViewAbstract::ParseJsDouble(info[7], image.dWidth);
            JSViewAbstract::ParseJsDouble(info[8], image.dHeight);
            if (isImage) {
                image.sx = SystemProperties::Vp2Px(image.sx);
                image.sy = SystemProperties::Vp2Px(image.sy);
                image.sWidth = SystemProperties::Vp2Px(image.sWidth);
                image.sHeight = SystemProperties::Vp2Px(image.sHeight);
            }
            image.dx = SystemProperties::Vp2Px(image.dx);
            image.dy = SystemProperties::Vp2Px(image.dy);
            image.dWidth = SystemProperties::Vp2Px(image.dWidth);
            image.dHeight = SystemProperties::Vp2Px(image.dHeight);
            break;
        default:
            break;
    }
}

void JSCanvasRenderer::JsCreatePattern(const JSCallbackInfo& info)
{
    if (info[0]->IsObject()) {
        JSRenderImage* jsImage = JSRef<JSObject>::Cast(info[0])->Unwrap<JSRenderImage>();
        if (jsImage == nullptr) {
            LOGE("jsImage is null");
            return;
        }
        std::string imageSrc = jsImage->GetSrc();
        double imgWidth = jsImage->GetWidth();
        double imgHeight = jsImage->GetHeight();
        std::string repeat;

        JSViewAbstract::ParseJsString(info[1], repeat);
        pattern_[patternCount_].SetImgSrc(imageSrc);
        pattern_[patternCount_].SetImageWidth(imgWidth);
        pattern_[patternCount_].SetImageHeight(imgHeight);
        pattern_[patternCount_].SetRepetition(repeat);

        auto retObj = JSRef<JSObject>::New();
        retObj->SetProperty("__type", "pattern");
        retObj->SetProperty("__id", patternCount_);
        patternCount_++;
        info.SetReturnValue(retObj);
    }
}

void JSCanvasRenderer::JsCreateImageData(const JSCallbackInfo& info)
{
    double width = 0;
    double height = 0;

    if (info.Length() == 2) {
        JSViewAbstract::ParseJsDouble(info[0], width);
        JSViewAbstract::ParseJsDouble(info[1], height);
        width = SystemProperties::Vp2Px(width);
        height = SystemProperties::Vp2Px(height);

    }
    if (info.Length() == 1 && info[0]->IsObject()) {
        width = imageData_.dirtyWidth;
        height = imageData_.dirtyHeight;
    }

    auto container = Container::Current();
    if (!container) {
        LOGW("container is null");
        return;
    }

    imageData_.dirtyWidth = width;
    imageData_.dirtyHeight = height;

    JSRef<JSArray> colorArray = JSRef<JSArray>::New();
    uint32_t count = 0;
    for (auto i = 0; i < width; i++) {
        for (auto j = 0; j < height; j++) {
            colorArray->SetValueAt(count, JSRef<JSVal>::Make(ToJSValue(255)));
            colorArray->SetValueAt(count + 1, JSRef<JSVal>::Make(ToJSValue(255)));
            colorArray->SetValueAt(count + 2, JSRef<JSVal>::Make(ToJSValue(255)));
            colorArray->SetValueAt(count + 3, JSRef<JSVal>::Make(ToJSValue(255)));
            count += 4;
        }
    }

    auto retObj = JSRef<JSObject>::New();
    retObj->SetProperty("width", width);
    retObj->SetProperty("height", height);
    retObj->SetPropertyObject("data", colorArray);
    info.SetReturnValue(retObj);
}

void JSCanvasRenderer::JsPutImageData(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    int32_t width = 0;
    int32_t height = 0;
    JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> widthValue = obj->GetProperty("width");
    JSRef<JSVal> heightValue = obj->GetProperty("height");
    ParseJsInt(widthValue, width);
    ParseJsInt(heightValue, height);

    ImageData imageData;
    std::vector<uint32_t> array;
    ParseImageData(info, imageData, array);

    int64_t num = 0;
    for (int32_t i = 0; i < height; ++i) {
        for (int32_t j = 0; j < width; ++j) {
            if ((i >= imageData.dirtyY) && (i - imageData.dirtyY < imageData.dirtyHeight) && (j >= imageData.dirtyX) &&
                (j - imageData.dirtyX < imageData.dirtyWidth)) {
                int32_t flag = j + width * i;
                if (array.size() > static_cast<uint32_t>(4 * flag + 3)) {
                    auto red = array[4 * flag];
                    auto green = array[4 * flag + 1];
                    auto blue = array[4 * flag + 2];
                    auto alpha = array[4 * flag + 3];
                    if (num < imageData.dirtyWidth * imageData.dirtyHeight) {
                        imageData.data.emplace_back(Color::FromARGB(alpha, red, green, blue));
                    }
                    num++;
                }
            }
        }
    }

    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            offscreenCanvasPattern_->PutImageData(imageData);
            return;
        }
        if (!isOffscreen_ && customPaintPattern_) {
            customPaintPattern_->PutImageData(imageData);
        }
        return;
    }
    if (isOffscreen_ && offscreenCanvas_) {
        offscreenCanvas_->PutImageData(imageData);
        return;
    }
    if (!isOffscreen_ && pool_) {
        pool_->PutImageData(imageData);
    }
}

void JSCanvasRenderer::ParseImageData(const JSCallbackInfo& info, ImageData& imageData, std::vector<uint32_t>& array)
{
    int32_t width = 0;
    int32_t height = 0;

    if (info[0]->IsObject()) {
        JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[0]);
        JSRef<JSVal> widthValue = obj->GetProperty("width");
        JSRef<JSVal> heightValue = obj->GetProperty("height");
        JSRef<JSVal> dataValue = obj->GetProperty("data");
        ParseJsInt(widthValue, width);
        ParseJsInt(heightValue, height);
        JSViewAbstract::ParseJsIntegerArray(dataValue, array);
    }

    ParseJsInt(info[1], imageData.x);
    ParseJsInt(info[2], imageData.y);
    imageData.x = SystemProperties::Vp2Px(imageData.x);
    imageData.y = SystemProperties::Vp2Px(imageData.y);

    imageData.dirtyWidth = width;
    imageData.dirtyHeight = height;

    if (info.Length() == 7) {
        ParseJsInt(info[3], imageData.dirtyX);
        ParseJsInt(info[4], imageData.dirtyY);
        ParseJsInt(info[5], imageData.dirtyWidth);
        ParseJsInt(info[6], imageData.dirtyHeight);
        imageData.dirtyX = SystemProperties::Vp2Px(imageData.dirtyX);
        imageData.dirtyY = SystemProperties::Vp2Px(imageData.dirtyY);
        imageData.dirtyWidth = SystemProperties::Vp2Px(imageData.dirtyWidth);
        imageData.dirtyHeight = SystemProperties::Vp2Px(imageData.dirtyHeight);
    }

    imageData.dirtyWidth = imageData.dirtyX < 0 ? std::min(imageData.dirtyX + imageData.dirtyWidth, width)
                                                : std::min(width - imageData.dirtyX, imageData.dirtyWidth);
    imageData.dirtyHeight = imageData.dirtyY < 0 ? std::min(imageData.dirtyY + imageData.dirtyHeight, height)
                                                 : std::min(height - imageData.dirtyY, imageData.dirtyHeight);
}

void JSCanvasRenderer::JsGetImageData(const JSCallbackInfo& info)
{
    double fLeft = 0.0;
    double fTop = 0.0;
    double fWidth = 0.0;
    double fHeight = 0.0;
    uint32_t final_width = 0.0;
    uint32_t final_height = 0.0;
    int32_t left = 0;
    int32_t top = 0;
    int32_t width = 0;
    int32_t height = 0;

    JSViewAbstract::ParseJsDouble(info[0], fLeft);
    JSViewAbstract::ParseJsDouble(info[1], fTop);
    JSViewAbstract::ParseJsDouble(info[2], fWidth);
    JSViewAbstract::ParseJsDouble(info[3], fHeight);

    left = fLeft;
    top = fTop;
    width = fWidth;
    height = fHeight;

    left = SystemProperties::Vp2Px(left);
    top = SystemProperties::Vp2Px(top);
    width = SystemProperties::Vp2Px(width);
    height = SystemProperties::Vp2Px(height);

    std::unique_ptr<ImageData> data;
    data = GetImageDataFromCanvas(left, top, width, height);

    final_height = static_cast<uint32_t>(data->dirtyHeight);
    final_width = static_cast<uint32_t>(data->dirtyWidth);

    JSRef<JSArray> colorArray = JSRef<JSArray>::New();
    uint32_t count = 0;
    for (uint32_t i = 0; i < final_height; i++) {
        for (uint32_t j = 0; j < final_width; j++) {
            int32_t idx = i * data->dirtyWidth + j;
            auto pixel = data->data[idx];

            colorArray->SetValueAt(count, JSRef<JSVal>::Make(ToJSValue(pixel.GetRed())));
            colorArray->SetValueAt(count + 1, JSRef<JSVal>::Make(ToJSValue(pixel.GetGreen())));
            colorArray->SetValueAt(count + 2, JSRef<JSVal>::Make(ToJSValue(pixel.GetBlue())));
            colorArray->SetValueAt(count + 3, JSRef<JSVal>::Make(ToJSValue(pixel.GetAlpha())));
            count += 4;
        }
    }

    auto retObj = JSRef<JSObject>::New();
    retObj->SetProperty("width", final_width);
    retObj->SetProperty("height", final_height);
    retObj->SetPropertyObject("data", colorArray);
    info.SetReturnValue(retObj);
}

std::unique_ptr<ImageData> JSCanvasRenderer::GetImageDataFromCanvas(
    const double& left, const double& top, const double& width, const double& height)
{
    std::unique_ptr<ImageData> data;

    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            data = offscreenCanvasPattern_->GetImageData(left, top, width, height);
        } else if (!isOffscreen_ && customPaintPattern_) {
            data = customPaintPattern_->GetImageData(left, top, width, height);
        }
    } else {
        if (isOffscreen_ && offscreenCanvas_) {
            data = offscreenCanvas_->GetImageData(left, top, width, height);
        } else if (!isOffscreen_ && pool_) {
            data = pool_->GetImageData(left, top, width, height);
        }
    }
    return data;
}

void JSCanvasRenderer::JsGetPixelMap(const JSCallbackInfo& info)
{
#ifdef PIXEL_MAP_SUPPORTED
    // 0 Get input param
    double fLeft = 0.0;
    double fTop = 0.0;
    double fWidth = 0.0;
    double fHeight = 0.0;
    int32_t left = 0;
    int32_t top = 0;
    int32_t width = 0;
    int32_t height = 0;
    uint32_t final_width = 0.0;
    uint32_t final_height = 0.0;

    JSViewAbstract::ParseJsDouble(info[0], fLeft);
    JSViewAbstract::ParseJsDouble(info[1], fTop);
    JSViewAbstract::ParseJsDouble(info[2], fWidth);
    JSViewAbstract::ParseJsDouble(info[3], fHeight);

    fLeft = SystemProperties::Vp2Px(fLeft);
    fTop = SystemProperties::Vp2Px(fTop);
    fWidth = SystemProperties::Vp2Px(fWidth);
    fHeight = SystemProperties::Vp2Px(fHeight);

    left = fLeft;
    top = fTop;
    width = round(fWidth);
    height = round(fHeight);

    // 1 Get data from canvas
    std::unique_ptr<ImageData> canvasData;
    canvasData = GetImageDataFromCanvas(left, top, width, height);

    final_height = static_cast<uint32_t>(canvasData->dirtyHeight);
    final_width = static_cast<uint32_t>(canvasData->dirtyWidth);
    uint32_t length = final_height * final_width;
    uint32_t* data = new uint32_t[length];
    for (uint32_t i = 0; i < final_height; i++) {
        for (uint32_t j = 0; j < final_width; j++) {
            uint32_t idx = i * final_width + j;
            Color pixel = canvasData->data[idx];
            data[idx] = pixel.GetValue();
        }
    }

    // 2 Create pixelmap
    OHOS::Media::InitializationOptions options;
    options.alphaType = OHOS::Media::AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    options.pixelFormat = OHOS::Media::PixelFormat::RGBA_8888;
    options.scaleMode = OHOS::Media::ScaleMode::CENTER_CROP;
    options.size.width = static_cast<int32_t>(final_width);
    options.size.height = static_cast<int32_t>(final_height);
    options.editable = true;
    std::unique_ptr<OHOS::Media::PixelMap> pixelmap = OHOS::Media::PixelMap::Create(data, length, options);
    delete[] data;

    // 3 pixelmap to NapiValue
    auto engine = EngineHelper::GetCurrentEngine();
    if (!engine) {
        LOGE("JsGetPixelMap engine is null");
        return;
    }
    NativeEngine* nativeEngine = engine->GetNativeEngine();
    napi_env env = reinterpret_cast<napi_env>(nativeEngine);
    std::shared_ptr<OHOS::Media::PixelMap> sharedPixelmap(pixelmap.release());
    napi_value napiValue = OHOS::Media::PixelMapNapi::CreatePixelMap(env, sharedPixelmap);

    // 4 NapiValue to JsValue
#ifdef USE_ARK_ENGINE
    NativeValue* nativeValue = reinterpret_cast<NativeValue*>(napiValue);
    auto jsValue = JsConverter::ConvertNativeValueToJsVal(nativeValue);
    info.SetReturnValue(jsValue);
#else
    napi_value temp = nullptr;
    napi_create_int32(env, 0, &temp);
    napi_set_named_property(env, napiValue, "index", temp);
#endif

#endif
}

void JSCanvasRenderer::JsSetPixelMap(const JSCallbackInfo& info)
{
    if (info.Length() != 1) {
        LOGE("The argv is wrong, it is supposed to have 1 argument");
        return;
    }
    CanvasImage image;
    RefPtr<PixelMap> pixelMap = nullptr;
    if (info[0]->IsObject()) {
#if !defined(PREVIEW)
        pixelMap = CreatePixelMapFromNapiValue(info[0]);
#endif
        if (!pixelMap) {
            LOGE("pixelMap is null");
            return;
        }

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->DrawPixelMap(pixelMap, image);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->DrawPixelMap(pixelMap, image);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->DrawPixelMap(pixelMap, image);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->DrawPixelMap(pixelMap, image);
        }
    }
}

void JSCanvasRenderer::JsDrawBitmapMesh(const JSCallbackInfo& info)
{
    RefPtr<OffscreenCanvas> offscreenCanvas;

    if (info.Length() != 4) {
        LOGE("info.Length is not right %{public}d", info.Length());
        return;
    }

    if (info[0]->IsObject()) {
        uint32_t id = 0;
        JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[0]);
        JSRef<JSVal> jsId = obj->GetProperty("__id");
        JSViewAbstract::ParseJsInteger(jsId, id);
        offscreenCanvas = JSOffscreenRenderingContext::GetOffscreenCanvas(id);
    } else {
        LOGE("info 0 is not object");
        return;
    }

    std::vector<double> mesh;
    double column;
    double row;
    if (!ParseJsDoubleArray(info[1], mesh)) {
        LOGE("info 1 is not double array");
        return;
    }
    if (!JSViewAbstract::ParseJsDouble(info[2], column)) {
        LOGE("info 2 is not double");
        return;
    }
    if (!JSViewAbstract::ParseJsDouble(info[3], row)) {
        LOGE("info 3 is not double");
        return;
    }
    if (pool_) {
        pool_->DrawBitmapMesh(offscreenCanvas, mesh, column, row);
        return;
    }
}

void JSCanvasRenderer::JsFilter(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsDirection(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsGetJsonData(const JSCallbackInfo& info)
{
    std::string path = "";
    std::string jsonData = "";

    if (info[0]->IsString()) {
        JSViewAbstract::ParseJsString(info[0], path);
        if (Container::IsCurrentUseNewPipeline() && !isOffscreen_ && customPaintPattern_) {
            jsonData = customPaintPattern_->GetJsonData(path);
        } else if (!isOffscreen_ && pool_) {
            jsonData = pool_->GetJsonData(path);
        }
        auto returnValue = JSVal(ToJSValue(jsonData));
        auto returnPtr = JSRef<JSVal>::Make(returnValue);
        info.SetReturnValue(returnPtr);
    }
}

void JSCanvasRenderer::JsToDataUrl(const JSCallbackInfo& info)
{
    std::string dataUrl = "";
    std::string result = "";
    double quality = DEFAULT_QUALITY;
    if (info[0]->IsString()) {
        JSViewAbstract::ParseJsString(info[0], dataUrl);
    }
    if (info.Length() > 1 && info[1]->IsNumber()) {
        JSViewAbstract::ParseJsDouble(info[1], quality);
    }

    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            result = offscreenCanvasPattern_->ToDataURL(dataUrl, quality);
        } else if (!isOffscreen_ && customPaintPattern_) {
            result = customPaintPattern_->ToDataURL(dataUrl + "," + std::to_string(quality));
        }
    } else {
        if (isOffscreen_ && offscreenCanvas_) {
            result = offscreenCanvas_->ToDataURL(dataUrl, quality);
        } else if (!isOffscreen_ && pool_) {
            result = pool_->ToDataURL(dataUrl + "," + std::to_string(quality));
        }
    }
    auto returnValue = JSVal(ToJSValue(result));
    auto returnPtr = JSRef<JSVal>::Make(returnValue);
    info.SetReturnValue(returnPtr);
}

void JSCanvasRenderer::JsSetLineCap(const JSCallbackInfo& info)
{
    if (info[0]->IsString()) {
        std::string capStr = "";
        JSViewAbstract::ParseJsString(info[0], capStr);
        static const LinearMapNode<LineCapStyle> lineCapTable[] = {
            { "butt", LineCapStyle::BUTT },
            { "round", LineCapStyle::ROUND },
            { "square", LineCapStyle::SQUARE },
        };
        auto lineCap = ConvertStrToEnum(capStr.c_str(), lineCapTable, ArraySize(lineCapTable), LineCapStyle::BUTT);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetLineCap(lineCap);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateLineCap(lineCap);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetLineCap(lineCap);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateLineCap(lineCap);
        }
    }
}

void JSCanvasRenderer::JsSetLineJoin(const JSCallbackInfo& info)
{
    if (info[0]->IsString()) {
        std::string joinStr = "";
        JSViewAbstract::ParseJsString(info[0], joinStr);
        static const LinearMapNode<LineJoinStyle> lineJoinTable[3] = {
            { "bevel", LineJoinStyle::BEVEL },
            { "miter", LineJoinStyle::MITER },
            { "round", LineJoinStyle::ROUND },
        };
        auto lineJoin = ConvertStrToEnum(
            joinStr.c_str(), lineJoinTable, ArraySize(lineJoinTable), LineJoinStyle::MITER);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetLineJoin(lineJoin);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateLineJoin(lineJoin);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetLineJoin(lineJoin);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateLineJoin(lineJoin);
        }
    }
}

void JSCanvasRenderer::JsSetMiterLimit(const JSCallbackInfo& info)
{
    if (info[0]->IsNumber()) {
        double limit = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], limit);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetMiterLimit(limit);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateMiterLimit(limit);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetMiterLimit(limit);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateMiterLimit(limit);
        }
    }
}

void JSCanvasRenderer::JsSetLineWidth(const JSCallbackInfo& info)
{
    if (info[0]->IsNumber()) {
        double lineWidth = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], lineWidth);
        lineWidth = SystemProperties::Vp2Px(lineWidth);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetLineWidth(lineWidth);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateLineWidth(lineWidth);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetLineWidth(lineWidth);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateLineWidth(lineWidth);
        }
    }
}

void JSCanvasRenderer::JsSetGlobalAlpha(const JSCallbackInfo& info)
{
    if (info[0]->IsNumber()) {
        double alpha = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], alpha);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetAlpha(alpha);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateGlobalAlpha(alpha);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetAlpha(alpha);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateGlobalAlpha(alpha);
        }
    }
}

void JSCanvasRenderer::JsSetGlobalCompositeOperation(const JSCallbackInfo& info)
{
    if (info[0]->IsString()) {
        std::string compositeStr = "";
        JSViewAbstract::ParseJsString(info[0], compositeStr);

        static const LinearMapNode<CompositeOperation> compositeOperationTable[] = {
        { "copy", CompositeOperation::COPY },
        { "destination-atop", CompositeOperation::DESTINATION_ATOP },
        { "destination-in", CompositeOperation::DESTINATION_IN },
        { "destination-out", CompositeOperation::DESTINATION_OUT },
        { "destination-over", CompositeOperation::DESTINATION_OVER },
        { "lighter", CompositeOperation::LIGHTER },
        { "source-atop", CompositeOperation::SOURCE_ATOP },

        { "source-in", CompositeOperation::SOURCE_IN },
        { "source-out", CompositeOperation::SOURCE_OUT },
        { "source-over", CompositeOperation::SOURCE_OVER },
        { "xor", CompositeOperation::XOR },
        };
        auto type = ConvertStrToEnum(
            compositeStr.c_str(), compositeOperationTable,
            ArraySize(compositeOperationTable), CompositeOperation::SOURCE_OVER);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetCompositeType(type);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateCompositeOperation(type);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetCompositeType(type);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateCompositeOperation(type);
        }
    }
}

void JSCanvasRenderer::JsSetLineDashOffset(const JSCallbackInfo& info)
{
    if (info[0]->IsNumber()) {
        double lineDashOffset = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], lineDashOffset);
        lineDashOffset = SystemProperties::Vp2Px(lineDashOffset);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetLineDashOffset(lineDashOffset);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateLineDashOffset(lineDashOffset);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetLineDashOffset(lineDashOffset);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateLineDashOffset(lineDashOffset);
        }
    }
}

void JSCanvasRenderer::JsSetShadowBlur(const JSCallbackInfo& info)
{
    if (info[0]->IsNumber()) {
        double blur = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], blur);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetShadowBlur(blur);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateShadowBlur(blur);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetShadowBlur(blur);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateShadowBlur(blur);
        }
    }
}

void JSCanvasRenderer::JsSetShadowColor(const JSCallbackInfo& info)
{
    if (info[0]->IsString()) {
        std::string colorStr = "";
        JSViewAbstract::ParseJsString(info[0], colorStr);
        auto color = Color::FromString(colorStr);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetShadowColor(color);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateShadowColor(color);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetShadowColor(color);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateShadowColor(color);
        }
    }
}

void JSCanvasRenderer::JsSetShadowOffsetX(const JSCallbackInfo& info)
{
    if (info[0]->IsNumber()) {
        double offsetX = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], offsetX);
        offsetX = SystemProperties::Vp2Px(offsetX);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetShadowOffsetX(offsetX);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateShadowOffsetX(offsetX);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetShadowOffsetX(offsetX);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateShadowOffsetX(offsetX);
        }
    }
}

void JSCanvasRenderer::JsSetShadowOffsetY(const JSCallbackInfo& info)
{
    if (info[0]->IsNumber()) {
        double offsetY = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], offsetY);
        offsetY = SystemProperties::Vp2Px(offsetY);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetShadowOffsetY(offsetY);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateShadowOffsetY(offsetY);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetShadowOffsetY(offsetY);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateShadowOffsetY(offsetY);
        }
    }
}

void JSCanvasRenderer::JsSetImageSmoothingEnabled(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }

    bool enabled = false;
    if (JSViewAbstract::ParseJsBool(info[0], enabled)) {
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetSmoothingEnabled(enabled);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateSmoothingEnabled(enabled);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetSmoothingEnabled(enabled);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateSmoothingEnabled(enabled);
        }
    }
}

void JSCanvasRenderer::JsSetImageSmoothingQuality(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }

    std::string quality = "";
    if (JSViewAbstract::ParseJsString(info[0], quality)) {
        if (QUALITY_TYPE.find(quality) == QUALITY_TYPE.end()) {
            return;
        }
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetSmoothingQuality(quality);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateSmoothingQuality(quality);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetSmoothingQuality(quality);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateSmoothingQuality(quality);
        }
    }
}

void JSCanvasRenderer::JsMoveTo(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber()) {
        double x = 0.0;
        double y = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], x);
        JSViewAbstract::ParseJsDouble(info[1], y);
        x = SystemProperties::Vp2Px(x);
        y = SystemProperties::Vp2Px(y);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->MoveTo(x, y);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->MoveTo(x, y);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->MoveTo(x, y);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->MoveTo(x, y);
        }
    }
}

void JSCanvasRenderer::JsLineTo(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber()) {
        double x = 0.0;
        double y = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], x);
        JSViewAbstract::ParseJsDouble(info[1], y);
        x = SystemProperties::Vp2Px(x);
        y = SystemProperties::Vp2Px(y);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->LineTo(x, y);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->LineTo(x, y);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->LineTo(x, y);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->LineTo(x, y);
        }
    }
}

void JSCanvasRenderer::JsBezierCurveTo(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()
        && info[4]->IsNumber() && info[5]->IsNumber()) {
        BezierCurveParam param;
        JSViewAbstract::ParseJsDouble(info[0], param.cp1x);
        JSViewAbstract::ParseJsDouble(info[1], param.cp1y);
        JSViewAbstract::ParseJsDouble(info[2], param.cp2x);
        JSViewAbstract::ParseJsDouble(info[3], param.cp2y);
        JSViewAbstract::ParseJsDouble(info[4], param.x);
        JSViewAbstract::ParseJsDouble(info[5], param.y);
        param.cp1x = SystemProperties::Vp2Px(param.cp1x);
        param.cp1y = SystemProperties::Vp2Px(param.cp1y);
        param.cp2x = SystemProperties::Vp2Px(param.cp2x);
        param.cp2y = SystemProperties::Vp2Px(param.cp2y);
        param.x = SystemProperties::Vp2Px(param.x);
        param.y = SystemProperties::Vp2Px(param.y);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->BezierCurveTo(param);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->BezierCurveTo(param);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->BezierCurveTo(param);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->BezierCurveTo(param);
        }
    }
}

void JSCanvasRenderer::JsQuadraticCurveTo(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()) {
        QuadraticCurveParam param;
        JSViewAbstract::ParseJsDouble(info[0], param.cpx);
        JSViewAbstract::ParseJsDouble(info[1], param.cpy);
        JSViewAbstract::ParseJsDouble(info[2], param.x);
        JSViewAbstract::ParseJsDouble(info[3], param.y);
        param.cpx = SystemProperties::Vp2Px(param.cpx);
        param.cpy = SystemProperties::Vp2Px(param.cpy);
        param.x = SystemProperties::Vp2Px(param.x);
        param.y = SystemProperties::Vp2Px(param.y);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->QuadraticCurveTo(param);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->QuadraticCurveTo(param);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->QuadraticCurveTo(param);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->QuadraticCurveTo(param);
        }
    }
}

void JSCanvasRenderer::JsArcTo(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()
        && info[4]->IsNumber()) {
        ArcToParam param;
        JSViewAbstract::ParseJsDouble(info[0], param.x1);
        JSViewAbstract::ParseJsDouble(info[1], param.y1);
        JSViewAbstract::ParseJsDouble(info[2], param.x2);
        JSViewAbstract::ParseJsDouble(info[3], param.y2);
        JSViewAbstract::ParseJsDouble(info[4], param.radius);
        param.x1 = SystemProperties::Vp2Px(param.x1);
        param.y1 = SystemProperties::Vp2Px(param.y1);
        param.x2 = SystemProperties::Vp2Px(param.x2);
        param.y2 = SystemProperties::Vp2Px(param.y2);
        param.radius = SystemProperties::Vp2Px(param.radius);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->ArcTo(param);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->ArcTo(param);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->ArcTo(param);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->ArcTo(param);
        }
    }
}

void JSCanvasRenderer::JsArc(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()
        && info[4]->IsNumber()) {
        ArcParam param;
        JSViewAbstract::ParseJsDouble(info[0], param.x);
        JSViewAbstract::ParseJsDouble(info[1], param.y);
        JSViewAbstract::ParseJsDouble(info[2], param.radius);
        JSViewAbstract::ParseJsDouble(info[3], param.startAngle);
        JSViewAbstract::ParseJsDouble(info[4], param.endAngle);
        param.x = SystemProperties::Vp2Px(param.x);
        param.y = SystemProperties::Vp2Px(param.y);
        param.radius = SystemProperties::Vp2Px(param.radius);

        if (info.Length() == 6) {
            JSViewAbstract::ParseJsBool(info[5], param.anticlockwise);
        }

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->Arc(param);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->Arc(param);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->Arc(param);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->Arc(param);
        }
    }
}

void JSCanvasRenderer::JsEllipse(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()
        && info[4]->IsNumber() && info[5]->IsNumber() && info[6]->IsNumber()) {
        EllipseParam param;
        JSViewAbstract::ParseJsDouble(info[0], param.x);
        JSViewAbstract::ParseJsDouble(info[1], param.y);
        JSViewAbstract::ParseJsDouble(info[2], param.radiusX);
        JSViewAbstract::ParseJsDouble(info[3], param.radiusY);
        JSViewAbstract::ParseJsDouble(info[4], param.rotation);
        JSViewAbstract::ParseJsDouble(info[5], param.startAngle);
        JSViewAbstract::ParseJsDouble(info[6], param.endAngle);
        param.x = SystemProperties::Vp2Px(param.x);
        param.y = SystemProperties::Vp2Px(param.y);
        param.radiusX = SystemProperties::Vp2Px(param.radiusX);
        param.radiusY = SystemProperties::Vp2Px(param.radiusY);

        if (info.Length() == 8) {
            JSViewAbstract::ParseJsBool(info[7], param.anticlockwise);
        }

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->Ellipse(param);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->Ellipse(param);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->Ellipse(param);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->Ellipse(param);
        }
    }
}

void JSCanvasRenderer::JsFill(const JSCallbackInfo& info)
{
    std::string ruleStr = "";
    if (info.Length() == 1 && info[0]->IsString()) {
        // fill(rule) uses fillRule specified by the application developers
        JSViewAbstract::ParseJsString(info[0], ruleStr);
    } else if (info.Length() == 2) {
        // fill(path, rule) uses fillRule specified by the application developers
        JSViewAbstract::ParseJsString(info[1], ruleStr);
    }
    auto fillRule = CanvasFillRule::NONZERO;
    if (ruleStr == "nonzero") {
        fillRule = CanvasFillRule::NONZERO;
    } else if (ruleStr == "evenodd") {
        fillRule = CanvasFillRule::EVENODD;
    }

    if (info.Length() == 0 ||
        (info.Length() == 1 && info[0]->IsString())) {
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetFillRuleForPath(fillRule);
                offscreenCanvasPattern_->Fill();
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateFillRuleForPath(fillRule);
                customPaintPattern_->Fill();
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetFillRuleForPath(fillRule);
            offscreenCanvas_->Fill();
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateFillRuleForPath(fillRule);
            pool_->Fill();
        }
    } else if (info.Length() == 2 ||
        (info.Length() == 1 && info[0]->IsObject())) {
        JSPath2D* jsCanvasPath = JSRef<JSObject>::Cast(info[0])->Unwrap<JSPath2D>();
        if (jsCanvasPath == nullptr) {
            LOGE("The arg is wrong, it is supposed to have JSPath2D argument");
            return;
        }
        auto path = jsCanvasPath->GetCanvasPath2d();

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetFillRuleForPath2D(fillRule);
                offscreenCanvasPattern_->Fill(path);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateFillRuleForPath2D(fillRule);
                customPaintPattern_->Fill(path);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetFillRuleForPath2D(fillRule);
            offscreenCanvas_->Fill(path);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateFillRuleForPath2D(fillRule);
            pool_->Fill(path);
        }
    }
}

void JSCanvasRenderer::JsStroke(const JSCallbackInfo& info)
{
    // stroke always uses non-zero fillRule
    auto fillRule = CanvasFillRule::NONZERO;
    if (info.Length() == 1) {
        JSPath2D* jsCanvasPath = JSRef<JSObject>::Cast(info[0])->Unwrap<JSPath2D>();
        if (jsCanvasPath == nullptr) {
            LOGE("The arg is wrong, it is supposed to have JSPath2D argument");
            return;
        }
        auto path = jsCanvasPath->GetCanvasPath2d();

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetFillRuleForPath(fillRule);
                offscreenCanvasPattern_->Stroke(path);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateFillRuleForPath2D(fillRule);
                customPaintPattern_->Stroke(path);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetFillRuleForPath(fillRule);
            offscreenCanvas_->Stroke(path);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateFillRuleForPath2D(fillRule);
            pool_->Stroke(path);
        }
        return;
    }

    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            offscreenCanvasPattern_->SetFillRuleForPath(fillRule);
            offscreenCanvasPattern_->Stroke();
            return;
        }
        if (!isOffscreen_ && customPaintPattern_) {
            customPaintPattern_->UpdateFillRuleForPath(fillRule);
            customPaintPattern_->Stroke();
        }
        return;
    }
    if (isOffscreen_ && offscreenCanvas_) {
        offscreenCanvas_->SetFillRuleForPath2D(fillRule);
        offscreenCanvas_->Stroke();
        return;
    }
    if (!isOffscreen_ && pool_) {
        pool_->UpdateFillRuleForPath(fillRule);
        pool_->Stroke();
    }
}

void JSCanvasRenderer::JsClip(const JSCallbackInfo& info)
{
    std::string ruleStr = "";
    if (info.Length() == 1 && info[0]->IsString()) {
        // clip(rule) uses fillRule specified by the application developers
        JSViewAbstract::ParseJsString(info[0], ruleStr);
    } else if (info.Length() == 2) {
        // clip(path, rule) uses fillRule specified by the application developers
        JSViewAbstract::ParseJsString(info[1], ruleStr);
    }
    auto fillRule = CanvasFillRule::NONZERO;
    if (ruleStr == "nonzero") {
        fillRule = CanvasFillRule::NONZERO;
    } else if (ruleStr == "evenodd") {
        fillRule = CanvasFillRule::EVENODD;
    }

    if (info.Length() == 0 ||
        (info.Length() == 1 && info[0]->IsString())) {
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetFillRuleForPath(fillRule);
                offscreenCanvasPattern_->Clip();
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateFillRuleForPath(fillRule);
                customPaintPattern_->Clip();
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetFillRuleForPath(fillRule);
            offscreenCanvas_->Clip();
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateFillRuleForPath(fillRule);
            pool_->Clip();
        }
    } else if (info.Length() == 2 ||
        (info.Length() == 1 && info[0]->IsObject())) {
        JSPath2D* jsCanvasPath = JSRef<JSObject>::Cast(info[0])->Unwrap<JSPath2D>();
        if (jsCanvasPath == nullptr) {
            LOGE("The arg is wrong, it is supposed to have JSPath2D argument");
            return;
        }
        auto path = jsCanvasPath->GetCanvasPath2d();

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetFillRuleForPath2D(fillRule);
                offscreenCanvasPattern_->Clip(path);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateFillRuleForPath2D(fillRule);
                customPaintPattern_->Clip(path);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetFillRuleForPath2D(fillRule);
            offscreenCanvas_->Clip(path);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateFillRuleForPath2D(fillRule);
            pool_->Clip(path);
        }
    }
}

void JSCanvasRenderer::JsRect(const JSCallbackInfo& info)
{
    Rect rect = GetJsRectParam(info);

    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            offscreenCanvasPattern_->AddRect(rect);
            return;
        }
        if (!isOffscreen_ && customPaintPattern_) {
            customPaintPattern_->AddRect(rect);
        }
        return;
    }
    if (isOffscreen_ && offscreenCanvas_) {
        offscreenCanvas_->AddRect(rect);
        return;
    }
    if (!isOffscreen_ && pool_) {
        pool_->AddRect(rect);
    }
}

void JSCanvasRenderer::JsBeginPath(const JSCallbackInfo& info)
{
    if (info.Length() != 0) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            offscreenCanvasPattern_->BeginPath();
            return;
        }
        if (!isOffscreen_ && customPaintPattern_) {
            customPaintPattern_->BeginPath();
        }
        return;
    }
    if (isOffscreen_ && offscreenCanvas_) {
        offscreenCanvas_->BeginPath();
        return;
    }
    if (!isOffscreen_ && pool_) {
        pool_->BeginPath();
    }
}

void JSCanvasRenderer::JsClosePath(const JSCallbackInfo& info)
{
    if (info.Length() != 0) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            offscreenCanvasPattern_->ClosePath();
            return;
        }
        if (!isOffscreen_ && customPaintPattern_) {
            customPaintPattern_->ClosePath();
        }
        return;
    }
    if (isOffscreen_ && offscreenCanvas_) {
        offscreenCanvas_->ClosePath();
        return;
    }
    if (!isOffscreen_ && pool_) {
        pool_->ClosePath();
    }
}

void JSCanvasRenderer::JsRestore(const JSCallbackInfo& info)
{
    if (info.Length() != 0) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            offscreenCanvasPattern_->Restore();
            return;
        }
        if (!isOffscreen_ && customPaintPattern_) {
            customPaintPattern_->Restore();
        }
        return;
    }
    if (isOffscreen_ && offscreenCanvas_) {
        offscreenCanvas_->Restore();
        return;
    }
    if (!isOffscreen_ && pool_) {
        pool_->Restore();
    }
}

void JSCanvasRenderer::JsSave(const JSCallbackInfo& info)
{
    if (info.Length() != 0) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            offscreenCanvasPattern_->Save();
            return;
        }
        if (!isOffscreen_ && customPaintPattern_) {
            customPaintPattern_->Save();
        }
        return;
    }
    if (isOffscreen_ && offscreenCanvas_) {
        offscreenCanvas_->Save();
        return;
    }
    if (!isOffscreen_ && pool_) {
        pool_->Save();
    }
}

void JSCanvasRenderer::JsRotate(const JSCallbackInfo& info)
{
    if (info.Length() != 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    double angle = 0.0;
    JSViewAbstract::ParseJsDouble(info[0], angle);

    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            offscreenCanvasPattern_->Rotate(angle);
            return;
        }
        if (!isOffscreen_ && customPaintPattern_) {
            customPaintPattern_->Rotate(angle);
        }
        return;
    }
    if (isOffscreen_ && offscreenCanvas_) {
        offscreenCanvas_->Rotate(angle);
        return;
    }
    if (!isOffscreen_ && pool_) {
        pool_->Rotate(angle);
    }
}

void JSCanvasRenderer::JsScale(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber()) {
        double x = 0.0;
        double y = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], x);
        JSViewAbstract::ParseJsDouble(info[1], y);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->Scale(x, y);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->Scale(x, y);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->Scale(x, y);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->Scale(x, y);
        }
    }
}

void JSCanvasRenderer::JsGetTransform(const JSCallbackInfo& info)
{
    return;
}

void JSCanvasRenderer::JsSetTransform(const JSCallbackInfo& info)
{
    if (info.Length() == 6) {
        if (!(info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()
            && info[4]->IsNumber() && info[5]->IsNumber())) {
            LOGE("The arg is not number.");
            return;
        }
        TransformParam param;
        JSViewAbstract::ParseJsDouble(info[0], param.scaleX);
        JSViewAbstract::ParseJsDouble(info[1], param.skewX);
        JSViewAbstract::ParseJsDouble(info[2], param.skewY);
        JSViewAbstract::ParseJsDouble(info[3], param.scaleY);
        JSViewAbstract::ParseJsDouble(info[4], param.translateX);
        JSViewAbstract::ParseJsDouble(info[5], param.translateY);
        param.translateX = SystemProperties::Vp2Px(param.translateX);
        param.translateY = SystemProperties::Vp2Px(param.translateY);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetTransform(param);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->SetTransform(param);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetTransform(param);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->SetTransform(param);
        }
        return;
    } else if (info.Length() == 1) {
        if (!info[0]->IsObject()) {
            LOGE("The arg is not Object or String.");
            return;
        }
        JSMatrix2d* jsContext = JSRef<JSObject>::Cast(info[0])->Unwrap<JSMatrix2d>();
        CHECK_NULL_VOID(jsContext);
        TransformParam param;
        param.scaleX = jsContext->JsGetScaleX();
        param.skewX = jsContext->JsGetRotateY();
        param.skewY = jsContext->JsGetRotateX();
        param.scaleY = jsContext->JsGetScaleY();
        param.translateX = jsContext->JsGetTranslateX();
        param.translateY = jsContext->JsGetTranslateY();
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetTransform(param);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->SetTransform(param);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetTransform(param);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->SetTransform(param);
        }
    } else {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }
}

void JSCanvasRenderer::JsResetTransform(const JSCallbackInfo& info)
{
    if (info.Length() != 0) {
        LOGE("The argv is wrong");
        return;
    }

    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            offscreenCanvasPattern_->ResetTransform();
            return;
        }
        if (!isOffscreen_ && customPaintPattern_) {
            customPaintPattern_->ResetTransform();
        }
        return;
    }
    if (isOffscreen_ && offscreenCanvas_) {
        offscreenCanvas_->ResetTransform();
        return;
    }
    if (!isOffscreen_ && pool_) {
        pool_->ResetTransform();
    }
}

void JSCanvasRenderer::JsTransform(const JSCallbackInfo& info)
{
    if (info.Length() < 6) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber() &&
        info[4]->IsNumber() && info[5]->IsNumber()) {
        TransformParam param;
        JSViewAbstract::ParseJsDouble(info[0], param.scaleX);
        JSViewAbstract::ParseJsDouble(info[1], param.skewX);
        JSViewAbstract::ParseJsDouble(info[2], param.skewY);
        JSViewAbstract::ParseJsDouble(info[3], param.scaleY);
        JSViewAbstract::ParseJsDouble(info[4], param.translateX);
        JSViewAbstract::ParseJsDouble(info[5], param.translateY);
        param.translateX = SystemProperties::Vp2Px(param.translateX);
        param.translateY = SystemProperties::Vp2Px(param.translateY);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->Transform(param);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->Transform(param);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->Transform(param);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->Transform(param);
        }
    }
}

void JSCanvasRenderer::JsTranslate(const JSCallbackInfo& info)
{
    if (info.Length() < 2) {
        LOGE("The arg is wrong, it is supposed to have at least 1 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber()) {
        double x = 0.0;
        double y = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], x);
        JSViewAbstract::ParseJsDouble(info[1], y);
        x = SystemProperties::Vp2Px(x);
        y = SystemProperties::Vp2Px(y);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->Translate(x, y);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->Translate(x, y);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->Translate(x, y);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->Translate(x, y);
        }
    }
}

void JSCanvasRenderer::JsSetLineDash(const JSCallbackInfo& info)
{
    std::vector<double> lineDash;
    ParseJsDoubleArray(info[0], lineDash);
    if (lineDash.size() % 2 != 0) {
        lineDash.insert(lineDash.end(), lineDash.begin(), lineDash.end());
    }

    if (Container::IsCurrentUseNewPipeline()) {
        if (isOffscreen_ && offscreenCanvasPattern_) {
            offscreenCanvasPattern_->SetLineDash(lineDash);
            return;
        }
        if (!isOffscreen_ && customPaintPattern_) {
            customPaintPattern_->UpdateLineDash(lineDash);
        }
        return;
    }
    if (isOffscreen_ && offscreenCanvas_) {
        offscreenCanvas_->SetLineDash(lineDash);
        return;
    }
    if (!isOffscreen_ && pool_) {
        pool_->UpdateLineDash(lineDash);
    }
}

Pattern JSCanvasRenderer::GetPattern(int32_t id)
{
    if (id < 0) {
        return Pattern();
    }
    return pattern_[id];
}

void JSCanvasRenderer::JsSetTextAlign(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    std::string value = "";
    if (info[0]->IsString()) {
        JSViewAbstract::ParseJsString(info[0], value);
        auto align = ConvertStrToTextAlign(value);
        paintState_.SetTextAlign(align);
        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetTextAlign(align);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateTextAlign(align);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetTextAlign(align);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateTextAlign(align);
        }
    }
}

void JSCanvasRenderer::JsSetTextBaseline(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }

    std::string textBaseline;
    if (info[0]->IsString()) {
        JSViewAbstract::ParseJsString(info[0], textBaseline);
        auto baseline = ConvertStrToEnum(
            textBaseline.c_str(), BASELINE_TABLE, ArraySize(BASELINE_TABLE), TextBaseline::ALPHABETIC);
        style_.SetTextBaseline(baseline);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->SetTextBaseline(baseline);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->UpdateTextBaseline(baseline);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->SetTextBaseline(baseline);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->UpdateTextBaseline(baseline);
        }
    }
}

void JSCanvasRenderer::JsMeasureText(const JSCallbackInfo& info)
{
    std::string text = "";
    paintState_.SetTextStyle(style_);
    double width = 0.0;
    double height = 0.0;
    double actualBoundingBoxLeft = 0.0;
    double actualBoundingBoxRight = 0.0;
    double actualBoundingBoxAscent = 0.0;
    double actualBoundingBoxDescent = 0.0;
    double hangingBaseline = 0.0;
    double alphabeticBaseline = 0.0;
    double ideographicBaseline = 0.0;
    double emHeightAscent = 0.0;
    double emHeightDescent = 0.0;
    double fontBoundingBoxAscent = 0.0;
    double fontBoundingBoxDescent = 0.0;
    if (info[0]->IsString()) {
        JSViewAbstract::ParseJsString(info[0], text);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                width = offscreenCanvasPattern_->MeasureText(text, paintState_);
                height = offscreenCanvasPattern_->MeasureTextHeight(text, paintState_);
            } else if (!isOffscreen_ && customPaintPattern_) {
                width = customPaintPattern_->MeasureText(text, paintState_);
                height = customPaintPattern_->MeasureTextHeight(text, paintState_);
            }
        } else {
            if (isOffscreen_ && offscreenCanvas_) {
                width = offscreenCanvas_->MeasureText(text, paintState_);
                height = offscreenCanvas_->MeasureTextHeight(text, paintState_);
            } else if (!isOffscreen_ && pool_) {
                width = pool_->MeasureText(text, paintState_);
                height = pool_->MeasureTextHeight(text, paintState_);
            }
        }

        auto retObj = JSRef<JSObject>::New();
        retObj->SetProperty("width", SystemProperties::Px2Vp(width));
        retObj->SetProperty("height", SystemProperties::Px2Vp(height));
        retObj->SetProperty("actualBoundingBoxLeft", SystemProperties::Px2Vp(actualBoundingBoxLeft));
        retObj->SetProperty("actualBoundingBoxRight", SystemProperties::Px2Vp(actualBoundingBoxRight));
        retObj->SetProperty("actualBoundingBoxAscent", SystemProperties::Px2Vp(actualBoundingBoxAscent));
        retObj->SetProperty("actualBoundingBoxDescent", SystemProperties::Px2Vp(actualBoundingBoxDescent));
        retObj->SetProperty("hangingBaseline", SystemProperties::Px2Vp(hangingBaseline));
        retObj->SetProperty("alphabeticBaseline", SystemProperties::Px2Vp(alphabeticBaseline));
        retObj->SetProperty("ideographicBaseline", SystemProperties::Px2Vp(ideographicBaseline));
        retObj->SetProperty("emHeightAscent", SystemProperties::Px2Vp(emHeightAscent));
        retObj->SetProperty("emHeightDescent", SystemProperties::Px2Vp(emHeightDescent));
        retObj->SetProperty("fontBoundingBoxAscent", SystemProperties::Px2Vp(fontBoundingBoxAscent));
        retObj->SetProperty("fontBoundingBoxDescent", SystemProperties::Px2Vp(fontBoundingBoxDescent));
        info.SetReturnValue(retObj);
    }
}

void JSCanvasRenderer::JsFillRect(const JSCallbackInfo& info)
{
    if (info.Length() < 4) {
        LOGE("The arg is wrong, it is supposed to have at least 4 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()) {
        double x = 0.0;
        double y = 0.0;
        double width = 0.0;
        double height = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], x);
        JSViewAbstract::ParseJsDouble(info[1], y);
        JSViewAbstract::ParseJsDouble(info[2], width);
        JSViewAbstract::ParseJsDouble(info[3], height);
        x = SystemProperties::Vp2Px(x);
        y = SystemProperties::Vp2Px(y);
        width = SystemProperties::Vp2Px(width);
        height = SystemProperties::Vp2Px(height);

        Rect rect = Rect(x, y, width, height);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->FillRect(rect);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->FillRect(rect);
            }
            return;
        } else {
            if (isOffscreen_ && offscreenCanvas_) {
                offscreenCanvas_->FillRect(rect);
                return;
            }
            if (!isOffscreen_ && pool_) {
                pool_->FillRect(rect);
            }
        }
    }
}

void JSCanvasRenderer::JsStrokeRect(const JSCallbackInfo& info)
{
    if (info.Length() < 4) {
        LOGE("The arg is wrong, it is supposed to have at least 4 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()) {
        double x = 0.0;
        double y = 0.0;
        double width = 0.0;
        double height = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], x);
        JSViewAbstract::ParseJsDouble(info[1], y);
        JSViewAbstract::ParseJsDouble(info[2], width);
        JSViewAbstract::ParseJsDouble(info[3], height);
        x = SystemProperties::Vp2Px(x);
        y = SystemProperties::Vp2Px(y);
        width = SystemProperties::Vp2Px(width);
        height = SystemProperties::Vp2Px(height);

        Rect rect = Rect(x, y, width, height);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->StrokeRect(rect);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->StrokeRect(rect);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->StrokeRect(rect);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->StrokeRect(rect);
        }
    }
}

void JSCanvasRenderer::JsClearRect(const JSCallbackInfo& info)
{
    if (info.Length() < 4) {
        LOGE("The arg is wrong, it is supposed to have at least 4 argument");
        return;
    }

    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()) {
        double x = 0.0;
        double y = 0.0;
        double width = 0.0;
        double height = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], x);
        JSViewAbstract::ParseJsDouble(info[1], y);
        JSViewAbstract::ParseJsDouble(info[2], width);
        JSViewAbstract::ParseJsDouble(info[3], height);
        x = SystemProperties::Vp2Px(x);
        y = SystemProperties::Vp2Px(y);
        width = SystemProperties::Vp2Px(width);
        height = SystemProperties::Vp2Px(height);

        Rect rect = Rect(x, y, width, height);

        if (Container::IsCurrentUseNewPipeline()) {
            if (isOffscreen_ && offscreenCanvasPattern_) {
                offscreenCanvasPattern_->ClearRect(rect);
                return;
            }
            if (!isOffscreen_ && customPaintPattern_) {
                customPaintPattern_->ClearRect(rect);
            }
            return;
        }
        if (isOffscreen_ && offscreenCanvas_) {
            offscreenCanvas_->ClearRect(rect);
            return;
        }
        if (!isOffscreen_ && pool_) {
            pool_->ClearRect(rect);
        }
    }
}

} // namespace OHOS::Ace::Framework
