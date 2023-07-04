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

#include "core/components_ng/render/adapter/pixelmap_image.h"

#include "base/image/pixel_map.h"
#include "base/utils/utils.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"

#ifdef ENABLE_ROSEN_BACKEND
#include "render_service_client/core/ui/rs_node.h"
#include "render_service_client/core/ui/rs_surface_node.h"
#include "render_service_client/core/ui/rs_ui_director.h"
#endif

namespace OHOS::Ace::NG {
namespace {
#ifdef ENABLE_ROSEN_BACKEND
constexpr uint8_t RADIUS_POINTS_SIZE = 4;
#endif
} // namespace
RefPtr<CanvasImage> CanvasImage::Create(const RefPtr<PixelMap>& pixelMap)
{
#ifndef NG_BUILD
    return AceType::MakeRefPtr<PixelMapImage>(pixelMap);
#else
    return nullptr;
#endif
}

int32_t PixelMapImage::GetWidth() const
{
    if (pixelMap_) {
        return pixelMap_->GetWidth();
    }
    LOGW("pixelMap_ is nullptr, return width 0.");
    return 0;
}

int32_t PixelMapImage::GetHeight() const
{
    if (pixelMap_) {
        return pixelMap_->GetHeight();
    }
    LOGW("rsCanvas is nullptr, return height 0.");
    return 0;
}

void PixelMapImage::DrawToRSCanvas(
    RSCanvas& canvas, const RSRect& /* srcRect */, const RSRect& /* dstRect */, const BorderRadiusArray& radiusXY)
{
    if (!pixelMap_) {
        return;
    }

#ifdef ENABLE_ROSEN_BACKEND
    auto rsCanvas = canvas.GetImpl<RSSkCanvas>();
    if (rsCanvas == nullptr) {
        LOGE("rsCanvas is nullptr.");
        return;
    }
    auto skCanvas = rsCanvas->ExportSkCanvas();
    if (skCanvas == nullptr) {
        LOGE("skCanvas is nullptr.");
        return;
    }
    auto recordingCanvas = static_cast<OHOS::Rosen::RSRecordingCanvas*>(skCanvas);
    if (recordingCanvas == nullptr) {
        LOGE("recordingCanvas is nullptr.");
        return;
    }
    SkPaint paint;
    SkVector radii[RADIUS_POINTS_SIZE] = { { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 } };
    if (radiusXY.size() == RADIUS_POINTS_SIZE) {
        radii[SkRRect::kUpperLeft_Corner].set(
            SkFloatToScalar(std::max(radiusXY[SkRRect::kUpperLeft_Corner].GetX(), 0.0f)),
            SkFloatToScalar(std::max(radiusXY[SkRRect::kUpperLeft_Corner].GetY(), 0.0f)));
        radii[SkRRect::kUpperRight_Corner].set(
            SkFloatToScalar(std::max(radiusXY[SkRRect::kUpperRight_Corner].GetX(), 0.0f)),
            SkFloatToScalar(std::max(radiusXY[SkRRect::kUpperRight_Corner].GetY(), 0.0f)));
        radii[SkRRect::kLowerLeft_Corner].set(
            SkFloatToScalar(std::max(radiusXY[SkRRect::kLowerRight_Corner].GetX(), 0.0f)),
            SkFloatToScalar(std::max(radiusXY[SkRRect::kLowerRight_Corner].GetY(), 0.0f)));
        radii[SkRRect::kLowerRight_Corner].set(
            SkFloatToScalar(std::max(radiusXY[SkRRect::kLowerLeft_Corner].GetX(), 0.0f)),
            SkFloatToScalar(std::max(radiusXY[SkRRect::kLowerLeft_Corner].GetY(), 0.0f)));
    }
    recordingCanvas->ClipAdaptiveRRect(radii);
    Rosen::RsImageInfo rsImageInfo(
        (int)(GetPaintConfig().imageFit_), (int)(GetPaintConfig().imageRepeat_), radii, 1.0, 0, 0, 0);
    recordingCanvas->DrawPixelMapWithParm(pixelMap_->GetPixelMapSharedPtr(), rsImageInfo, paint);
#endif
}

} // namespace OHOS::Ace::NG