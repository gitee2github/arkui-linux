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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_TABS_TAB_BAR_PAINT_METHOD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_TABS_TAB_BAR_PAINT_METHOD_H

#include "core/components_ng/render/node_paint_method.h"

namespace OHOS::Ace::NG {

class TabBarPaintMethod : public NodePaintMethod {
    DECLARE_ACE_TYPE(TabBarPaintMethod, NodePaintMethod)

public:
    explicit TabBarPaintMethod(float currentIndicatorOffset) : currentIndicatorOffset_(currentIndicatorOffset) {}
    ~TabBarPaintMethod() override = default;

    CanvasDrawFunction GetForegroundDrawFunction(PaintWrapper* paintWrapper) override;

private:
    float currentIndicatorOffset_ = 0.0f;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_TABS_TAB_BAR_PAINT_METHOD_H
