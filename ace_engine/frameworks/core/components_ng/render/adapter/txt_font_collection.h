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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_ADAPTER_TXT_FONT_COLLECTION_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_ADAPTER_TXT_FONT_COLLECTION_H

#include "txt/font_collection.h"

#include "base/memory/ace_type.h"
#include "base/utils/macros.h"
#include "core/components_ng/render/font_collection.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT TxtFontCollection : public FontCollection {
    DECLARE_ACE_TYPE(TxtFontCollection, FontCollection)
public:
    explicit TxtFontCollection(const std::shared_ptr<txt::FontCollection>& fontCollection);
    ~TxtFontCollection() override = default;

    const std::shared_ptr<txt::FontCollection>& GetRawFontCollection()
    {
        return collection_;
    }

private:
    std::shared_ptr<txt::FontCollection> collection_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_ADAPTER_TXT_FONT_COLLECTION_H
