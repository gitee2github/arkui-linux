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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_IMAGE_IMAGE_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_IMAGE_IMAGE_PATTERN_H

#include <memory>

#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/image/image_event_hub.h"
#include "core/components_ng/pattern/image/image_layout_algorithm.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_render_property.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT ImagePattern : public Pattern {
    DECLARE_ACE_TYPE(ImagePattern, Pattern);

public:
    ImagePattern() = default;
    ~ImagePattern() override = default;

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override;

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<ImageLayoutProperty>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<ImageRenderProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<ImageLayoutAlgorithm>(loadingCtx_, altLoadingCtx_);
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<ImageEventHub>();
    }

    // Called on main thread to check if need rerender of the content.
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, false };
    }

    void LoadImageDataIfNeed();
    void OnNotifyMemoryLevel(int32_t level) override;
    void OnWindowHide() override;
    void OnWindowShow() override;
    void OnVisibleChange(bool isVisible) override;

    void EnableDrag();
    void SetDraggable(bool draggable)
    {
        draggable_ = draggable;
    }

private:
    void OnAttachToFrameNode() override;
    void OnDetachFromFrameNode(FrameNode* frameNode) override;

    void OnModifyDone() override;

    void PaintImage(RenderContext* renderContext, const OffsetF& offset);

    void OnImageDataReady();
    void OnImageLoadFail();
    void OnImageLoadSuccess();
    void SetImagePaintConfig(
        const RefPtr<CanvasImage>& canvasImage, const RectF& srcRect, const RectF& dstRect, bool isSvg);
    void UpdateInternalResource(ImageSourceInfo& sourceInfo);

    void PrepareAnimation();
    void SetRedrawCallback();
    void RegisterVisibleAreaChange();

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override;

    DataReadyNotifyTask CreateDataReadyCallback();
    LoadSuccessNotifyTask CreateLoadSuccessCallback();
    LoadFailNotifyTask CreateLoadFailCallback();

    DataReadyNotifyTask CreateDataReadyCallbackForAlt();
    LoadSuccessNotifyTask CreateLoadSuccessCallbackForAlt();
    LoadFailNotifyTask CreateLoadFailCallbackForAlt();

    RefPtr<ImageLoadingContext> loadingCtx_;
    RefPtr<CanvasImage> image_;
    RectF dstRect_;
    RectF srcRect_;

    bool draggable_ = false;
    bool isShow_ = true; // TODO: remove it later when use [isActive_] to determine image data management

    // clear alt data after [OnImageLoadSuccess] being called
    RefPtr<ImageLoadingContext> altLoadingCtx_;
    RefPtr<CanvasImage> altImage_;
    std::unique_ptr<RectF> altDstRect_;
    std::unique_ptr<RectF> altSrcRect_;

    ACE_DISALLOW_COPY_AND_MOVE(ImagePattern);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_IMAGE_IMAGE_PATTERN_H
