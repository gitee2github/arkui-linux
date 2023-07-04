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

#include "core/components_ng/pattern/gauge/gauge_model_ng.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/gauge/gauge_paint_property.h"
#include "core/components_ng/pattern/gauge/gauge_pattern.h"
#include "core/components_ng/pattern/gauge/gauge_theme.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

void GaugeModelNG::Create(float value, float min, float max)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::GAUGE_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<GaugePattern>(); });
    stack->Push(frameNode);

    ACE_UPDATE_PAINT_PROPERTY(GaugePaintProperty, Value, value);
    ACE_UPDATE_PAINT_PROPERTY(GaugePaintProperty, Max, max);
    ACE_UPDATE_PAINT_PROPERTY(GaugePaintProperty, Min, min);
}

void GaugeModelNG::SetValue(float value)
{
    ACE_UPDATE_PAINT_PROPERTY(GaugePaintProperty, Value, value);
}

void GaugeModelNG::SetStartAngle(float startAngle)
{
    ACE_UPDATE_PAINT_PROPERTY(GaugePaintProperty, StartAngle, startAngle);
}

void GaugeModelNG::SetEndAngle(float endAngle)
{
    ACE_UPDATE_PAINT_PROPERTY(GaugePaintProperty, EndAngle, endAngle);
}

void GaugeModelNG::SetColors(const std::vector<Color>& colors, const std::vector<float>& values)
{
    ACE_UPDATE_PAINT_PROPERTY(GaugePaintProperty, Colors, colors);
    ACE_UPDATE_PAINT_PROPERTY(GaugePaintProperty, Values, values);
}

void GaugeModelNG::SetStrokeWidth(const Dimension& strokeWidth)
{
    ACE_UPDATE_PAINT_PROPERTY(GaugePaintProperty, StrokeWidth, strokeWidth);
}

void GaugeModelNG::SetLabelMarkedText(std::string labelTextString) {}

void GaugeModelNG::SetMarkedTextColor(const Color color) {}

} // namespace OHOS::Ace::NG