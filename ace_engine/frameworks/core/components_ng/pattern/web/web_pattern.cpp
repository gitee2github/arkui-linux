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

#include "core/components_ng/pattern/web/web_pattern.h"

#include "base/geometry/ng/offset_t.h"
#include "base/mousestyle/mouse_style.h"
#include "base/utils/linear_map.h"
#include "base/utils/utils.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/components/web/resource/web_delegate.h"
#include "core/components/web/web_property.h"
#include "core/components_ng/pattern/web/web_event_hub.h"
#include "core/event/key_event.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"

#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/base/view_stack_processor.h"

namespace OHOS::Ace::NG {
namespace {
const LinearEnumMapNode<OHOS::NWeb::CursorType, MouseFormat> g_cursorTypeMap[] = {
    { OHOS::NWeb::CursorType::CT_CROSS, MouseFormat::CROSS },
    { OHOS::NWeb::CursorType::CT_HAND, MouseFormat::HAND_POINTING },
    { OHOS::NWeb::CursorType::CT_IBEAM, MouseFormat::TEXT_CURSOR },
    { OHOS::NWeb::CursorType::CT_HELP, MouseFormat::HELP },
    { OHOS::NWeb::CursorType::CT_EASTRESIZE, MouseFormat::WEST_EAST },
    { OHOS::NWeb::CursorType::CT_NORTHRESIZE, MouseFormat::NORTH_SOUTH },
    { OHOS::NWeb::CursorType::CT_NORTHEASTRESIZE, MouseFormat::NORTH_EAST_SOUTH_WEST },
    { OHOS::NWeb::CursorType::CT_NORTHWESTRESIZE, MouseFormat::NORTH_WEST_SOUTH_EAST },
    { OHOS::NWeb::CursorType::CT_SOUTHRESIZE, MouseFormat::NORTH_SOUTH },
    { OHOS::NWeb::CursorType::CT_SOUTHEASTRESIZE, MouseFormat::NORTH_WEST_SOUTH_EAST },
    { OHOS::NWeb::CursorType::CT_SOUTHWESTRESIZE, MouseFormat::NORTH_EAST_SOUTH_WEST },
    { OHOS::NWeb::CursorType::CT_WESTRESIZE, MouseFormat::WEST_EAST },
    { OHOS::NWeb::CursorType::CT_NORTHSOUTHRESIZE, MouseFormat::NORTH_SOUTH },
    { OHOS::NWeb::CursorType::CT_EASTWESTRESIZE, MouseFormat::WEST_EAST },
    { OHOS::NWeb::CursorType::CT_NORTHEASTSOUTHWESTRESIZE, MouseFormat::NORTH_EAST_SOUTH_WEST },
    { OHOS::NWeb::CursorType::CT_NORTHWESTSOUTHEASTRESIZE, MouseFormat::NORTH_WEST_SOUTH_EAST },
    { OHOS::NWeb::CursorType::CT_COLUMNRESIZE, MouseFormat::RESIZE_LEFT_RIGHT },
    { OHOS::NWeb::CursorType::CT_ROWRESIZE, MouseFormat::RESIZE_UP_DOWN },
    { OHOS::NWeb::CursorType::CT_MOVE, MouseFormat::CURSOR_MOVE },
    { OHOS::NWeb::CursorType::CT_NODROP, MouseFormat::CURSOR_FORBID },
    { OHOS::NWeb::CursorType::CT_COPY, MouseFormat::CURSOR_COPY },
    { OHOS::NWeb::CursorType::CT_NOTALLOWED, MouseFormat::CURSOR_FORBID },
    { OHOS::NWeb::CursorType::CT_ZOOMIN, MouseFormat::ZOOM_IN },
    { OHOS::NWeb::CursorType::CT_ZOOMOUT, MouseFormat::ZOOM_OUT },
    { OHOS::NWeb::CursorType::CT_GRABBING, MouseFormat::HAND_GRABBING },
};
} // namespace

constexpr int32_t SINGLE_CLICK_NUM = 1;
constexpr int32_t DOUBLE_CLICK_NUM = 2;
constexpr double DEFAULT_DBCLICK_INTERVAL = 0.5;
constexpr double DEFAULT_DBCLICK_OFFSET = 2.0;
constexpr double DEFAULT_AXIS_RATIO = -0.06;

WebPattern::WebPattern() = default;

WebPattern::WebPattern(std::string webSrc, const RefPtr<WebController>& webController)
    : webSrc_(std::move(webSrc)), webController_(webController)
{}

WebPattern::WebPattern(std::string webSrc, const SetWebIdCallback& setWebIdCallback)
    : webSrc_(std::move(webSrc)), setWebIdCallback_(setWebIdCallback)
{}

WebPattern::~WebPattern()
{
    LOGD("WebPattern::~WebPattern");
    if (observer_) {
        LOGD("WebPattern::~WebPattern NotifyDestory");
        observer_->NotifyDestory();
    }
}

void WebPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
    host->GetRenderContext()->UpdateBackgroundColor(Color::WHITE);
    host->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
}

void WebPattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    CHECK_NULL_VOID(delegate_);
    isFocus_ = false;
    delegate_->OnBlur();
    OnQuickMenuDismissed();
}

void WebPattern::InitEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);

    auto gestureHub = eventHub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);

    InitTouchEvent(gestureHub);
    InitDragEvent(gestureHub);
    InitPanEvent(gestureHub);

    auto inputHub = eventHub->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);
    InitMouseEvent(inputHub);

    auto focusHub = eventHub->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    InitFocusEvent(focusHub);

    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto langTask = [weak = AceType::WeakClaim(this)]() {
        auto WebPattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(WebPattern);
        WebPattern->UpdateLocale();
    };
    context->SetConfigChangedCallback(std::move(langTask));
}

void WebPattern::InitPanEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (panEvent_) {
        return;
    }
    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& event) {
        return;
    };
    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& event) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleDragMove(event);
    };
    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        return;
    };
    auto actionCancelTask = [weak = WeakClaim(this)]() {
        return;
    };
    PanDirection panDirection;
    panDirection.type = PanDirection::VERTICAL;
    panEvent_ = MakeRefPtr<PanEvent>(
        std::move(actionStartTask), std::move(actionUpdateTask), std::move(actionEndTask), std::move(actionCancelTask));
    gestureHub->AddPanEvent(panEvent_, panDirection, DEFAULT_PAN_FINGER, DEFAULT_PAN_DISTANCE);
}

void WebPattern::HandleDragMove(const GestureEvent& event)
{
    if (event.GetInputEventType() == InputEventType::AXIS) {
        CHECK_NULL_VOID(delegate_);
        auto localLocation = event.GetLocalLocation();
        delegate_->HandleAxisEvent(
            localLocation.GetX(), localLocation.GetY(),
            event.GetDelta().GetX() * DEFAULT_AXIS_RATIO,
            event.GetDelta().GetY() * DEFAULT_AXIS_RATIO);
    }
}

void WebPattern::InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (touchEvent_) {
        return;
    }

    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (info.GetChangedTouches().empty()) {
            LOGE("touch point is null");
            return;
        }

        // only handle touch event
        if (info.GetSourceDevice() != SourceType::TOUCH) {
            return;
        }

        const auto& changedPoint = info.GetChangedTouches().front();
        if (changedPoint.GetTouchType() == TouchType::DOWN) {
            pattern->HandleTouchDown(info, false);
            return;
        }
        if (changedPoint.GetTouchType() == TouchType::MOVE) {
            pattern->HandleTouchMove(info, false);
            return;
        }
        if (changedPoint.GetTouchType() == TouchType::UP) {
            pattern->HandleTouchUp(info, false);
            return;
        }
        if (changedPoint.GetTouchType() == TouchType::CANCEL) {
            pattern->HandleTouchCancel(info);
            return;
        }
    };
    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gestureHub->AddTouchEvent(touchEvent_);
}

void WebPattern::InitMouseEvent(const RefPtr<InputEventHub>& inputHub)
{
    if (mouseEvent_) {
        return;
    }

    auto mouseTask = [weak = WeakClaim(this)](MouseInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleMouseEvent(info);
    };

    mouseEvent_ = MakeRefPtr<InputEvent>(std::move(mouseTask));
    inputHub->AddOnMouseEvent(mouseEvent_);
}

void WebPattern::HandleMouseEvent(MouseInfo& info)
{
    WebOnMouseEvent(info);

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto mouseEventCallback = eventHub->GetOnMouseEvent();
    CHECK_NULL_VOID(mouseEventCallback);
    mouseEventCallback(info);
}

void WebPattern::WebOnMouseEvent(const MouseInfo& info)
{
    CHECK_NULL_VOID(delegate_);
    if (info.GetAction() == MouseAction::RELEASE) {
        WebRequestFocus();
    }

    auto localLocation = info.GetLocalLocation();
    if (!HandleDoubleClickEvent(info)) {
        delegate_->OnMouseEvent(
            localLocation.GetX(), localLocation.GetY(), info.GetButton(), info.GetAction(), SINGLE_CLICK_NUM);
    }
}

bool WebPattern::HandleDoubleClickEvent(const MouseInfo& info)
{
    if (info.GetButton() != MouseButton::LEFT_BUTTON || info.GetAction() != MouseAction::PRESS) {
        return false;
    }
    auto localLocation = info.GetLocalLocation();
    MouseClickInfo clickInfo;
    clickInfo.x = localLocation.GetX();
    clickInfo.y = localLocation.GetY();
    clickInfo.start = info.GetTimeStamp();
    if (doubleClickQueue_.empty()) {
        doubleClickQueue_.push(clickInfo);
        return false;
    }
    std::chrono::duration<float> timeout_ = clickInfo.start - doubleClickQueue_.back().start;
    double offsetX = clickInfo.x - doubleClickQueue_.back().x;
    double offsetY = clickInfo.y - doubleClickQueue_.back().y;
    double offset = sqrt(offsetX * offsetX + offsetY * offsetY);
    if (timeout_.count() < DEFAULT_DBCLICK_INTERVAL && offset < DEFAULT_DBCLICK_OFFSET) {
        SendDoubleClickEvent(clickInfo);
        std::queue<MouseClickInfo> empty;
        swap(empty, doubleClickQueue_);
        return true;
    }
    if (doubleClickQueue_.size() == 1) {
        doubleClickQueue_.push(clickInfo);
        return false;
    }
    doubleClickQueue_.pop();
    doubleClickQueue_.push(clickInfo);
    return false;
}

void WebPattern::SendDoubleClickEvent(const MouseClickInfo& info)
{
    CHECK_NULL_VOID(delegate_);
    delegate_->OnMouseEvent(info.x, info.y, MouseButton::LEFT_BUTTON, MouseAction::PRESS, DOUBLE_CLICK_NUM);
}

bool WebPattern::GenerateDragDropInfo(NG::DragDropInfo& dragDropInfo)
{
    if (delegate_) {
        dragDropInfo.pixelMap = delegate_->GetDragPixelMap();
    }

    if (dragDropInfo.pixelMap) {
        LOGI("get w3c drag info success");
        isW3cDragEvent_ = true;
        return true;
    }

    return false;
}

void WebPattern::InitCommonDragDropEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);

    auto userOnDragStartFunc = eventHub->GetOnDragStart();
    auto onDragStartId = [weak = WeakClaim(this), userOnDragStartFunc](const RefPtr<OHOS::Ace::DragEvent>& info,
                                const std::string& extraParams) -> NG::DragDropInfo {
        auto pattern = weak.Upgrade();
        NG::DragDropInfo dragDropInfo;
        CHECK_NULL_RETURN_NOLOG(pattern, dragDropInfo);
        if (pattern->GenerateDragDropInfo(dragDropInfo)) {
            return dragDropInfo;
        }

        LOGI("not w3c drag event, try to get drag info from user callback");
        if (userOnDragStartFunc) {
            LOGI("user callback has been set, get from user callback");
            dragDropInfo = userOnDragStartFunc(info, extraParams);
            return dragDropInfo;
        }
        return dragDropInfo;
    };

    // init common drag drop event
    gestureHub->InitDragDropEvent();
    // set custom OnDragStart function
    eventHub->SetOnDragStart(std::move(onDragStartId));
}

void WebPattern::InitDragEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (dragEvent_) {
        return;
    }

    LOGI("web init DragEvent");
    InitCommonDragDropEvent(gestureHub);

    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleDragStart(info);
    };

    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleDragUpdate(info);
    };

    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleDragEnd(info);
    };

    auto actionCancelTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleDragCancel();
    };

    float distance = DEFAULT_PAN_DISTANCE;
    auto host = GetHost();
    if (host) {
        auto context = host->GetContext();
        if (context) {
            distance = static_cast<float>(context->NormalizeToPx(Dimension(DEFAULT_PAN_DISTANCE, DimensionUnit::VP)));
        }
    }

    dragEvent_ = MakeRefPtr<DragEvent>(
        std::move(actionStartTask), std::move(actionUpdateTask), std::move(actionEndTask), std::move(actionCancelTask));
    gestureHub->SetCustomDragEvent(dragEvent_, { PanDirection::ALL }, DEFAULT_PAN_FINGER, DEFAULT_PAN_DISTANCE);
}

void WebPattern::HandleDragStart(const GestureEvent& info)
{
    isDragging_ = true;
    if (!isW3cDragEvent_ || !delegate_) {
        LOGI("drag start, don't need to invoke web delegate interface");
        return;
    }

    LOGI("web drag action start");
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = host->GetContext();
    CHECK_NULL_VOID(pipelineContext);
    int32_t globalX = static_cast<int32_t>(info.GetGlobalPoint().GetX());
    int32_t globalY = static_cast<int32_t>(info.GetGlobalPoint().GetY());
    auto viewScale = pipelineContext->GetViewScale();
    auto offset = GetCoordinatePoint();
    int32_t localX = static_cast<int32_t>(globalX - offset.value_or(OffsetF()).GetX());
    int32_t localY = static_cast<int32_t>(globalY - offset.value_or(OffsetF()).GetY());
    delegate_->HandleDragEvent(localX * viewScale, localY * viewScale, DragAction::DRAG_ENTER);
}

void WebPattern::HandleDragUpdate(const GestureEvent& info)
{
    if (!isW3cDragEvent_ || !delegate_) {
        LOGD("drag update, don't need to invoke web delegate interface");
        return;
    }

    LOGD("web drag action update");
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = host->GetContext();
    CHECK_NULL_VOID(pipelineContext);
    int32_t globalX = static_cast<int32_t>(info.GetGlobalPoint().GetX());
    int32_t globalY = static_cast<int32_t>(info.GetGlobalPoint().GetY());
    LOGD("web drag position update, x = %{public}d, y = %{public}d", globalX, globalY);
    auto viewScale = pipelineContext->GetViewScale();
    auto offset = GetCoordinatePoint();
    int32_t localX = static_cast<int32_t>(globalX - offset.value_or(OffsetF()).GetX());
    int32_t localY = static_cast<int32_t>(globalY - offset.value_or(OffsetF()).GetY());
    delegate_->HandleDragEvent(localX * viewScale, localY * viewScale, DragAction::DRAG_OVER);
}

void WebPattern::HandleDragEnd(const GestureEvent& info)
{
    isDragging_ = false;
    if (!isW3cDragEvent_ || !delegate_) {
        LOGI("drag end, don't need to invoke web delegate interface");
        return;
    }

    LOGI("web drag action end");
    isW3cDragEvent_ = false;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = host->GetContext();
    CHECK_NULL_VOID(pipelineContext);
    auto viewScale = pipelineContext->GetViewScale();
    auto offset = GetCoordinatePoint();
    int32_t localX = static_cast<int32_t>(info.GetGlobalPoint().GetX() - offset.value_or(OffsetF()).GetX());
    int32_t localY = static_cast<int32_t>(info.GetGlobalPoint().GetY() - offset.value_or(OffsetF()).GetY());
    delegate_->HandleDragEvent(localX * viewScale, localY * viewScale, DragAction::DRAG_DROP);
    delegate_->HandleDragEvent(localX * viewScale, localY * viewScale, DragAction::DRAG_END);
}

void WebPattern::HandleDragCancel()
{
    isDragging_ = false;
    if (!isW3cDragEvent_ || !delegate_) {
        LOGI("drag cancel, don't need to invoke web delegate interface");
        return;
    }

    LOGI("web drag cancel");
    isW3cDragEvent_ = false;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipelineContext = host->GetContext();
    CHECK_NULL_VOID(pipelineContext);
    delegate_->HandleDragEvent(0, 0, DragAction::DRAG_CANCEL);
}

void WebPattern::InitFocusEvent(const RefPtr<FocusHub>& focusHub)
{
    auto focusTask = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleFocusEvent();
    };
    focusHub->SetOnFocusInternal(focusTask);

    auto blurTask = [weak = WeakClaim(this)](const BlurReason& blurReason) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(pattern);
        pattern->HandleBlurEvent(blurReason);
    };
    focusHub->SetOnBlurReasonInternal(blurTask);

    auto keyTask = [weak = WeakClaim(this)](const KeyEvent& keyEvent) -> bool {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->HandleKeyEvent(keyEvent);
    };
    focusHub->SetOnKeyEventInternal(keyTask);
}

void WebPattern::HandleFocusEvent()
{
    CHECK_NULL_VOID(delegate_);
    isFocus_ = true;
    if (needOnFocus_) {
        delegate_->OnFocus();
    } else {
        needOnFocus_ = true;
    }
}

void WebPattern::HandleBlurEvent(const BlurReason& blurReason)
{
    CHECK_NULL_VOID(delegate_);
    isFocus_ = false;
    if (!selectPopupMenuShowing_) {
        delegate_->SetBlurReason(static_cast<OHOS::NWeb::BlurReason>(blurReason));
        delegate_->OnBlur();
    }
    OnQuickMenuDismissed();
}

bool WebPattern::HandleKeyEvent(const KeyEvent& keyEvent)
{
    bool ret = false;

    auto host = GetHost();
    CHECK_NULL_RETURN(host, ret);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_RETURN(eventHub, ret);

    KeyEventInfo info(keyEvent);
    auto keyEventCallback = eventHub->GetOnKeyEvent();
    if (keyEventCallback) {
        keyEventCallback(info);
    }

    auto preKeyEventCallback = eventHub->GetOnPreKeyEvent();
    if (preKeyEventCallback) {
        ret = preKeyEventCallback(info);
        if (ret) {
            LOGI("keyevent consumed in hap");
            return ret;
        }
    }

    ret = WebOnKeyEvent(keyEvent);
    return ret;
}

bool WebPattern::WebOnKeyEvent(const KeyEvent& keyEvent)
{
    CHECK_NULL_RETURN(delegate_, false);
    return delegate_->OnKeyEvent(static_cast<int32_t>(keyEvent.code), static_cast<int32_t>(keyEvent.action));
}

void WebPattern::WebRequestFocus()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto focusHub = eventHub->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);

    focusHub->RequestFocusImmediately();
}

bool WebPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (!config.contentSizeChange) {
        return false;
    }
    CHECK_NULL_RETURN(delegate_, false);
    CHECK_NULL_RETURN(dirty, false);
    auto geometryNode = dirty->GetGeometryNode();
    auto drawSize = Size(geometryNode->GetContentSize().Width(), geometryNode->GetContentSize().Height());
    if (drawSize.IsInfinite() || drawSize.IsEmpty()) {
        LOGE("resize invalid %{public}f, %{public}f", drawSize.Width(), drawSize.Height());
        return false;
    }

    drawSize_ = Size(drawSize.Width(), drawSize.Height());
    drawSizeCache_ = drawSize_;
    auto offset = Offset(GetCoordinatePoint()->GetX(), GetCoordinatePoint()->GetY());
    delegate_->SetBoundsOrResize(drawSize_, offset);
    // first update size to load url.
    if (!isUrlLoaded_) {
        isUrlLoaded_ = true;
        if (webSrc_) {
            delegate_->LoadUrl();
        } else if (webData_) {
            delegate_->LoadDataWithRichText();
        }
    }

    return false;
}

void WebPattern::OnAreaChangedInner()
{
    auto offset = OffsetF(GetCoordinatePoint()->GetX(), GetCoordinatePoint()->GetY());
    if (webOffset_ == offset) {
        return;
    }
    webOffset_ = offset;
    UpdateTouchHandleForOverlay();
    auto resizeOffset = Offset(offset.GetX(), offset.GetY());
    delegate_->SetBoundsOrResize(drawSize_, resizeOffset);
}

void WebPattern::OnWebSrcUpdate()
{
    if (delegate_ && isUrlLoaded_) {
        delegate_->LoadUrl();
    }
}

void WebPattern::OnWebDataUpdate()
{
    if (delegate_ && isUrlLoaded_) {
        delegate_->LoadDataWithRichText();
    }
}

void WebPattern::OnJsEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateJavaScriptEnabled(value);
    }
}

void WebPattern::OnMediaPlayGestureAccessUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateMediaPlayGestureAccess(value);
    }
}

void WebPattern::OnFileAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateAllowFileAccess(value);
    }
}

void WebPattern::OnOnLineImageAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateBlockNetworkImage(!value);
    }
}

void WebPattern::OnDomStorageAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateDomStorageEnabled(value);
    }
}

void WebPattern::OnImageAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateLoadsImagesAutomatically(value);
    }
}

void WebPattern::OnMixedModeUpdate(MixedModeContent value)
{
    if (delegate_) {
        delegate_->UpdateMixedContentMode(value);
    }
}

void WebPattern::OnZoomAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateSupportZoom(value);
    }
}

void WebPattern::OnGeolocationAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateGeolocationEnabled(value);
    }
}

void WebPattern::OnUserAgentUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateUserAgent(value);
    }
}

void WebPattern::OnCacheModeUpdate(WebCacheMode value)
{
    if (delegate_) {
        delegate_->UpdateCacheMode(value);
    }
}

void WebPattern::OnDarkModeUpdate(WebDarkMode mode)
{
    if (delegate_) {
        delegate_->UpdateDarkMode(mode);
    }
}

void WebPattern::OnForceDarkAccessUpdate(bool access)
{
    if (delegate_) {
        delegate_->UpdateForceDarkAccess(access);
    }
}

void WebPattern::OnOverviewModeAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateOverviewModeEnabled(value);
    }
}

void WebPattern::OnFileFromUrlAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateFileFromUrlEnabled(value);
    }
}

void WebPattern::OnDatabaseAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateDatabaseEnabled(value);
    }
}

void WebPattern::OnTextZoomRatioUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateTextZoomRatio(value);
    }
}

void WebPattern::OnWebDebuggingAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateWebDebuggingAccess(value);
    }
}

void WebPattern::OnPinchSmoothModeEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdatePinchSmoothModeEnabled(value);
    }
}

void WebPattern::OnBackgroundColorUpdate(int32_t value)
{
    UpdateBackgroundColorRightNow(value);
    if (delegate_) {
        delegate_->UpdateBackgroundColor(value);
    }
}

void WebPattern::OnInitialScaleUpdate(float value)
{
    if (delegate_) {
        delegate_->UpdateInitialScale(value);
    }
}

void WebPattern::OnMultiWindowAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateMultiWindowAccess(value);
    }
}

void WebPattern::OnWebCursiveFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebCursiveFont(value);
    }
}

void WebPattern::OnWebFantasyFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebFantasyFont(value);
    }
}

void WebPattern::OnWebFixedFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebFixedFont(value);
    }
}

void WebPattern::OnWebSansSerifFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebSansSerifFont(value);
    }
}

void WebPattern::OnWebSerifFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebSerifFont(value);
    }
}

void WebPattern::OnWebStandardFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebStandardFont(value);
    }
}

void WebPattern::OnDefaultFixedFontSizeUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateDefaultFixedFontSize(value);
    }
}

void WebPattern::OnDefaultFontSizeUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateDefaultFontSize(value);
    }
}

void WebPattern::OnMinFontSizeUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateMinFontSize(value);
    }
}

void WebPattern::OnMinLogicalFontSizeUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateMinLogicalFontSize(value);
    }
}

void WebPattern::OnBlockNetworkUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateBlockNetwork(value);
    }
}

void WebPattern::OnHorizontalScrollBarAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateHorizontalScrollBarAccess(value);
    }
}

void WebPattern::OnVerticalScrollBarAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateVerticalScrollBarAccess(value);
    }
}

void WebPattern::RegistVirtualKeyBoardListener()
{
    if (!needUpdateWeb_) {
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID_NOLOG(pipelineContext);
    pipelineContext->SetVirtualKeyBoardCallback(
        [weak = AceType::WeakClaim(this)](int32_t width, int32_t height, double keyboard) {
            auto webPattern = weak.Upgrade();
            CHECK_NULL_RETURN_NOLOG(webPattern, false);
            return webPattern->ProcessVirtualKeyBoard(width, height, keyboard);
        });
    needUpdateWeb_ = false;
}

void WebPattern::InitEnhanceSurfaceFlag()
{
    if (SystemProperties::GetExtSurfaceEnabled()) {
        isEnhanceSurface_ = true;
    } else {
        isEnhanceSurface_ = false;
    }
}

void WebPattern::OnModifyDone()
{
    // called in each update function.
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    RegistVirtualKeyBoardListener();
    if (!delegate_) {
        // first create case,
        delegate_ = AceType::MakeRefPtr<WebDelegate>(PipelineContext::GetCurrentContext(), nullptr, "");
        CHECK_NULL_VOID(delegate_);
        observer_ = AceType::MakeRefPtr<WebDelegateObserver>(delegate_, PipelineContext::GetCurrentContext());
        CHECK_NULL_VOID(observer_);
        delegate_->SetObserver(observer_);
        InitEnhanceSurfaceFlag();
        delegate_->SetNGWebPattern(Claim(this));
        delegate_->SetEnhanceSurfaceFlag(isEnhanceSurface_);
        delegate_->SetPopup(isPopup_);
        delegate_->SetParentNWebId(parentNWebId_);
        if (isEnhanceSurface_) {
            auto drawSize = Size(1, 1);
            delegate_->SetDrawSize(drawSize);
            delegate_->InitOHOSWeb(PipelineContext::GetCurrentContext());
        } else {
            renderSurface_->SetRenderContext(host->GetRenderContext());
            renderSurface_->InitSurface();
            delegate_->InitOHOSWeb(PipelineContext::GetCurrentContext(), renderSurface_);
        }
        delegate_->UpdateBackgroundColor(GetBackgroundColorValue(
            static_cast<int32_t>(renderContext->GetBackgroundColor().value_or(Color::WHITE).GetValue())));
        delegate_->UpdateJavaScriptEnabled(GetJsEnabledValue(true));
        delegate_->UpdateBlockNetworkImage(!GetOnLineImageAccessEnabledValue(true));
        delegate_->UpdateAllowFileAccess(GetFileAccessEnabledValue(true));
        delegate_->UpdateLoadsImagesAutomatically(GetImageAccessEnabledValue(true));
        delegate_->UpdateMixedContentMode(GetMixedModeValue(MixedModeContent::MIXED_CONTENT_NEVER_ALLOW));
        delegate_->UpdateSupportZoom(GetZoomAccessEnabledValue(true));
        delegate_->UpdateDomStorageEnabled(GetDomStorageAccessEnabledValue(false));
        delegate_->UpdateGeolocationEnabled(GetGeolocationAccessEnabledValue(true));
        delegate_->UpdateCacheMode(GetCacheModeValue(WebCacheMode::DEFAULT));
        delegate_->UpdateDarkMode(GetDarkModeValue(WebDarkMode::Off));
        delegate_->UpdateForceDarkAccess(GetForceDarkAccessValue(false));
        delegate_->UpdateOverviewModeEnabled(GetOverviewModeAccessEnabledValue(true));
        delegate_->UpdateFileFromUrlEnabled(GetFileFromUrlAccessEnabledValue(false));
        delegate_->UpdateDatabaseEnabled(GetDatabaseAccessEnabledValue(false));
        delegate_->UpdateTextZoomRatio(GetTextZoomRatioValue(DEFAULT_TEXT_ZOOM_RATIO));
        delegate_->UpdateWebDebuggingAccess(GetWebDebuggingAccessEnabledValue(false));
        delegate_->UpdateMediaPlayGestureAccess(GetMediaPlayGestureAccessValue(true));
        delegate_->UpdatePinchSmoothModeEnabled(GetPinchSmoothModeEnabledValue(false));
        delegate_->UpdateMultiWindowAccess(GetMultiWindowAccessEnabledValue(false));
        delegate_->UpdateWebCursiveFont(GetWebCursiveFontValue(DEFAULT_CURSIVE_FONT_FAMILY));
        delegate_->UpdateWebFantasyFont(GetWebFantasyFontValue(DEFAULT_FANTASY_FONT_FAMILY));
        delegate_->UpdateWebFixedFont(GetWebFixedFontValue(DEFAULT_FIXED_fONT_FAMILY));
        delegate_->UpdateWebSansSerifFont(GetWebSansSerifFontValue(DEFAULT_SANS_SERIF_FONT_FAMILY));
        delegate_->UpdateWebSerifFont(GetWebSerifFontValue(DEFAULT_SERIF_FONT_FAMILY));
        delegate_->UpdateWebStandardFont(GetWebStandardFontValue(DEFAULT_STANDARD_FONT_FAMILY));
        delegate_->UpdateDefaultFixedFontSize(GetDefaultFixedFontSizeValue(DEFAULT_FIXED_FONT_SIZE));
        delegate_->UpdateDefaultFontSize(GetDefaultFontSizeValue(DEFAULT_FONT_SIZE));
        delegate_->UpdateMinFontSize(GetMinFontSizeValue(DEFAULT_MINIMUM_FONT_SIZE));
        delegate_->UpdateMinLogicalFontSize(GetMinLogicalFontSizeValue(DEFAULT_MINIMUM_LOGICAL_FONT_SIZE));
        delegate_->UpdateHorizontalScrollBarAccess(GetHorizontalScrollBarAccessEnabledValue(true));
        delegate_->UpdateVerticalScrollBarAccess(GetVerticalScrollBarAccessEnabledValue(true));
        if (GetBlockNetwork()) {
            delegate_->UpdateBlockNetwork(GetBlockNetwork().value());
        }
        if (GetUserAgent()) {
            delegate_->UpdateUserAgent(GetUserAgent().value());
        }
        if (GetInitialScale()) {
            delegate_->UpdateInitialScale(GetInitialScale().value());
        }
    }

    // Initialize events such as keyboard, focus, etc.
    InitEvent();

    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->AddOnAreaChangeNode(host->GetId());
}

bool WebPattern::ProcessVirtualKeyBoard(int32_t width, int32_t height, double keyboard)
{
    LOGI("Web ProcessVirtualKeyBoard width=%{public}d height=%{public}d keyboard=%{public}f",
        width, height, keyboard);
    CHECK_NULL_RETURN(delegate_, false);
    if (!isFocus_) {
        if (isVirtualKeyBoardShow_ == VkState::VK_SHOW) {
            drawSize_.SetSize(drawSizeCache_);
            UpdateWebLayoutSize(width, height);
            isVirtualKeyBoardShow_ = VkState::VK_HIDE;
        }
        return false;
    }
    if (NearZero(keyboard)) {
        drawSize_.SetSize(drawSizeCache_);
        UpdateWebLayoutSize(width, height);
        isVirtualKeyBoardShow_ = VkState::VK_HIDE;
    } else if (isVirtualKeyBoardShow_ != VkState::VK_SHOW) {
        drawSizeCache_.SetSize(drawSize_);
        if (drawSize_.Height() <= (height - keyboard - GetCoordinatePoint()->GetY())) {
            isVirtualKeyBoardShow_ = VkState::VK_SHOW;
            return true;
        }
        if (height - GetCoordinatePoint()->GetY() < keyboard) {
            return true;
        }
        drawSize_.SetHeight(height - keyboard - GetCoordinatePoint()->GetY());
        UpdateWebLayoutSize(width, height);
        isVirtualKeyBoardShow_ = VkState::VK_SHOW;
    }
    return true;
}

void WebPattern::UpdateWebLayoutSize(int32_t width, int32_t height)
{
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto rect = frameNode->GetGeometryNode()->GetFrameRect();
    auto offset = Offset(GetCoordinatePoint()->GetX(), GetCoordinatePoint()->GetY());
    delegate_->SetBoundsOrResize(drawSize_, offset);
    rect.SetSize(SizeF(drawSize_.Width(), drawSize_.Height()));
    frameNode->GetRenderContext()->SyncGeometryProperties(rect);
    frameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    context->SetRootRect(width, height, 0);
}

void WebPattern::HandleTouchDown(const TouchEventInfo& info, bool fromOverlay)
{
    CHECK_NULL_VOID(delegate_);
    Offset touchOffset = Offset(0, 0);
    std::list<TouchInfo> touchInfos;
    if (!ParseTouchInfo(info, touchInfos)) {
        LOGE("handle touch down error");
        return;
    }
    for (auto& touchPoint : touchInfos) {
        if (fromOverlay) {
            touchPoint.x -= webOffset_.GetX();
            touchPoint.y -= webOffset_.GetY();
        }
        delegate_->HandleTouchDown(touchPoint.id, touchPoint.x, touchPoint.y);
    }
}

void WebPattern::HandleTouchUp(const TouchEventInfo& info, bool fromOverlay)
{
    CHECK_NULL_VOID(delegate_);
    std::list<TouchInfo> touchInfos;
    if (!ParseTouchInfo(info, touchInfos)) {
        LOGE("handle touch up error");
        return;
    }
    for (auto& touchPoint : touchInfos) {
        if (fromOverlay) {
            touchPoint.x -= webOffset_.GetX();
            touchPoint.y -= webOffset_.GetY();
        }
        delegate_->HandleTouchUp(touchPoint.id, touchPoint.x, touchPoint.y);
    }
    if (!touchInfos.empty()) {
        WebRequestFocus();
    }
}

void WebPattern::HandleTouchMove(const TouchEventInfo& info, bool fromOverlay)
{
    if (isDragging_) {
        return;
    }
    CHECK_NULL_VOID(delegate_);
    std::list<TouchInfo> touchInfos;
    if (!ParseTouchInfo(info, touchInfos)) {
        LOGE("handle touch move error");
        return;
    }
    for (auto& touchPoint : touchInfos) {
        if (fromOverlay) {
            touchPoint.x -= webOffset_.GetX();
            touchPoint.y -= webOffset_.GetY();
        }
        delegate_->HandleTouchMove(touchPoint.id, touchPoint.x, touchPoint.y);
    }
}

void WebPattern::HandleTouchCancel(const TouchEventInfo& /*info*/)
{
    CHECK_NULL_VOID(delegate_);
    delegate_->HandleTouchCancel();
}

bool WebPattern::ParseTouchInfo(const TouchEventInfo& info, std::list<TouchInfo>& touchInfos)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    auto viewScale = context->GetViewScale();
    if (info.GetTouches().empty()) {
        return false;
    }
    for (const auto& point : info.GetTouches()) {
        TouchInfo touchInfo;
        touchInfo.id = point.GetFingerId();
        const Offset& location = point.GetLocalLocation();
        touchInfo.x = static_cast<float>(location.GetX() * viewScale);
        touchInfo.y = static_cast<float>(location.GetY() * viewScale);
        touchInfos.emplace_back(touchInfo);
    }
    return true;
}

void WebPattern::RequestFullScreen()
{
    if (isFullScreen_) {
        LOGE("The Web is already full screen when FullScreen");
        return;
    }
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto fullScreenManager = context->GetFullScreenManager();
    CHECK_NULL_VOID(fullScreenManager);
    fullScreenManager->RequestFullScreen(host);
    isFullScreen_ = true;
}

void WebPattern::ExitFullScreen()
{
    if (!isFullScreen_) {
        LOGE("The Web is not full screen when ExitFullScreen");
        return;
    }
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto fullScreenManager = context->GetFullScreenManager();
    CHECK_NULL_VOID(fullScreenManager);
    fullScreenManager->ExitFullScreen(host);
    isFullScreen_ = false;
}

bool WebPattern::IsTouchHandleValid(
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> handle)
{
    return (handle != nullptr) && (handle->IsEnable());
}

bool WebPattern::IsTouchHandleShow(
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> handle)
{
    CHECK_NULL_RETURN_NOLOG(handle, false);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN_NOLOG(pipeline, false);
    int y = static_cast<int32_t>(handle->GetY() / pipeline->GetDipScale());
    int edgeHeight = static_cast<int32_t>(handle->GetEdgeHeight() / pipeline->GetDipScale()) - 1;
    if (handle->GetAlpha() > 0 && y >= edgeHeight &&
        GreatNotEqual(GetHostFrameSize().value_or(SizeF()).Height(), handle->GetY())) {
        return true;
    }
    return false;
}

WebOverlayType WebPattern::GetTouchHandleOverlayType(
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> insertHandle,
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> startSelectionHandle,
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endSelectionHandle)
{
    if (IsTouchHandleValid(insertHandle) &&
        !IsTouchHandleValid(startSelectionHandle) &&
        !IsTouchHandleValid(endSelectionHandle)) {
        return INSERT_OVERLAY;
    }

    if (!IsTouchHandleValid(insertHandle) &&
        IsTouchHandleValid(startSelectionHandle) &&
        IsTouchHandleValid(endSelectionHandle)) {
        return SELECTION_OVERLAY;
    }

    return INVALID_OVERLAY;
}

std::optional<OffsetF> WebPattern::GetCoordinatePoint()
{
    auto frameNode = GetHost();
    CHECK_NULL_RETURN_NOLOG(frameNode, std::nullopt);
    return frameNode->GetTransformRelativeOffset();
}

RectF WebPattern::ComputeTouchHandleRect(std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> touchHandle)
{
    RectF paintRect;
    auto offset = GetCoordinatePoint().value_or(OffsetF());
    auto size = GetHostFrameSize().value_or(SizeF());
    float edgeHeight = touchHandle->GetEdgeHeight();
    float x = touchHandle->GetX();
    float y = touchHandle->GetY();
    if (x > size.Width()) {
        x = offset.GetX() + size.Width();
    } else {
        x = x + offset.GetX();
    }

    if (y < 0) {
        y = offset.GetY();
    } else if (y > size.Height()) {
        y = offset.GetY() + size.Height();
    } else {
        y = y + offset.GetY();
        y = y - edgeHeight;
    }

    x = x - SelectHandleInfo::GetDefaultLineWidth().ConvertToPx() / 2;
    paintRect.SetOffset({ x, y });
    paintRect.SetSize({ SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), edgeHeight });
    return paintRect;
}

void WebPattern::CloseSelectOverlay()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    if (selectOverlayProxy_) {
        selectOverlayProxy_->Close();
        pipeline->GetSelectOverlayManager()->DestroySelectOverlay(selectOverlayProxy_);
        selectOverlayProxy_ = nullptr;
    }
}

void WebPattern::RegisterSelectOverlayCallback(SelectOverlayInfo& selectInfo,
    std::shared_ptr<OHOS::NWeb::NWebQuickMenuParams> params,
    std::shared_ptr<OHOS::NWeb::NWebQuickMenuCallback> callback)
{
    int32_t flags = params->GetEditStateFlags();
    if (flags & OHOS::NWeb::NWebQuickMenuParams::QM_EF_CAN_CUT) {
        selectInfo.menuCallback.onCut = [weak = AceType::WeakClaim(this), callback]() {
            CHECK_NULL_VOID_NOLOG(callback);
            callback->Continue(OHOS::NWeb::NWebQuickMenuParams::QM_EF_CAN_CUT,
                OHOS::NWeb::MenuEventFlags::EF_LEFT_MOUSE_BUTTON);
        };
    } else {
        selectInfo.menuInfo.showCut = false;
    }
    if (flags & OHOS::NWeb::NWebQuickMenuParams::QM_EF_CAN_COPY) {
        selectInfo.menuCallback.onCopy = [weak = AceType::WeakClaim(this), callback]() {
            CHECK_NULL_VOID_NOLOG(callback);
            callback->Continue(OHOS::NWeb::NWebQuickMenuParams::QM_EF_CAN_COPY,
                OHOS::NWeb::MenuEventFlags::EF_LEFT_MOUSE_BUTTON);
        };
    } else {
        selectInfo.menuInfo.showCopy = false;
    }
    if (flags & OHOS::NWeb::NWebQuickMenuParams::QM_EF_CAN_PASTE) {
        selectInfo.menuCallback.onPaste = [weak = AceType::WeakClaim(this), callback]() {
            CHECK_NULL_VOID_NOLOG(callback);
            callback->Continue(OHOS::NWeb::NWebQuickMenuParams::QM_EF_CAN_PASTE,
                OHOS::NWeb::MenuEventFlags::EF_LEFT_MOUSE_BUTTON);
        };
    } else {
        selectInfo.menuInfo.showPaste = false;
    }
    if (flags & OHOS::NWeb::NWebQuickMenuParams::QM_EF_CAN_SELECT_ALL) {
        selectInfo.menuCallback.onSelectAll = [weak = AceType::WeakClaim(this), callback]() {
            CHECK_NULL_VOID_NOLOG(callback);
            callback->Continue(OHOS::NWeb::NWebQuickMenuParams::QM_EF_CAN_SELECT_ALL,
                OHOS::NWeb::MenuEventFlags::EF_LEFT_MOUSE_BUTTON);
        };
    } else {
        selectInfo.menuInfo.showCopyAll = false;
    }
}

void WebPattern::RegisterSelectOverlayEvent(SelectOverlayInfo& selectInfo)
{
    selectInfo.onHandleMoveDone = [weak = AceType::WeakClaim(this)](const RectF& rectF, bool isFirst) {
        auto webPattern = weak.Upgrade();
        CHECK_NULL_VOID(webPattern);
        webPattern->UpdateTouchHandleForOverlay();
        webPattern->SetSelectOverlayDragging(false);
    };
    selectInfo.onTouchDown = [weak = AceType::WeakClaim(this)](const TouchEventInfo& info) {
        auto webPattern = weak.Upgrade();
        CHECK_NULL_VOID(webPattern);
        webPattern->HandleTouchDown(info, true);
    };
    selectInfo.onTouchUp = [weak = AceType::WeakClaim(this)](const TouchEventInfo& info) {
        auto webPattern = weak.Upgrade();
        CHECK_NULL_VOID(webPattern);
        webPattern->HandleTouchUp(info, true);
    };
    selectInfo.onTouchMove = [weak = AceType::WeakClaim(this)](const TouchEventInfo& info) {
        auto webPattern = weak.Upgrade();
        CHECK_NULL_VOID(webPattern);
        if (webPattern->IsSelectOverlayDragging()) {
            webPattern->HandleTouchMove(info, true);
        }
    };
    selectInfo.onHandleMoveStart = [weak = AceType::WeakClaim(this)](bool isFirst) {
        auto webPattern = weak.Upgrade();
        CHECK_NULL_VOID(webPattern);
        webPattern->SetSelectOverlayDragging(true);
    };
}

bool WebPattern::RunQuickMenu(std::shared_ptr<OHOS::NWeb::NWebQuickMenuParams> params,
    std::shared_ptr<OHOS::NWeb::NWebQuickMenuCallback> callback)
{
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> insertTouchHandle =
        params->GetTouchHandleState(OHOS::NWeb::NWebTouchHandleState::TouchHandleType::INSERT_HANDLE);
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> beginTouchHandle =
        params->GetTouchHandleState(OHOS::NWeb::NWebTouchHandleState::TouchHandleType::SELECTION_BEGIN_HANDLE);
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endTouchHandle =
        params->GetTouchHandleState(OHOS::NWeb::NWebTouchHandleState::TouchHandleType::SELECTION_END_HANDLE);
    WebOverlayType overlayType = GetTouchHandleOverlayType(insertTouchHandle,
                                                           beginTouchHandle,
                                                           endTouchHandle);
    if (overlayType == INVALID_OVERLAY) {
        return false;
    }
    if (selectOverlayProxy_) {
        CloseSelectOverlay();
    }
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto theme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_RETURN(theme, false);
    selectHotZone_ = theme->GetHandleHotZoneRadius().ConvertToPx();
    SelectOverlayInfo selectInfo;
    selectInfo.isSingleHandle = (overlayType == INSERT_OVERLAY);
    selectInfo.hitTestMode = HitTestMode::HTMDEFAULT;
    if (selectInfo.isSingleHandle) {
        selectInfo.firstHandle.isShow = IsTouchHandleShow(insertTouchHandle);
        selectInfo.firstHandle.paintRect = ComputeTouchHandleRect(insertTouchHandle);
        selectInfo.secondHandle.isShow = false;
    } else {
        selectInfo.firstHandle.isShow = IsTouchHandleShow(beginTouchHandle);
        selectInfo.firstHandle.paintRect = ComputeTouchHandleRect(beginTouchHandle);
        selectInfo.secondHandle.isShow = IsTouchHandleShow(endTouchHandle);
        selectInfo.secondHandle.paintRect = ComputeTouchHandleRect(endTouchHandle);
    }
    selectInfo.menuInfo.menuIsShow = true;
    RegisterSelectOverlayCallback(selectInfo, params, callback);
    RegisterSelectOverlayEvent(selectInfo);
    selectOverlayProxy_ = pipeline->GetSelectOverlayManager()->CreateAndShowSelectOverlay(selectInfo);
    selectMenuInfo_ = selectInfo.menuInfo;
    insertHandle_ = insertTouchHandle;
    startSelectionHandle_ = beginTouchHandle;
    endSelectionHandle_ = endTouchHandle;
    return selectOverlayProxy_ ? true : false;
}

void WebPattern::OnQuickMenuDismissed()
{
    CloseSelectOverlay();
}

void WebPattern::OnTouchSelectionChanged(std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> insertHandle,
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> startSelectionHandle,
    std::shared_ptr<OHOS::NWeb::NWebTouchHandleState> endSelectionHandle)
{
    WebOverlayType overlayType = GetTouchHandleOverlayType(insertHandle,
                                                           startSelectionHandle,
                                                           endSelectionHandle);
    if (overlayType == INVALID_OVERLAY) {
        CloseSelectOverlay();
        return;
    }
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(theme);
    selectHotZone_ = theme->GetHandleHotZoneRadius().ConvertToPx();
    insertHandle_ = insertHandle;
    startSelectionHandle_ = startSelectionHandle;
    endSelectionHandle_ = endSelectionHandle;
    if (!selectOverlayProxy_) {
        if (overlayType == INSERT_OVERLAY) {
            SelectOverlayInfo selectInfo;
            selectInfo.isSingleHandle = true;
            selectInfo.firstHandle.isShow = IsTouchHandleShow(insertHandle_);
            selectInfo.firstHandle.paintRect = ComputeTouchHandleRect(insertHandle_);
            selectInfo.secondHandle.isShow = false;
            selectInfo.menuInfo.menuDisable = true;
            selectInfo.menuInfo.menuIsShow = false;
            selectInfo.hitTestMode = HitTestMode::HTMDEFAULT;
            RegisterSelectOverlayEvent(selectInfo);
            selectOverlayProxy_ = pipeline->GetSelectOverlayManager()->CreateAndShowSelectOverlay(selectInfo);
        }
    } else {
        if (overlayType == INSERT_OVERLAY) {
            UpdateTouchHandleForOverlay();
        }
    }
}

bool WebPattern::OnCursorChange(const OHOS::NWeb::CursorType& type, const OHOS::NWeb::NWebCursorInfo& info)
{
    (void)info;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto windowId = pipeline->GetWindowId();
    auto mouseStyle = MouseStyle::CreateMouseStyle();
    int32_t curPointerStyle = 0;
    if (mouseStyle->GetPointerStyle(windowId, curPointerStyle) == -1) {
        LOGE("OnCursorChange GetPointerStyle failed");
        return false;
    }
    MouseFormat pointStyle = MouseFormat::DEFAULT;
    int64_t idx = BinarySearchFindIndex(g_cursorTypeMap, ArraySize(g_cursorTypeMap), type);
    if (idx >= 0) {
        pointStyle = g_cursorTypeMap[idx].value;
    }
    if ((int32_t)pointStyle != curPointerStyle) {
        mouseStyle->SetPointerStyle(windowId, pointStyle);
    }
    return true;
}

void WebPattern::OnSelectPopupMenu(std::shared_ptr<OHOS::NWeb::NWebSelectPopupMenuParam> params,
    std::shared_ptr<OHOS::NWeb::NWebSelectPopupMenuCallback> callback)
{
    CHECK_NULL_VOID(params);
    CHECK_NULL_VOID(callback);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto id = host->GetId();
    std::vector<SelectParam> selectParam;
    for (auto& item : params->menuItems) {
        selectParam.push_back({
            item.label, ""
        });
    }
    auto menu = MenuView::Create(selectParam, id);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto eventHub = host->GetEventHub<WebEventHub>();
    auto destructor = [weak = WeakClaim(this), id]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto pipeline = NG::PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto overlayManager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(overlayManager);
        pattern->SetSelectPopupMenuShowing(false);
        overlayManager->DeleteMenu(id);
    };
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnDisappear(destructor);

    WebPattern::RegisterSelectPopupCallback(menu, callback);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    overlayManager->RegisterOnHideMenu([weak = WeakClaim(this), callback]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        callback->Cancel();
        pattern->SetSelectPopupMenuShowing(false);
    });
    auto offset = GetSelectPopupPostion(params->bounds);
    selectPopupMenuShowing_ = true;
    overlayManager->ShowMenu(id, offset, menu);
}

void WebPattern::RegisterSelectPopupCallback(RefPtr<FrameNode>& menu,
    std::shared_ptr<OHOS::NWeb::NWebSelectPopupMenuCallback> callback)
{
    auto menuContainer = AceType::DynamicCast<FrameNode>(menu->GetChildAtIndex(0));
    CHECK_NULL_VOID(menuContainer);
    auto menuPattern = menuContainer->GetPattern<MenuPattern>();
    CHECK_NULL_VOID(menuPattern);
    auto options = menuPattern->GetOptions();
    for (auto&& option : options) {
        auto selectCallback = [callback](int32_t index) {
            std::vector<int32_t> indices { static_cast<int32_t>(index) };
            callback->Continue(indices);
        };
        auto optionNode = AceType::DynamicCast<FrameNode>(option);
        if (optionNode) {
            auto hub = optionNode->GetEventHub<OptionEventHub>();
            if (!hub) {
                continue;
            }
            hub->SetOnSelect(std::move(selectCallback));
            optionNode->MarkModifyDone();
        }
    }
}

OffsetF WebPattern::GetSelectPopupPostion(const OHOS::NWeb::SelectMenuBound& bounds)
{
    auto offset = GetCoordinatePoint().value_or(OffsetF());
    offset.AddX(bounds.x);
    offset.AddY(bounds.y + bounds.height);
    return offset;
}

void WebPattern::UpdateTouchHandleForOverlay()
{
    WebOverlayType overlayType = GetTouchHandleOverlayType(insertHandle_,
                                                           startSelectionHandle_,
                                                           endSelectionHandle_);
    CHECK_NULL_VOID_NOLOG(selectOverlayProxy_);
    if (overlayType == INVALID_OVERLAY) {
        CloseSelectOverlay();
        return;
    }
    SelectHandleInfo firstHandleInfo;
    SelectHandleInfo secondHandleInfo;
    SelectMenuInfo menuInfo;
    if (overlayType == INSERT_OVERLAY) {
        firstHandleInfo.isShow = IsTouchHandleShow(insertHandle_);
        firstHandleInfo.paintRect = ComputeTouchHandleRect(insertHandle_);
        menuInfo.menuDisable = true;
        menuInfo.menuIsShow = false;
        selectOverlayProxy_->UpdateFirstSelectHandleInfo(firstHandleInfo);
        selectOverlayProxy_->UpdateSelectMenuInfo(menuInfo);
    } else {
        firstHandleInfo.isShow = IsTouchHandleShow(startSelectionHandle_);
        firstHandleInfo.paintRect = ComputeTouchHandleRect(startSelectionHandle_);
        secondHandleInfo.isShow = IsTouchHandleShow(endSelectionHandle_);
        secondHandleInfo.paintRect = ComputeTouchHandleRect(endSelectionHandle_);
        selectOverlayProxy_->UpdateFirstSelectHandleInfo(firstHandleInfo);
        selectOverlayProxy_->UpdateSecondSelectHandleInfo(secondHandleInfo);
        selectOverlayProxy_->UpdateSelectMenuInfo(selectMenuInfo_);
    }
}

void WebPattern::UpdateLocale()
{
    CHECK_NULL_VOID(delegate_);
    delegate_->UpdateLocale();
}

void WebPattern::OnWindowShow()
{
    if (isWindowShow_) {
        return;
    }

    LOGI("web OnWindowShow called");
    CHECK_NULL_VOID(delegate_);
    delegate_->ShowWebView();
    isWindowShow_ = true;
}

void WebPattern::OnWindowHide()
{
    if (!isWindowShow_) {
        return;
    }

    LOGI("web OnWindowHide called");
    CHECK_NULL_VOID(delegate_);
    delegate_->HideWebView();
    needOnFocus_ = false;
    isWindowShow_ = false;
}

void WebPattern::OnInActive()
{
    if (!isActive_) {
        return;
    }

    LOGI("web OnInActive called");
    CHECK_NULL_VOID(delegate_);
    delegate_->OnInactive();
    isActive_ = false;
}

void WebPattern::OnActive()
{
    if (isActive_) {
        return;
    }

    LOGI("web OnActive called");
    CHECK_NULL_VOID(delegate_);
    delegate_->OnActive();
    isActive_ = true;
}

void WebPattern::OnVisibleChange(bool isVisible)
{
    if (!isVisible) {
        LOGI("web is not visible");
        CloseSelectOverlay();
    }
}

void WebPattern::UpdateBackgroundColorRightNow(int32_t color)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateBackgroundColor(Color(static_cast<uint32_t>(color)));
}
} // namespace OHOS::Ace::NG
