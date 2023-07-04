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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_LIST_ITEM_MODEL_IMPL_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_LIST_ITEM_MODEL_IMPL_H

#include "core/components_ng/pattern/list/list_item_model.h"

namespace OHOS::Ace::Framework {

class ListItemModelImpl : public ListItemModel {
public:
    ListItemModelImpl() = default;
    ~ListItemModelImpl() override = default;

    void Create(std::function<void(int32_t)>&& deepRenderFunc) override;
    void Create() override;
    void SetBorderRadius(const Dimension& borderRadius) override;
    void SetType(const std::string& type) override;
    void SetIsLazyCreating(bool isLazy) override;
    void SetSticky(V2::StickyMode stickyMode) override;
    void SetEditMode(uint32_t editMode) override;
    void SetSelectable(bool selectable) override;
    void SetSwiperAction(std::function<void()>&& startAction, std::function<void()>&& endAction,
        V2::SwipeEdgeEffect edgeEffect) override;
    void SetSelectCallback(OnSelectFunc&& selectCallback) override;
    void SetOnDragStart(NG::OnDragStartFunc&& onDragStart) override;

private:
    static std::unique_ptr<ListItemModel> instance_;
};

} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_LIST_ITEM_MODEL_IMPL_H
