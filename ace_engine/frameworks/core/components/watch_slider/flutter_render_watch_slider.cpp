/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "core/components/watch_slider/flutter_render_watch_slider.h"

#include "core/SkRRect.h"

#include "base/geometry/dimension.h"
#include "core/pipeline/base/scoped_canvas_state.h"

namespace OHOS::Ace {
namespace {

constexpr Dimension THICKNESS = 52.0_vp;
constexpr Dimension DEFAULT_PADDING = 3.0_vp;

} // namespace

void FlutterRenderWatchSlider::Paint(RenderContext& context, const Offset& offset)
{
    UpdatePosition(offset);
    ScopedCanvas canvas = ScopedCanvas::Create(context);
    if (!canvas) {
        LOGE("Paint canvas is null");
        return;
    }
    double topX = offset.GetX() + GetLayoutSize().Width() / 2.0 - NormalizeToPx(THICKNESS) / 2.0;
    double topY = offset.GetY() + NormalizeToPx(DEFAULT_PADDING);
    SkRect bottomClipRect = { topX, topY, topX + NormalizeToPx(THICKNESS),
        topY + GetLayoutSize().Height() - 2.0 * NormalizeToPx(DEFAULT_PADDING) };
    SkRRect bottomClipLayer =
        SkRRect::MakeRectXY(bottomClipRect, NormalizeToPx(THICKNESS) / 2.0, NormalizeToPx(THICKNESS) / 2.0);
    canvas.GetSkCanvas()->clipRRect(bottomClipLayer, SkClipOp::kIntersect, true);
    flutter::Paint paint;
    flutter::PaintData paintData;
    paint.paint()->setColor(backgroundColor_.GetValue());
    canvas->drawRect(topX, topY, topX + NormalizeToPx(THICKNESS),
        topY + GetLayoutSize().Height() - 2.0 * NormalizeToPx(DEFAULT_PADDING), paint, paintData);
    paint.paint()->setColor(selectColor_.GetValue());
    double maxRegion = 0.0;
    if (!NearEqual(max_, min_)) {
        maxRegion =
            (GetLayoutSize().Height() - 2.0 * NormalizeToPx(DEFAULT_PADDING)) * ((max_ - value_) / (max_ - min_));
    }
    canvas->drawRect(topX, topY + maxRegion, topX + NormalizeToPx(THICKNESS),
        topY + GetLayoutSize().Height() - 2.0 * NormalizeToPx(DEFAULT_PADDING), paint, paintData);
    double trackLength = GetLayoutSize().Height() - 2.0 * NormalizeToPx(DEFAULT_PADDING);
    if (!isContinuous_) {
        paint.paint()->setColor(Color::BLACK.GetValue());
        // equally divided track into three pieces. each block takes 1/4
        canvas->drawRect(topX, topY + trackLength * 0.25, topX + NormalizeToPx(THICKNESS),
            topY + trackLength * 0.25 + 2.0, paint, paintData);
        canvas->drawRect(topX, topY + trackLength * 0.5, topX + NormalizeToPx(THICKNESS),
            topY + trackLength * 0.5 + 2.0, paint, paintData);
        canvas->drawRect(topX, topY + trackLength * 0.75, topX + NormalizeToPx(THICKNESS),
            topY + trackLength * 0.75 + 2.0, paint, paintData);
    }

    bottomIcon_->RenderWithContext(context, bottomIconPosition_);
    topIcon_->RenderWithContext(context, topIconPosition_);
}

} // namespace OHOS::Ace
