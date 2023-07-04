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

#include "third_party/flutter/engine/flutter/third_party/txt/src/txt/font_collection.h"

#include "core/components_ng/render/adapter/txt_font_collection.h"
#include "core/components_ng/render/font_collection.h"

namespace OHOS::Ace::NG {
TxtFontCollection::TxtFontCollection(const std::shared_ptr<txt::FontCollection>& fontCollection)
    : collection_(fontCollection)
{}
RefPtr<FontCollection> FontCollection::Current()
{
    std::shared_ptr<txt::FontCollection> fontCollection;
    return AceType::MakeRefPtr<TxtFontCollection>(fontCollection);
}
} // namespace OHOS::Ace::NG