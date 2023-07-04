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

#include "core/components_ng/pattern/text_picker/textpicker_paint_method.h"

#include "core/components/common/properties/color.h"
#include "core/components/picker/picker_theme.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

namespace {
constexpr float DIVIDER_LINE_WIDTH = 1.0f;
constexpr uint8_t ENABLED_ALPHA = 255;
constexpr uint8_t DISABLED_ALPHA = 102;
} // namespace

CanvasDrawFunction TextPickerPaintMethod::GetForegroundDrawFunction(PaintWrapper* paintWrapper)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto theme = pipeline->GetTheme<PickerTheme>();
    auto dividerColor = theme->GetDividerColor();
    auto dividerSpacing = pipeline->NormalizeToPx(theme->GetDividerSpacing());
    auto pressColor = theme->GetPressColor();
    const auto& geometryNode = paintWrapper->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, nullptr);
    auto frameRect = geometryNode->GetFrameRect();
    return [weak = WeakClaim(this), dividerLineWidth = DIVIDER_LINE_WIDTH, frameRect, dividerColor, dividerSpacing,
               pressColor, enabled = enabled_](RSCanvas& canvas) {
        auto picker = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(picker);
        DividerPainter dividerPainter(dividerLineWidth, frameRect.Width(), false, dividerColor, LineCap::SQUARE);
        double upperLine = (frameRect.Height() - picker->defaultPickerItemHeight_) / 2.0;
        double downLine = (frameRect.Height() + picker->defaultPickerItemHeight_) / 2.0;
        OffsetF offset = OffsetF(0.0f, upperLine);
        dividerPainter.DrawLine(canvas, offset);
        OffsetF offsetY = OffsetF(0.0f, downLine);
        dividerPainter.DrawLine(canvas, offsetY);
        if (enabled) {
            picker->PaintGradient(canvas, frameRect);
        } else {
            picker->PaintDisable(canvas, frameRect.Width(), frameRect.Height());
        }
    };
}

void TextPickerPaintMethod::PaintGradient(RSCanvas& canvas, const RectF& frameRect)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<PickerTheme>();
    double gradientHeight = pipeline->NormalizeToPx(theme->GetGradientHeight());
    if (NearZero(gradientHeight)) {
        return;
    }

    auto height = static_cast<float>((frameRect.Height() - theme->GetDividerSpacing().ConvertToPx()) / 2);
    // Paint gradient rect over the picker content.
    RSBrush topBrush;
    RSRect rect(0.0f, 0.0f, frameRect.Right(), frameRect.Bottom());
    RSPoint topStartPoint;
    topStartPoint.SetX(0.0f);
    topStartPoint.SetY(0.0f);
    RSPoint topEndPoint;
    topEndPoint.SetX(0.0f);
    topEndPoint.SetY(frameRect.Height());
    auto backDecoration = theme->GetPopupDecoration(false);
    Color endColor = backDecoration ? backDecoration->GetBackgroundColor() : Color::WHITE;
    Color middleColor = endColor.ChangeAlpha(0);
    std::vector<float> topPos { 0.0f, height / frameRect.Bottom(), (frameRect.Bottom() - height) / frameRect.Bottom(),
        1.0f };
    std::vector<RSColorQuad> topColors { endColor.GetValue(), middleColor.GetValue(), middleColor.GetValue(),
        endColor.GetValue() };
    topBrush.SetShaderEffect(
        RSShaderEffect::CreateLinearGradient(topStartPoint, topEndPoint, topColors, topPos, RSTileMode::CLAMP));
    canvas.DetachPen().AttachBrush(topBrush);
    canvas.DrawRect(rect);
    canvas.DetachBrush();
    canvas.Restore();
}

void TextPickerPaintMethod::PaintDisable(RSCanvas& canvas, double X, double Y)
{
    double centerY = Y;
    double centerX = X;
    RSRect rRect(0, 0, centerX, centerY);
    RSPath path;
    path.AddRoundRect(rRect, 0, 0, RSPathDirection::CW_DIRECTION);
    RSPen pen;
    RSBrush brush;
    brush.SetColor(float(DISABLED_ALPHA) / ENABLED_ALPHA);
    pen.SetColor(float(DISABLED_ALPHA) / ENABLED_ALPHA);
    canvas.AttachBrush(brush);
    canvas.AttachPen(pen);
    canvas.DrawPath(path);
}
} // namespace OHOS::Ace::NG