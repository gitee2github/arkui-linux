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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_INSPECTOR_LIST_ITEM_GROUP_COMPOSED_ELEMENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_INSPECTOR_LIST_ITEM_GROUP_COMPOSED_ELEMENT_H

#include "core/components_v2/inspector/inspector_composed_element.h"
#include "core/components_v2/list/list_item_group_element.h"
#include "core/pipeline/base/composed_element.h"

namespace OHOS::Ace::V2 {

class ACE_EXPORT ListItemGroupComposedElement : public InspectorComposedElement {
    DECLARE_ACE_TYPE(ListItemGroupComposedElement, InspectorComposedElement)

public:
    explicit ListItemGroupComposedElement(const ComposeId& id) : InspectorComposedElement(id) {}

    std::unique_ptr<JsonValue> ToJsonObject() const override;
    void Dump() override;
    std::string GetSpace() const;
    std::unique_ptr<JsonValue> GetDivider() const;

    AceType::IdType GetTargetTypeId() const override
    {
        return ListItemGroupElement::TypeId();
    }
    RefPtr<Element> GetElementChildBySlot(const RefPtr<Element>& element, int32_t& slot) const override;
    RefPtr<Element> GetRenderElement() const override
    {
        return GetContentElement<ListItemGroupElement>(ListItemGroupElement::TypeId());
    }
};

} // namespace OHOS::Ace::V2

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_V2_INSPECTOR_LIST_ITEM_GROUP_COMPOSED_ELEMENT_H