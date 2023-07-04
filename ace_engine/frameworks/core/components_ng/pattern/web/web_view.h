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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_WEB_VIEW_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_WEB_VIEW_H

#include <functional>
#include <string>

#include "core/components/web/web_property.h"

namespace OHOS::Ace::NG {

using OnWebSyncFunc = std::function<bool(const std::shared_ptr<BaseEventInfo>& info)>;
using OnWebAsyncFunc = std::function<void(const std::shared_ptr<BaseEventInfo>& info)>;
using SetWebIdCallback = std::function<void(int32_t)>;
using JsProxyCallback = std::function<void()>;

class ACE_EXPORT WebView {
public:
    static void Create(const std::string& webData);
    static void Create(const std::string& src, const RefPtr<WebController>& webController);
    static void Create(const std::string& src, SetWebIdCallback&& setWebIdCallback,
        int32_t parentWebId = -1, bool popup = false);
    static void SetOnCommonDialogImpl(OnWebSyncFunc&& onCommonDialogImpl, DialogEventType dialogEventType);
    static void SetOnPageStart(OnWebAsyncFunc&& onPageStart);
    static void SetOnPageFinish(OnWebAsyncFunc&& onPageEnd);
    static void SetOnHttpErrorReceive(OnWebAsyncFunc&& onHttpErrorReceive);
    static void SetOnErrorReceive(OnWebAsyncFunc&& onErrorReceive);
    static void SetOnConsole(OnWebSyncFunc&& onConsole);
    static void SetJsEnabled(bool isJsEnabled);
    static void SetProgressChangeImpl(OnWebAsyncFunc&& onProgressChangeImpl);
    static void SetTitleReceiveEventId(OnWebAsyncFunc&& titleReceiveEventId);
    static void SetFullScreenExitEventId(OnWebAsyncFunc&& fullScreenExitEventId);
    static void SetGeolocationHideEventId(OnWebAsyncFunc&& geolocationHideEventId);
    static void SetGeolocationShowEventId(OnWebAsyncFunc&& geolocationShowEventId);
    static void SetRequestFocusEventId(OnWebAsyncFunc&& requestFocusEventId);
    static void SetDownloadStartEventId(OnWebAsyncFunc&& downloadStartEventId);
    static void SetOnFullScreenEnterImpl(OnWebAsyncFunc&& onFullScreenEnterImpl);
    static void SetOnHttpAuthRequestImpl(OnWebSyncFunc&& onHttpAuthRequestImpl);
    static void SetOnSslErrorRequestImpl(OnWebSyncFunc&& onSslErrorRequestImpl);
    static void SetOnSslSelectCertRequestImpl(OnWebSyncFunc&& onSslSelectCertRequestImpl);
    static void SetMediaPlayGestureAccess(bool isNeedGestureAccess);
    static void SetOnKeyEventCallback(std::function<void(KeyEventInfo& keyEventInfo)>&& onKeyEventId);
    static void SetOnInterceptRequest(
        std::function<RefPtr<WebResponse>(const std::shared_ptr<BaseEventInfo>& info)>&& onInterceptRequestImpl);
    static void SetOnUrlLoadIntercept(OnWebSyncFunc&& onUrlLoadInterceptImpl);
    static void SetOnFileSelectorShow(OnWebSyncFunc&& onFileSelectorShowImpl);
    static void SetOnContextMenuShow(OnWebSyncFunc&& onContextMenuImpl);
    static void SetFileAccessEnabled(bool isFileAccessEnabled);
    static void SetOnLineImageAccessEnabled(bool isOnLineImageAccessEnabled);
    static void SetDomStorageAccessEnabled(bool isDomStorageAccessEnabled);
    static void SetImageAccessEnabled(bool isImageAccessEnabled);
    static void SetMixedMode(MixedModeContent mixedContentMode);
    static void SetZoomAccessEnabled(bool isZoomAccessEnabled);
    static void SetGeolocationAccessEnabled(bool isGeolocationAccessEnabled);
    static void SetUserAgent(const std::string& userAgent);
    static void SetCustomScheme(const std::string& customScheme);
    static void SetRenderExitedId(OnWebAsyncFunc&& renderExitedId);
    static void SetRefreshAccessedHistoryId(OnWebAsyncFunc&& refreshAccessedHistoryId);
    static void SetCacheMode(WebCacheMode mode);
    static void SetOverviewModeAccessEnabled(bool isOverviewModeAccessEnabled);
    static void SetFileFromUrlAccessEnabled(bool isFileFromUrlAccessEnabled);
    static void SetDatabaseAccessEnabled(bool isDatabaseAccessEnabled);
    static void SetTextZoomRatio(int32_t textZoomRatioNum);
    static void SetWebDebuggingAccessEnabled(bool isWebDebuggingAccessEnabled);
    static void SetOnMouseEventCallback(std::function<void(MouseInfo& info)>&& onMouseId);
    static void SetResourceLoadId(OnWebAsyncFunc&& resourceLoadId);
    static void SetScaleChangeId(OnWebAsyncFunc&& scaleChangeId);
    static void SetScrollId(OnWebAsyncFunc&& scrollId);
    static void SetPermissionRequestEventId(OnWebAsyncFunc&& permissionRequestEventId);
    static void SetBackgroundColor(int32_t backgroundColor);
    static void SetInitialScale(float scale);
    static void SetSearchResultReceiveEventId(OnWebAsyncFunc&& searchResultReceiveEventId);
    static void SetPinchSmoothModeEnabled(bool isPinchSmoothModeEnabled);
    static void SetWindowNewEvent(OnWebAsyncFunc&& windowNewEventId);
    static void SetWindowExitEventId(OnWebAsyncFunc&& windowExitEventId);
    static void SetMultiWindowAccessEnabled(bool isMultiWindowAccessEnabled);
    static void SetJsProxyCallback(JsProxyCallback&& jsProxyCallback);
    static void SetWebCursiveFont(const std::string& cursiveFontFamily);
    static void SetWebFantasyFont(const std::string& fantasyFontFamily);
    static void SetWebFixedFont(const std::string& fixedFontFamily);
    static void SetWebSansSerifFont(const std::string& sansSerif);
    static void SetWebSerifFont(const std::string& serifFontFamily);
    static void SetWebStandardFont(const std::string& standardFontFamily);
    static void SetDefaultFixedFontSize(int32_t defaultFixedFontSize);
    static void SetDefaultFontSize(int32_t defaultFontSize);
    static void SetMinFontSize(int32_t minFontSize);
    static void SetMinLogicalFontSize(int32_t minLogicalFontSize);
    static void SetBlockNetwork(bool isNetworkBlocked);
    static void SetPageVisibleId(OnWebAsyncFunc&& pageVisibleId);
    static void SetOnInterceptKeyEventCallback(std::function<bool(KeyEventInfo& keyEventInfo)>&& onPreKeyEventId);
    static void SetDataResubmittedId(OnWebAsyncFunc&& DataResubmittedId);
    static void SetFaviconReceivedId(OnWebAsyncFunc&& faviconReceivedId);
    static void SetTouchIconUrlId(OnWebAsyncFunc&& touchIconUrlId);
    static void SetDarkMode(WebDarkMode mode);
    static void SetForceDarkAccess(bool access);
    static void SetHorizontalScrollBarAccessEnabled(bool isHorizontalScrollBarAccessEnabled);
    static void SetVerticalScrollBarAccessEnabled(bool isVerticalScrollBarAccessEnabled);
    static void NotifyPopupWindowResult(int32_t webId, bool result);

private:
    static void RegisterPipelineCallback(int32_t nodeId);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_WEB_VIEW_H
