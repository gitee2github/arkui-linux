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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_STEPPER_MODEL_IMPL_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_STEPPER_MODEL_IMPL_H

#include "core/components_ng/pattern/stepper/stepper_model.h"

namespace OHOS::Ace::Framework {

class ACE_EXPORT StepperModelImpl : public StepperModel {
public:
    void Create(uint32_t index) override;
    void SetOnFinish(RoutineCallbackEvent&& eventOnFinish) override;
    void SetOnSkip(RoutineCallbackEvent&& eventOnSkip) override;
    void SetOnChange(IndexCallbackEvent&& eventOnChange) override;
    void SetOnNext(IndexCallbackEvent&& eventOnNext) override;
    void SetOnPrevious(IndexCallbackEvent&& eventOnPrevious) override;
};

} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_STEPPER_MODEL_IMPL_H