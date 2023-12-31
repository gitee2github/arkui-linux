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

#include "base/utils/system_properties.h"

namespace OHOS::Ace {
DeviceType SystemProperties::deviceType_ = DeviceType::UNKNOWN;
DeviceOrientation SystemProperties::orientation_ { DeviceOrientation::PORTRAIT };
bool SystemProperties::isHookModeEnabled_ = false;
bool SystemProperties::rosenBackendEnabled_ = true;
bool SystemProperties::windowAnimationEnabled_ = true;
double SystemProperties::resolution_ = 0.0;
constexpr float defaultAnimationScale = 1.0f;
bool SystemProperties::extSurfaceEnabled_ = false;

float SystemProperties::GetFontWeightScale()
{
    // Default value of font weight scale is 1.0.
    return 1.0f;
}

DeviceType SystemProperties::GetDeviceType()
{
    return DeviceType::PHONE;
}

bool SystemProperties::GetDebugEnabled()
{
    return false;
}

float SystemProperties::GetAnimationScale()
{
    return defaultAnimationScale;
}
} // namespace OHOS::Ace
