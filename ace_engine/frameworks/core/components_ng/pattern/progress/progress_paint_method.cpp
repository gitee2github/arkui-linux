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

#include "core/components_ng/pattern/progress/progress_paint_method.h"

#include <algorithm>
#include <optional>

#include "base/geometry/ng/point_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/log/log_wrapper.h"
#include "base/utils/utils.h"
#include "core/components/picker/picker_option_component.h"
#include "core/components/progress/progress_theme.h"
#include "core/components/theme/theme_manager.h"
#include "core/components_ng/pattern/progress/progress_pattern.h"

namespace OHOS::Ace::NG {

void ProgressPaintMethod::GetThemeDate()
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto progressTheme = pipeline->GetTheme<ProgressTheme>();
    CHECK_NULL_VOID(progressTheme);
    color_ = progressTheme->GetTrackSelectedColor();
    bgColor_ = progressTheme->GetTrackBgColor();
    scaleWidth_ = progressTheme->GetScaleWidth().ConvertToPx();
    scaleCount_ = progressTheme->GetScaleNumber();
}

void ProgressPaintMethod::PaintLinear(RSCanvas& canvas, const OffsetF& offset, const SizeF& frameSize) const
{
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(ToRSColor(bgColor_));
    double radius = strokeWidth_ / 2;
    if (frameSize.Width() >= frameSize.Height()) {
        double dateLength = frameSize.Width() * value_ / maxValue_;
        canvas.AttachBrush(brush);
        canvas.DrawRoundRect(
            { { offset.GetX(), offset.GetY(), frameSize.Width() + offset.GetX(), strokeWidth_ + offset.GetY() }, radius,
                radius });
        brush.SetColor(ToRSColor((color_)));
        canvas.AttachBrush(brush);
        if (!NearEqual(dateLength, 0.0)) {
            canvas.DrawRoundRect(
                { { offset.GetX(), offset.GetY(), dateLength + offset.GetX(), strokeWidth_ + offset.GetY() }, radius,
                    radius });
        }
    } else {
        double dateLength = frameSize.Height() * value_ / maxValue_;
        canvas.AttachBrush(brush);
        canvas.DrawRoundRect(
            { { offset.GetX(), offset.GetY(), strokeWidth_ + offset.GetX(), frameSize.Height() + offset.GetY() },
                radius, radius });
        brush.SetColor(ToRSColor((color_)));
        canvas.AttachBrush(brush);
        if (!NearEqual(dateLength, 0.0)) {
            canvas.DrawRoundRect(
                { { offset.GetX(), offset.GetY(), strokeWidth_ + offset.GetX(), dateLength + offset.GetY() }, radius,
                    radius });
        }
    }
}

void ProgressPaintMethod::PaintRing(RSCanvas& canvas, const OffsetF& offset, const SizeF& frameSize) const
{
    static int32_t totalDegree = 360;
    PointF centerPt = PointF(frameSize.Width() / 2, frameSize.Height() / 2) + offset;
    double radius = std::min(frameSize.Width() / 2, frameSize.Height() / 2);
    RSPen pen;
    pen.SetAntiAlias(true);
    double widthOfLine = strokeWidth_;
    if (widthOfLine >= radius) {
        LOGI("strokeWidth is lager than radius,  auto set strokeWidth as half of radius");
        widthOfLine = radius / 2;
    }
    radius = radius - widthOfLine / 2;
    pen.SetWidth(widthOfLine);
    pen.SetCapStyle(ToRSCapStyle(LineCap::ROUND));
    pen.SetColor(ToRSColor(bgColor_));
    canvas.AttachPen(pen);
    canvas.DrawCircle(ToRSPoint(centerPt), radius);
    pen.SetColor(ToRSColor((color_)));
    canvas.AttachPen(pen);
    double angle = (value_ / maxValue_) * totalDegree;
    canvas.DrawArc(
        { centerPt.GetX() - radius, centerPt.GetY() - radius, centerPt.GetX() + radius, centerPt.GetY() + radius }, 270,
        angle);
}

void ProgressPaintMethod::PaintScaleRing(RSCanvas& canvas, const OffsetF& offset, const SizeF& frameSize) const
{
    static int32_t totalDegree = 360;
    PointF centerPt = PointF(frameSize.Width() / 2, frameSize.Height() / 2) + offset;
    double radius = std::min(frameSize.Width() / 2, frameSize.Height() / 2);
    double lengthOfScale = strokeWidth_;
    if (lengthOfScale > radius) {
        LOGI("strokeWidth is lager than radius,  auto set strokeWidth as half of radius");
        lengthOfScale = radius / 2;
    }
    double pathDistance = 2.0 * M_PI * radius / scaleCount_;
    if (scaleWidth_ > pathDistance) {
        LOGI("scaleWidth is lager than pathDistance,  auto changeto paint ring");
        PaintRing(canvas, offset, frameSize);
        return;
    }
    double widthOfLine = scaleWidth_;
    RSPen pen;
    RSPath path;
    pen.SetWidth(widthOfLine);
    LOGD("scaleWidth %{public}lf strokeWidth  %{public}lf, radius %{public}lf pathDistance %{public}lf ", widthOfLine,
        lengthOfScale, radius, pathDistance);
    path.AddRoundRect(
        { 0, 0, widthOfLine, lengthOfScale }, widthOfLine / 2, widthOfLine / 2, RSPathDirection::CW_DIRECTION);
    pen.SetAntiAlias(true);
    pen.SetCapStyle(ToRSCapStyle(LineCap::ROUND));
    pen.SetPathEffect(RSPathEffect::CreatePathDashEffect(path, pathDistance, 0.0f, RSPathDashStyle::ROTATE));
    pen.SetColor(ToRSColor(bgColor_));
    canvas.AttachPen(pen);
    canvas.DrawArc(
        { centerPt.GetX() - radius, centerPt.GetY() - radius, centerPt.GetX() + radius, centerPt.GetY() + radius }, 270,
        totalDegree);
    pen.SetColor(ToRSColor((color_)));
    canvas.AttachPen(pen);
    double angle = (value_ / maxValue_) * totalDegree;
    canvas.DrawArc(
        { centerPt.GetX() - radius, centerPt.GetY() - radius, centerPt.GetX() + radius, centerPt.GetY() + radius }, 270,
        angle);
}

void ProgressPaintMethod::PaintMoon(RSCanvas& canvas, const OffsetF& offset, const SizeF& frameSize) const
{
    static int32_t totalDegree = 1;
    PointF centerPt = PointF(frameSize.Width() / 2, frameSize.Height() / 2) + offset;
    double radius = std::min(frameSize.Width() / 2, frameSize.Height() / 2);
    RSBrush brush;
    brush.SetAlpha(true);
    brush.SetColor(ToRSColor(bgColor_));
    double angle = (value_ / maxValue_) * totalDegree;
    RSPath path;
    canvas.AttachBrush(brush);
    canvas.DrawCircle(ToRSPoint(centerPt), radius);
    brush.SetColor(ToRSColor((color_)));
    canvas.AttachBrush(brush);
    path.AddArc(
        { centerPt.GetX() - radius, centerPt.GetY() - radius, centerPt.GetX() + radius, centerPt.GetY() + radius }, 90,
        180);
    if (angle <= 0.5) {
        double progressOffset = radius - radius * angle / 0.5;
        path.MoveTo(centerPt.GetX(), centerPt.GetY() - radius);
        // startAngle:270  sweepAngle:-180
        path.AddArc({ centerPt.GetX() - progressOffset, centerPt.GetY() - radius, centerPt.GetX() + progressOffset,
                        centerPt.GetY() + radius },
            270, -180);
        canvas.DrawPath(path);
    } else {
        double progressOffset = radius * (angle - 0.5) / 0.5;
        path.MoveTo(centerPt.GetX(), centerPt.GetY() - radius);
        // startAngle:270  sweepAngle:180
        path.AddArc({ centerPt.GetX() - progressOffset, centerPt.GetY() - radius, centerPt.GetX() + progressOffset,
                        centerPt.GetY() + radius },
            270, 180);
        canvas.DrawPath(path);
    }
}

void ProgressPaintMethod::PaintCapsule(RSCanvas& canvas, const OffsetF& offset, const SizeF& frameSize) const
{
    static int32_t totalDegree = 1;
    double radius = std::min(frameSize.Width() / 2, frameSize.Height() / 2);
    double offsetX = offset.GetX();
    double offsetY = offset.GetY();
    double progressWidth = (value_ / maxValue_) * totalDegree * frameSize.Width();
    RSBrush brush;
    brush.SetAlpha(true);
    brush.SetColor(ToRSColor(bgColor_));
    RSPath path;
    canvas.AttachBrush(brush);
    canvas.DrawRoundRect(
        { { offsetX, offsetY, frameSize.Width() + offsetX, frameSize.Height() + offsetY }, radius, radius });
    brush.SetColor(ToRSColor((color_)));
    canvas.AttachBrush(brush);
    path.AddArc({ offsetX, offsetY, 2 * radius + offsetX, frameSize.Height() + offsetY }, 90, 180);
    if (LessNotEqual(progressWidth, radius)) {
        // startAngle:270  sweepAngle:-180
        path.AddArc(
            { offsetX + progressWidth, offsetY, 2 * radius - progressWidth + offsetX, frameSize.Height() + offsetY },
            270, -180);
    } else if (GreatNotEqual(progressWidth, frameSize.Width() - radius)) {
        path.AddRect({ offsetX + radius, offsetY, frameSize.Width() + offsetX - radius, frameSize.Height() + offsetY });
        // startAngle:270  sweepAngle:180
        path.AddArc({ offsetX + (frameSize.Width() - radius) * 2.0 - progressWidth, offsetY, offsetX + progressWidth,
                        frameSize.Height() + offsetY },
            270, 180);
    } else {
        path.AddRect({ radius + offsetX, offsetY, progressWidth + offsetX, frameSize.Height() + offsetY });
    }
    canvas.DrawPath(path);
}

void ProgressPaintMethod::PaintVerticalCapsule(RSCanvas& canvas, const OffsetF& offset, const SizeF& frameSize) const
{
    static int32_t totalDegree = 1;
    double radius = std::min(frameSize.Width() / 2, frameSize.Height() / 2);
    double offsetX = offset.GetX();
    double offsetY = offset.GetY();
    double progressWidth = (value_ / maxValue_) * totalDegree * frameSize.Height();
    RSBrush brush;
    brush.SetAlpha(true);
    brush.SetColor(ToRSColor(bgColor_));
    RSPath path;
    canvas.AttachBrush(brush);
    canvas.DrawRoundRect(
        { { offsetX, offsetY, frameSize.Width() + offsetX, frameSize.Height() + offsetY }, radius, radius });
    brush.SetColor(ToRSColor((color_)));
    canvas.AttachBrush(brush);
    path.AddArc({ offsetX, offsetY, frameSize.Width() + offsetX, frameSize.Width() + offsetY }, 0, -180);
    if (LessNotEqual(progressWidth, radius)) {
        // startAngle:180  sweepAngle:180
        path.AddArc({ offsetX, offsetY + progressWidth, frameSize.Width() + offsetX,
                        frameSize.Width() - progressWidth + offsetY },
            180, 180);
    } else if (GreatNotEqual(progressWidth, frameSize.Height() - radius)) {
        path.AddRect({ offsetX, offsetY + radius, frameSize.Width() + offsetX, frameSize.Height() - radius + offsetY });
        // startAngle:180  sweepAngle:-180
        path.AddArc({ offsetX, offsetY + (frameSize.Height() - radius) * 2.0 - progressWidth,
                        frameSize.Width() + offsetX, progressWidth + offsetY },
            180, -180);
    } else {
        path.AddRect({ offsetX, radius + offsetY, offsetX + frameSize.Width(), progressWidth + offsetY });
    }
    canvas.DrawPath(path);
}

} // namespace OHOS::Ace::NG