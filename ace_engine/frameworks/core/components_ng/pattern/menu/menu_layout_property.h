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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MENU_MENU_LAYOUT_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MENU_MENU_LAYOUT_PROPERTY_H

#include <string>
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/pattern/option/option_pattern.h"
#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT MenuLayoutProperty : public LayoutProperty {
    DECLARE_ACE_TYPE(MenuLayoutProperty, LayoutProperty);

public:
    MenuLayoutProperty() = default;

    ~MenuLayoutProperty() override = default;

    RefPtr<LayoutProperty> Clone() const override
    {
        auto value = MakeRefPtr<MenuLayoutProperty>();
        value->LayoutProperty::UpdateLayoutProperty(DynamicCast<LayoutProperty>(this));
        value->propMenuOffset_ = CloneMenuOffset();
        value->propTargetSize_ = CloneTargetSize();

        return value;
    }

    void Reset() override
    {
        LayoutProperty::Reset();
        ResetMenuOffset();
        ResetTargetSize();
    }

    // target frameNode that this menu belongs to
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(MenuOffset, NG::OffsetF, PROPERTY_UPDATE_MEASURE);
    // target frameNode size, null for click show menu
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(TargetSize, NG::SizeF, PROPERTY_UPDATE_MEASURE);

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override;

    ACE_DISALLOW_COPY_AND_MOVE(MenuLayoutProperty);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_MENU_MENU_LAYOUT_PROPERTY_H
