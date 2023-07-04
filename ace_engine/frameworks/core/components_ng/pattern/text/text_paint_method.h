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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_PAINT_METHOD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_PAINT_METHOD_H

#include <utility>

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/macros.h"
#include "base/utils/utils.h"
#include "core/components_ng/render/node_paint_method.h"
#include "core/components_ng/render/paragraph.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT TextPaintMethod : public NodePaintMethod {
    DECLARE_ACE_TYPE(TextPaintMethod, NodePaintMethod)
public:
    TextPaintMethod(const WeakPtr<Pattern>& pattern, RefPtr<Paragraph> paragraph, float baselineOffset)
        : pattern_(pattern), paragraph_(std::move(paragraph)), baselineOffset_(baselineOffset)
    {}
    ~TextPaintMethod() override = default;

    CanvasDrawFunction GetContentDrawFunction(PaintWrapper* paintWrapper) override
    {
        CHECK_NULL_RETURN(paragraph_, nullptr);
        auto offset = paintWrapper->GetContentOffset();
        auto paintOffset = offset - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
        return [paragraph = paragraph_, paintOffset](
                   RSCanvas& canvas) { paragraph->Paint(canvas, paintOffset.GetX(), paintOffset.GetY()); };
    }

    CanvasDrawFunction GetOverlayDrawFunction(PaintWrapper* paintWrapper) override;

private:
    void PaintSelection(RSCanvas& canvas, PaintWrapper* paintWrapper);

    WeakPtr<Pattern> pattern_;
    RefPtr<Paragraph> paragraph_;
    float baselineOffset_;

    ACE_DISALLOW_COPY_AND_MOVE(TextPaintMethod);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_PAINT_METHOD_H
