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

#include "bridge/declarative_frontend/jsview/models/circle_model_impl.h"
#include "base/memory/referenced.h"
#include "bridge/declarative_frontend/jsview/js_shape_abstract.h"
#include "bridge/declarative_frontend/view_stack_processor.h"
#include "core/components/shape/shape_component.h"

namespace OHOS::Ace::Framework {
void CircleModelImpl::Create()
{
    RefPtr<Component> circleComponent = AceType::MakeRefPtr<OHOS::Ace::ShapeComponent>(ShapeType::CIRCLE);
    ViewStackProcessor::GetInstance()->ClaimElementId(circleComponent);
    ViewStackProcessor::GetInstance()->Push(circleComponent);
}
} // namespace OHOS::Ace::Framework