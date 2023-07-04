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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_LAYOUT_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_LAYOUT_PROPERTY_H

#include <string>

#include "base/geometry/dimension.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/pattern/scroll/inner/scroll_bar.h"
#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT GridLayoutProperty : public LayoutProperty {
    DECLARE_ACE_TYPE(GridLayoutProperty, LayoutProperty);

public:
    GridLayoutProperty() = default;
    ~GridLayoutProperty() override = default;

    RefPtr<LayoutProperty> Clone() const override
    {
        auto value = MakeRefPtr<GridLayoutProperty>();
        value->LayoutProperty::UpdateLayoutProperty(DynamicCast<LayoutProperty>(this));
        value->propRowsTemplate_ = CloneRowsTemplate();
        value->propColumnsTemplate_ = CloneColumnsTemplate();
        value->propRowsGap_ = CloneRowsGap();
        value->propColumnsGap_ = CloneColumnsGap();
        value->propCachedCount_ = CloneCachedCount();
        value->propLayoutDirection_ = CloneLayoutDirection();
        value->propMaxCount_ = CloneMaxCount();
        value->propMinCount_ = CloneMinCount();
        value->propCellLength_ = CloneCellLength();
        return value;
    }

    void Reset() override
    {
        LayoutProperty::Reset();
        ResetColumnsTemplate();
        ResetRowsTemplate();
        ResetColumnsGap();
        ResetRowsGap();
        ResetCachedCount();
        ResetLayoutDirection();
        ResetMaxCount();
        ResetMinCount();
        ResetCellLength();
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override;

    bool IsVertical() const
    {
        bool columnsTemplateValid = propColumnsTemplate_.has_value() && !propColumnsTemplate_.value().empty();
        bool rowsTemplateValid = propRowsTemplate_.has_value() && !propRowsTemplate_.value().empty();
        return columnsTemplateValid ||
               (!columnsTemplateValid && !rowsTemplateValid); // TODO: take layoutDirection into account
    }

    bool IsConfiguredScrollable() const
    {
        bool columnsTemplateSet = !propColumnsTemplate_.value_or("").empty();
        bool rowsTemplateSet = !propRowsTemplate_.value_or("").empty();
        bool verticalScrollable = (columnsTemplateSet && !rowsTemplateSet);
        bool horizontalScrollable = (!columnsTemplateSet && rowsTemplateSet);
        return verticalScrollable || horizontalScrollable;
    }

    ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP(ColumnsTemplate, std::string);
    void OnColumnsTemplateUpdate(const std::string& /*columnsTemplate*/) const
    {
        ResetGridLayoutInfoAndMeasure();
    }

    ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP(RowsTemplate, std::string);
    void OnRowsTemplateUpdate(const std::string& /*rowsTemplate*/) const
    {
        ResetGridLayoutInfoAndMeasure();
    }

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ColumnsGap, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(RowsGap, Dimension, PROPERTY_UPDATE_MEASURE);

    ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP(CachedCount, int32_t);
    void OnCachedCountUpdate(int32_t /*cachedCount*/) const
    {
        ResetGridLayoutInfoAndMeasure();
    }

    ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP(LayoutDirection, FlexDirection);
    void OnLayoutDirectionUpdate(FlexDirection /*layoutDirection*/) const
    {
        ResetGridLayoutInfoAndMeasure();
    }

    ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP(MaxCount, int32_t);
    void OnMaxCountUpdate(int32_t /*maxCount*/) const
    {
        ResetGridLayoutInfoAndMeasure();
    }

    ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP(MinCount, int32_t);
    void OnMinCountUpdate(int32_t /*minCount*/) const
    {
        ResetGridLayoutInfoAndMeasure();
    }

    ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP(CellLength, int32_t);
    void OnCellLengthUpdate(int32_t /*cellLength*/) const
    {
        ResetGridLayoutInfoAndMeasure();
    }

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(Editable, bool, PROPERTY_UPDATE_LAYOUT);

private:
    ACE_DISALLOW_COPY_AND_MOVE(GridLayoutProperty);

    void ResetGridLayoutInfoAndMeasure() const;
    std::string GetBarStateString() const;
    std::string GetLayoutDirectionStr() const;
    Color GetBarColor() const;
    Dimension GetBarWidth() const;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_LAYOUT_PROPERTY_H
