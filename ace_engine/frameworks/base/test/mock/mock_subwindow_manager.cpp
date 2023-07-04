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

#include "base/subwindow/subwindow_manager.h"
namespace OHOS::Ace {
std::shared_ptr<SubwindowManager> SubwindowManager::instance_;

std::shared_ptr<SubwindowManager> SubwindowManager::GetInstance()
{
    if (!instance_) {
        instance_ = std::make_shared<SubwindowManager>();
    }
    return instance_;
}

void SubwindowManager::HidePopupNG(int32_t targetId)
{
    LOGI("SubwindowManager::HidePopupNG MOCK");
}

void SubwindowManager::ShowPopupNG(int32_t targetId, const NG::PopupInfo& popupInfo) {}

void SubwindowManager::HideMenuNG(int32_t targetId) {}

void SubwindowManager::HideMenuNG() {}

void SubwindowManager::SetHotAreas(const std::vector<Rect>& rects) {}
} // namespace OHOS::Ace