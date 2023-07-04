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

#include "core/common/container.h"

#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/ace_engine.h"
#include "core/common/container_scope.h"

namespace OHOS::Ace {

int32_t Container::CurrentId()
{
    return ContainerScope::CurrentId();
}

RefPtr<Container> Container::Current()
{
    return AceEngine::Get().GetContainer(ContainerScope::CurrentId());
}

RefPtr<Container> Container::GetActive()
{
    RefPtr<Container> activeContainer;
    AceEngine::Get().NotifyContainers([&activeContainer](const RefPtr<Container>& container) {
        auto front = container->GetFrontend();
        if (front && front->IsForeground()) {
            activeContainer = container;
        }
    });
    return activeContainer;
}

RefPtr<TaskExecutor> Container::CurrentTaskExecutor()
{
    auto curContainer = Current();
    CHECK_NULL_RETURN_NOLOG(curContainer, nullptr);
    return curContainer->GetTaskExecutor();
}

void Container::UpdateCurrent(int32_t id)
{
    ContainerScope::UpdateCurrent(id);
}

bool Container::UpdateState(const Frontend::State& state)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (state_ == state) {
        return false;
    }
    state_ = state;
    return true;
}

} // namespace OHOS::Ace