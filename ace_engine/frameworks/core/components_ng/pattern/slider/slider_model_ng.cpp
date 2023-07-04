/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/slider/slider_model_ng.h"

#include "core/components/slider/slider_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/slider/slider_layout_property.h"
#include "core/components_ng/pattern/slider/slider_paint_property.h"
#include "core/components_ng/pattern/slider/slider_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
void SliderModelNG::Create(float value, float step, float min, float max)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::SLIDER_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<SliderPattern>(); });
    stack->Push(frameNode);
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, Value, value);
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, Step, step);
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, Min, min);
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, Max, max);
}
void SliderModelNG::SetSliderMode(const SliderMode& value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(SliderLayoutProperty, SliderMode, value);
}
void SliderModelNG::SetDirection(Axis value)
{
    ACE_UPDATE_LAYOUT_PROPERTY(SliderLayoutProperty, Direction, value);
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, Direction, value);
}
void SliderModelNG::SetReverse(bool value)
{
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, Reverse, value);
}
void SliderModelNG::SetBlockColor(const Color& value)
{
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, BlockColor, value);
}
void SliderModelNG::SetTrackBackgroundColor(const Color& value)
{
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, TrackBackgroundColor, value);
}
void SliderModelNG::SetSelectColor(const Color& value)
{
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, SelectColor, value);
}
void SliderModelNG::SetMinLabel(float value)
{
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, Min, value);
}
void SliderModelNG::SetMaxLabel(float value)
{
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, Max, value);
}
void SliderModelNG::SetShowSteps(bool value)
{
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, ShowSteps, value);
}
void SliderModelNG::SetShowTips(bool value)
{
    ACE_UPDATE_PAINT_PROPERTY(SliderPaintProperty, ShowTips, value);
}
void SliderModelNG::SetThickness(const Dimension& value)
{
    if (value.IsNonPositive()) {
        auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        CHECK_NULL_VOID(frameNode);
        auto layoutProperty = frameNode->GetLayoutProperty<SliderLayoutProperty>();
        CHECK_NULL_VOID(layoutProperty);
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto theme = pipeline->GetTheme<SliderTheme>();
        CHECK_NULL_VOID(theme);
        auto sliderMode = layoutProperty->GetSliderModeValue(SliderModel::SliderMode::OUTSET);
        auto themeTrackThickness = sliderMode == SliderModel::SliderMode::OUTSET ? theme->GetOutsetTrackThickness()
                                                                                 : theme->GetInsetTrackThickness();
        ACE_UPDATE_LAYOUT_PROPERTY(SliderLayoutProperty, Thickness, themeTrackThickness);
    } else {
        ACE_UPDATE_LAYOUT_PROPERTY(SliderLayoutProperty, Thickness, value);
    }
}
void SliderModelNG::SetOnChange(SliderOnChangeEvent&& eventOnChange)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<SliderEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnChange(std::move(eventOnChange));
}
} // namespace OHOS::Ace::NG
