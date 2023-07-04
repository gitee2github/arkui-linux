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
#include "core/components_ng/pattern/bubble/bubble_paint_method.h"

#include <vector>

#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/rect.h"
#include "base/geometry/rrect.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/common/properties/border.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/decoration.h"
#include "core/components/common/properties/placement.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/popup/popup_theme.h"
#include "core/components/theme/theme_manager.h"
#include "core/components_ng/pattern/bubble/bubble_pattern.h"
#include "core/components_ng/pattern/bubble/bubble_render_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr Dimension BEZIER_WIDTH_HALF = 16.0_vp;
constexpr Dimension BEZIER_HORIZON_OFFSET_FIRST = 1.3_vp;
constexpr Dimension BEZIER_HORIZON_OFFSET_SECOND = 3.2_vp;
constexpr Dimension BEZIER_HORIZON_OFFSET_THIRD = 6.6_vp;
constexpr Dimension BEZIER_HORIZON_OFFSET_FOURTH = 16.0_vp;
constexpr Dimension BEZIER_VERTICAL_OFFSET_FIRST = 0.1_vp;
constexpr Dimension BEZIER_VERTICAL_OFFSET_SECOND = 3.0_vp;
constexpr Dimension BEZIER_VERTICAL_OFFSET_THIRD = 8.0_vp;
constexpr Dimension ARROW_WIDTH = 32.0_vp;
constexpr Dimension ARROW_ZERO_PERCENT_VALUE = Dimension(0.0, DimensionUnit::PERCENT);
constexpr Dimension ARROW_HALF_PERCENT_VALUE = Dimension(0.5, DimensionUnit::PERCENT);
constexpr Dimension ARROW_ONE_HUNDRED_PERCENT_VALUE = Dimension(1.0, DimensionUnit::PERCENT);

} // namespace

float ModifyBorderRadius(float borderRadius, float halfChildHeight)
{
    return GreatOrEqual(borderRadius, halfChildHeight) ? halfChildHeight : borderRadius;
}

void BubblePaintMethod::PaintMask(RSCanvas& canvas, PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    auto paintProperty = DynamicCast<BubbleRenderProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_VOID(paintProperty);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto popupTheme = pipelineContext->GetTheme<PopupTheme>();
    CHECK_NULL_VOID(popupTheme);
    auto maskColor = paintProperty->GetMaskColor().value_or(popupTheme->GetMaskColor());
    auto layoutSize = paintWrapper->GetContentSize();
    canvas.Save();
    RSBrush brush;
    brush.SetColor(maskColor.GetValue());
    canvas.AttachBrush(brush);
    canvas.DrawRect(RSRect(0.0, 0.0, layoutSize.Width(), layoutSize.Height()));
    canvas.Restore();
}

void BubblePaintMethod::PaintBorder(RSCanvas& canvas, PaintWrapper* paintWrapper)
{
    BorderEdge edge = border_.Left();
    if (!border_.IsAllEqual()) {
        edge = border_.GetValidEdge();
        border_ = Border(edge);
    }
    if (!border_.HasValue()) {
        return;
    }
    float borderWidth = edge.GetWidth().ConvertToPx();
    RSPen paint;
    paint.SetWidth(borderWidth);
    paint.SetColor(edge.GetColor().GetValue());
    paint.SetAntiAlias(true);
    if (edge.GetBorderStyle() == BorderStyle::DOTTED) {
        RSPath dotPath;
        dotPath.AddCircle(0.0f, 0.0f, borderWidth / 2.0);
        paint.SetPathEffect(
            RSPathEffect::CreatePathDashEffect(dotPath, borderWidth * 2.0, 0.0, RSPathDashStyle::ROTATE));
    } else if (edge.GetBorderStyle() == BorderStyle::DASHED) {
        const float intervals[] = { borderWidth, borderWidth };
        paint.SetPathEffect(RSPathEffect::CreateDashPathEffect(intervals, 2, 0.0));
        canvas.AttachPen(paint);
        canvas.DrawPath(path_);
    } else {
        paint.SetPathEffect(nullptr);
    }

    canvas.Save();
    canvas.Translate(childOffset_.GetX() + childSize_.Width() / 2.0, childOffset_.GetY() + childSize_.Height() / 2.0);
    canvas.Scale(1.0 - (borderWidth / childSize_.Width()), 1.0 - (borderWidth / childSize_.Height()));
    canvas.Translate(
        -(childOffset_.GetX() + childSize_.Width() / 2.0), -(childOffset_.GetY() + childSize_.Height() / 2.0));
    if ((arrowPlacement_ == Placement::TOP && showTopArrow_) ||
        (arrowPlacement_ == Placement::BOTTOM && showBottomArrow_)) {
        canvas.AttachPen(paint);
        canvas.DrawPath(path_);
    } else {
        canvas.AttachPen(paint);
        auto rect = MakeRRect();
        canvas.DrawRoundRect(rect);
    }
    canvas.Restore();
}

void BubblePaintMethod::PaintBubble(RSCanvas& canvas, PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    auto paintProperty = DynamicCast<BubbleRenderProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_VOID(paintProperty);
    useCustom_ = paintProperty->GetUseCustom().value_or(false);
    enableArrow_ = paintProperty->GetEnableArrow().value_or(true);
    arrowPlacement_ = paintProperty->GetPlacement().value_or(Placement::BOTTOM);
    UpdateArrowOffset(paintProperty->GetArrowOffset(), arrowPlacement_);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto popupTheme = pipelineContext->GetTheme<PopupTheme>();
    CHECK_NULL_VOID(popupTheme);
    backgroundColor_ = paintProperty->GetBackgroundColor().value_or(popupTheme->GetBackgroundColor());
    border_.SetBorderRadius(popupTheme->GetRadius());
    padding_ = popupTheme->GetPadding();
    RSPen paint;
    paint.SetColor(backgroundColor_.GetValue());
    paint.SetAntiAlias(true);
    // TODO: color is not correct
    RSBrush brush;
    brush.SetColor(static_cast<int>(backgroundColor_.GetValue()));
    canvas.AttachPen(paint);
    canvas.AttachBrush(brush);
    if (!useCustom_) {
        PaintNonCustomPopup(canvas, paintWrapper);
        return;
    }
    if (enableArrow_ && showCustomArrow_) {
        PaintBubbleWithArrow(canvas, paintWrapper);
    } else {
        PaintDefaultBubble(canvas);
    }
}

void BubblePaintMethod::UpdateArrowOffset(const std::optional<Dimension>& offset, const Placement& placement)
{
    if (offset.has_value()) {
        arrowOffset_ = offset.value();
        if (arrowOffset_.Unit() == DimensionUnit::PERCENT) {
            arrowOffset_.SetValue(std::clamp(arrowOffset_.Value(), 0.0, 1.0));
        }
        return;
    }
    switch (placement) {
        case Placement::LEFT:
        case Placement::RIGHT:
        case Placement::TOP:
        case Placement::BOTTOM:
            arrowOffset_ = ARROW_HALF_PERCENT_VALUE;
            break;
        case Placement::TOP_LEFT:
        case Placement::BOTTOM_LEFT:
        case Placement::LEFT_TOP:
        case Placement::RIGHT_TOP:
            arrowOffset_ = ARROW_ZERO_PERCENT_VALUE;
            break;
        case Placement::TOP_RIGHT:
        case Placement::BOTTOM_RIGHT:
        case Placement::LEFT_BOTTOM:
        case Placement::RIGHT_BOTTOM:
            arrowOffset_ = ARROW_ONE_HUNDRED_PERCENT_VALUE;
            break;
        default:
            break;
    }
}

void BubblePaintMethod::PaintNonCustomPopup(RSCanvas& canvas, PaintWrapper* wrapper)
{
    switch (arrowPlacement_) {
        case Placement::TOP:
            showTopArrow_ ? PaintTopBubble(canvas) : PaintDefaultBubble(canvas);
            break;
        case Placement::BOTTOM:
            showBottomArrow_ ? PaintBottomBubble(canvas) : PaintDefaultBubble(canvas);
            break;
        default:
            break;
    }
}

void BubblePaintMethod::PaintTopBubble(RSCanvas& rsCanvas)
{
    float childHeight = childSize_.Height();
    float childWidth = childSize_.Width();
    float childOffsetX = childOffset_.GetX();
    float childOffsetY = childOffset_.GetY();
    float arrowPositionX = arrowPosition_.GetX();
    float arrowPositionY = arrowPosition_.GetY();
    float arrowOffset = GetArrowOffset(Placement::TOP);
    path_.Reset();
    path_.MoveTo(arrowPositionX + arrowOffset, arrowPositionY);
    path_.QuadTo(arrowPositionX + BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx() + arrowOffset,
        arrowPositionY + BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx(),
        arrowPositionX + BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx() + arrowOffset,
        arrowPositionY - BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx());
    path_.QuadTo(arrowPositionX + BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx() + arrowOffset,
        arrowPositionY - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
        arrowPositionX + BEZIER_HORIZON_OFFSET_FOURTH.ConvertToPx() + arrowOffset,
        arrowPositionY - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx());

    // when borderRadius greatOrEqual halfChildHeight, borderRadius equal halfChildHeight
    auto halfChildHeight = childHeight / 2;
    auto borderBottomRightRadiusY =
        ModifyBorderRadius(border_.BottomRightRadius().GetY().ConvertToPx(), halfChildHeight);
    auto borderBottomRightRadiusX =
        ModifyBorderRadius(border_.BottomRightRadius().GetX().ConvertToPx(), halfChildHeight);
    auto borderTopRightRadiusX = ModifyBorderRadius(border_.TopRightRadius().GetX().ConvertToPx(), halfChildHeight);
    auto borderTopRightRadiusY = ModifyBorderRadius(border_.TopRightRadius().GetY().ConvertToPx(), halfChildHeight);
    auto borderTopLeftRadiusX = ModifyBorderRadius(border_.TopLeftRadius().GetX().ConvertToPx(), halfChildHeight);
    auto borderTopLeftRadiusY = ModifyBorderRadius(border_.TopLeftRadius().GetY().ConvertToPx(), halfChildHeight);
    auto borderBottomLeftRadiusX = ModifyBorderRadius(border_.BottomLeftRadius().GetX().ConvertToPx(), halfChildHeight);
    auto borderBottomLeftRadiusY = ModifyBorderRadius(border_.BottomLeftRadius().GetY().ConvertToPx(), halfChildHeight);

    path_.LineTo(
        childOffsetX + childWidth - border_.BottomRightRadius().GetX().ConvertToPx(), childOffsetY + childHeight);
    path_.ArcTo(borderBottomRightRadiusX, borderBottomRightRadiusY, 0.0f, RSPathDirection::CCW_DIRECTION,
        childOffsetX + childWidth, childOffsetY + childHeight - border_.BottomRightRadius().GetY().ConvertToPx());

    path_.LineTo(childOffsetX + childWidth, childOffsetY + border_.TopRightRadius().GetY().ConvertToPx());
    path_.ArcTo(borderTopRightRadiusX, borderTopRightRadiusY, 0.0f, RSPathDirection::CCW_DIRECTION,
        childOffsetX + childWidth - border_.TopRightRadius().GetX().ConvertToPx(), childOffsetY);

    path_.LineTo(childOffsetX + border_.TopLeftRadius().GetX().ConvertToPx(), childOffsetY);
    path_.ArcTo(borderTopLeftRadiusX, borderTopLeftRadiusY, 0.0f, RSPathDirection::CCW_DIRECTION, childOffsetX,
        childOffsetY + border_.TopLeftRadius().GetY().ConvertToPx());

    path_.LineTo(childOffsetX, childOffsetY + childHeight - border_.BottomLeftRadius().GetY().ConvertToPx());
    path_.ArcTo(borderBottomLeftRadiusX, borderBottomLeftRadiusY, 0.0f, RSPathDirection::CCW_DIRECTION,
        childOffsetX + border_.BottomLeftRadius().GetX().ConvertToPx(), childOffsetY + childHeight);

    path_.LineTo(arrowPositionX - BEZIER_HORIZON_OFFSET_FOURTH.ConvertToPx() + arrowOffset,
        arrowPositionY - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx());
    path_.QuadTo(arrowPositionX - BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx() + arrowOffset,
        arrowPositionY - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
        arrowPositionX - BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx() + arrowOffset,
        arrowPositionY - BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx());
    path_.QuadTo(arrowPositionX - BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx() + arrowOffset,
        arrowPositionY + BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx(), arrowPositionX + arrowOffset, arrowPositionY);
    path_.Close();
    // paint shadow
    PaintShadow(path_, ShadowConfig::DefaultShadowM, rsCanvas);
    rsCanvas.DrawPath(path_);
    rsCanvas.ClipPath(path_, RSClipOp::INTERSECT);
}

void BubblePaintMethod::PaintShadow(const RSPath& path, const Shadow& shadow, RSCanvas& canvas)
{
    canvas.Save();
    RSPath rsPath = path;
    rsPath.Offset(shadow.GetOffset().GetX(), shadow.GetOffset().GetY());
    RSColor spotColor = ToRSColor(shadow.GetColor());
    RSPoint3 planeParams = { 0.0f, 0.0f, shadow.GetElevation() };
    RSPoint3 lightPos = { rsPath.GetBounds().GetLeft() / 2 + rsPath.GetBounds().GetRight(),
        rsPath.GetBounds().GetTop() / 2.0 + rsPath.GetBounds().GetBottom() / 2.0, shadow.GetLightHeight() };
    RSColor ambientColor = RSColor(0, 0, 0, 0);
    canvas.DrawShadow(rsPath, planeParams, lightPos, shadow.GetLightRadius(), ambientColor, spotColor,
        RSShadowFlags::TRANSPARENT_OCCLUDER);
    canvas.Restore();
}

void BubblePaintMethod::PaintBottomBubble(RSCanvas& canvas)
{
    float childHeight = childSize_.Height();
    float childWidth = childSize_.Width();
    float childOffsetX = childOffset_.GetX();
    float childOffsetY = childOffset_.GetY();
    float arrowPositionX = arrowPosition_.GetX();
    float arrowPositionY = arrowPosition_.GetY();
    float arrowOffset = GetArrowOffset(Placement::BOTTOM);
    path_.Reset();
    path_.MoveTo(arrowPositionX + arrowOffset, arrowPositionY);
    path_.QuadTo(arrowPositionX + BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx() + arrowOffset,
        arrowPositionY - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx(),
        arrowPositionX + BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx() + arrowOffset,
        arrowPositionY + BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx());
    path_.QuadTo(arrowPositionX + BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx() + arrowOffset,
        arrowPositionY + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
        arrowPositionX + BEZIER_HORIZON_OFFSET_FOURTH.ConvertToPx() + arrowOffset,
        arrowPositionY + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx());

    // when borderRadius greatOrEqual halfChildHeight, borderRadius equal halfChildHeight
    auto halfChildHeight = childHeight / 2;
    auto borderBottomRightRadiusY =
        ModifyBorderRadius(border_.BottomRightRadius().GetY().ConvertToPx(), halfChildHeight);
    auto borderBottomRightRadiusX =
        ModifyBorderRadius(border_.BottomRightRadius().GetX().ConvertToPx(), halfChildHeight);
    auto borderTopRightRadiusX = ModifyBorderRadius(border_.TopRightRadius().GetX().ConvertToPx(), halfChildHeight);
    auto borderTopRightRadiusY = ModifyBorderRadius(border_.TopRightRadius().GetY().ConvertToPx(), halfChildHeight);
    auto borderTopLeftRadiusX = ModifyBorderRadius(border_.TopLeftRadius().GetX().ConvertToPx(), halfChildHeight);
    auto borderTopLeftRadiusY = ModifyBorderRadius(border_.TopLeftRadius().GetY().ConvertToPx(), halfChildHeight);
    auto borderBottomLeftRadiusX = ModifyBorderRadius(border_.BottomLeftRadius().GetX().ConvertToPx(), halfChildHeight);
    auto borderBottomLeftRadiusY = ModifyBorderRadius(border_.BottomLeftRadius().GetY().ConvertToPx(), halfChildHeight);

    path_.LineTo(childOffsetX + childWidth - border_.TopRightRadius().GetX().ConvertToPx(), childOffsetY);
    path_.ArcTo(borderBottomRightRadiusX, borderBottomRightRadiusY, 0.0f, RSPathDirection::CW_DIRECTION,
        childOffsetX + childWidth, childOffsetY + border_.TopRightRadius().GetY().ConvertToPx());

    path_.LineTo(
        childOffsetX + childWidth, childOffsetY + childHeight - border_.BottomRightRadius().GetY().ConvertToPx());
    path_.ArcTo(borderTopRightRadiusX, borderTopRightRadiusY, 0.0f, RSPathDirection::CW_DIRECTION,
        childOffsetX + childWidth - border_.BottomRightRadius().GetX().ConvertToPx(), childOffsetY + childHeight);

    path_.LineTo(childOffsetX + border_.BottomLeftRadius().GetX().ConvertToPx(), childOffsetY + childHeight);
    path_.ArcTo(borderTopLeftRadiusX, borderTopLeftRadiusY, 0.0f, RSPathDirection::CW_DIRECTION, childOffsetX,
        childOffsetY + childHeight - border_.BottomLeftRadius().GetY().ConvertToPx());

    path_.LineTo(childOffsetX, childOffsetY + border_.TopLeftRadius().GetY().ConvertToPx());
    path_.ArcTo(borderBottomLeftRadiusX, borderBottomLeftRadiusY, 0.0f, RSPathDirection::CW_DIRECTION,
        childOffsetX + border_.TopLeftRadius().GetX().ConvertToPx(), childOffsetY);

    path_.LineTo(arrowPositionX - BEZIER_HORIZON_OFFSET_FOURTH.ConvertToPx() + arrowOffset,
        arrowPositionY + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx());
    path_.QuadTo(arrowPositionX - BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx() + arrowOffset,
        arrowPositionY + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
        arrowPositionX - BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx() + arrowOffset,
        arrowPositionY + BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx());
    path_.QuadTo(arrowPositionX - BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx() + arrowOffset,
        arrowPositionY - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx(), arrowPositionX + arrowOffset, arrowPositionY);
    path_.Close();
    // paint shadow
    PaintShadow(path_, ShadowConfig::DefaultShadowM, canvas);
    canvas.DrawPath(path_);
    canvas.ClipPath(path_, RSClipOp::INTERSECT);
}

void BubblePaintMethod::PaintDefaultBubble(RSCanvas& canvas)
{
    auto rrect = MakeRRect();
    PaintShadow(path_, ShadowConfig::DefaultShadowM, canvas);
    canvas.DrawRoundRect(rrect);
    canvas.ClipRoundRect(rrect, RSClipOp::INTERSECT);
}

RSRoundRect BubblePaintMethod::MakeRRect()
{
    auto rect = RSRect(childOffset_.GetX(), childOffset_.GetY(), childOffset_.GetX() + childSize_.Width(),
        childOffset_.GetY() + childSize_.Height());
    std::vector<RSPoint> rectRadii;
    rectRadii.resize(4);
    rectRadii[RSRoundRect::TOP_LEFT_POS] =
        RSPoint(border_.TopLeftRadius().GetX().ConvertToPx(), border_.TopLeftRadius().GetY().ConvertToPx());
    rectRadii[RSRoundRect::TOP_RIGHT_POS] =
        RSPoint(border_.TopRightRadius().GetX().ConvertToPx(), border_.TopRightRadius().GetY().ConvertToPx());
    rectRadii[RSRoundRect::BOTTOM_RIGHT_POS] =
        RSPoint(border_.BottomRightRadius().GetX().ConvertToPx(), border_.BottomRightRadius().GetY().ConvertToPx());
    rectRadii[RSRoundRect::BOTTOM_LEFT_POS] =
        RSPoint(border_.BottomLeftRadius().GetX().ConvertToPx(), border_.BottomLeftRadius().GetY().ConvertToPx());
    return RSRoundRect(rect, rectRadii);
}

void BubblePaintMethod::PaintBubbleWithArrow(RSCanvas& canvas, PaintWrapper* paintWrapper)
{
    BuildCompletePath(path_);
    PaintShadow(path_, ShadowConfig::DefaultShadowM, canvas);
    canvas.DrawPath(path_);
    canvas.ClipPath(path_, RSClipOp::INTERSECT);
}

void BubblePaintMethod::BuildCompletePath(RSPath& path)
{
    float arrowOffset = GetArrowOffset(arrowPlacement_);
    auto borderRadius = ModifyBorderRadius(border_.BottomLeftRadius().GetY().ConvertToPx(), childSize_.Height() / 2);
    float radiusPx = borderRadius;
    path.Reset();
    path.MoveTo(childOffset_.GetX() + radiusPx, childOffset_.GetY());
    BuildTopLinePath(path, arrowOffset, radiusPx);
    BuildCornerPath(path, Placement::TOP_RIGHT, radiusPx);
    BuildRightLinePath(path, arrowOffset, radiusPx);
    BuildCornerPath(path, Placement::BOTTOM_RIGHT, radiusPx);
    BuildBottomLinePath(path, arrowOffset, radiusPx);
    BuildCornerPath(path, Placement::BOTTOM_LEFT, radiusPx);
    BuildLeftLinePath(path, arrowOffset, radiusPx);
    BuildCornerPath(path, Placement::TOP_LEFT, radiusPx);
    path.Close();
}

void BubblePaintMethod::BuildTopLinePath(RSPath& path, float arrowOffset, float radius)
{
    float childOffsetY = childOffset_.GetY();
    float arrowPositionY = arrowPosition_.GetY();
    switch (arrowPlacement_) {
        case Placement::BOTTOM:
        case Placement::BOTTOM_LEFT:
        case Placement::BOTTOM_RIGHT:
            path.LineTo(arrowPosition_.GetX() + arrowOffset - BEZIER_WIDTH_HALF.ConvertToPx(), childOffsetY);
            path.QuadTo(arrowPosition_.GetX() + arrowOffset - BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
                arrowPosition_.GetX() + arrowOffset - BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx(),
                arrowPositionY + BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() - BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx() + arrowOffset,
                arrowPositionY - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx(), arrowPosition_.GetX() + arrowOffset,
                arrowPositionY);
            path.QuadTo(arrowPosition_.GetX() + arrowOffset + BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx(),
                arrowPosition_.GetX() + arrowOffset + BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx(),
                arrowPositionY + BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() + arrowOffset + BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
                arrowPosition_.GetX() + arrowOffset + BEZIER_HORIZON_OFFSET_FOURTH.ConvertToPx(),
                arrowPositionY + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx());
            break;
        default:
            break;
    }
    path.LineTo(childOffset_.GetX() + childSize_.Width() - radius, childOffsetY);
}

void BubblePaintMethod::BuildCornerPath(RSPath& path, const Placement& placement, float radius)
{
    float childOffsetY = childOffset_.GetY();
    switch (placement) {
        case Placement::TOP_LEFT:
            path.ArcTo(radius, radius, 0.0f, RSPathDirection::CW_DIRECTION, childOffset_.GetX() + radius, childOffsetY);
            break;
        case Placement::TOP_RIGHT:
            path.ArcTo(radius, radius, 0.0f, RSPathDirection::CW_DIRECTION, childOffset_.GetX() + childSize_.Width(),
                childOffsetY + radius);
            break;
        case Placement::BOTTOM_RIGHT:
            path.ArcTo(radius, radius, 0.0f, RSPathDirection::CW_DIRECTION,
                childOffset_.GetX() + childSize_.Width() - radius, childOffsetY + childSize_.Height());
            break;
        case Placement::BOTTOM_LEFT:
            path.ArcTo(radius, radius, 0.0f, RSPathDirection::CW_DIRECTION, childOffset_.GetX(),
                childOffsetY + childSize_.Height() - radius);
            break;
        default:
            break;
    }
}

void BubblePaintMethod::BuildRightLinePath(RSPath& path, float arrowOffset, float radius)
{
    float childOffsetY = childOffset_.GetY();
    float arrowPositionY = arrowPosition_.GetY();
    switch (arrowPlacement_) {
        case Placement::LEFT:
        case Placement::LEFT_TOP:
        case Placement::LEFT_BOTTOM:
            path.LineTo(childOffset_.GetX() + childSize_.Width(),
                arrowPositionY + arrowOffset - BEZIER_WIDTH_HALF.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx(),
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx(),
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(), arrowPosition_.GetX(),
                arrowPositionY + arrowOffset);
            path.QuadTo(arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx(),
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(),
                arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx(),
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_FOURTH.ConvertToPx());
            break;
        default:
            break;
    }
    path.LineTo(childOffset_.GetX() + childSize_.Width(), childOffsetY + childSize_.Height() - radius);
}

void BubblePaintMethod::BuildBottomLinePath(RSPath& path, float arrowOffset, float radius)
{
    float childOffsetY = childOffset_.GetY();
    float arrowPositionY = arrowPosition_.GetY();
    switch (arrowPlacement_) {
        case Placement::TOP:
        case Placement::TOP_LEFT:
        case Placement::TOP_RIGHT:
            path.LineTo(arrowPosition_.GetX() + arrowOffset + BEZIER_WIDTH_HALF.ConvertToPx(),
                childOffsetY + childSize_.Height());
            path.QuadTo(arrowPosition_.GetX() + arrowOffset + BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
                arrowPosition_.GetX() + arrowOffset + BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() + arrowOffset + BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx(), arrowPosition_.GetX() + arrowOffset,
                arrowPositionY);
            path.QuadTo(arrowPosition_.GetX() + arrowOffset - BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx(),
                arrowPosition_.GetX() + arrowOffset - BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() + arrowOffset - BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
                arrowPosition_.GetX() + arrowOffset - BEZIER_HORIZON_OFFSET_FOURTH.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx());
            break;
        default:
            break;
    }
    path.LineTo(childOffset_.GetX() + radius, childOffsetY + childSize_.Height());
}

void BubblePaintMethod::BuildLeftLinePath(RSPath& path, float arrowOffset, float radius)
{
    float childOffsetY = childOffset_.GetY();
    float arrowPositionY = arrowPosition_.GetY();
    switch (arrowPlacement_) {
        case Placement::RIGHT:
        case Placement::RIGHT_TOP:
        case Placement::RIGHT_BOTTOM:
            path.LineTo(childOffset_.GetX(), arrowPositionY + arrowOffset + BEZIER_WIDTH_HALF.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx(),
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx(),
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(), arrowPosition_.GetX(),
                arrowPositionY + arrowOffset);
            path.QuadTo(arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx(),
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(),
                arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx(),
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_FOURTH.ConvertToPx());
            break;
        default:
            break;
    }
    path.LineTo(childOffset_.GetX(), childOffsetY + radius);
}

float BubblePaintMethod::GetArrowOffset(const Placement& placement)
{
    double motionRange = 0.0;
    Edge edge;
    InitEdgeSize(edge);
    switch (placement) {
        case Placement::TOP_LEFT:
        case Placement::TOP_RIGHT:
            motionRange = childSize_.Width() - edge.Top().Value() - ARROW_WIDTH.ConvertToPx();
            break;
        case Placement::TOP:
            motionRange = childSize_.Width() - edge.Top().Value() - ARROW_WIDTH.ConvertToPx();
            break;
        case Placement::BOTTOM:
            motionRange = childSize_.Width() - edge.Bottom().Value() - ARROW_WIDTH.ConvertToPx();
            break;
        case Placement::LEFT:
        case Placement::LEFT_TOP:
        case Placement::LEFT_BOTTOM:
            motionRange = childSize_.Height() - edge.Left().Value() - ARROW_WIDTH.ConvertToPx();
            break;
        case Placement::RIGHT:
        case Placement::RIGHT_TOP:
        case Placement::RIGHT_BOTTOM:
            motionRange = childSize_.Height() - edge.Right().Value() - ARROW_WIDTH.ConvertToPx();
            break;
        case Placement::BOTTOM_LEFT:
        case Placement::BOTTOM_RIGHT:
            motionRange = childSize_.Width() - edge.Bottom().Value() - ARROW_WIDTH.ConvertToPx();
            break;
        default:
            break;
    }
    return std::clamp(
        arrowOffset_.Unit() == DimensionUnit::PERCENT ? arrowOffset_.Value() * motionRange : arrowOffset_.ConvertToPx(),
        0.0, motionRange);
}

void BubblePaintMethod::InitEdgeSize(Edge& edge)
{
    edge.SetTop(Dimension(std::max(padding_.Left().ConvertToPx(), border_.TopLeftRadius().GetX().ConvertToPx()) +
                          std::max(padding_.Right().ConvertToPx(), border_.TopRightRadius().GetX().ConvertToPx())));
    edge.SetBottom(
        Dimension(std::max(padding_.Left().ConvertToPx(), border_.BottomLeftRadius().GetX().ConvertToPx()) +
                  std::max(padding_.Right().ConvertToPx(), border_.BottomRightRadius().GetX().ConvertToPx())));
    edge.SetLeft(
        Dimension(std::max(padding_.Top().ConvertToPx(), border_.TopRightRadius().GetY().ConvertToPx()) +
                  std::max(padding_.Bottom().ConvertToPx(), border_.BottomRightRadius().GetY().ConvertToPx())));
    edge.SetRight(
        Dimension(std::max(padding_.Top().ConvertToPx(), border_.TopLeftRadius().GetY().ConvertToPx()) +
                  std::max(padding_.Bottom().ConvertToPx(), border_.BottomLeftRadius().GetY().ConvertToPx())));
}

} // namespace OHOS::Ace::NG
