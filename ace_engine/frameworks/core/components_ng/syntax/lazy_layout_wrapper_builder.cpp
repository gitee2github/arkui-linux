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

#include "core/components_ng/syntax/lazy_layout_wrapper_builder.h"

#include <cstdint>
#include <iterator>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>

#include "base/log/ace_trace.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"

namespace OHOS::Ace::NG {

LazyLayoutWrapperBuilder::LazyLayoutWrapperBuilder(
    const RefPtr<LazyForEachBuilder>& builder, const WeakPtr<LazyForEachNode>& host)
    : builder_(builder), host_(host)
{}

void LazyLayoutWrapperBuilder::SwapDirtyAndUpdateBuildCache()
{
    auto host = host_.Upgrade();
    CHECK_NULL_VOID(host);

    if (childWrappers_.empty()) {
        decltype(nodeIds_) nodeIds;
        std::unordered_map<int32_t, std::optional<std::string>> cacheItems;
        host->UpdateLazyForEachItems(-1, -1, std::move(nodeIds), std::move(cacheItems));
        return;
    }

    // check front active flag.
    auto item = childWrappers_.front();
    decltype(nodeIds_) frontNodeIds;
    decltype(nodeIds_) backNodeIds;

    while (item && !item->IsActive()) {
        childWrappers_.pop_front();
        frontNodeIds.push_front(nodeIds_.front());
        nodeIds_.pop_front();
        startIndex_ = startIndex_.value() + 1;
        item = childWrappers_.empty() ? nullptr : childWrappers_.front();
    }
    // check end active flag.
    item = childWrappers_.empty() ? nullptr : childWrappers_.back();
    while (item && !item->IsActive()) {
        backNodeIds.push_front(nodeIds_.back());
        nodeIds_.pop_back();
        childWrappers_.pop_back();
        endIndex_ = endIndex_.value() - 1;
        item = childWrappers_.empty() ? nullptr : childWrappers_.back();
    }

    for (const auto& wrapper : childWrappers_) {
        wrapper->SwapDirtyLayoutWrapperOnMainThread();
    }
    int32_t frontCount = 0;
    int32_t backCount = 0;
    auto totalCount = OnGetTotalCount();

    std::list<int32_t> idleIndexes;
    std::unordered_map<int32_t, std::optional<std::string>> cacheItems;
    for (int32_t i = 0; i < cacheCount_; i++) {
        if (frontNodeIds.empty()) {
            if (startIndex_.value() - i > 0) {
                auto idleIndex = startIndex_.value() - 1 - i;
                auto cacheInfo = builder_->GetCacheItemInfo(idleIndex);
                if (!cacheInfo) {
                    cacheInfo = GetKeyByIndexFromPreNodes(idleIndex);
                }
                if (!cacheInfo) {
                    idleIndexes.emplace_back(idleIndex);
                }
                cacheItems.try_emplace(idleIndex, std::move(cacheInfo));
            }
        } else {
            cacheItems.try_emplace(startIndex_.value() - frontCount - 1, frontNodeIds.front());
            frontNodeIds.pop_front();
            frontCount++;
        }

        if (backNodeIds.empty()) {
            if (endIndex_.value() + i < (totalCount - 1)) {
                auto idleIndex = endIndex_.value() + 1 + i;
                auto cacheInfo = builder_->GetCacheItemInfo(idleIndex);
                if (!cacheInfo) {
                    idleIndexes.emplace_back(idleIndex);
                }
                if (!cacheInfo) {
                    cacheInfo = GetKeyByIndexFromPreNodes(idleIndex);
                }
                cacheItems.try_emplace(idleIndex, std::move(cacheInfo));
            }
        } else {
            cacheItems.try_emplace(endIndex_.value() + 1 + i, backNodeIds.front());
            backNodeIds.pop_front();
            backCount++;
        }
    }
    host->UpdateLazyForEachItems(startIndex_.value(), endIndex_.value(), std::move(nodeIds_), std::move(cacheItems));
    host->PostIdleTask(std::move(idleIndexes));
}

void LazyLayoutWrapperBuilder::AdjustGridOffset()
{
    for (const auto& wrapper : childWrappers_) {
        wrapper->GetHostNode()->AdjustGridOffset();
    }
}

int32_t LazyLayoutWrapperBuilder::OnGetTotalCount()
{
    CHECK_NULL_RETURN_NOLOG(builder_, 0);
    return builder_->GetTotalCount();
}

RefPtr<LayoutWrapper> LazyLayoutWrapperBuilder::OnGetOrCreateWrapperByIndex(int32_t index)
{
    LOGD("OnGetOrCreateWrapperByIndex index: %{private}d startIndex: %{private}d endIndex: %{private}d", index,
        startIndex_.value_or(-1), endIndex_.value_or(-1));
    auto totalCount = GetTotalCount();
    if ((index < 0) || (index >= totalCount)) {
        LOGE("index is illegal: %{public}d", index);
        return nullptr;
    }
    // check if the index needs to be converted to virtual index.
    if (lazySwiper_ && startIndex_ && index < startIndex_.value()) {
        index += totalCount;
    }
    return OnGetOrCreateWrapperByIndexLegacy(index);
}

RefPtr<LayoutWrapper> LazyLayoutWrapperBuilder::OnGetOrCreateWrapperByIndexLegacy(int32_t index)
{
    auto totalCount = GetTotalCount();
    // The first time get the item, do not do the range check, and the subsequent get the item
    // needs to check whether it is in the upper and lower bounds (-1, +1) of the existing index.
    if (!startIndex_) {
        startIndex_ = index;
        endIndex_ = index;
    } else {
        if ((index >= startIndex_.value()) && (index <= endIndex_.value())) {
            auto iter = childWrappers_.begin();
            std::advance(iter, index - startIndex_.value());
            return *iter;
        }
        if ((index < (startIndex_.value() - 1)) || (index > (endIndex_.value() + 1))) {
            LOGE("need to obtain the item node in order and by step one: %{public}d", index);
            return nullptr;
        }
    }

    CHECK_NULL_RETURN(builder_, nullptr);
    RefPtr<UINode> uiNode;
    std::string id;
    // get frame node from previous cached.
    if ((index >= preStartIndex_) && (index <= preEndIndex_)) {
        auto iter = preNodeIds_.begin();
        std::advance(iter, index - preStartIndex_);
        if ((iter != preNodeIds_.end()) && (iter->has_value())) {
            id = iter->value();
            uiNode = builder_->GetChildByKey(id);
        }
    }
    if (!uiNode) {
        // convert index to real index.
        int32_t realIndex = index;
        if (lazySwiper_ && index >= totalCount) {
            realIndex -= totalCount;
        }
        // create frame node.
        auto itemInfo = builder_->CreateChildByIndex(realIndex);
        id = itemInfo.first;
        uiNode = itemInfo.second;
    }
    CHECK_NULL_RETURN(uiNode, nullptr);
    RefPtr<LayoutWrapper> wrapper;
    auto frameNode = DynamicCast<FrameNode>(uiNode);
    if (frameNode) {
        wrapper = frameNode->CreateLayoutWrapper(forceMeasure_, forceLayout_);
    } else {
        wrapper = uiNode->CreateLayoutWrapper(forceMeasure_, forceLayout_);
    }
    CHECK_NULL_RETURN(wrapper, nullptr);
    if (index == (startIndex_.value() - 1)) {
        // insert at begin.
        startIndex_ = index;
        childWrappers_.emplace_front(wrapper);
        nodeIds_.emplace_front(id);
        return wrapper;
    }
    // insert at end.
    endIndex_ = index;
    childWrappers_.emplace_back(wrapper);
    nodeIds_.emplace_back(id);
    return wrapper;
}

const std::list<RefPtr<LayoutWrapper>>& LazyLayoutWrapperBuilder::GetCachedChildLayoutWrapper()
{
    return childWrappers_;
}

const std::list<RefPtr<LayoutWrapper>>& LazyLayoutWrapperBuilder::OnExpandChildLayoutWrapper()
{
    auto total = GetTotalCount();
    if (!childWrappers_.empty()) {
        if (static_cast<int32_t>(childWrappers_.size()) == total) {
            return childWrappers_;
        }
        LOGE("can not mix lazy get and full get method!");
        childWrappers_.clear();
        return childWrappers_;
    }

    CHECK_NULL_RETURN(builder_, childWrappers_);
    for (int32_t index = 0; index < total; ++index) {
        auto itemInfo = builder_->CreateChildByIndex(index);
        RefPtr<LayoutWrapper> wrapper;
        auto frameNode = DynamicCast<FrameNode>(itemInfo.second);
        auto uiNode = itemInfo.second;
        if (frameNode) {
            wrapper = frameNode->CreateLayoutWrapper(forceMeasure_, forceLayout_);
        } else if (uiNode) {
            wrapper = uiNode->CreateLayoutWrapper(forceMeasure_, forceLayout_);
        }
        if (!wrapper) {
            LOGE("fail to create wrapper");
            childWrappers_.clear();
            return childWrappers_;
        }
        nodeIds_.emplace_back(itemInfo.first);
        childWrappers_.emplace_back(wrapper);
    }
    startIndex_ = 0;
    endIndex_ = total - 1;
    return childWrappers_;
}

std::optional<std::string> LazyLayoutWrapperBuilder::GetKeyByIndexFromPreNodes(int32_t index)
{
    if ((index >= preStartIndex_) && (index <= preEndIndex_)) {
        auto iter = preNodeIds_.begin();
        std::advance(iter, index - preStartIndex_);
        if ((iter != preNodeIds_.end()) && (iter->has_value())) {
            return iter->value();
        }
    }
    return std::nullopt;
}

} // namespace OHOS::Ace::NG
