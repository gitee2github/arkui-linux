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

#include "core/components_ng/pattern/text/text_accessibility_property.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/text/text_layout_property.h"

namespace OHOS::Ace::NG {
std::string TextAccessibilityProperty::GetText() const
{
    auto frameNode = host_.Upgrade();
    if (frameNode) {
        auto textProperty = frameNode->GetLayoutProperty<TextLayoutProperty>();
        if (textProperty) {
            return textProperty->GetContent().value_or("");
        }
    }
    return "";
}
} // namespace OHOS::Ace::NG