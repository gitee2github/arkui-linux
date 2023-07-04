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

#include "core/common/ace_engine.h"

#include <csignal>
#include <cstdio>

#include "base/log/dump_log.h"
#include "base/log/log.h"
#include "base/memory/memory_monitor.h"
#include "base/thread/background_task_executor.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/common/ace_page.h"
#ifdef PLUGIN_COMPONENT_SUPPORTED
#include "core/common/plugin_manager.h"
#endif
#include "core/image/image_cache.h"

namespace OHOS::Ace {
namespace {

constexpr int SIGNAL_JS_HEAP = 39;
constexpr int SIGNAL_JS_HEAP_PRIV = 40;

void HandleSignal(int signo)
{
    switch (signo) {
        case SIGNAL_JS_HEAP:
            AceEngine::Get().DumpJsHeap(false);
            break;
        case SIGNAL_JS_HEAP_PRIV:
            AceEngine::Get().DumpJsHeap(true);
            break;
        default:
            break;
    }
}

}
std::atomic<bool> AceEngine::isAlive_ = true;

AceEngine::AceEngine()
{
    isAlive_.store(true);
    if (!SystemProperties::GetHookModeEnabled()) {
        watchDog_ = AceType::MakeRefPtr<WatchDog>();
    }
}

AceEngine::~AceEngine()
{
    LOGI("~AceEngine");
    isAlive_.store(false);
    std::shared_lock<std::shared_mutex> lock(mutex_);
    LOGI("~AceEngine: containerMap_ size: %{public}zu", containerMap_.size());
    for (const auto& [first, second] : containerMap_) {
        LOGI("~AceEngine: container (%{public}s) instance id: %{public}d(%{public}d)",
            second->TypeName(), first, second->GetInstanceId());
    }
}

AceEngine& AceEngine::Get()
{
    static AceEngine engine;
    return engine;
}

void AceEngine::InitJsDumpHeadSignal()
{
    signal(SIGNAL_JS_HEAP, HandleSignal);
    signal(SIGNAL_JS_HEAP_PRIV, HandleSignal);
}

void AceEngine::AddContainer(int32_t instanceId, const RefPtr<Container>& container)
{
    if (!isAlive_.load()) {
        return;
    }
    LOGI("AddContainer %{public}d", instanceId);
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto result = containerMap_.try_emplace(instanceId, container);
    if (!result.second) {
        LOGW("already have container of this instance id: %{public}d", instanceId);
    }
}

void AceEngine::RemoveContainer(int32_t instanceId)
{
    if (!isAlive_.load()) {
        return;
    }
    LOGI("RemoveContainer %{public}d", instanceId);
    size_t num = 0;
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        num = containerMap_.erase(instanceId);
    }
    if (num == 0) {
        LOGW("container not found with instance id: %{public}d", instanceId);
    }
}

void AceEngine::Dump(const std::vector<std::string>& params) const
{
    if (!isAlive_.load()) {
        return;
    }
    std::unordered_map<int32_t, RefPtr<Container>> copied;
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        copied = containerMap_;
    }
    for (const auto& container : copied) {
        auto pipelineContext = container.second->GetPipelineContext();
        if (!pipelineContext) {
            LOGW("the pipeline context is nullptr, pa container");
            continue;
        }
        pipelineContext->GetTaskExecutor()->PostSyncTask(
            [params, container = container.second]() { container->Dump(params); }, TaskExecutor::TaskType::UI);
    }
}

RefPtr<Container> AceEngine::GetContainer(int32_t instanceId)
{
    if (!isAlive_.load()) {
        return nullptr;
    }
#ifdef PLUGIN_COMPONENT_SUPPORTED
    if (instanceId >= MIN_PLUGIN_SUBCONTAINER_ID) {
        instanceId = PluginManager::GetInstance().GetPluginParentContainerId(instanceId);
    }
#endif
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto container = containerMap_.find(instanceId);
    if (container != containerMap_.end()) {
        return container->second;
    } else {
        return nullptr;
    }
}

void AceEngine::RegisterToWatchDog(int32_t instanceId, const RefPtr<TaskExecutor>& taskExecutor, bool useUIAsJSThread)
{
    CHECK_NULL_VOID_NOLOG(watchDog_);
    watchDog_->Register(instanceId, taskExecutor, useUIAsJSThread);
}

void AceEngine::UnRegisterFromWatchDog(int32_t instanceId)
{
    CHECK_NULL_VOID_NOLOG(watchDog_);
    watchDog_->Unregister(instanceId);
}

void AceEngine::BuriedBomb(int32_t instanceId, uint64_t bombId)
{
    CHECK_NULL_VOID_NOLOG(watchDog_);
    watchDog_->BuriedBomb(instanceId, bombId);
}

void AceEngine::DefusingBomb(int32_t instanceId)
{
    CHECK_NULL_VOID_NOLOG(watchDog_);
    watchDog_->DefusingBomb(instanceId);
}

void AceEngine::TriggerGarbageCollection()
{
    if (!isAlive_.load()) {
        return;
    }
    std::unordered_map<int32_t, RefPtr<Container>> copied;
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        if (containerMap_.empty()) {
            return;
        }
        copied = containerMap_;
    }

    auto taskExecutor = copied.begin()->second->GetTaskExecutor();
    taskExecutor->PostTask([] { PurgeMallocCache(); }, TaskExecutor::TaskType::PLATFORM);
#if defined(OHOS_PLATFORM) && defined(ENABLE_NATIVE_VIEW)
    // GPU and IO thread is shared while enable native view
    taskExecutor->PostTask([] { PurgeMallocCache(); }, TaskExecutor::TaskType::GPU);
    taskExecutor->PostTask([] { PurgeMallocCache(); }, TaskExecutor::TaskType::IO);
#endif

    for (const auto& container : copied) {
        container.second->TriggerGarbageCollection();
    }

    ImageCache::Purge();
    BackgroundTaskExecutor::GetInstance().TriggerGarbageCollection();
    PurgeMallocCache();
}

void AceEngine::NotifyContainers(const std::function<void(const RefPtr<Container>&)>& callback)
{
    if (!isAlive_.load()) {
        return;
    }
    CHECK_NULL_VOID_NOLOG(callback);
    std::shared_lock<std::shared_mutex> lock(mutex_);
    for (const auto& [first, second] : containerMap_) {
        // first = container ID
        ContainerScope scope(first);
        callback(second);
    }
}

void AceEngine::DumpJsHeap(bool isPrivate) const
{
    if (!isAlive_.load()) {
        return;
    }
    std::unordered_map<int32_t, RefPtr<Container>> copied;
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        copied = containerMap_;
    }
    for (const auto& container : copied) {
        container.second->DumpHeapSnapshot(isPrivate);
    }
}

} // namespace OHOS::Ace
