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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SYNTAX_LAZY_FOR_EACH_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SYNTAX_LAZY_FOR_EACH_NODE_H

#include <list>
#include <optional>
#include <stdint.h>
#include <string>
#include <unordered_set>
#include <utility>

#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/syntax/lazy_for_each_builder.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT LazyForEachNode : public UINode, public V2::DataChangeListener {
    DECLARE_ACE_TYPE(LazyForEachNode, UINode);

public:
    static RefPtr<LazyForEachNode> GetOrCreateLazyForEachNode(
        int32_t nodeId, const RefPtr<LazyForEachBuilder>& forEachBuilder);

    LazyForEachNode(int32_t nodeId, const RefPtr<LazyForEachBuilder>& forEachBuilder)
        : UINode(V2::JS_LAZY_FOR_EACH_ETS_TAG, nodeId, false), builder_(forEachBuilder)
    {}

    ~LazyForEachNode() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    int32_t FrameCount() const override
    {
        return builder_ ? builder_->GetTotalCount() : 0;
    }

    void AdjustLayoutWrapperTree(const RefPtr<LayoutWrapper>& parent, bool forceMeasure, bool forceLayout) override;

    void UpdateLazyForEachItems(int32_t newStartIndex, int32_t newEndIndex,
        std::list<std::optional<std::string>>&& nodeIds,
        std::unordered_map<int32_t, std::optional<std::string>>&& cachedItems);

    void OnDataReloaded() override;
    void OnDataAdded(size_t index) override;
    void OnDataDeleted(size_t index) override;
    void OnDataChanged(size_t index) override;
    void OnDataMoved(size_t from, size_t to) override;

    void PostIdleTask(std::list<int32_t>&& items);

private:
    void OnAttachToMainTree() override
    {
        CHECK_NULL_VOID(builder_);
        builder_->RegisterDataChangeListener(Claim(this));
    }

    void OnDetachFromMainTree() override
    {
        CHECK_NULL_VOID(builder_);
        builder_->UnregisterDataChangeListener(Claim(this));
    }

    void NotifyDataCountChanged(int32_t index);

    // The index values of the start and end of the current children nodes and the corresponding keys.
    int32_t startIndex_ = -1;
    int32_t endIndex_ = -1;
    std::list<std::optional<std::string>> ids_;
    std::list<int32_t> predictItems_;
    bool needPredict = false;

    RefPtr<LazyForEachBuilder> builder_;

    ACE_DISALLOW_COPY_AND_MOVE(LazyForEachNode);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_SYNTAX_LAZY_FOR_EACH_NODE_H
