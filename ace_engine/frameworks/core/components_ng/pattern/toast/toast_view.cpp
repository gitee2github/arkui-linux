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

#include "core/components_ng/pattern/toast/toast_view.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/toast/toast_theme.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {
namespace {
float GetTextHeight(const RefPtr<FrameNode>& textNode)
{
    auto textlayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textlayoutProperty, 0.0f);
    auto layoutConstraint = textlayoutProperty->GetLayoutConstraint();

    auto textLayoutWrapper = textNode->CreateLayoutWrapper();
    CHECK_NULL_RETURN(textLayoutWrapper, 0.0f);
    textLayoutWrapper->Measure(layoutConstraint);
    auto textGeometry = textLayoutWrapper->GetGeometryNode();
    CHECK_NULL_RETURN(textGeometry, 0.0f);
    auto textSize = textGeometry->GetMarginFrameSize();
    return textSize.Height();
}
} // namespace

RefPtr<FrameNode> ToastView::CreateToastNode(const std::string& message, const std::string& bottom, bool isRightToLeft)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, nullptr);
    auto textId = ElementRegister::GetInstance()->MakeUniqueId();
    auto toastId = ElementRegister::GetInstance()->MakeUniqueId();
    LOGI("begin to show toast, toast id is %{public}d, message is %{public}s", toastId, message.c_str());

    // make toast node
    auto toastNode =
        FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, toastId, AceType::MakeRefPtr<LinearLayoutPattern>(true));
    CHECK_NULL_RETURN(toastNode, nullptr);
    auto toastProperty = toastNode->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(toastProperty, nullptr);
    auto toastContext = toastNode->GetRenderContext();
    CHECK_NULL_RETURN(toastContext, nullptr);

    auto textNode = FrameNode::CreateFrameNode(V2::TOAST_ETS_TAG, textId, AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textNode, nullptr);
    auto textlayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textlayoutProperty, nullptr);
    auto textContext = textNode->GetRenderContext();
    CHECK_NULL_RETURN(textContext, nullptr);

    auto toastTheme = context->GetTheme<ToastTheme>();
    CHECK_NULL_RETURN(toastTheme, nullptr);

    // update toast props
    auto rootHeight = Dimension(context->GetRootHeight());
    auto rootWidth = Dimension(context->GetRootWidth());
    toastProperty->UpdateUserDefinedIdealSize(CalcSize(NG::CalcLength(rootWidth), std::nullopt));

    auto bottomPosition =
        Dimension(StringUtils::StringToDimensionWithThemeValue(bottom, true, toastTheme->GetBottom()).ConvertToPx());
    Color textColor;
    Color toastBackgroundColor;
    Dimension fontSize;
    Dimension minWidth;
    Dimension maxWidth;
    Dimension minHeight;
    FontWeight fontWeight;
    Dimension toastBottom;
    BorderRadiusProperty borderRadius;
    PaddingProperty paddings;

    toastBackgroundColor = toastTheme->GetBackgroundColor();
    fontWeight = toastTheme->GetTextStyle().GetFontWeight();
    textColor = toastTheme->GetTextStyle().GetTextColor();
    fontSize = toastTheme->GetTextStyle().GetFontSize();
    minWidth = Dimension(toastTheme->GetMinWidth().ConvertToPx());
    minHeight = Dimension(toastTheme->GetMinHeight().ConvertToPx());
    toastBottom = Dimension(
        GreatOrEqual(bottomPosition.Value(), 0.0) ? bottomPosition.Value() : toastTheme->GetBottom().ConvertToPx());
    auto radius = toastTheme->GetRadius();
    borderRadius.SetRadius(Dimension(radius.GetX().ConvertToPx()));
    auto padding = toastTheme->GetPadding();
    paddings.top = NG::CalcLength(padding.Top());
    paddings.bottom = NG::CalcLength(padding.Bottom());
    paddings.left = NG::CalcLength(padding.Left());
    paddings.right = NG::CalcLength(padding.Right());

    auto gridColumnInfo = GridSystemManager::GetInstance().GetInfoByType(GridColumnType::TOAST);
    auto parent = gridColumnInfo->GetParent();
    if (parent) {
        parent->BuildColumnWidth();
    }
    maxWidth = Dimension(gridColumnInfo->GetMaxWidth());

    textlayoutProperty->UpdateContent(message);
    textlayoutProperty->UpdateTextColor(textColor);
    textlayoutProperty->UpdateTextAlign(TextAlign::CENTER);
    textlayoutProperty->UpdateFontWeight(fontWeight);
    textlayoutProperty->UpdateFontSize(fontSize);
    textlayoutProperty->UpdateCalcMaxSize(CalcSize(NG::CalcLength(maxWidth), std::nullopt));
    textlayoutProperty->UpdateCalcMinSize(CalcSize(NG::CalcLength(minWidth), NG::CalcLength(minHeight)));
    textlayoutProperty->UpdateLayoutDirection((isRightToLeft ? TextDirection::RTL : TextDirection::LTR));
    textlayoutProperty->UpdatePadding(paddings);

    textContext->UpdateBackgroundColor(toastBackgroundColor);
    textContext->UpdateBorderRadius(borderRadius);
    textContext->UpdateBackShadow(ShadowConfig::DefaultShadowL);
    toastNode->MarkModifyDone();

    toastContext->UpdateOffset(
        OffsetT<Dimension>(0.0_px, rootHeight - toastBottom - Dimension(GetTextHeight(textNode))));

    textNode->MountToParent(toastNode);
    return toastNode;
}

} // namespace OHOS::Ace::NG
