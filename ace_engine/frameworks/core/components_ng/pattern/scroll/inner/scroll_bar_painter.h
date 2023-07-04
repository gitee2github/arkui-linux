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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_INNER_SCROLL_BAR_PAINTER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_INNER_SCROLL_BAR_PAINTER_H

#include "base/memory/ace_type.h"
#include "core/components_ng/pattern/scroll/inner/scroll_bar.h"
#include "core/components_ng/render/drawing.h"

namespace OHOS::Ace::NG {

class ScrollBarPainter : public virtual AceType {
    DECLARE_ACE_TYPE(ScrollBarPainter, AceType);

public:
    ScrollBarPainter() = default;
    ~ScrollBarPainter() override  = default;

    static void PaintRectBar(RSCanvas& canvas, const RefPtr<ScrollBar>& scrollBar);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLL_INNER_SCROLL_BAR_PAINTER_H
