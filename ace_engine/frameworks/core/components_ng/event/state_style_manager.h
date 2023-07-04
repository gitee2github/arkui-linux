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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_STATE_STYLE_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_STATE_STYLE_MANAGER_H

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"

namespace OHOS::Ace::NG {

class FrameNode;
class TouchEventImpl;

using UIState = uint64_t;
inline constexpr UIState UI_STATE_NORMAL = 0;
inline constexpr UIState UI_STATE_PRESSED = 1;
inline constexpr UIState UI_STATE_FOCUSED = 1 << 1;
inline constexpr UIState UI_STATE_DISABLED = 1 << 2;
// used for radio, checkbox, switch.
inline constexpr UIState UI_STATE_CHECKED = 1 << 3;

// StateStyleManager is mainly used to manage the setting and refresh of state styles.
class StateStyleManager : public virtual AceType {
    DECLARE_ACE_TYPE(StateStyleManager, AceType)

public:
    explicit StateStyleManager(WeakPtr<FrameNode> frameNode);
    ~StateStyleManager() override;

    bool HasStateStyle(UIState state) const
    {
        return (supportedStates_ & state) == state;
    }

    UIState GetCurrentUIState() const
    {
        return currentState_;
    }

    void AddSupportedState(UIState state)
    {
        supportedStates_ = supportedStates_ | state;
    }

    void SetSupportedStates(UIState state)
    {
        supportedStates_ = state;
    }

    bool IsCurrentStateOn(UIState state) const
    {
        if (state == UI_STATE_NORMAL) {
            return currentState_ == state;
        }
        return (currentState_ & state) == state;
    }

    void UpdateCurrentUIState(UIState state)
    {
        if (!HasStateStyle(state)) {
            return;
        }
        auto temp = currentState_ | state;
        if (temp != currentState_) {
            currentState_ = temp;
            FireStateFunc();
        }
    }

    void ResetCurrentUIState(UIState state)
    {
        if (!HasStateStyle(state)) {
            return;
        }
        if ((currentState_ & state) != state) {
            LOGD("current %{public}d state is not set yet.", static_cast<int32_t>(state));
            return;
        }
        auto temp = currentState_ ^ state;
        if (temp != currentState_) {
            currentState_ = temp;
            FireStateFunc();
        }
    }

    const RefPtr<TouchEventImpl>& GetPressedListener();

private:
    void FireStateFunc();

    WeakPtr<FrameNode> host_;
    RefPtr<TouchEventImpl> pressedFunc_;

    UIState supportedStates_ = UI_STATE_NORMAL;
    UIState currentState_ = UI_STATE_NORMAL;

    ACE_DISALLOW_COPY_AND_MOVE(StateStyleManager);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_STATE_STYLE_MANAGER_H