/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_RENDER_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_RENDER_PROPERTY_H

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/vector.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/decoration.h"
#include "core/components/common/properties/shadow.h"
#include "core/components/common/properties/clip_path.h"
#include "core/components_ng/property/border_property.h"
#include "core/components_ng/property/overlay_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/property/gradient_property.h"
#include "core/components_ng/property/transition_property.h"
#include "core/image/image_source_info.h"

namespace OHOS::Ace {
enum class BlurStyle;
} // namespace OHOS::Ace

namespace OHOS::Ace::NG {

struct BackgroundProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackgroundImage, ImageSourceInfo);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackgroundImageRepeat, ImageRepeat);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackgroundImageSize, BackgroundImageSize);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackgroundImagePosition, BackgroundImagePosition);
    bool CheckBlurStyleOption(const BlurStyleOption& option) const
    {
        if (!propBlurStyleOption.has_value()) {
            return false;
        }
        return NearEqual(propBlurStyleOption.value(), option);
    }
    bool CheckBlurRadius(const Dimension& radius) const
    {
        if (!propBlurRadius.has_value()) {
            return false;
        }
        return NearEqual(propBlurRadius.value(), radius);
    }
    std::optional<BlurStyleOption> propBlurStyleOption;
    std::optional<Dimension> propBlurRadius;
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct BorderImageProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderImage, RefPtr<BorderImage>);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderImageSource, ImageSourceInfo);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(HasBorderImageSlice, bool);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(HasBorderImageWidth, bool);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(HasBorderImageOutset, bool);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(HasBorderImageRepeat, bool);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderImageGradient, Gradient);

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const
    {
        static const char* REPEAT_MODE[] = {
            "RepeatMode.Space",
            "RepeatMode.Stretch",
            "RepeatMode.Repeat",
            "RepeatMode.Round",
        };
        auto jsonBorderImage = JsonUtil::Create(true);
        jsonBorderImage->Put("source", propBorderImage.value_or(AceType::MakeRefPtr<BorderImage>())->GetSrc().c_str());
        jsonBorderImage->Put("slice", propBorderImage.value_or(AceType::MakeRefPtr<BorderImage>())
            ->GetBorderImageEdge(BorderImageDirection::LEFT).GetBorderImageSlice().ToString().c_str());
        jsonBorderImage->Put("width", propBorderImage.value_or(AceType::MakeRefPtr<BorderImage>())
            ->GetBorderImageEdge(BorderImageDirection::LEFT).GetBorderImageWidth().ToString().c_str());
        jsonBorderImage->Put("outset", propBorderImage.value_or(AceType::MakeRefPtr<BorderImage>())
            ->GetBorderImageEdge(BorderImageDirection::LEFT).GetBorderImageOutset().ToString().c_str());
        jsonBorderImage->Put("repeat", REPEAT_MODE[static_cast<int>(
            propBorderImage.value_or(AceType::MakeRefPtr<BorderImage>())->GetRepeatMode())]);
        jsonBorderImage->Put("fill", propBorderImage.value_or(AceType::MakeRefPtr<BorderImage>())
            ->GetNeedFillCenter() ? "true" : "false");
        json->Put("borderImage", jsonBorderImage->ToString().c_str());
    }
};

struct BorderProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderRadius, BorderRadiusProperty);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderColor, BorderColorProperty);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderStyle, BorderStyleProperty);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BorderWidth, BorderWidthProperty);

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const
    {
        static const char* BORDER_STYLE[] = {
            "BorderStyle.Solid",
            "BorderStyle.Dashed",
            "BorderStyle.Dotted",
        };
        json->Put("borderStyle",
            BORDER_STYLE[static_cast<int>(
                propBorderStyle.value_or(BorderStyleProperty()).styleLeft.value_or(BorderStyle::SOLID))]);
        json->Put("borderColor",
            propBorderColor.value_or(BorderColorProperty()).leftColor.value_or(Color()).ColorToString().c_str());
        json->Put("borderRadius", propBorderRadius.value_or(BorderRadiusProperty())
	    .radiusTopLeft.value_or(Dimension(0.0, DimensionUnit::VP)).ToString().c_str());
        auto jsonBorder = JsonUtil::Create(true);
        jsonBorder->Put("width", propBorderWidth.value_or(BorderWidthProperty())
            .leftDimen.value_or(Dimension(0.0, DimensionUnit::VP)).ToString().c_str());
        jsonBorder->Put("color",
            propBorderColor.value_or(BorderColorProperty()).leftColor.value_or(Color()).ColorToString().c_str());
        jsonBorder->Put("radius", propBorderRadius.value_or(BorderRadiusProperty())
	    .radiusTopLeft.value_or(Dimension(0.0, DimensionUnit::VP)).ToString().c_str());
        jsonBorder->Put("style",
            BORDER_STYLE[static_cast<int>(
                propBorderStyle.value_or(BorderStyleProperty()).styleLeft.value_or(BorderStyle::SOLID))]);
        json->Put("border", jsonBorder->ToString().c_str());
    }
};

struct TransformProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TransformScale, VectorF);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TransformCenter, DimensionOffset);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TransformTranslate, TranslateOptions);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(TransformRotate, Vector4F);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct GraphicsProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontBrightness, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontGrayScale, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontContrast, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontSaturate, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontSepia, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontInvert, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontHueRotate, float);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontColorBlend, Color);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(FrontBlurRadius, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BlurRadius, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackShadow, Shadow);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct RenderPositionProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Position, OffsetT<Dimension>);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Offset, OffsetT<Dimension>);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Anchor, OffsetT<Dimension>);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct ClipProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ClipShape, RefPtr<BasicShape>);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ClipEdge, bool);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ClipMask, RefPtr<BasicShape>);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct GradientProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(LinearGradient, NG::Gradient);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(SweepGradient, NG::Gradient);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(RadialGradient, NG::Gradient);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const;
};

struct OverlayProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(OverlayText, OverlayOptions);
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const
    {
        propOverlayText.value_or(OverlayOptions()).ToJsonValue(json);
    }
};

struct MotionPathProperty {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(MotionPath, MotionPathOption);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_RENDER_PROPERTY_H
