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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_RESSCHED_RESSCHED_REPORT_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_RESSCHED_RESSCHED_REPORT_H

#include <string>
#include <unordered_map>

#include "base/utils/macros.h"
#include "base/utils/noncopyable.h"
#include "core/event/touch_event.h"

namespace OHOS::Ace {
using ReportDataFunc = void (*)(uint32_t resType, int64_t value,
    const std::unordered_map<std::string, std::string>& payload);

ReportDataFunc ACE_EXPORT LoadReportDataFunc();

class ACE_EXPORT ResSchedReport final {
public:
    static ResSchedReport& GetInstance();
    void ResSchedDataReport(const char* name, const std::unordered_map<std::string, std::string>& param = {});
    void ResSchedDataReport(uint32_t resType, int32_t value = 0,
        const std::unordered_map<std::string, std::string>& payload = {});
    void OnTouchEvent(const TouchType& touchType);

private:
    ResSchedReport() {}
    ~ResSchedReport() {}
    ReportDataFunc reportDataFunc_ = nullptr;
};

class ACE_EXPORT ResSchedReportScope final {
public:
    ACE_DISALLOW_COPY_AND_MOVE(ResSchedReportScope);

    explicit ResSchedReportScope(const std::string& name,
        const std::unordered_map<std::string, std::string>& param = {});
    ~ResSchedReportScope();

private:
    std::string name_;
    std::unordered_map<std::string, std::string> payload_;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_RESSCHED_RESSCHED_REPORT_H
