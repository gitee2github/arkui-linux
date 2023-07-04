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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_CUSTOM_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_CUSTOM_NODE_H

#include <functional>
#include <string>

#include "base/utils/macros.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/custom/custom_node_base.h"
#include "core/components_ng/pattern/custom/custom_node_pattern.h"

namespace OHOS::Ace::NG {
// CustomNode is the frame node of @Component struct.
class ACE_EXPORT CustomNode : public UINode, public CustomNodeBase {
    DECLARE_ACE_TYPE(CustomNode, UINode, CustomNodeBase);

public:
    static RefPtr<CustomNode> CreateCustomNode(int32_t nodeId, const std::string& viewKey);

    CustomNode(int32_t nodeId, const std::string& viewKey);
    ~CustomNode() override = default;

    void AdjustLayoutWrapperTree(const RefPtr<LayoutWrapper>& parent, bool forceMeasure, bool forceLayout) override;

    RefPtr<LayoutWrapper> CreateLayoutWrapper(bool forceMeasure = false, bool forceLayout = false) override;

    bool IsAtomicNode() const override
    {
        return true;
    }

    void SetRenderFunction(const RenderFunction& renderFunction) override
    {
        renderFunction_ = renderFunction;
    }

    void Build() override;

    int32_t FrameCount() const override
    {
        return 1;
    }

    void SetCompleteReloadFunc(RenderFunction&& func) override
    {
        completeReloadFunc_ = std::move(func);
    }
    void FlushReload();

private:
    std::string viewKey_;
    RenderFunction renderFunction_;
    RenderFunction completeReloadFunc_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_CUSTOM_NODE_H
