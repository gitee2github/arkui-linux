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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_GAUGE_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_GAUGE_MODEL_H

#include "core/components/common/properties/color.h"
#include "frameworks/base/geometry/dimension.h"

namespace OHOS::Ace {

class ACE_EXPORT GaugeModel {
public:
    static GaugeModel* GetInstance();
    virtual ~GaugeModel() = default;

    virtual void Create(float values, float min, float max) = 0;
    virtual void SetValue(float value) = 0;
    virtual void SetStartAngle(float startAngle) = 0;
    virtual void SetEndAngle(float endAngle) = 0;
    virtual void SetColors(const std::vector<Color>& colors, const std::vector<float>& values) = 0;
    virtual void SetStrokeWidth(const Dimension& strokeWidth) = 0;
    virtual void SetLabelMarkedText(std::string labelTextString) = 0;
    virtual void SetMarkedTextColor(const Color color) = 0;

private:
    static std::unique_ptr<GaugeModel> instance_;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_GAUGE_MODEL_H