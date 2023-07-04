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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_SUBWINDOW_SUBWINDOW_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_SUBWINDOW_SUBWINDOW_H

#include <set>

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components/dialog/dialog_properties.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/pipeline/base/component.h"

namespace OHOS::Ace {

class ACE_EXPORT Subwindow : public AceType {
    DECLARE_ACE_TYPE(Subwindow, AceType)

public:
    static RefPtr<Subwindow> CreateSubwindow(int32_t instanceId);

    virtual void InitContainer() = 0;
    virtual void ResizeWindow() = 0;
    virtual void ShowMenu(const RefPtr<Component>& newComponent) = 0;
    virtual void ShowMenuNG(const RefPtr<NG::FrameNode> menuNode, int32_t targetId, const NG::OffsetF& offset) = 0;
    virtual void HideMenuNG(int32_t targetId) = 0;
    virtual void HideMenuNG() = 0;
    virtual void ShowPopup(const RefPtr<Component>& newComponent, bool disableTouchEvent = true) = 0;
    virtual void ShowPopupNG(int32_t targetId, const NG::PopupInfo& popupInfo) = 0;
    virtual void HidePopupNG(int32_t targetId) = 0;
    virtual void HidePopupNG() = 0;
    virtual bool CancelPopup(const std::string& id) = 0;
    virtual void CloseMenu() = 0;
    virtual void ClearMenu() {};
    virtual void ClearMenuNG() = 0;
    virtual RefPtr<NG::FrameNode> ShowDialogNG(
        const DialogProperties& dialogProps, const RefPtr<NG::UINode>& customNode) = 0;
    virtual void HideSubWindowNG() = 0;

    // Add interface for hot regions
    virtual void SetHotAreas(const std::vector<Rect>& rects) {};

    int32_t GetSubwindowId() const
    {
        return subwindowId_;
    }

    void SetSubwindowId(int32_t id)
    {
        subwindowId_ = id;
    }

    virtual void ShowToast(const std::string& message, int32_t duration, const std::string& bottom) = 0;
    virtual void ShowDialog(const std::string& title, const std::string& message,
        const std::vector<ButtonInfo>& buttons, bool autoCancel, std::function<void(int32_t, int32_t)>&& callback,
        const std::set<std::string>& callbacks) = 0;
    virtual void ShowActionMenu(const std::string& title, const std::vector<ButtonInfo>& button,
        std::function<void(int32_t, int32_t)>&& callback) = 0;
    virtual void CloseDialog(int32_t instanceId) = 0;

private:
    int32_t subwindowId_ = 0;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_SUBWINDOW_SUBWINDOW_H
