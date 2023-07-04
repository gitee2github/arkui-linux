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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_VIEW_ABSTRACT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_VIEW_ABSTRACT_H

#include <cstdint>
#include <functional>

#include "base/geometry/dimension.h"
#include "base/geometry/matrix4.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/ng/vector.h"
#include "base/geometry/offset.h"
#include "base/memory/referenced.h"
#include "base/subwindow/subwindow_manager.h"
#include "core/common/container.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/layout/grid_layout_info.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/common/properties/clip_path.h"
#include "core/components/common/properties/decoration.h"
#include "core/components/common/properties/motion_path_option.h"
#include "core/components/common/properties/popup_param.h"
#include "core/components/common/properties/shared_transition_option.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/property/border_property.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/gradient_property.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/overlay_property.h"
#include "core/components_ng/property/transition_property.h"

namespace OHOS::Ace::NG {

using OptionParam = std::pair<std::string, std::function<void()>>;

class ACE_EXPORT ViewAbstract {
public:
    static void SetWidth(const CalcLength& width);
    static void SetHeight(const CalcLength& height);
    static void ClearWidthOrHeight(bool isWidth);
    static void SetMinWidth(const CalcLength& minWidth);
    static void SetMinHeight(const CalcLength& minHeight);
    static void SetMaxWidth(const CalcLength& maxWidth);
    static void SetMaxHeight(const CalcLength& maxHeight);

    static void SetAspectRatio(float ratio);
    static void SetLayoutWeight(int32_t value);
    static void SetLayoutDirection(TextDirection value);

    static void SetBackgroundColor(const Color& color);
    static void SetBackgroundImage(const std::string& src);
    static void SetBackgroundImageRepeat(const ImageRepeat& imageRepeat);
    static void SetBackgroundImageSize(const BackgroundImageSize& bgImgSize);
    static void SetBackgroundImagePosition(const BackgroundImagePosition& bgImgPosition);
    static void SetBackgroundBlurStyle(const BlurStyleOption& bgBlurStyle);
    static void SetPadding(const CalcLength& value);
    static void SetPadding(const PaddingProperty& value);
    static void SetMargin(const CalcLength& value);
    static void SetMargin(const PaddingProperty& value);
    static void SetBorderRadius(const BorderRadiusProperty& value);
    static void SetBorderRadius(const Dimension& value);
    static void SetBorderColor(const Color& value);
    static void SetBorderColor(const BorderColorProperty& value);
    static void SetBorderWidth(const Dimension& value);
    static void SetBorderWidth(const BorderWidthProperty& value);
    static void SetBorderStyle(const BorderStyle& value);
    static void SetBorderStyle(const BorderStyleProperty& value);
    static void SetOpacity(double opacity);

    static void SetBorderImage(const RefPtr<BorderImage>& borderImage);
    static void SetBorderImageSource(const std::string& bdImageSrc);
    static void SetHasBorderImageSlice(bool tag);
    static void SetHasBorderImageWidth(bool tag);
    static void SetHasBorderImageOutset(bool tag);
    static void SetHasBorderImageRepeat(bool tag);
    static void SetBorderImageGradient(const NG::Gradient& gradient);

    // decoration
    static void SetBackdropBlur(const Dimension& radius);
    static void SetFrontBlur(const Dimension& radius);
    static void SetBackShadow(const Shadow& shadow);

    // graphics
    static void SetBrightness(const Dimension& value);
    static void SetGrayScale(const Dimension& value);
    static void SetContrast(const Dimension& value);
    static void SetSaturate(const Dimension& value);
    static void SetSepia(const Dimension& value);
    static void SetInvert(const Dimension& value);
    static void SetHueRotate(float value);
    static void SetColorBlend(const Color& value);

    // gradient
    static void SetLinearGradient(const NG::Gradient& gradient);
    static void SetSweepGradient(const NG::Gradient& gradient);
    static void SetRadialGradient(const NG::Gradient& gradient);

    // layout
    static void SetAlign(Alignment alignment);
    static void SetAlignRules(const std::map<AlignDirection, AlignRule>& alignRules);
    static void SetVisibility(VisibleType visible);
    static void SetGrid(
        std::optional<int32_t> span, std::optional<int32_t> offset, GridSizeType type = GridSizeType::UNDEFINED);

    // position
    static void SetPosition(const OffsetT<Dimension>& value);
    static void SetOffset(const OffsetT<Dimension>& value);
    static void MarkAnchor(const OffsetT<Dimension>& value);

    // render position
    static void SetZIndex(int32_t value);

    // transform
    static void SetScale(const NG::VectorF& value);
    static void SetPivot(const DimensionOffset& value);
    static void SetTranslate(const NG::TranslateOptions& value);
    static void SetRotate(const NG::Vector4F& value);

    static void SetTransformMatrix(const Matrix4& matrix);

    // event
    static void SetOnClick(GestureEventFunc&& clickEventFunc);
    static void SetOnTouch(TouchEventFunc&& touchEventFunc);
    static void SetOnMouse(OnMouseEventFunc&& onMouseEventFunc);
    static void SetOnHover(OnHoverEventFunc&& onHoverEventFunc);
    static void SetHoverEffect(HoverEffectType hoverEffect);
    static void SetHoverEffectAuto(HoverEffectType hoverEffect);
    static void SetEnabled(bool enabled);
    static void SetFocusable(bool focusable);
    static void SetOnFocus(OnFocusFunc&& onFocusCallback);
    static void SetOnBlur(OnBlurFunc&& onBlurCallback);
    static void SetOnKeyEvent(OnKeyCallbackFunc&& onKeyCallback);
    static void SetTabIndex(int32_t index);
    static void SetFocusOnTouch(bool isSet);
    static void SetDefaultFocus(bool isSet);
    static void SetGroupDefaultFocus(bool isSet);
    static void SetOnAppear(std::function<void()>&& onAppear);
    static void SetOnDisappear(std::function<void()>&& onDisappear);
    static void SetOnAreaChanged(
        std::function<void(const RectF& oldRect, const OffsetF& oldOrigin, const RectF& rect, const OffsetF& origin)>&&
            onAreaChanged);
    static void SetOnVisibleChange(
        std::function<void(bool, double)>&& onVisibleChange, const std::vector<double>& ratioList);
    static void SetResponseRegion(const std::vector<DimensionRect>& responseRegion);
    static void SetTouchable(bool touchable);
    static void SetHitTestMode(HitTestMode hitTestMode);
    static void SetOnDragStart(
        std::function<DragDropInfo(const RefPtr<OHOS::Ace::DragEvent>&, const std::string&)>&& onDragStart);
    static void SetOnDragEnter(
        std::function<void(const RefPtr<OHOS::Ace::DragEvent>&, const std::string&)>&& onDragEnter);
    static void SetOnDragLeave(
        std::function<void(const RefPtr<OHOS::Ace::DragEvent>&, const std::string&)>&& onDragLeave);
    static void SetOnDragMove(
        std::function<void(const RefPtr<OHOS::Ace::DragEvent>&, const std::string&)>&& onDragMove);
    static void SetOnDrop(std::function<void(const RefPtr<OHOS::Ace::DragEvent>&, const std::string&)>&& onDrop);

    // flex properties
    static void SetAlignSelf(FlexAlign value);
    static void SetFlexShrink(float value);
    static void SetFlexGrow(float value);
    static void SetFlexBasis(const Dimension& value);
    static void SetDisplayIndex(int32_t value);

    // Bind properties
    static void BindPopup(
        const RefPtr<PopupParam>& param, const RefPtr<FrameNode>& targetNode, const RefPtr<UINode>& customNode);
    static void BindMenuWithItems(std::vector<OptionParam>&& params, const RefPtr<FrameNode>& targetNode,
        const NG::OffsetF& offset);
    static void BindMenuWithCustomNode(const RefPtr<UINode>& customNode, const RefPtr<FrameNode>& targetNode,
        bool isContextMenu, const NG::OffsetF& offset);
    static void ShowMenu(int32_t targetId, const NG::OffsetF& offset, bool isContextMenu = false);
    // inspector
    static void SetInspectorId(const std::string& inspectorId);
    // inspector debugLine
    static void SetDebugLine(const std::string& line);
    // transition
    static void SetTransition(const TransitionOptions& options);
    // sharedTransition
    static void SetSharedTransition(const std::string& shareId, const std::shared_ptr<SharedTransitionOption>& option);
    // clip and mask
    static void SetClipShape(const RefPtr<BasicShape>& basicShape);
    static void SetClipEdge(bool isClip);
    static void SetMask(const RefPtr<BasicShape>& basicShape);
    // overlay
    static void SetOverlay(const NG::OverlayOptions& overlay);
    // motionPath
    static void SetMotionPath(const MotionPathOption& motionPath);

    static void Pop();

private:
    static void AddDragFrameNodeToManager();
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_VIEW_ABSTRACT_H
