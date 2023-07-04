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

#include "frameworks/bridge/declarative_frontend/ng/declarative_frontend_ng.h"

#include "base/log/dump_log.h"
#include "core/common/thread_checker.h"
#include "frameworks/bridge/common/utils/utils.h"

namespace OHOS::Ace {

DeclarativeFrontendNG::~DeclarativeFrontendNG() noexcept
{
    LOG_DESTROY();
}

void DeclarativeFrontendNG::Destroy()
{
    CHECK_RUN_ON(JS);
    LOGI("DeclarativeFrontendNG Destroy begin.");
    // To guarantee the jsEngine_ and delegate_ released in js thread
    delegate_.Reset();
    jsEngine_->Destroy();
    jsEngine_.Reset();
    LOGI("DeclarativeFrontendNG Destroy end.");
}

bool DeclarativeFrontendNG::Initialize(FrontendType type, const RefPtr<TaskExecutor>& taskExecutor)
{
    LOGI("DeclarativeFrontendNG initialize begin.");
    type_ = type;
    ACE_DCHECK(type_ == FrontendType::DECLARATIVE_JS);
    InitializeDelegate(taskExecutor);
    bool needPostJsTask = true;
    auto container = Container::Current();
    if (container) {
        const auto& setting = container->GetSettings();
        needPostJsTask = !(setting.usePlatformAsUIThread && setting.useUIAsJSThread);
    }
    auto initJSEngineTask = [weakEngine = WeakPtr<Framework::JsEngine>(jsEngine_), delegate = delegate_] {
        auto jsEngine = weakEngine.Upgrade();
        if (!jsEngine) {
            return;
        }
        jsEngine->Initialize(delegate);
    };
    if (needPostJsTask) {
        taskExecutor->PostTask(initJSEngineTask, TaskExecutor::TaskType::JS);
    } else {
        initJSEngineTask();
    }
    taskExecutor_ = taskExecutor;
    LOGI("DeclarativeFrontendNG initialize end.");
    return true;
}

void DeclarativeFrontendNG::AttachPipelineContext(const RefPtr<PipelineBase>& context)
{
    LOGI("DeclarativeFrontendNG AttachPipelineContext.");
    if (delegate_) {
        delegate_->AttachPipelineContext(context);
    }
}

void DeclarativeFrontendNG::AttachSubPipelineContext(const RefPtr<PipelineContext>& context)
{
    LOGI("DeclarativeFrontendNG AttachSubPipelineContext.");
}

void DeclarativeFrontendNG::SetAssetManager(const RefPtr<AssetManager>& assetManager)
{
    LOGI("DeclarativeFrontendNG SetAssetManager.");
    if (delegate_) {
        delegate_->SetAssetManager(assetManager);
    }
}

void DeclarativeFrontendNG::InitializeDelegate(const RefPtr<TaskExecutor>& taskExecutor)
{
    auto pageRouterManager = AceType::MakeRefPtr<NG::PageRouterManager>();
    auto loadPageCallback = [weakEngine = WeakPtr<Framework::JsEngine>(jsEngine_)](const std::string& url,
        const std::function<void(const std::string&, int32_t)>& errorCallback) {
        auto jsEngine = weakEngine.Upgrade();
        if (!jsEngine) {
            return false;
        }
        return jsEngine->LoadPageSource(url, errorCallback);
    };
    pageRouterManager->SetLoadJsCallback(std::move(loadPageCallback));

    delegate_ = AceType::MakeRefPtr<Framework::FrontendDelegateDeclarativeNG>(taskExecutor);
    delegate_->SetPageRouterManager(pageRouterManager);
    if (jsEngine_) {
        delegate_->SetGroupJsBridge(jsEngine_->GetGroupJsBridge());
    }
}

RefPtr<NG::PageRouterManager> DeclarativeFrontendNG::GetPageRouterManager() const
{
    CHECK_NULL_RETURN(delegate_, nullptr);
    return delegate_->GetPageRouterManager();
}

void DeclarativeFrontendNG::RunPage(int32_t pageId, const std::string& url, const std::string& params)
{
    auto container = Container::Current();
    auto isStageModel = container ? container->IsUseStageModel() : false;
    if (!isStageModel) {
        // In NG structure and fa mode, first load app.js
        auto taskExecutor = container ? container->GetTaskExecutor() : nullptr;
        CHECK_NULL_VOID(taskExecutor);
        taskExecutor->PostTask(
            [weak = AceType::WeakClaim(this)]() {
                auto frontend = weak.Upgrade();
                CHECK_NULL_VOID(frontend);
                CHECK_NULL_VOID(frontend->jsEngine_);
                frontend->jsEngine_->LoadFaAppSource();
            },
            TaskExecutor::TaskType::JS);
    }
    // Not use this pageId from backend, manage it in FrontendDelegateDeclarative.
    if (delegate_) {
        delegate_->RunPage(url, params, pageProfile_);
    }
}

void DeclarativeFrontendNG::ReplacePage(const std::string& url, const std::string& params)
{
    if (delegate_) {
        delegate_->Replace(url, params);
    }
}

void DeclarativeFrontendNG::PushPage(const std::string& url, const std::string& params)
{
    if (delegate_) {
        delegate_->Push(url, params);
    }
}

void DeclarativeFrontendNG::NavigatePage(uint8_t type, const PageTarget& target, const std::string& params)
{
    if (delegate_) {
        delegate_->NavigatePage(type, target, params);
    }
}

void DeclarativeFrontendNG::OnWindowDisplayModeChanged(bool isShownInMultiWindow, const std::string& data)
{
    LOGW("OnWindowDisplayModeChanged not implemented");
}

RefPtr<AccessibilityManager> DeclarativeFrontendNG::GetAccessibilityManager() const
{
    return accessibilityManager_;
}

WindowConfig& DeclarativeFrontendNG::GetWindowConfig()
{
    if (!delegate_) {
        static WindowConfig windowConfig;
        LOGW("delegate is null, return default config");
        return windowConfig;
    }
    return delegate_->GetWindowConfig();
}

bool DeclarativeFrontendNG::OnBackPressed()
{
    CHECK_NULL_RETURN(delegate_, false);
    return delegate_->OnPageBackPress();
}

void DeclarativeFrontendNG::OnShow()
{
    foregroundFrontend_ = true;
    CHECK_NULL_VOID(delegate_);
    delegate_->OnPageShow();
}

void DeclarativeFrontendNG::OnHide()
{
    foregroundFrontend_ = false;
    CHECK_NULL_VOID(delegate_);
    delegate_->OnPageHide();
}

void DeclarativeFrontendNG::CallRouterBack()
{
    if (delegate_) {
        if (delegate_->GetStackSize() == 1 && isSubWindow_) {
            LOGW("Can't back because this is the last page of sub window!");
            return;
        }
        delegate_->Back("", "");
    }
}

void DeclarativeFrontendNG::OnSurfaceChanged(int32_t width, int32_t height)
{
    // TODO: update media query infos
}

void DeclarativeFrontendNG::DumpFrontend() const {}

std::string DeclarativeFrontendNG::GetPagePath() const
{
    if (!delegate_) {
        return "";
    }
    int32_t routerIndex = 0;
    std::string routerName;
    std::string routerPath;
    delegate_->GetState(routerIndex, routerName, routerPath);
    return routerPath + routerName;
}

void DeclarativeFrontendNG::TriggerGarbageCollection()
{
    if (jsEngine_) {
        jsEngine_->RunGarbageCollection();
    }
}

void DeclarativeFrontendNG::DumpHeapSnapshot(bool isPrivate)
{
    if (jsEngine_) {
        jsEngine_->DumpHeapSnapshot(isPrivate);
    }
}

void DeclarativeFrontendNG::SetColorMode(ColorMode colorMode)
{
    // TODO: update media query infos
}

void DeclarativeFrontendNG::RebuildAllPages()
{
    LOGW("RebuildAllPages not implemented");
}

void DeclarativeFrontendNG::HotReload()
{
    auto manager = GetPageRouterManager();
    CHECK_NULL_VOID(manager);
    manager->FlushFrontend();
}
} // namespace OHOS::Ace
