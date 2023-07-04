/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/text_field/text_field_manager.h"

#include "base/memory/ace_type.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"

namespace OHOS::Ace::NG {
const RefPtr<KeyEventHandler>& TextFieldManagerNG::GetKeyEventHandler()
{
    if (!keyEventHandler_) {
        keyEventHandler_ = AceType::MakeRefPtr<KeyEventHandler>();
    }
    return keyEventHandler_;
}

void TextFieldManagerNG::ClearOnFocusTextField()
{
    onFocusTextField_ = nullptr;
    CHECK_NULL_VOID(keyEventHandler_);
    keyEventHandler_->ClearClient();
}

} // namespace OHOS::Ace::NG
