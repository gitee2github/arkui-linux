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

#include "core/components_ng/pattern/shape/shape_container_pattern.h"

#include <algorithm>

#include "base/geometry/ng/rect_t.h"
#include "base/utils/utils.h"
#include "core/components_ng/render/adapter/skia_decoration_painter.h"

namespace OHOS::Ace::NG {
bool ShapeContainerPattern::OnDirtyLayoutWrapperSwap(
    const RefPtr<LayoutWrapper>& dirty, bool skipMeasure, bool skipLayout)
{
    if (skipMeasure || dirty->SkipMeasureContent() || isShapeContainerInit_) {
        return false;
    }
    ViewPortTransform();
    return true;
}

void ShapeContainerPattern::ViewPortTransform()
{
    auto curFrameNode = GetHost();
    auto renderContext = curFrameNode->GetRenderContext();
    auto geoNode = curFrameNode->GetGeometryNode();
    CHECK_NULL_VOID_NOLOG(geoNode);
    auto contentSize = geoNode->GetContentSize();
    auto paintProperty = curFrameNode->GetPaintProperty<ShapeContainerPaintProperty>();
    if (paintProperty->HasShapeViewBox() && paintProperty->GetShapeViewBoxValue().IsValid()) {
        double portWidth = paintProperty->GetShapeViewBoxValue().Width().ConvertToPx();
        double portHeight = paintProperty->GetShapeViewBoxValue().Height().ConvertToPx();
        double portLeft = paintProperty->GetShapeViewBoxValue().Left().ConvertToPx();
        double portTop = paintProperty->GetShapeViewBoxValue().Top().ConvertToPx();
        if (contentSize.IsPositive() && GreatNotEqual(portWidth, 0.0) && GreatNotEqual(portHeight, 0.0)) {
            double scale = std::min(contentSize.Width() / portWidth, contentSize.Height() / portHeight);
            double tx = contentSize.Width() * 0.5 - (portWidth * 0.5 + portLeft) * scale;
            double ty = contentSize.Height() * 0.5 - (portHeight * 0.5 + portTop) * scale;
            for (const auto& child : curFrameNode->GetChildren()) {
                auto node = AceType::DynamicCast<FrameNode>(child);
                CHECK_NULL_VOID(node);
                auto context = node->GetRenderContext();
                CHECK_NULL_VOID(context);
                context->UpdateTransformCenter(DimensionOffset(Offset(0.0, 0.0)));
                context->OnTransformTranslateUpdate({ tx, ty, 0 });
                context->OnTransformScaleUpdate({ scale, scale });

                if (context->HasOffset()) {
                    auto currentOffset = context->GetOffset();
                    auto newOffset = OffsetT(Dimension(currentOffset->GetX() * scale),
                        Dimension(currentOffset->GetY() * scale));
                    context->UpdateOffset(newOffset);
                    context->OnOffsetUpdate(newOffset);
                    node->MarkModifyDone();
                }
            }
        }
    }
    isShapeContainerInit_ = true;
}

void ShapeContainerPattern::OnModifyDone()
{
    for (auto childNode : childNodes_) {
        auto child = childNode.Upgrade();
        if (!child) {
            continue;
        }
        child->MarkNeedRenderOnly();
    }
    childNodes_.clear();
}

void ShapeContainerPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetLayoutProperty()->UpdateAlignment(Alignment::TOP_LEFT);
    host->GetRenderContext()->SetClipToFrame(true);
}

} // namespace OHOS::Ace::NG
