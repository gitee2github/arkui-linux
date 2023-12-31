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

#ifndef _NATIVE_INTERFACE_XCOMPONENT_IMPL_
#define _NATIVE_INTERFACE_XCOMPONENT_IMPL_

#include <string>
#include <unistd.h>
#include <vector>

#include "interfaces/native/native_interface_xcomponent.h"

#include "base/memory/ace_type.h"

struct XComponentTouchPoint {
    float tiltX = 0.0f;
    float tiltY = 0.0f;
    OH_NativeXComponent_TouchPointToolType sourceToolType =
        OH_NativeXComponent_TouchPointToolType::OH_NATIVEXCOMPONENT_TOOL_TYPE_UNKNOWN;
};

namespace OHOS::Ace {
class NativeXComponentImpl : public virtual AceType {
    DECLARE_ACE_TYPE(NativeXComponentImpl, AceType);

public:
    NativeXComponentImpl() {}

    ~NativeXComponentImpl() {}

    void SetXComponentId(const std::string& id)
    {
        xcomponentId_ = id;
    }

    const std::string& GetXComponentId() const
    {
        return xcomponentId_;
    }

    void SetXComponentWidth(const int width)
    {
        width_ = width;
    }

    int GetXComponentWidth() const
    {
        return width_;
    }

    void SetXComponentHeight(const int height)
    {
        height_ = height;
    }

    int GetXComponentHeight() const
    {
        return height_;
    }

    void SetXComponentOffsetX(const double x)
    {
        x_ = x;
    }

    double GetXComponentOffsetX() const
    {
        return x_;
    }

    void SetXComponentOffsetY(const double y)
    {
        y_ = y;
    }

    double GetXComponentOffsetY() const
    {
        return y_;
    }

    void SetSurface(void* window)
    {
        window_ = window;
    }

    const void* GetSurface() const
    {
        return window_;
    }

    void SetCallback(OH_NativeXComponent_Callback* callback)
    {
        callback_ = callback;
    }

    const OH_NativeXComponent_Callback* GetCallback() const
    {
        return callback_;
    }

    void SetMouseEventCallback(OH_NativeXComponent_MouseEvent_Callback* callback)
    {
        mouseEventCallback_ = callback;
    }

    const OH_NativeXComponent_MouseEvent_Callback* GetMouseEventCallback()
    {
        return mouseEventCallback_;
    }

    void SetTouchEvent(const OH_NativeXComponent_TouchEvent touchEvent)
    {
        touchEvent_ = touchEvent;
    }

    void SetTouchPoint(const std::vector<XComponentTouchPoint>& xComponentTouchPoints)
    {
        touchPoints_ = xComponentTouchPoints;
    }

    void SetMouseEvent(const OH_NativeXComponent_MouseEvent mouseEvent)
    {
        mouseEvent_ = mouseEvent;
    }

    const OH_NativeXComponent_TouchEvent GetTouchEvent() const
    {
        return touchEvent_;
    }

    const OH_NativeXComponent_MouseEvent GetMouseEvent() const
    {
        return mouseEvent_;
    }

    void SetToolType(size_t pointIndex, OH_NativeXComponent_TouchPointToolType toolType)
    {
        if (pointIndex >= OH_MAX_TOUCH_POINTS_NUMBER || pointIndex >= touchPoints_.size()) {
            return;
        }
        touchPoints_[pointIndex].sourceToolType = toolType;
    }

    OH_NativeXComponent_TouchPointToolType GetToolType(size_t pointIndex) const
    {
        if (pointIndex >= OH_MAX_TOUCH_POINTS_NUMBER || pointIndex >= touchPoints_.size()) {
            return OH_NativeXComponent_TouchPointToolType::OH_NATIVEXCOMPONENT_TOOL_TYPE_UNKNOWN;
        }
        return touchPoints_[pointIndex].sourceToolType;
    }

    float GetTiltX(size_t pointIndex) const
    {
        if (pointIndex >= OH_MAX_TOUCH_POINTS_NUMBER || pointIndex >= touchPoints_.size()) {
            return 0.0f;
        }
        return touchPoints_[pointIndex].tiltX;
    }

    float GetTiltY(size_t pointIndex) const
    {
        if (pointIndex >= OH_MAX_TOUCH_POINTS_NUMBER || pointIndex >= touchPoints_.size()) {
            return 0.0f;
        }
        return touchPoints_[pointIndex].tiltY;
    }

private:
    std::string xcomponentId_;
    void* window_ = nullptr;
    int width_ = 0;
    int height_ = 0;
    double x_ = 0.0;
    double y_ = 0.0;
    OH_NativeXComponent_TouchEvent touchEvent_;
    OH_NativeXComponent_MouseEvent mouseEvent_;
    OH_NativeXComponent_Callback* callback_ = nullptr;
    OH_NativeXComponent_MouseEvent_Callback* mouseEventCallback_ = nullptr;
    std::vector<XComponentTouchPoint> touchPoints_;
};
} // namespace OHOS::Ace

struct OH_NativeXComponent {
    explicit OH_NativeXComponent(OHOS::Ace::NativeXComponentImpl* xComponentImpl) : xcomponentImpl_(xComponentImpl) {}
    ~OH_NativeXComponent() {}
    int32_t GetXComponentId(char* id, uint64_t* size);
    int32_t GetNativeWindow(void** window);
    int32_t GetXComponentSize(const void* window, uint64_t* width, uint64_t* height);
    int32_t GetXComponentOffset(const void* window, double* x, double* y);
    int32_t GetTouchEvent(const void* window, OH_NativeXComponent_TouchEvent* touchEvent);
    int32_t GetMouseEvent(const void* window, OH_NativeXComponent_MouseEvent* mouseEvent);
    int32_t RegisterCallback(OH_NativeXComponent_Callback* callback);
    int32_t RegisterMouseEventCallback(OH_NativeXComponent_MouseEvent_Callback* callback);
    int32_t GetToolType(size_t pointIndex, OH_NativeXComponent_TouchPointToolType* toolType);
    int32_t GetTiltX(size_t pointIndex, float* tiltX);
    int32_t GetTiltY(size_t pointIndex, float* tiltY);

private:
    OHOS::Ace::NativeXComponentImpl* xcomponentImpl_ = nullptr;
};

#endif // _NATIVE_INTERFACE_XCOMPONENT_IMPL_