/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SLIDER_ROSEN_RENDER_CIRCLE_BLOCK_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SLIDER_ROSEN_RENDER_CIRCLE_BLOCK_H

#include "core/SkCanvas.h"

#include "core/components/slider/render_block.h"

namespace OHOS::Ace {

class RosenRenderCircleBlock final : public RenderBlock {
    DECLARE_ACE_TYPE(RosenRenderCircleBlock, RenderBlock);

public:
    RosenRenderCircleBlock() = default;
    ~RosenRenderCircleBlock() override = default;

    void Update(const RefPtr<Component>& component) override;
    void Paint(RenderContext& context, const Offset& offset) override;
    void SyncGeometryProperties() override;

private:
    void PaintShadow(RenderContext& context, const Offset& offset, double radius);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SLIDER_ROSEN_RENDER_CIRCLE_BLOCK_H
