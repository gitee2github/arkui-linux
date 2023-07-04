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

#include "core/components_ng/pattern/select_overlay/select_overlay_paint_method.h"

#include "base/geometry/ng/offset_t.h"
#include "base/utils/utils.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_layout_algorithm.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
CanvasDrawFunction SelectOverlayPaintMethod::GetContentDrawFunction(PaintWrapper* /*paintWrapper*/)
{
    return [info = info_](RSCanvas& canvas) { SelectOverlayPaintMethod::DrawHandles(info, canvas); };
}

void SelectOverlayPaintMethod::DrawHandles(const std::shared_ptr<SelectOverlayInfo>& info, RSCanvas& canvas)
{
    if (!SelectOverlayLayoutAlgorithm::CheckInShowArea(info)) {
        LOGD("hide handles due to handle is out of show area");
        return;
    }
    LOGD("paint handles");
    if (info->isSingleHandle) {
        // Paint one handle.
        if (info->firstHandle.isShow) {
            PaintHandle(canvas, info->firstHandle.paintRect, false);
            return;
        }
        if (info->secondHandle.isShow) {
            PaintHandle(canvas, info->secondHandle.paintRect, false);
        }
    } else {
        if (info->firstHandle.isShow) {
            PaintHandle(canvas, info->firstHandle.paintRect, !info->handleReverse);
        }
        if (info->secondHandle.isShow) {
            PaintHandle(canvas, info->secondHandle.paintRect, info->handleReverse);
        }
    }
}

void SelectOverlayPaintMethod::PaintHandle(RSCanvas& canvas, const RectF& handleRect, bool handleOnTop)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(theme);
    auto handleColor = theme->GetHandleColor();
    auto innerHandleColor = theme->GetHandleColorInner();
    auto handleRadius = theme->GetHandleDiameter().ConvertToPx() / 2.0f;
    auto innerHandleRadius = theme->GetHandleDiameterInner().ConvertToPx() / 2.0f;
    auto rectTopX = (handleRect.Left() + handleRect.Right()) / 2.0f;
    auto centerOffset = OffsetF(rectTopX, 0.0f);
    if (handleOnTop) {
        centerOffset.SetY(handleRect.Top() - handleRadius);
    } else {
        centerOffset.SetY(handleRect.Bottom() + handleRadius);
    }
    canvas.Save();
    RSBrush brush;
    brush.SetAntiAlias(true);
    canvas.Translate(centerOffset.GetX(), centerOffset.GetY());
    // Paint outer circle.
    brush.SetColor(handleColor.GetValue());
    canvas.AttachBrush(brush);
    canvas.DrawCircle({ 0.0, 0.0 }, handleRadius);
    // Paint inner circle.
    brush.SetColor(innerHandleColor.GetValue());
    canvas.AttachBrush(brush);
    canvas.DrawCircle({ 0.0, 0.0 }, innerHandleRadius);
    canvas.DetachBrush();

    RSPen pen;
    pen.SetAntiAlias(true);
    // Paint line of handle.
    pen.SetColor(handleColor.GetValue());
    pen.SetWidth(handleRect.Width());
    pen.SetCapStyle(RSPen::CapStyle::ROUND_CAP);
    canvas.AttachPen(pen);
    // 1.0 is avoid separation of handle circle and handle line.
    OffsetF startPoint(0.0, -handleRadius + 1.0f);
    // 1.0_dp is designed by UX, handle line is higher than height of select region.
    OffsetF endPoint(0.0, -handleRadius - handleRect.Height());
    if (handleOnTop) {
        startPoint.SetY(handleRadius - 1.0);
        endPoint.SetY(handleRadius + handleRect.Height());
    }
    canvas.DrawLine({ startPoint.GetX(), startPoint.GetY() }, { endPoint.GetX(), endPoint.GetY() });
    canvas.DetachPen();
    canvas.Restore();
}

} // namespace OHOS::Ace::NG