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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_PROGRESS_PROGRESS_DATE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_PROGRESS_PROGRESS_DATE_H

#include "base/geometry/dimension.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {

enum class ProgressType {
    LINEAR = 1,
    RING = 2,
    SCALE = 3,
    CIRCLE = 4,
    GAUGE = 5,
    ARC = 6,
    MOON = 7,
    BUBBLE = 8,
    CAPSULE = 9,
};

class ProgressTypeUtils {
public:
    static std::string ConvertProgressTypeToString(ProgressType progressType)
    {
        std::string progressTypeUtils = "";
        switch (progressType) {
            case ProgressType::LINEAR:
                progressTypeUtils = "ProgressStyle.Linear";
                break;
            case ProgressType::MOON:
                progressTypeUtils = "ProgressStyle.Eclipse";
                break;
            case ProgressType::SCALE:
                progressTypeUtils = "ProgressStyle.ScaleRing";
                break;
            case ProgressType::RING:
                progressTypeUtils = "ProgressStyle.Ring";
                break;
            case ProgressType::CAPSULE:
                progressTypeUtils = "ProgressStyle.Capsule";
                break;
            default:
                progressTypeUtils = "ProgressStyle.Linear";
        }
        return progressTypeUtils;
    }
};

struct ProgressDate {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(MaxValue, double);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Value, double);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(StrokeWidth, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ScaleCount, int32_t);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ScaleWidth, Dimension);
};

struct ProgressStyle {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Color, Color);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(BackgroundColor, Color);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Type, ProgressType);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_PROGRESS_PROGRESS_DATE_H
