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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_BLANK_BLANK_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_BLANK_BLANK_PATTERN_H

#include "base/memory/referenced.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/blank/blank_layout_property.h"
#include "core/components_ng/pattern/pattern.h"
namespace OHOS::Ace::NG {

class BlankPattern : public Pattern {
    DECLARE_ACE_TYPE(BlankPattern, Pattern);

public:
    BlankPattern() = default;
    ~BlankPattern() override = default;
    std::string GetColorString() const;
    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override;

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<BlankLayoutProperty>();
    }

private:
    RefPtr<FrameNode> GetParentFrameNode(RefPtr<FrameNode> node);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_BLANK_BLANK_PATTERN_H
