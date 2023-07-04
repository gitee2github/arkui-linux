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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_CLOCK_LAYOUT_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_CLOCK_LAYOUT_PROPERTY_H

#include <string>
#include <sys/time.h>

#include "core/components/common/properties/text_style.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT TextClockLayoutProperty final : public TextLayoutProperty {
    DECLARE_ACE_TYPE(TextClockLayoutProperty, TextLayoutProperty);

public:
    TextClockLayoutProperty() = default;

    ~TextClockLayoutProperty() override = default;

    RefPtr<LayoutProperty> Clone() const override
    {
        auto value = MakeRefPtr<TextClockLayoutProperty>();
        TextLayoutProperty::Clone(value);
        value->LayoutProperty::UpdateLayoutProperty(DynamicCast<LayoutProperty>(this));
        value->propFormat_ = CloneFormat();
        value->propHoursWest_ = CloneHoursWest();
        return value;
    }

    void Reset() override
    {
        TextLayoutProperty::Reset();
        ResetFormat();
        ResetHoursWest();
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override;

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(Format, std::string, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(HoursWest, int32_t, PROPERTY_UPDATE_MEASURE);

private:
    ACE_DISALLOW_COPY_AND_MOVE(TextClockLayoutProperty);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_CLOCK_LAYOUT_PROPERTY_H
