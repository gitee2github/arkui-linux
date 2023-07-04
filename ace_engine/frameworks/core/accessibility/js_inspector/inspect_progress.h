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

#ifndef FOUNDATION_ACE_ADAPTER_PREVIEW_INSPECTOR_INSPECT_PROGRESS_H
#define FOUNDATION_ACE_ADAPTER_PREVIEW_INSPECTOR_INSPECT_PROGRESS_H

#include "inspect_node.h"

namespace OHOS::Ace::Framework {
class InspectProgress : public InspectNode {
    DECLARE_ACE_TYPE(InspectProgress, InspectNode);

public:
    InspectProgress(NodeId nodeId, const std::string& nodeName);
    ~InspectProgress() override = default;
    void PackAttrAndStyle() override;
};
} // namespace OHOS::Ace::Framework

#endif // FOUNDATION_ACE_ADAPTER_PREVIEW_INSPECTOR_INSPECT_PROGRESS_H
