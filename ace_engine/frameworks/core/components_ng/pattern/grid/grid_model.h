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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_MODEL_H

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/memory/referenced.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components/scroll_bar/scroll_bar_proxy.h"
#include "core/components_v2/grid/grid_position_controller.h"
#include "core/event/ace_events.h"
#include "core/gestures/gesture_info.h"

namespace OHOS::Ace {

class GridModel {
public:
    static GridModel* GetInstance();
    virtual ~GridModel() = default;

    virtual void Create(
        const RefPtr<ScrollControllerBase>& positionController, const RefPtr<ScrollProxy>& scrollBarProxy) = 0;
    virtual void Pop() = 0;
    virtual void SetColumnsTemplate(const std::string& value) = 0;
    virtual void SetRowsTemplate(const std::string& value) = 0;
    virtual void SetColumnsGap(const Dimension& value) = 0;
    virtual void SetRowsGap(const Dimension& value) = 0;
    virtual void SetGridHeight(const Dimension& value) = 0;
    virtual void SetScrollBarMode(int32_t value) = 0;
    virtual void SetScrollBarColor(const std::string& value) = 0;
    virtual void SetScrollBarWidth(const std::string& value) = 0;
    virtual void SetCachedCount(int32_t value) = 0;
    virtual void SetIsRTL(bool rightToLeft) = 0;
    virtual void SetLayoutDirection(FlexDirection value) = 0;
    virtual void SetMaxCount(int32_t value) = 0;
    virtual void SetMinCount(int32_t value) = 0;
    virtual void SetCellLength(int32_t value) = 0;
    virtual void SetEditable(bool value) = 0;
    virtual void SetMultiSelectable(bool value) = 0;
    virtual void SetSupportAnimation(bool value) = 0;
    virtual void SetSupportDragAnimation(bool value) = 0;
    virtual void SetEdgeEffect(EdgeEffect edgeEffect) = 0;
    virtual void SetOnScrollToIndex(std::function<void(const BaseEventInfo*)>&& value) = 0;
    virtual void SetOnItemDragStart(std::function<void(const ItemDragInfo&, int32_t)>&& value) = 0;
    virtual void SetOnItemDragEnter(std::function<void(const ItemDragInfo&)>&& value) = 0;
    virtual void SetOnItemDragMove(std::function<void(const ItemDragInfo&, int32_t, int32_t)>&& value) = 0;
    virtual void SetOnItemDragLeave(std::function<void(const ItemDragInfo&, int32_t)>&& value) = 0;
    virtual void SetOnItemDrop(std::function<void(const ItemDragInfo&, int32_t, int32_t, bool)>&& value) = 0;
    virtual RefPtr<ScrollControllerBase> CreatePositionController();
    virtual RefPtr<ScrollProxy> CreateScrollBarProxy();

private:
    static std::unique_ptr<GridModel> instance_;
};

} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_MODEL_H
