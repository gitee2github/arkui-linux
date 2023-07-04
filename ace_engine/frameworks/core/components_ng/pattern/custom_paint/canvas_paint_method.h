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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CUSTOM_PAINT_CANVAS_PAINT_METHOD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CUSTOM_PAINT_CANVAS_PAINT_METHOD_H

#include "core/components_ng/pattern/custom_paint/custom_paint_paint_method.h"
#include "core/components_ng/pattern/custom_paint/offscreen_canvas_pattern.h"

namespace OHOS::Ace::NG {
class CanvasPaintMethod;
using TaskFunc = std::function<void(CanvasPaintMethod&, PaintWrapper*)>;
class CanvasPaintMethod : public CustomPaintPaintMethod {
    DECLARE_ACE_TYPE(CanvasPaintMethod, CustomPaintPaintMethod)
public:
    CanvasPaintMethod() = default;
    explicit CanvasPaintMethod(const WeakPtr<PipelineBase> context)
    {
        context_ = context;
        auto* currentDartState = flutter::UIDartState::Current();
        CHECK_NULL_VOID(currentDartState);
        renderTaskHolder_ = MakeRefPtr<FlutterRenderTaskHolder>(currentDartState->GetSkiaUnrefQueue(),
            currentDartState->GetIOManager(), currentDartState->GetTaskRunners().GetIOTaskRunner());

        InitImageCallbacks();
    }

    ~CanvasPaintMethod() override = default;

    CanvasDrawFunction GetForegroundDrawFunction(PaintWrapper* paintWrapper) override;

    void PushTask(const TaskFunc& task)
    {
        tasks_.emplace_back(task);
    }

    bool HasTask() const
    {
        return !tasks_.empty();
    }

    double GetWidth()
    {
        return lastLayoutSize_.Width();
    }

    double GetHeight()
    {
        return lastLayoutSize_.Height();
    }

    void DrawImage(PaintWrapper* paintWrapper, const Ace::CanvasImage& canvasImage, double width, double height);
    void DrawPixelMap(RefPtr<PixelMap> pixelMap, const Ace::CanvasImage& canvasImage);
    std::unique_ptr<Ace::ImageData> GetImageData(double left, double top, double width, double height);
    void TransferFromImageBitmap(PaintWrapper* paintWrapper, const RefPtr<OffscreenCanvasPattern>& offscreenCanvas);
    std::string ToDataURL(const std::string& args);
    std::string GetJsonData(const std::string& path);

    void FillText(PaintWrapper* paintWrapper, const std::string& text, double x, double y);
    void StrokeText(PaintWrapper* paintWrapper, const std::string& text, double x, double y);
    double MeasureText(const std::string& text, const PaintState& state);
    double MeasureTextHeight(const std::string& text, const PaintState& state);
    TextMetrics MeasureTextMetrics(const std::string& text, const PaintState& state);
    void SetTransform(const TransformParam& param) override;

private:
    void PaintCustomPaint(RSCanvas& canvas, PaintWrapper* paintWrapper);
    void CreateBitmap(SizeF contentSize);

    void ImageObjReady(const RefPtr<Ace::ImageObject>& imageObj) override;
    void ImageObjFailed() override;
    sk_sp<SkImage> GetImage(const std::string& src) override;
    void SetPaintImage() override {};

    void PaintText(
        const OffsetF& offset, const SizeF& contentSize, double x, double y, bool isStroke, bool hasShadow = false);
    double GetAlignOffset(TextAlign align, std::unique_ptr<txt::Paragraph>& paragraph);
    double GetBaselineOffset(TextBaseline baseline, std::unique_ptr<txt::Paragraph>& paragraph);
    bool UpdateParagraph(const OffsetF& offset, const std::string& text, bool isStroke, bool hasShadow = false);
    void UpdateTextStyleForeground(const OffsetF& offset, bool isStroke, txt::TextStyle& txtStyle, bool hasShadow);
    void PaintShadow(const SkPath& path, const Shadow& shadow, SkCanvas* canvas) override;
    OffsetF GetContentOffset(PaintWrapper* paintWrapper) const override
    {
        return paintWrapper ? paintWrapper->GetContentOffset() : OffsetF(0.0f, 0.0f);
    }
    void Path2DRect(const OffsetF& offset, const PathArgs& args) override;
    SkCanvas* GetRawPtrOfSkCanvas() override
    {
        return skCanvas_.get();
    }

    std::list<TaskFunc> tasks_;
    SizeF lastLayoutSize_;

    RefPtr<Ace::ImageObject> imageObj_ = nullptr;
    RefPtr<ImageCache> imageCache_;

    ACE_DISALLOW_COPY_AND_MOVE(CanvasPaintMethod);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_CUSTOM_PAINT_CANVAS_PAINT_METHOD_H
