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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MARQUEE_LAYOUT_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MARQUEE_LAYOUT_PROPERTY_H

#include <string>

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT MarqueeLayoutProperty final : public LayoutProperty {
    DECLARE_ACE_TYPE(MarqueeLayoutProperty, LayoutProperty);

public:
    MarqueeLayoutProperty() = default;

    ~MarqueeLayoutProperty() override = default;

    RefPtr<LayoutProperty> Clone() const override
    {
        auto value = MakeRefPtr<MarqueeLayoutProperty>();
        value->LayoutProperty::UpdateLayoutProperty(DynamicCast<LayoutProperty>(this));
        value->propPlayerStatus_ = ClonePlayerStatus();
        value->propScrollAmount_ = CloneScrollAmount();
        value->propDirection_ = CloneDirection();
        value->propLoop_ = CloneLoop();
        value->propAllowScale_ = CloneAllowScale();
        return value;
    }

    void Reset() override
    {
        LayoutProperty::Reset();
        ResetPlayerStatus();
        ResetScrollAmount();
        ResetDirection();
        ResetLoop();
        ResetAllowScale();
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override
    {
        LayoutProperty::ToJsonValue(json);
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto textChild = AceType::DynamicCast<FrameNode>(host->GetChildren().front());
        CHECK_NULL_VOID(textChild);
        auto textLayoutProperty = textChild->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_VOID(textLayoutProperty);
        json->Put("src", textLayoutProperty->GetContent().value_or("").c_str());
        constexpr double DEFAULT_MARQUEE_SCROLL_AMOUNT = 6.0;
        json->Put("step", std::to_string(propScrollAmount_.value_or(DEFAULT_MARQUEE_SCROLL_AMOUNT)).c_str());
        json->Put("loop", std::to_string(propLoop_.value_or(-1)).c_str());
        json->Put("start", propPlayerStatus_.value_or(true) ? "true" : "false");
        json->Put(
            "fromStart", propDirection_.value_or(MarqueeDirection::RIGHT) == MarqueeDirection::LEFT ? "true" : "false");
        json->Put("allowScale", propAllowScale_.value_or(false) ? "true" : "false");
        json->Put("fontSize", textLayoutProperty->GetFontSize().value_or(10.0_vp).ToString().c_str());
        json->Put("fontColor", textLayoutProperty->GetForegroundColor()
            .value_or(textLayoutProperty->GetTextColor().value_or(Color::BLACK)).ColorToString().c_str());
        json->Put("fontWeight", V2::ConvertWrapFontWeightToStirng(textLayoutProperty->GetFontWeight()
            .value_or(FontWeight::NORMAL)).c_str());
        std::vector<std::string> fontFamilyVector =
            textLayoutProperty->GetFontFamily().value_or<std::vector<std::string>>({ "HarmonyOS Sans" });
        if (fontFamilyVector.empty()) {
            fontFamilyVector = std::vector<std::string>({ "HarmonyOS Sans" });
        }
        std::string fontFamily = fontFamilyVector.at(0);
        for (uint32_t i = 1; i < fontFamilyVector.size(); ++i) {
            fontFamily += ',' + fontFamilyVector.at(i);
        }
        json->Put("fontFamily", fontFamily.c_str());
    }

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(PlayerStatus, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ScrollAmount, double, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(Loop, int32_t, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(Direction, MarqueeDirection, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(AllowScale, bool, PROPERTY_UPDATE_MEASURE);

private:
    ACE_DISALLOW_COPY_AND_MOVE(MarqueeLayoutProperty);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MARQUEE_LAYOUT_PROPERTY_H
