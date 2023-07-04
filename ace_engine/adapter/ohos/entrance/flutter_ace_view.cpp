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

#include "adapter/ohos/entrance/flutter_ace_view.h"

#include <EGL/egl.h>
#include <algorithm>
#include <fstream>

#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/shell/common/shell_io_manager.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"
#include "key_event.h"
#include "pointer_event.h"

#include "base/utils/utils.h"

#if defined(ENABLE_ROSEN_BACKEND) and !defined(UPLOAD_GPU_DISABLED)
#include "adapter/ohos/entrance/ace_rosen_sync_task.h"
#endif

#include "adapter/ohos/entrance/ace_container.h"
#include "base/log/ace_trace.h"
#include "base/log/dump_log.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/macros.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/common/container_scope.h"
#include "core/components/theme/app_theme.h"
#include "core/components/theme/theme_manager.h"
#include "core/event/axis_event.h"
#include "core/event/key_event.h"
#include "core/event/mouse_event.h"
#include "core/event/touch_event.h"
#include "core/image/image_cache.h"
#include "core/pipeline/layers/flutter_scene_builder.h"

namespace OHOS::Ace::Platform {
namespace {

constexpr int32_t ROTATION_DIVISOR = 64;
constexpr double PERMIT_ANGLE_VALUE = 0.5;

template<typename E>
void GetEventDevice(int32_t sourceType, E& event)
{
    switch (sourceType) {
        case OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN:
            event.sourceType = SourceType::TOUCH;
            break;
        case OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHPAD:
            event.sourceType = SourceType::TOUCH_PAD;
            break;
        case OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE:
            event.sourceType = SourceType::MOUSE;
            break;
        default:
            event.sourceType = SourceType::NONE;
            break;
    }
}

SourceTool GetSourceTool(int32_t orgToolType)
{
    switch (orgToolType) {
        case OHOS::MMI::PointerEvent::TOOL_TYPE_FINGER:
            return SourceTool::FINGER;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_PEN:
            return SourceTool::PEN;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_RUBBER:
            return SourceTool::RUBBER;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_BRUSH:
            return SourceTool::BRUSH;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_PENCIL:
            return SourceTool::PENCIL;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_AIRBRUSH:
            return SourceTool::AIRBRUSH;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_MOUSE:
            return SourceTool::MOUSE;
        case OHOS::MMI::PointerEvent::TOOL_TYPE_LENS:
            return SourceTool::LENS;
        default:
            LOGW("unknown tool type");
            return SourceTool::UNKNOWN;
    }
}

TouchPoint ConvertTouchPoint(const MMI::PointerEvent::PointerItem& pointerItem)
{
    TouchPoint touchPoint;
    // just get the max of width and height
    touchPoint.size = std::max(pointerItem.GetWidth(), pointerItem.GetHeight()) / 2.0;
    touchPoint.id = pointerItem.GetPointerId();
    touchPoint.downTime = TimeStamp(std::chrono::microseconds(pointerItem.GetDownTime()));
    touchPoint.x = pointerItem.GetWindowX();
    touchPoint.y = pointerItem.GetWindowY();
    touchPoint.screenX = pointerItem.GetDisplayX();
    touchPoint.screenY = pointerItem.GetDisplayY();
    touchPoint.isPressed = pointerItem.IsPressed();
    touchPoint.force = static_cast<float>(pointerItem.GetPressure());
    touchPoint.tiltX = pointerItem.GetTiltX();
    touchPoint.tiltY = pointerItem.GetTiltY();
    touchPoint.sourceTool = GetSourceTool(pointerItem.GetToolType());
    return touchPoint;
}

void UpdateTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, TouchEvent& touchEvent)
{
    auto ids = pointerEvent->GetPointerIds();
    for (auto&& id : ids) {
        MMI::PointerEvent::PointerItem item;
        bool ret = pointerEvent->GetPointerItem(id, item);
        if (!ret) {
            LOGE("get pointer item failed.");
            continue;
        }
        auto touchPoint = ConvertTouchPoint(item);
        touchEvent.pointers.emplace_back(std::move(touchPoint));
    }
}

TouchEvent ConvertTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    int32_t pointerID = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    bool ret = pointerEvent->GetPointerItem(pointerID, item);
    if (!ret) {
        LOGE("get pointer item failed.");
        return TouchEvent();
    }
    auto touchPoint = ConvertTouchPoint(item);
    std::chrono::microseconds microseconds(pointerEvent->GetActionTime());
    TimeStamp time(microseconds);
    TouchEvent event { touchPoint.id, touchPoint.x, touchPoint.y, touchPoint.screenX, touchPoint.screenY,
        TouchType::UNKNOWN, time, touchPoint.size, touchPoint.force, touchPoint.tiltX, touchPoint.tiltY,
        pointerEvent->GetDeviceId(), SourceType::NONE, touchPoint.sourceTool };
    int32_t orgDevice = pointerEvent->GetSourceType();
    GetEventDevice(orgDevice, event);
    int32_t orgAction = pointerEvent->GetPointerAction();
    switch (orgAction) {
        case OHOS::MMI::PointerEvent::POINTER_ACTION_CANCEL:
            event.type = TouchType::CANCEL;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_DOWN:
            event.type = TouchType::DOWN;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_MOVE:
            event.type = TouchType::MOVE;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_UP:
            event.type = TouchType::UP;
            break;
        default:
            LOGW("unknown type");
            break;
    }
    UpdateTouchEvent(pointerEvent, event);
    return event;
}

void GetMouseEventAction(int32_t action, MouseEvent& events)
{
    switch (action) {
        case OHOS::MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN:
            events.action = MouseAction::PRESS;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_BUTTON_UP:
            events.action = MouseAction::RELEASE;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW:
            events.action = MouseAction::WINDOW_ENTER;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_LEAVE_WINDOW:
            events.action = MouseAction::WINDOW_LEAVE;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_MOVE:
            events.action = MouseAction::MOVE;
            break;
        default:
            events.action = MouseAction::NONE;
            break;
    }
}

void GetMouseEventButton(int32_t button, MouseEvent& events)
{
    switch (button) {
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_LEFT:
            events.button = MouseButton::LEFT_BUTTON;
            break;
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_RIGHT:
            events.button = MouseButton::RIGHT_BUTTON;
            break;
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_MIDDLE:
            events.button = MouseButton::MIDDLE_BUTTON;
            break;
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_SIDE:
            events.button = MouseButton::BACK_BUTTON;
            break;
        case OHOS::MMI::PointerEvent::MOUSE_BUTTON_EXTRA:
            events.button = MouseButton::FORWARD_BUTTON;
            break;
        default:
            events.button = MouseButton::NONE_BUTTON;
            break;
    }
}

void ConvertMouseEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, MouseEvent& events)
{
    int32_t pointerID = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    bool ret = pointerEvent->GetPointerItem(pointerID, item);
    if (!ret) {
        LOGE("get pointer: %{public}d item failed.", pointerID);
        return;
    }

    events.x = item.GetWindowX();
    events.y = item.GetWindowY();
    events.screenX = item.GetDisplayX();
    events.screenY = item.GetDisplayY();
    int32_t orgAction = pointerEvent->GetPointerAction();
    GetMouseEventAction(orgAction, events);
    int32_t orgButton = pointerEvent->GetButtonId();
    GetMouseEventButton(orgButton, events);
    int32_t orgDevice = pointerEvent->GetSourceType();
    GetEventDevice(orgDevice, events);

    std::set<int32_t> pressedSet = pointerEvent->GetPressedButtons();
    uint32_t pressedButtons = 0;
    if (pressedSet.find(OHOS::MMI::PointerEvent::MOUSE_BUTTON_LEFT) != pressedSet.end()) {
        pressedButtons &= static_cast<uint32_t>(MouseButton::LEFT_BUTTON);
    }
    if (pressedSet.find(OHOS::MMI::PointerEvent::MOUSE_BUTTON_RIGHT) != pressedSet.end()) {
        pressedButtons &= static_cast<uint32_t>(MouseButton::RIGHT_BUTTON);
    }
    if (pressedSet.find(OHOS::MMI::PointerEvent::MOUSE_BUTTON_MIDDLE) != pressedSet.end()) {
        pressedButtons &= static_cast<uint32_t>(MouseButton::MIDDLE_BUTTON);
    }
    events.pressedButtons = static_cast<int32_t>(pressedButtons);

    std::chrono::microseconds microseconds(pointerEvent->GetActionTime());
    TimeStamp time(microseconds);
    events.time = time;
    LOGD("ConvertMouseEvent: (x,y): (%{public}f,%{public}f). Button: %{public}d. Action: %{public}d. "
         "DeviceType: %{public}d. PressedButton: %{public}d. Time: %{public}lld",
        events.x, events.y, events.button, events.action, events.sourceType, events.pressedButtons,
        (long long)pointerEvent->GetActionTime());
}

void GetAxisEventAction(int32_t action, AxisEvent& event)
{
    switch (action) {
        case OHOS::MMI::PointerEvent::POINTER_ACTION_AXIS_BEGIN:
            event.action = AxisAction::BEGIN;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_AXIS_UPDATE:
            event.action = AxisAction::UPDATE;
            break;
        case OHOS::MMI::PointerEvent::POINTER_ACTION_AXIS_END:
            event.action = AxisAction::END;
            break;
        default:
            event.action = AxisAction::NONE;
            break;
    }
}

void ConvertAxisEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, AxisEvent& event)
{
    int32_t pointerID = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    bool ret = pointerEvent->GetPointerItem(pointerID, item);
    if (!ret) {
        LOGE("get pointer: %{public}d item failed.", pointerID);
        return;
    }

    event.id = item.GetPointerId();
    event.x = static_cast<float>(item.GetWindowX());
    event.y = static_cast<float>(item.GetWindowY());
    event.horizontalAxis = pointerEvent->GetAxisValue(OHOS::MMI::PointerEvent::AxisType::AXIS_TYPE_SCROLL_HORIZONTAL);
    event.verticalAxis = pointerEvent->GetAxisValue(OHOS::MMI::PointerEvent::AxisType::AXIS_TYPE_SCROLL_VERTICAL);
    event.pinchAxisScale = pointerEvent->GetAxisValue(OHOS::MMI::PointerEvent::AxisType::AXIS_TYPE_PINCH);
    int32_t orgAction = pointerEvent->GetPointerAction();
    GetAxisEventAction(orgAction, event);
    int32_t orgDevice = pointerEvent->GetSourceType();
    GetEventDevice(orgDevice, event);

    std::chrono::microseconds microseconds(pointerEvent->GetActionTime());
    TimeStamp time(microseconds);
    event.time = time;
    LOGD("ConvertAxisEvent: id: %{public}d, (x,y): (%{public}f,%{public}f). HorizontalAxis: %{public}f. VerticalAxis: "
         "%{public}f. "
         "Action: %{public}d. DeviceType: %{public}d. Time: %{public}lld",
        event.id, event.x, event.y, event.horizontalAxis, event.verticalAxis, event.action, event.sourceType,
        (long long)pointerEvent->GetActionTime());
}

void ConvertKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent, KeyEvent& event)
{
    event.code = static_cast<KeyCode>(keyEvent->GetKeyCode());
    if (keyEvent->GetKeyAction() == OHOS::MMI::KeyEvent::KEY_ACTION_UP) {
        event.action = KeyAction::UP;
    } else if (keyEvent->GetKeyAction() == OHOS::MMI::KeyEvent::KEY_ACTION_DOWN) {
        event.action = KeyAction::DOWN;
    } else {
        event.action = KeyAction::UNKNOWN;
    }
    std::chrono::microseconds microseconds(keyEvent->GetActionTime());
    TimeStamp time(microseconds);
    event.timeStamp = time;
    event.key = MMI::KeyEvent::KeyCodeToString(keyEvent->GetKeyCode());
    event.deviceId = keyEvent->GetDeviceId();
    event.sourceType = SourceType::KEYBOARD;
    std::string pressedKeyStr = "Pressed Keys: ";
    for (const auto& curCode : keyEvent->GetPressedKeys()) {
        pressedKeyStr += (std::to_string(curCode) + " ");
        event.pressedCodes.emplace_back(static_cast<KeyCode>(curCode));
    }
    LOGD("ConvertKeyEvent: KeyCode: %{private}d. KeyAction: %{public}d. PressedCodes: %{private}s. Time: %{public}lld",
        event.code, event.action, pressedKeyStr.c_str(), (long long)(keyEvent->GetActionTime()));
}

void LogPointInfo(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    LOGD("point source: %{public}d", pointerEvent->GetSourceType());
    auto actionId = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    if (pointerEvent->GetPointerItem(actionId, item)) {
        LOGD("action point info: id: %{public}d, x: %{public}d, y: %{public}d, action: %{public}d, pressure: "
             "%{public}f, tiltX: %{public}f, tiltY: %{public}f",
            actionId, item.GetWindowX(), item.GetWindowY(), pointerEvent->GetPointerAction(), item.GetPressure(),
            item.GetTiltX(), item.GetTiltY());
    }
    auto ids = pointerEvent->GetPointerIds();
    for (auto&& id : ids) {
        MMI::PointerEvent::PointerItem item;
        if (pointerEvent->GetPointerItem(id, item)) {
            LOGD("all point info: id: %{public}d, x: %{public}d, y: %{public}d, isPressed: %{public}d, pressure: "
                 "%{public}f, tiltX: %{public}f, tiltY: %{public}f",
                actionId, item.GetWindowX(), item.GetWindowY(), item.IsPressed(), item.GetPressure(), item.GetTiltX(),
                item.GetTiltY());
        }
    }
}

} // namespace

FlutterAceView* FlutterAceView::CreateView(int32_t instanceId, bool useCurrentEventRunner, bool usePlatformThread)
{
    FlutterAceView* aceSurface = new Platform::FlutterAceView(instanceId);
    if (aceSurface != nullptr) {
        aceSurface->IncRefCount();
    }
    flutter::Settings settings;
    settings.instanceId = instanceId;
    settings.platform = flutter::AcePlatform::ACE_PLATFORM_OHOS;
#ifndef GPU_DISABLED
    settings.enable_software_rendering = false;
#else
    settings.enable_software_rendering = true;
#endif
#ifdef ENABLE_ROSEN_BACKEND
    settings.use_system_render_thread = SystemProperties::GetRosenBackendEnabled();
#endif
    settings.platform_as_ui_thread = usePlatformThread;
    settings.use_current_event_runner = useCurrentEventRunner;
    LOGD("software render: %{public}s", settings.enable_software_rendering ? "true" : "false");
    LOGD("use platform as ui thread: %{public}s", settings.platform_as_ui_thread ? "true" : "false");
    settings.idle_notification_callback = [instanceId](int64_t deadline) {
        ContainerScope scope(instanceId);
        auto container = Container::Current();
        CHECK_NULL_VOID_NOLOG(container);
        auto context = container->GetPipelineContext();
        CHECK_NULL_VOID_NOLOG(context);
        context->GetTaskExecutor()->PostTask(
            [context, deadline]() { context->OnIdle(deadline); }, TaskExecutor::TaskType::UI);
    };
    auto shell_holder = std::make_unique<flutter::OhosShellHolder>(settings, false);
    if (aceSurface != nullptr) {
        aceSurface->SetShellHolder(std::move(shell_holder));
    }
    return aceSurface;
}

void FlutterAceView::SurfaceCreated(FlutterAceView* view, OHOS::sptr<OHOS::Rosen::Window> window)
{
    CHECK_NULL_VOID(window);
    CHECK_NULL_VOID(view);
    LOGD(">>> FlutterAceView::SurfaceCreated, pWnd:%{public}p", &(*window));
    auto platformView = view->GetShellHolder()->GetPlatformView();
    LOGD("FlutterAceView::SurfaceCreated, GetPlatformView");
    if (platformView && !SystemProperties::GetRosenBackendEnabled()) {
        LOGD("FlutterAceView::SurfaceCreated, call NotifyCreated");
        platformView->NotifyCreated(window);
    }

    LOGD("<<< FlutterAceView::SurfaceCreated, end");
}

void FlutterAceView::SurfaceChanged(
    FlutterAceView* view, int32_t width, int32_t height, int32_t orientation, WindowSizeChangeReason type)
{
    CHECK_NULL_VOID(view);
    view->NotifySurfaceChanged(width, height, type);
    auto platformView = view->GetShellHolder()->GetPlatformView();
    LOGD("FlutterAceView::SurfaceChanged, GetPlatformView");
    if (platformView) {
        LOGD("FlutterAceView::SurfaceChanged, call NotifyChanged");
        platformView->NotifyChanged(SkISize::Make(width, height));
    }

    auto instanceId = view->GetInstanceId();
    auto container = Platform::AceContainer::GetContainer(instanceId);
    if (container) {
        auto pipelineContext = AceType::DynamicCast<PipelineContext>(container->GetPipelineContext());
        CHECK_NULL_VOID(pipelineContext);
        pipelineContext->CloseContextMenu();
        auto textOverlayManager = pipelineContext->GetTextOverlayManager();
        CHECK_NULL_VOID(textOverlayManager);
        textOverlayManager->PopTextOverlay();
    }

    LOGD("<<< FlutterAceView::SurfaceChanged, end");
}

void FlutterAceView::SurfacePositionChanged(FlutterAceView* view, int32_t posX, int32_t posY)
{
    CHECK_NULL_VOID_NOLOG(view);
    view->NotifySurfacePositionChanged(posX, posY);
}

void FlutterAceView::SetViewportMetrics(FlutterAceView* view, const flutter::ViewportMetrics& metrics)
{
    CHECK_NULL_VOID_NOLOG(view);
    view->NotifyDensityChanged(metrics.device_pixel_ratio);
    view->NotifySystemBarHeightChanged(metrics.physical_padding_top, metrics.physical_view_inset_bottom);
    auto platformView = view->GetShellHolder()->GetPlatformView();
    CHECK_NULL_VOID_NOLOG(platformView);
    platformView->SetViewportMetrics(metrics);
}

void FlutterAceView::DispatchTouchEvent(FlutterAceView* view, const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    LogPointInfo(pointerEvent);
    if (pointerEvent->GetSourceType() == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        // mouse event
        if (pointerEvent->GetPointerAction() >= MMI::PointerEvent::POINTER_ACTION_AXIS_BEGIN &&
            pointerEvent->GetPointerAction() <= MMI::PointerEvent::POINTER_ACTION_AXIS_END) {
            LOGD("ProcessAxisEvent");
            view->ProcessAxisEvent(pointerEvent);
        } else {
            LOGD("ProcessMouseEvent");
            view->ProcessMouseEvent(pointerEvent);
        }
    } else {
        // touch event
        LOGD("ProcessTouchEvent");
        view->ProcessTouchEvent(pointerEvent);
    }
}

bool FlutterAceView::DispatchKeyEvent(FlutterAceView* view, const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    CHECK_NULL_RETURN(view, false);
    return view->ProcessKeyEvent(keyEvent);
}

bool FlutterAceView::DispatchRotationEvent(FlutterAceView* view, float rotationValue)
{
    CHECK_NULL_RETURN(view, false);
    return view->ProcessRotationEvent(rotationValue);
}

void FlutterAceView::RegisterTouchEventCallback(TouchEventCallback&& callback)
{
    ACE_DCHECK(callback);
    touchEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterDragEventCallback(DragEventCallBack&& callback)
{
    ACE_DCHECK(callback);
    dragEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterKeyEventCallback(KeyEventCallback&& callback)
{
    ACE_DCHECK(callback);
    keyEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterMouseEventCallback(MouseEventCallback&& callback)
{
    ACE_DCHECK(callback);
    mouseEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterAxisEventCallback(AxisEventCallback&& callback)
{
    ACE_DCHECK(callback);
    axisEventCallback_ = std::move(callback);
}

void FlutterAceView::RegisterRotationEventCallback(RotationEventCallBack&& callback)
{
    ACE_DCHECK(callback);
    rotationEventCallBack_ = std::move(callback);
}

void FlutterAceView::Launch()
{
    LOGD("Launch shell holder.");
    if (!viewLaunched_) {
        flutter::RunConfiguration config;
        shell_holder_->Launch(std::move(config));
        viewLaunched_ = true;
    }
}

void FlutterAceView::SetShellHolder(std::unique_ptr<flutter::OhosShellHolder> holder)
{
    shell_holder_ = std::move(holder);
}

void FlutterAceView::ProcessTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    CHECK_NULL_VOID_NOLOG(pointerEvent);
    TouchEvent touchPoint = ConvertTouchEvent(pointerEvent);
    if (SystemProperties::GetDebugEnabled()) {
        ACE_SCOPED_TRACE("ProcessTouchEvent pointX=%f pointY=%f type=%d timeStamp=%lld id=%d", touchPoint.x,
            touchPoint.y, (int)touchPoint.type, touchPoint.time.time_since_epoch().count(), touchPoint.id);
    }
    auto markProcess = [pointerEvent]() {
        CHECK_NULL_VOID_NOLOG(pointerEvent);
        LOGI("Mark %{public}d id Touch Event Processed", pointerEvent->GetPointerId());
        pointerEvent->MarkProcessed();
    };
    if (touchPoint.type != TouchType::UNKNOWN) {
        if (touchEventCallback_) {
            touchEventCallback_(touchPoint, markProcess);
        }
    } else {
        LOGW("Unknown event.");
    }
}

void FlutterAceView::ProcessDragEvent(int32_t x, int32_t y, const DragEventAction& action)
{
    LOGD("ProcessDragEvent");
    CHECK_NULL_VOID_NOLOG(dragEventCallback_);
    dragEventCallback_(x, y, action);
}

void FlutterAceView::ProcessMouseEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    MouseEvent event;
    if (pointerEvent) {
        ConvertMouseEvent(pointerEvent, event);
    }
    auto markProcess = [pointerEvent]() {
        CHECK_NULL_VOID_NOLOG(pointerEvent);
        LOGI("Mark %{public}d id Mouse Event Processed", pointerEvent->GetPointerId());
        pointerEvent->MarkProcessed();
    };

    CHECK_NULL_VOID_NOLOG(mouseEventCallback_);
    mouseEventCallback_(event, markProcess);
}

void FlutterAceView::ProcessAxisEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    AxisEvent event;
    if (pointerEvent) {
        ConvertAxisEvent(pointerEvent, event);
    }
    auto markProcess = [pointerEvent]() {
        CHECK_NULL_VOID_NOLOG(pointerEvent);
        LOGI("Mark %{public}d id Axis Event Processed", pointerEvent->GetPointerId());
        pointerEvent->MarkProcessed();
    };

    CHECK_NULL_VOID_NOLOG(axisEventCallback_);
    axisEventCallback_(event, markProcess);
}

bool FlutterAceView::ProcessKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    CHECK_NULL_RETURN_NOLOG(keyEventCallback_, false);
    KeyEvent event;
    ConvertKeyEvent(keyEvent, event);
    return keyEventCallback_(event);
}

const void* FlutterAceView::GetNativeWindowById(uint64_t textureId)
{
    return nullptr;
}

bool FlutterAceView::ProcessRotationEvent(float rotationValue)
{
    CHECK_NULL_RETURN_NOLOG(rotationEventCallBack_, false);
    RotationEvent event { .value = rotationValue * ROTATION_DIVISOR };
    return rotationEventCallBack_(event);
}

bool FlutterAceView::Dump(const std::vector<std::string>& params)
{
    if (params.empty() || params[0] != "-drawcmd") {
        LOGE("Unsupported parameters.");
        return false;
    }
#ifdef DUMP_DRAW_CMD
    if (shell_holder_) {
        auto screenShot = shell_holder_->Screenshot(flutter::Rasterizer::ScreenshotType::SkiaPicture, false);
        if (screenShot.data->data() != nullptr) {
            auto byteData = screenShot.data;
            static int32_t count = 0;
            auto path = ImageCache::GetImageCacheFilePath() + "/picture_" + std::to_string(count++) + ".mskp";
            if (DumpLog::GetInstance().GetDumpFile()) {
                DumpLog::GetInstance().AddDesc("Dump draw command to path: " + path);
                DumpLog::GetInstance().Print(0, "Info:", 0);
            }
            std::ofstream outFile(path, std::fstream::out | std::fstream::binary);
            if (!outFile.is_open()) {
                LOGE("Open file %{private}s failed.", path.c_str());
                return false;
            }
            outFile.write(reinterpret_cast<const char*>(byteData->data()), byteData->size());
            outFile.close();
            return true;
        }
    }
#else
    if (DumpLog::GetInstance().GetDumpFile()) {
        DumpLog::GetInstance().AddDesc("Dump draw command not support on this version.");
        DumpLog::GetInstance().Print(0, "Info:", 0);
        return true;
    }
#endif
    return false;
}

void FlutterAceView::InitCacheFilePath(const std::string& path)
{
    if (!path.empty()) {
        ImageCache::SetImageCacheFilePath(path);
        ImageCache::SetCacheFileInfo();
    } else {
        LOGW("image cache path empty");
    }
}

bool FlutterAceView::IsLastPage() const
{
    auto container = AceEngine::Get().GetContainer(instanceId_);
    CHECK_NULL_RETURN_NOLOG(container, false);
    ContainerScope scope(instanceId_);
    auto context = container->GetPipelineContext();
    CHECK_NULL_RETURN_NOLOG(context, false);
    auto taskExecutor = context->GetTaskExecutor();

    bool isLastPage = false;
    CHECK_NULL_RETURN_NOLOG(taskExecutor, isLastPage);
    taskExecutor->PostSyncTask(
        [context, &isLastPage]() { isLastPage = context->IsLastPage(); }, TaskExecutor::TaskType::UI);
    return isLastPage;
}

uint32_t FlutterAceView::GetBackgroundColor()
{
    return Color::WHITE.GetValue();
}

// On watch device, it's probable to quit the application unexpectedly when we slide our finger diagonally upward on the
// screen, so we do restrictions here.
bool FlutterAceView::IsNeedForbidToPlatform(TouchEvent point)
{
    if (point.type == TouchType::DOWN) {
        auto result = touchPointInfoMap_.try_emplace(point.id, TouchPointInfo(point.GetOffset()));
        if (!result.second) {
            result.first->second = TouchPointInfo(point.GetOffset());
        }

        return false;
    }

    auto iter = touchPointInfoMap_.find(point.id);
    if (iter == touchPointInfoMap_.end()) {
        return false;
    }
    if (iter->second.eventState_ == EventState::HORIZONTAL_STATE) {
        return false;
    } else if (iter->second.eventState_ == EventState::VERTICAL_STATE) {
        return true;
    }

    Offset offset = point.GetOffset() - iter->second.offset_;
    double deltaX = offset.GetX();
    double deltaY = std::abs(offset.GetY());

    if (point.type == TouchType::MOVE) {
        if (deltaX > 0.0) {
            if (deltaY / deltaX > PERMIT_ANGLE_VALUE) {
                iter->second.eventState_ = EventState::VERTICAL_STATE;
                return true;
            } else {
                iter->second.eventState_ = EventState::HORIZONTAL_STATE;
            }
        }

        return false;
    }

    touchPointInfoMap_.erase(point.id);
    return deltaX > 0.0 && deltaY / deltaX > PERMIT_ANGLE_VALUE;
}

std::unique_ptr<DrawDelegate> FlutterAceView::GetDrawDelegate()
{
    auto drawDelegate = std::make_unique<DrawDelegate>();

    drawDelegate->SetDrawFrameCallback([this](RefPtr<Flutter::Layer>& layer, const Rect& dirty) {
        CHECK_NULL_VOID(layer);
        RefPtr<Flutter::FlutterSceneBuilder> flutterSceneBuilder = AceType::MakeRefPtr<Flutter::FlutterSceneBuilder>();
        layer->AddToScene(*flutterSceneBuilder, 0.0, 0.0);
        auto scene = flutterSceneBuilder->Build();
        CHECK_NULL_VOID(flutter::UIDartState::Current());
        auto window = flutter::UIDartState::Current()->window();
        if (window != nullptr && window->client() != nullptr) {
            window->client()->Render(scene.get());
        } else {
            LOGE("window is nullptr");
        }
    });

    return drawDelegate;
}

std::unique_ptr<PlatformWindow> FlutterAceView::GetPlatformWindow()
{
    return nullptr;
}

void FlutterAceView::InitIOManager(RefPtr<TaskExecutor> taskExecutor)
{
    if (!SystemProperties::GetGpuUploadEnabled()) {
        return;
    }
#if defined(ENABLE_ROSEN_BACKEND) and !defined(UPLOAD_GPU_DISABLED)
    ACE_SCOPED_TRACE("InitIOManager");
    EGLContext shareContext = nullptr;
    EGLSurface surface = nullptr;
    auto callback = [&shareContext, &surface]() {
        ACE_SCOPED_TRACE("create egl ");
        EGLContext context = eglGetCurrentContext();
        if (context == EGL_NO_CONTEXT) {
            LOGE("eglGetCurrentContext failed errorCode = [%{public}d]", eglGetError());
            return;
        }
        EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            LOGE("eglGetDisplay failed errorCode = [%{public}d]", eglGetError());
            return;
        }
        EGLint attributes[] = {
            // clang-format off
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_DEPTH_SIZE,      0,
        EGL_STENCIL_SIZE,    0,
        EGL_NONE,            // termination sentinel
            // clang-format on
        };
        EGLint config_count = 0;
        EGLConfig egl_config = nullptr;
        if (eglChooseConfig(display, attributes, &egl_config, 1, &config_count) != EGL_TRUE) {
            LOGE("Get EGLConfig failed errorCode = [%{public}d]", eglGetError());
            return;
        }
        EGLint contextAttr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
        shareContext = eglCreateContext(display, egl_config, context, contextAttr);
        if (shareContext == EGL_NO_CONTEXT) {
            LOGE("eglCreateContext failed errorCode = [%{public}d]", eglGetError());
            return;
        }
        const EGLint attribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
        surface = eglCreatePbufferSurface(display, egl_config, attribs);
        if (surface == EGL_NO_SURFACE) {
            LOGE("eglCreatePbufferSurface failed errorCode = [%{public}d]", eglGetError());
            return;
        }
        LOGI("create egl success");
    };
    auto task = std::make_shared<AceRosenSyncTask>(std::move(callback));
    Rosen::RSTransactionProxy::GetInstance()->ExecuteSynchronousTask(task);

    if (shareContext == EGL_NO_CONTEXT || surface == EGL_NO_SURFACE) {
        LOGW("create egl env failed, image should not upload to gpu.");
        return;
    }
    auto state = flutter::UIDartState::Current()->GetStateById(instanceId_);
    CHECK_NULL_VOID(state);
    fml::WeakPtr<flutter::ShellIOManager> ioManager = state->GetIOManager();
    taskExecutor->PostSyncTask(
        [surface, shareContext, ioManager]() {
            ACE_SCOPED_TRACE("create resource_context ");
            if (eglMakeCurrent(eglGetDisplay(EGL_DEFAULT_DISPLAY), surface, surface, shareContext) == EGL_TRUE) {
                sk_sp<GrContext> resource_context = flutter::ShellIOManager::CreateCompatibleResourceLoadingContext(
                    GrBackend::kOpenGL_GrBackend, flutter::GPUSurfaceGLDelegate::GetDefaultPlatformGLInterface());

                ioManager->NotifyResourceContextAvailable(resource_context);
            }
        },
        TaskExecutor::TaskType::IO);
#endif
}

} // namespace OHOS::Ace::Platform