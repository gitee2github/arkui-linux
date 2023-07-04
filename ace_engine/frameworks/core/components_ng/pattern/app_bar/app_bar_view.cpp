/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/app_bar/app_bar_view.h"

#include "core/components_ng/pattern/app_bar/app_bar_theme.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/stage/stage_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {

const Dimension MARGIN_TEXT = 24.0_vp;
const Dimension MARGIN_BUTTON = 12.0_vp;
const Dimension MARGIN_BACK_BUTTON_RIGHT = -20.0_vp;

} // namespace

RefPtr<FrameNode> AppBarView::Create(RefPtr<FrameNode>& content)
{
    auto column = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto titleBar = BuildBarTitle();
    column->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
    column->AddChild(titleBar);
    column->AddChild(content);
    content->GetLayoutProperty()->UpdateLayoutWeight(1.0f);
    content->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
    auto stagePattern = content->GetPattern<StagePattern>();
    if (stagePattern) {
        stagePattern->SetOnRebuildFrameCallback([titleBar, content]() {
            CHECK_NULL_VOID(titleBar);
            CHECK_NULL_VOID(content);
            auto backButton = AceType::DynamicCast<FrameNode>(titleBar->GetFirstChild());
            CHECK_NULL_VOID(backButton);
            if (content->GetChildren().size() > 1) {
                backButton->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
                return;
            }
            backButton->GetLayoutProperty()->UpdateVisibility(VisibleType::GONE);
        });
    }
    return column;
}

RefPtr<FrameNode> AppBarView::BuildBarTitle()
{
    auto appBarRow = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    auto layoutProperty = appBarRow->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, nullptr);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto appBarTheme = pipeline->GetTheme<AppBarTheme>();
    layoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(1.0, DimensionUnit::PERCENT), CalcLength(appBarTheme->GetAppBarHeight())));
    layoutProperty->UpdateMainAxisAlign(FlexAlign::FLEX_START);
    layoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    auto renderContext = appBarRow->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, nullptr);
    renderContext->UpdateBackgroundColor(appBarTheme->GetBgColor());

    // create title label
    auto titleLabel = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    auto textLayoutProperty = titleLabel->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, nullptr);
    auto themeManager = pipeline->GetThemeManager();
    CHECK_NULL_RETURN(themeManager, nullptr);
    auto themeConstants = themeManager->GetThemeConstants();
    CHECK_NULL_RETURN(themeConstants, nullptr);
#ifdef PREVIEW
    auto label = themeConstants->GetString(pipeline->GetAppLabelId());
    if (label.empty()) {
        label = "label";
        LOGW("[Engine Log] Unable to get label for shared library in the Previewer. Perform this operation on the "
             "emulator or a real device instead.");
    }
    textLayoutProperty->UpdateContent(label);
#else
    textLayoutProperty->UpdateContent(themeConstants->GetString(pipeline->GetAppLabelId()));
#endif
    textLayoutProperty->UpdateMaxLines(2);
    textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    textLayoutProperty->UpdateFontSize(appBarTheme->GetFontSize());
    textLayoutProperty->UpdateTextColor(appBarTheme->GetTextColor());
    textLayoutProperty->UpdateFontWeight(FontWeight::MEDIUM);
    textLayoutProperty->UpdateAlignment(Alignment::CENTER_LEFT);
    textLayoutProperty->UpdateLayoutWeight(1.0f);

    MarginProperty margin;
    margin.left = CalcLength(MARGIN_TEXT);
    margin.right = CalcLength(MARGIN_TEXT);
    textLayoutProperty->UpdateMargin(margin);

    appBarRow->AddChild(BuildIconButton(
        InternalResource::ResourceId::APP_BAR_BACK_SVG,
        [pipeline](GestureEvent& info) {
            if (pipeline) {
                pipeline->CallRouterBackToPopPage();
            }
        },
        true));
    appBarRow->AddChild(titleLabel);

#ifndef IS_EMULATOR
    if (SystemProperties::GetExtSurfaceEnabled()) {
        appBarRow->AddChild(BuildIconButton(
            InternalResource::ResourceId::APP_BAR_FA_SVG,
            [pipeline, appBarTheme](GestureEvent& info) {
#ifdef PREVIEW
                LOGW("[Engine Log] Unable to show the SharePanel in the Previewer. Perform this operation on the "
                     "emulator or a real device instead.");
#else
                if (pipeline && appBarTheme) {
                    LOGI("start panel bundleName is %{public}s, abilityName is %{public}s",
                        appBarTheme->GetBundleName().c_str(), appBarTheme->GetAbilityName().c_str());
                    pipeline->FireSharePanelCallback(appBarTheme->GetBundleName(), appBarTheme->GetAbilityName());
                }
#endif
            },
            false));
    }
#endif

    return appBarRow;
}

RefPtr<FrameNode> AppBarView::BuildIconButton(
    InternalResource::ResourceId icon, GestureEventFunc&& clickCallback, bool isBackButton)
{
    // button image icon
    ImageSourceInfo imageSourceInfo;
    auto imageIcon = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());

    imageSourceInfo.SetResourceId(icon);
    auto imageLayoutProperty = imageIcon->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(imageLayoutProperty, nullptr);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto appBarTheme = pipeline->GetTheme<AppBarTheme>();
    imageLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(appBarTheme->GetIconSize()), CalcLength(appBarTheme->GetIconSize())));
    imageLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);
    imageIcon->MarkModifyDone();

    auto buttonNode = FrameNode::CreateFrameNode(
        V2::BUTTON_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ButtonPattern>());

    auto renderContext = buttonNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, nullptr);
    renderContext->UpdateBackgroundColor(Color::TRANSPARENT);

    auto buttonPattern = AceType::DynamicCast<ButtonPattern>(buttonNode->GetPattern());
    CHECK_NULL_RETURN(buttonPattern, nullptr);
    buttonPattern->SetClickedColor(appBarTheme->GetClickEffectColor());

    auto buttonEventHub = buttonNode->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(buttonEventHub, nullptr);
    auto clickEvent = AceType::MakeRefPtr<ClickEvent>(std::move(clickCallback));
    buttonEventHub->AddClickEvent(clickEvent);

    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_RETURN(buttonLayoutProperty, nullptr);
    buttonLayoutProperty->UpdateType(ButtonType::NORMAL);
    buttonLayoutProperty->UpdateBorderRadius(appBarTheme->GetIconCornerRadius());
    buttonLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(appBarTheme->GetIconSize() * 2), CalcLength(appBarTheme->GetIconSize() * 2)));
    MarginProperty margin;
    margin.left = CalcLength(isBackButton ? MARGIN_BUTTON : -MARGIN_BUTTON);
    margin.right = CalcLength(isBackButton ? MARGIN_BACK_BUTTON_RIGHT : MARGIN_BUTTON);
    buttonLayoutProperty->UpdateMargin(margin);
    buttonNode->MarkModifyDone();

    buttonNode->AddChild(imageIcon);
    return buttonNode;
}

} // namespace OHOS::Ace::NG
