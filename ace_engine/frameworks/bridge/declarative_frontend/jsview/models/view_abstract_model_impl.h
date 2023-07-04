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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_VIEW_ABSTRACT_MODEL_IMPL_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_VIEW_ABSTRACT_MODEL_IMPL_H

#include "base/utils/macros.h"
#include "core/components_ng/base/view_abstract_model.h"
#include "core/components_ng/event/gesture_event_hub.h"

namespace OHOS::Ace::Framework {

class ViewAbstractModelImpl : public ViewAbstractModel {
public:
    ~ViewAbstractModelImpl() override = default;

    static void SwapBackBorder(const RefPtr<Decoration>& decoration);
    static OnDragFunc ToDragFunc(NG::OnDragStartFunc&& onDragStart);

    void SetWidth(const Dimension& width) override;
    void SetHeight(const Dimension& height) override;
    void ClearWidthOrHeight(bool isWidth) override {};
    void SetMinWidth(const Dimension& minWidth) override;
    void SetMinHeight(const Dimension& minHeight) override;
    void SetMaxWidth(const Dimension& maxWidth) override;
    void SetMaxHeight(const Dimension& maxHeight) override;

    void SetBackgroundColor(const Color& color) override;
    void SetBackgroundImage(const std::string& src, RefPtr<ThemeConstants> themeConstant) override;
    void SetBackgroundImageRepeat(const ImageRepeat& imageRepeat) override;
    void SetBackgroundImageSize(const BackgroundImageSize& bgImgSize) override;
    void SetBackgroundImagePosition(const BackgroundImagePosition& bgImgPosition) override;
    void SetBackgroundBlurStyle(const BlurStyleOption& bgBlurStyle) override;
    void SetPadding(const Dimension& value) override;
    void SetPaddings(const std::optional<Dimension>& top, const std::optional<Dimension>& bottom,
        const std::optional<Dimension>& left, const std::optional<Dimension>& right) override;
    void SetMargin(const Dimension& value) override;
    void SetMargins(const std::optional<Dimension>& top, const std::optional<Dimension>& bottom,
        const std::optional<Dimension>& left, const std::optional<Dimension>& right) override;
    void SetBorderRadius(const Dimension& value) override;
    void SetBorderRadius(const std::optional<Dimension>& radiusTopLeft, const std::optional<Dimension>& radiusTopRight,
        const std::optional<Dimension>& radiusBottomLeft, const std::optional<Dimension>& radiusBottomRight) override;
    void SetBorderColor(const Color& value) override;
    void SetBorderColor(const std::optional<Color>& colorLeft, const std::optional<Color>& colorRight,
        const std::optional<Color>& colorTop, const std::optional<Color>& colorBottom) override;
    void SetBorderWidth(const Dimension& value) override;
    void SetBorderWidth(const std::optional<Dimension>& left, const std::optional<Dimension>& right,
        const std::optional<Dimension>& top, const std::optional<Dimension>& bottom) override;
    void SetBorderStyle(const BorderStyle& value) override;
    void SetBorderStyle(const std::optional<BorderStyle>& styleLeft, const std::optional<BorderStyle>& styleRight,
        const std::optional<BorderStyle>& styleTop, const std::optional<BorderStyle>& styleBottom) override;
    void SetBorderImage(const RefPtr<BorderImage>& borderImage, uint8_t bitset) override;
    void SetBorderImageGradient(const NG::Gradient& gradient) override;

    void SetLayoutPriority(int32_t priority) override;
    void SetLayoutWeight(int32_t value) override;
    void SetLayoutDirection(TextDirection value) override;
    void SetAspectRatio(float ratio) override;
    void SetAlign(const Alignment& alignment) override;
    void SetAlignRules(const std::map<AlignDirection, AlignRule>& alignRules) override;
    void SetUseAlign(
        AlignDeclarationPtr declaration, AlignDeclaration::Edge edge, const std::optional<Dimension>& offset) override;
    void SetGrid(std::optional<uint32_t> span, std::optional<int32_t> offset,
        GridSizeType type = GridSizeType::UNDEFINED) override;
    void SetZIndex(int32_t value) override;

    void SetPosition(const Dimension& x, const Dimension& y) override;
    void SetOffset(const Dimension& x, const Dimension& y) override;
    void MarkAnchor(const Dimension& x, const Dimension& y) override;

    void SetScale(float x, float y, float z) override;
    void SetPivot(const Dimension& x, const Dimension& y) override;
    void SetTranslate(const Dimension& x, const Dimension& y, const Dimension& z) override;
    void SetRotate(float x, float y, float z, float angle) override;
    void SetTransformMatrix(const std::vector<float>& matrix) override;

    void SetOpacity(double opacity, bool passThrough = false) override;
    void SetTransition(const NG::TransitionOptions& transitionOptions, bool passThrough = false) override;
    void SetOverlay(const std::string& text, const std::optional<Alignment>& align,
        const std::optional<Dimension>& offsetX, const std::optional<Dimension>& offsetY) override;
    void SetVisibility(VisibleType visible, std::function<void(int32_t)>&& changeEventFunc) override;
    void SetSharedTransition(
        const std::string& shareId, const std::shared_ptr<SharedTransitionOption>& option) override;
    void SetGeometryTransition(const std::string& id) override;
    void SetMotionPath(const MotionPathOption& option) override;

    void SetFlexBasis(const Dimension& value) override;
    void SetAlignSelf(FlexAlign value) override;
    void SetFlexShrink(float value) override;
    void SetFlexGrow(float value) override;
    void SetDisplayIndex(int32_t value) override;

    void SetLinearGradient(const NG::Gradient& gradient) override;
    void SetSweepGradient(const NG::Gradient& gradient) override;
    void SetRadialGradient(const NG::Gradient& gradient) override;

    void SetClipShape(const RefPtr<BasicShape>& shape) override;
    void SetClipEdge(bool isClip) override;
    void SetMask(const RefPtr<BasicShape>& shape) override;

    void SetBackdropBlur(const Dimension& radius) override;
    void SetFrontBlur(const Dimension& radius) override;
    void SetBackShadow(const std::vector<Shadow>& shadows) override;
    void SetColorBlend(const Color& value) override;
    void SetWindowBlur(float progress, WindowBlurStyle blurStyle) override;
    void SetBrightness(const Dimension& value) override;
    void SetGrayScale(const Dimension& value) override;
    void SetContrast(const Dimension& value) override;
    void SetSaturate(const Dimension& value) override;
    void SetSepia(const Dimension& value) override;
    void SetInvert(const Dimension& value) override;
    void SetHueRotate(float value) override;

    void SetOnClick(GestureEventFunc&& tapEventFunc, ClickEventFunc&& clickEventFunc) override;
    void SetOnTouch(TouchEventFunc&& touchEventFunc) override;
    void SetOnKeyEvent(OnKeyCallbackFunc&& onKeyCallback) override;
    void SetOnMouse(OnMouseEventFunc&& onMouseEventFunc) override;
    void SetOnHover(OnHoverEventFunc&& onHoverEventFunc) override;
    void SetOnDelete(std::function<void()>&& onDeleteCallback) override;
    void SetOnAppear(std::function<void()>&& onAppearCallback) override;
    void SetOnDisAppear(std::function<void()>&& onDisAppearCallback) override;
    void SetOnAccessibility(std::function<void(const std::string&)>&& onAccessibilityCallback) override;
    void SetOnRemoteMessage(RemoteCallback&& onRemoteCallback) override;
    void SetOnFocusMove(std::function<void(int32_t)>&& onFocusMoveCallback) override;
    void SetOnFocus(OnFocusFunc&& onFocusCallback) override;
    void SetOnBlur(OnBlurFunc&& onBlurCallback) override;
    void SetOnDragStart(NG::OnDragStartFunc&& onDragStart) override;
    void SetOnDragEnter(NG::OnDragDropFunc&& onDragEnter) override;
    void SetOnDragLeave(NG::OnDragDropFunc&& onDragLeave) override;
    void SetOnDragMove(NG::OnDragDropFunc&& onDragMove) override;
    void SetOnDrop(NG::OnDragDropFunc&& onDrop) override;
    void SetOnVisibleChange(
        std::function<void(bool, double)>&& onVisibleChange, const std::vector<double>& ratios) override;
    void SetOnAreaChanged(
        std::function<void(const Rect& oldRect, const Offset& oldOrigin, const Rect& rect, const Offset& origin)>&&
            onAreaChanged) override;

    void SetResponseRegion(const std::vector<DimensionRect>& responseRegion) override;
    void SetEnabled(bool enabled) override;
    void SetTouchable(bool touchable) override;
    void SetFocusable(bool focusable) override;
    void SetFocusNode(bool focus) override;
    void SetTabIndex(int32_t index) override;
    void SetFocusOnTouch(bool isSet) override;
    void SetDefaultFocus(bool isSet) override;
    void SetGroupDefaultFocus(bool isSet) override;
    void SetInspectorId(const std::string& inspectorId) override;
    void SetRestoreId(int32_t restoreId) override;
    void SetDebugLine(const std::string& line) override;
    void SetHoverEffect(HoverEffectType hoverEffect) override;
    void SetHitTestMode(NG::HitTestMode hitTestMode) override;

    void BindPopup(const RefPtr<PopupParam>& param, const RefPtr<AceType>& customNode) override;
    void BindMenu(std::vector<NG::OptionParam>&& params, std::function<void()>&& buildFunc) override;
    void BindContextMenu(ResponseType type, std::function<void()>&& buildFunc) override;

    void SetAccessibilityGroup(bool accessible) override;
    void SetAccessibilityText(const std::string& text) override;
    void SetAccessibilityDescription(const std::string& description) override;
    void SetAccessibilityImportance(const std::string& importance) override;
};

} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_VIEW_ABSTRACT_MODEL_IMPL_H
