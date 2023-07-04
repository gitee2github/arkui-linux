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
#include "core/components_ng/pattern/rating/rating_pattern.h"

#include <cstdint>
#include <iomanip>
#include <sstream>

#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/color.h"
#include "core/components/theme/icon_theme.h"
#include "core/components_ng/pattern/rating/rating_model_ng.h"
#include "core/components_ng/pattern/rating/rating_paint_method.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"
#include "core/image/image_source_info.h"

namespace OHOS::Ace::NG {
constexpr int32_t RATING_IMAGE_SUCCESS_CODE = 0b111;
constexpr int32_t DEFAULT_RATING_TOUCH_STAR_NUMBER = 0;

void RatingPattern::CheckImageInfoHasChangedOrNot(
    int32_t imageFlag, const ImageSourceInfo& sourceInfo, const std::string& lifeCycleTag)
{
    auto ratingLayoutProperty = GetLayoutProperty<RatingLayoutProperty>();
    CHECK_NULL_VOID(ratingLayoutProperty);
    auto currentSourceInfo = ImageSourceInfo("");
    switch (imageFlag) {
        case 0b001:
            currentSourceInfo = ratingLayoutProperty->GetForegroundImageSourceInfo().value_or(ImageSourceInfo(""));
            CHECK_NULL_VOID(currentSourceInfo == sourceInfo);
            if (lifeCycleTag == "ImageDataFailed") {
                LOGE("Rating load foreground image failed");
            }
            break;
        case 0b010:
            currentSourceInfo = ratingLayoutProperty->GetSecondaryImageSourceInfo().value_or(ImageSourceInfo(""));
            CHECK_NULL_VOID(currentSourceInfo == sourceInfo);
            if (lifeCycleTag == "ImageDataFailed") {
                LOGE("Rating load secondary image failed");
            }
            break;
        case 0b100:
            currentSourceInfo = ratingLayoutProperty->GetBackgroundImageSourceInfo().value_or(ImageSourceInfo(""));
            CHECK_NULL_VOID(currentSourceInfo == sourceInfo);
            if (lifeCycleTag == "ImageDataFailed") {
                LOGE("Rating load background image failed");
            }
            break;
        default:
            break;
    }
}

LoadSuccessNotifyTask RatingPattern::CreateLoadSuccessCallback(int32_t imageFlag)
{
    auto task = [weak = WeakClaim(this), imageFlag](const ImageSourceInfo& sourceInfo) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        // check image info has changed or not
        pattern->CheckImageInfoHasChangedOrNot(imageFlag, sourceInfo, "ImageDataSuccess");
        pattern->OnImageLoadSuccess(imageFlag);
    };
    return task;
}

DataReadyNotifyTask RatingPattern::CreateDataReadyCallback(int32_t imageFlag)
{
    auto task = [weak = WeakClaim(this), imageFlag](const ImageSourceInfo& sourceInfo) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        // check image info has changed or not
        pattern->CheckImageInfoHasChangedOrNot(imageFlag, sourceInfo, "ImageDataReady");
        pattern->OnImageDataReady(imageFlag);
    };
    return task;
}

LoadFailNotifyTask RatingPattern::CreateLoadFailCallback(int32_t imageFlag)
{
    auto task = [weak = WeakClaim(this), imageFlag](const ImageSourceInfo& sourceInfo) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        // check image info has changed or not
        pattern->CheckImageInfoHasChangedOrNot(imageFlag, sourceInfo, "ImageDataFailed");
    };
    return task;
}

void RatingPattern::OnImageLoadSuccess(int32_t imageFlag)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (imageFlag == 0b001) {
        foregroundImageCanvas_ = foregroundImageLoadingCtx_->MoveCanvasImage();
        foregroundConfig_.srcRect_ = foregroundImageLoadingCtx_->GetSrcRect();
        foregroundConfig_.dstRect_ = foregroundImageLoadingCtx_->GetDstRect();
        imageSuccessStateCode_ = imageFlag | imageSuccessStateCode_;
    }
    if (imageFlag == 0b010) {
        secondaryImageCanvas_ = secondaryImageLoadingCtx_->MoveCanvasImage();
        secondaryConfig_.srcRect_ = secondaryImageLoadingCtx_->GetSrcRect();
        secondaryConfig_.dstRect_ = secondaryImageLoadingCtx_->GetDstRect();
        imageSuccessStateCode_ = imageFlag | imageSuccessStateCode_;
    }

    if (imageFlag == 0b100) {
        backgroundImageCanvas_ = backgroundImageLoadingCtx_->MoveCanvasImage();
        backgroundConfig_.srcRect_ = backgroundImageLoadingCtx_->GetSrcRect();
        backgroundConfig_.dstRect_ = backgroundImageLoadingCtx_->GetDstRect();
        imageSuccessStateCode_ = imageFlag | imageSuccessStateCode_;
    }
    // only when foreground, secondary and background image are all loaded successfully, mark dirty to update rendering.
    if (imageSuccessStateCode_ == RATING_IMAGE_SUCCESS_CODE) {
        LOGD("Rating foreground, secondary and background image are loaded successfully.");
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

void RatingPattern::OnImageDataReady(int32_t imageFlag)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    imageReadyStateCode_ = imageReadyStateCode_ | imageFlag;

    // 3 images are ready, invoke to update layout to calculate single star size.
    if (imageReadyStateCode_ == RATING_IMAGE_SUCCESS_CODE) {
        LOGD("Rating foreground, secondary and background image are ready.");
        host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
    }
}

RefPtr<NodePaintMethod> RatingPattern::CreateNodePaintMethod()
{
    CHECK_NULL_RETURN(foregroundImageCanvas_, nullptr);
    CHECK_NULL_RETURN(secondaryImageCanvas_, nullptr);
    CHECK_NULL_RETURN(backgroundImageCanvas_, nullptr);
    auto layoutProperty = GetLayoutProperty<RatingLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, nullptr);
    auto starNum =
        layoutProperty->GetStars().value_or(GetStarNumFromTheme().value_or(OHOS::Ace::DEFAULT_RATING_STAR_NUM));
    foregroundConfig_.imageFit_ = ImageFit::TOP_LEFT;
    secondaryConfig_.imageFit_ = ImageFit::TOP_LEFT;
    backgroundConfig_.imageFit_ = ImageFit::TOP_LEFT;
    if (!ratingModifier_) {
        ratingModifier_ = AceType::MakeRefPtr<RatingModifier>();
    }
    // when frameNode mark dirty to update rendering, only when 3 images are all loaded successfully and JudgeImageUri
    // is true, pattern will update ratingModifier's CanvasImage.
    if (ratingModifier_->JudgeImageUri(
        layoutProperty->GetForegroundImageSourceInfo()->GetSrc(),
        layoutProperty->GetSecondaryImageSourceInfo()->GetSrc(),
        layoutProperty->GetBackgroundImageSourceInfo()->GetSrc(), foregroundConfig_.dstRect_) &&
        imageSuccessStateCode_ == RATING_IMAGE_SUCCESS_CODE) {
        ratingModifier_->UpdateImageUri(
            layoutProperty->GetForegroundImageSourceInfo()->GetSrc(),
            layoutProperty->GetSecondaryImageSourceInfo()->GetSrc(),
            layoutProperty->GetBackgroundImageSourceInfo()->GetSrc());
        ratingModifier_->UpdateCanvasImage(foregroundImageCanvas_, secondaryImageCanvas_, backgroundImageCanvas_,
            foregroundConfig_, secondaryConfig_, backgroundConfig_);
    }
    return MakeRefPtr<RatingPaintMethod>(ratingModifier_, starNum, state_);
}

bool RatingPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure || dirty->SkipMeasureContent()) {
        return false;
    }
    if (!foregroundImageCanvas_ || !secondaryImageCanvas_ || !backgroundImageCanvas_) {
        return false;
    }
    return true;
}

ImageSourceInfo RatingPattern::GetImageSourceInfoFromTheme(int32_t imageFlag)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ImageSourceInfo imageSourceInfo;
    CHECK_NULL_RETURN(pipeline, imageSourceInfo);
    auto ratingTheme = pipeline->GetTheme<RatingTheme>();
    CHECK_NULL_RETURN(ratingTheme, imageSourceInfo);

    switch (imageFlag) {
        case 0b001:
            imageSourceInfo.SetResourceId(ratingTheme->GetForegroundResourceId());
            break;
        case 0b010:
            imageSourceInfo.SetResourceId(ratingTheme->GetSecondaryResourceId());
            break;
        case 0b100:
            imageSourceInfo.SetResourceId(ratingTheme->GetBackgroundResourceId());
            break;
        default:
            break;
    }
    return imageSourceInfo;
}

void RatingPattern::ConstrainsRatingScore()
{
    auto ratingLayoutProperty = GetLayoutProperty<RatingLayoutProperty>();
    auto ratingRenderProperty = GetPaintProperty<RatingRenderProperty>();

    // constrains ratingScore and StarNum.
    // check if starNum is not positive, assign the value defined in theme.
    if (ratingLayoutProperty->HasStars() && ratingLayoutProperty->GetStars().value() <= 0) {
        ratingLayoutProperty->UpdateStars(GetStarNumFromTheme().value_or(OHOS::Ace::DEFAULT_RATING_STAR_NUM));
    }

    // if ratingScore < 0, assign the value defined in theme.
    if (ratingRenderProperty->HasRatingScore()) {
        if (LessOrEqual(ratingRenderProperty->GetRatingScore().value(), 0.0)) {
            ratingRenderProperty->UpdateRatingScore(GetRatingScoreFromTheme().value_or(0.0));
        }
    }

    // steps max is stars, if steps > stars, assign the value defined in theme.
    if (ratingRenderProperty->HasStepSize()) {
        if (GreatNotEqual(
            ratingRenderProperty->GetStepSizeValue(
                GetStepSizeFromTheme().value_or(OHOS::Ace::DEFAULT_RATING_STEP_SIZE)),
            ratingLayoutProperty->GetStarsValue(
                GetStarNumFromTheme().value_or(OHOS::Ace::DEFAULT_RATING_STAR_NUM)))) {
            ratingRenderProperty->UpdateStepSize(GetStepSizeFromTheme().value_or(OHOS::Ace::DEFAULT_RATING_STEP_SIZE));
        }
    }

    // Calculate drewScore based on the stepSize, and it is cannot be greater than starNum.
    const double ratingScore = ratingRenderProperty->GetRatingScore().value_or(GetRatingScoreFromTheme().value_or(0.0));
    const double stepSize = ratingRenderProperty->GetStepSize().value_or(
        GetStepSizeFromTheme().value_or(OHOS::Ace::DEFAULT_RATING_STEP_SIZE));
    const int32_t starNum =
        ratingLayoutProperty->GetStars().value_or(GetStarNumFromTheme().value_or(OHOS::Ace::DEFAULT_RATING_STAR_NUM));
    const double drawScore = fmin(Round(ratingScore / stepSize) * stepSize, static_cast<double>(starNum));

    // do not fire onChange callback when rating is initialized for the first time.
    if (hasInit_ && lastRatingScore_ != drawScore) {
        FireChangeEvent();
    }
    if (!hasInit_) {
        hasInit_ = true;
    }
    ratingRenderProperty->UpdateRatingScore(drawScore);
    lastRatingScore_ = drawScore;
}

void RatingPattern::RecalculatedRatingScoreBasedOnEventPoint(double eventPointX, bool isDrag)
{
    auto ratingLayoutProperty = GetLayoutProperty<RatingLayoutProperty>();
    CHECK_NULL_VOID(ratingLayoutProperty);
    auto ratingRenderProperty = GetPaintProperty<RatingRenderProperty>();
    CHECK_NULL_VOID(ratingRenderProperty);

    // step1: calculate the number of star which the touch point falls on.
    double wholeStarNum = 0.0;
    const float singleWidth = foregroundConfig_.dstRect_.Width();
    wholeStarNum = floor(eventPointX / singleWidth);

    // step2: calculate relative position where the touch point falls on the wholeStarNum star.
    double posInSingle = 0.0;
    posInSingle = (eventPointX - wholeStarNum * singleWidth) / singleWidth;
    // step3: calculate the new ratingScore according to the touch point.
    double ratingScore = wholeStarNum + posInSingle;
    const int32_t starNum =
        ratingLayoutProperty->GetStars().value_or(GetStarNumFromTheme().value_or(OHOS::Ace::DEFAULT_RATING_STAR_NUM));
    const double stepSize = ratingRenderProperty->GetStepSize().value_or(GetStepSizeFromTheme().value_or(0.5));
    // step3.1: constrain ratingScore which cannot be greater than starNum and be less than 0.0.
    ratingScore = ratingScore > starNum ? starNum : ratingScore;
    ratingScore = (ratingScore < 0.0) ? 0.0 : ratingScore;
    const double newDrawScore = fmin(ceil(ratingScore / stepSize) * stepSize, starNum);
    // step3.2: Determine whether the old and new ratingScores are same or not.
    const double oldRatingScore = ratingRenderProperty->GetRatingScoreValue();
    const double oldDrawScore = fmin(Round(oldRatingScore / stepSize) * stepSize, static_cast<double>(starNum));

    CHECK_NULL_VOID_NOLOG(!NearEqual(newDrawScore, oldDrawScore));

    // step4: Update the ratingScore saved in renderProperty and update render.
    ratingRenderProperty->UpdateRatingScore(newDrawScore);
    if (isDrag) {
        ratingRenderProperty->UpdateTouchStar(static_cast<int32_t>(wholeStarNum));
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

bool RatingPattern::IsIndicator()
{
    auto ratingLayoutProperty = GetLayoutProperty<RatingLayoutProperty>();
    CHECK_NULL_RETURN(ratingLayoutProperty, true);
    // Do not handle event when rating is set as indicator or single star size is invalid.
    return ratingLayoutProperty->GetIndicator().value_or(false) || !foregroundConfig_.dstRect_.IsValid();
}

void RatingPattern::HandleDragUpdate(const GestureEvent& info)
{
    CHECK_NULL_VOID_NOLOG(!IsIndicator());
    RecalculatedRatingScoreBasedOnEventPoint(info.GetLocalLocation().GetX(), true);
}

void RatingPattern::FireChangeEvent() const
{
    auto ratingEventHub = GetEventHub<RatingEventHub>();
    CHECK_NULL_VOID(ratingEventHub);
    auto ratingRenderProperty = GetPaintProperty<RatingRenderProperty>();
    CHECK_NULL_VOID(ratingRenderProperty);
    std::stringstream ss;
    ss << std::setprecision(2) << ratingRenderProperty->GetRatingScoreValue();
    ratingEventHub->FireChangeEvent(ss.str());
}

void RatingPattern::HandleDragEnd()
{
    CHECK_NULL_VOID_NOLOG(!IsIndicator());
    FireChangeEvent();
}

void RatingPattern::InitPanEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID_NOLOG(!(IsIndicator() || panEvent_));

    auto actionStartTask = [weak = WeakClaim(this)](const GestureEvent& /*info*/) { LOGD("Pan event start"); };

    auto actionUpdateTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleDragUpdate(info);
    };

    auto actionEndTask = [weak = WeakClaim(this)](const GestureEvent& /*info*/) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        // invoke onChange callback
        pattern->HandleDragEnd();
    };

    auto actionCancelTask = [weak = WeakClaim(this)]() { LOGD("Pan event cancel"); };

    float distance = DEFAULT_PAN_DISTANCE;
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    distance = static_cast<float>(pipeline->NormalizeToPx(Dimension(DEFAULT_PAN_DISTANCE, DimensionUnit::VP)));
    PanDirection panDirection;
    panDirection.type = PanDirection::HORIZONTAL;

    panEvent_ = MakeRefPtr<PanEvent>(
        std::move(actionStartTask), std::move(actionUpdateTask), std::move(actionEndTask), std::move(actionCancelTask));
    gestureHub->AddPanEvent(panEvent_, panDirection, 1, distance);
}

void RatingPattern::InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID_NOLOG(!(IsIndicator() || touchEvent_));

    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        if (info.GetTouches().front().GetTouchType() == TouchType::DOWN) {
            auto localPosition = info.GetTouches().front().GetLocalLocation();
            // handle touch down event and draw touch down effect.
            pattern->HandleTouchDown(localPosition);
        }
        if (info.GetTouches().front().GetTouchType() == TouchType::UP) {
            // handle touch up event and remove touch down effect.
            pattern->HandleTouchUp();
        }
    };

    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gestureHub->AddTouchEvent(touchEvent_);
}

void RatingPattern::HandleTouchUp()
{
    CHECK_NULL_VOID_NOLOG(!IsIndicator());
    state_ = isHover_ ? RatingModifier::RatingAnimationType::PRESSTOHOVER : RatingModifier::RatingAnimationType::NONE;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RatingPattern::HandleTouchDown(const Offset& localPosition)
{
    CHECK_NULL_VOID_NOLOG(!IsIndicator());

    auto ratingRenderProperty = GetPaintProperty<RatingRenderProperty>();
    CHECK_NULL_VOID(ratingRenderProperty);
    // calculate the number of star the touch point falls on and trigger render update.
    const int32_t touchStar = floor(localPosition.GetX() / foregroundConfig_.dstRect_.Width());
    ratingRenderProperty->UpdateTouchStar(touchStar);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    state_ = isHover_ ? RatingModifier::RatingAnimationType::HOVERTOPRESS : RatingModifier::RatingAnimationType::PRESS;
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RatingPattern::HandleClick(const GestureEvent& info)
{
    CHECK_NULL_VOID_NOLOG(!IsIndicator());
    RecalculatedRatingScoreBasedOnEventPoint(info.GetLocalLocation().GetX(), false);
    FireChangeEvent();
}

void RatingPattern::InitClickEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID_NOLOG(!(IsIndicator() || clickEvent_));

    auto touchTask = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        pattern->HandleClick(info);
    };

    clickEvent_ = MakeRefPtr<ClickEvent>(std::move(touchTask));
    gestureHub->AddClickEvent(clickEvent_);
}

void RatingPattern::UpdateInternalResource(ImageSourceInfo& sourceInfo, int32_t imageFlag)
{
    CHECK_NULL_VOID_NOLOG(sourceInfo.IsInternalResource());
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto iconTheme = pipeline->GetTheme<IconTheme>();
    CHECK_NULL_VOID(iconTheme);
    auto iconPath = iconTheme->GetIconPath(sourceInfo.GetResourceId());
    if (!iconPath.empty()) {
        sourceInfo.SetSrc(iconPath, sourceInfo.GetFillColor());
        auto ratingLayoutProperty = GetLayoutProperty<RatingLayoutProperty>();
        CHECK_NULL_VOID(ratingLayoutProperty);
        switch (imageFlag) {
            case 0b001:
                ratingLayoutProperty->UpdateForegroundImageSourceInfo(sourceInfo);
                break;
            case 0b010:
                ratingLayoutProperty->UpdateSecondaryImageSourceInfo(sourceInfo);
                break;
            case 0b100:
                ratingLayoutProperty->UpdateBackgroundImageSourceInfo(sourceInfo);
                break;
            default:
                break;
        }
    }
}

void RatingPattern::GetInnerFocusPaintRect(RoundRect& paintRect)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto property = GetPaintProperty<RatingRenderProperty>();
    CHECK_NULL_VOID(property);
    auto ratingScore = property->GetRatingScoreValue();
    auto singleStarWith = foregroundConfig_.dstRect_.Width();
    auto wholeStarNum = fmax(ceil(ratingScore) - 1, 0.0);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto ratingTheme = pipeline->GetTheme<RatingTheme>();
    CHECK_NULL_VOID(ratingTheme);
    auto radius = ratingTheme->GetFocusBorderRadius();

    paintRect.SetRect(RectF(
        static_cast<float>(wholeStarNum) * singleStarWith, 0.0f, singleStarWith, foregroundConfig_.dstRect_.Height()));
    paintRect.SetCornerRadius(RoundRect::CornerPos::TOP_LEFT_POS, static_cast<RSScalar>(radius.ConvertToPx()),
        static_cast<RSScalar>(radius.ConvertToPx()));
    paintRect.SetCornerRadius(RoundRect::CornerPos::TOP_RIGHT_POS, static_cast<RSScalar>(radius.ConvertToPx()),
        static_cast<RSScalar>(radius.ConvertToPx()));
    paintRect.SetCornerRadius(RoundRect::CornerPos::BOTTOM_LEFT_POS, static_cast<RSScalar>(radius.ConvertToPx()),
        static_cast<RSScalar>(radius.ConvertToPx()));
    paintRect.SetCornerRadius(RoundRect::CornerPos::BOTTOM_RIGHT_POS, static_cast<RSScalar>(radius.ConvertToPx()),
        static_cast<RSScalar>(radius.ConvertToPx()));
}

void RatingPattern::PaintFocusState(double ratingScore)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    RoundRect focusRect;
    GetInnerFocusPaintRect(focusRect);

    auto focusHub = host->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->PaintInnerFocusState(focusRect);

    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

bool RatingPattern::OnKeyEvent(const KeyEvent& event)
{
    if (event.action != KeyAction::DOWN) {
        return false;
    }
    auto ratingRenderProperty = GetPaintProperty<RatingRenderProperty>();
    double ratingScore = ratingRenderProperty->GetRatingScoreValue();
    const double stepSize = ratingRenderProperty->GetStepSize().value_or(
        GetStepSizeFromTheme().value_or(OHOS::Ace::DEFAULT_RATING_STEP_SIZE));
    if (event.code == KeyCode::KEY_DPAD_LEFT) {
        ratingScore = fmax(ratingScore - stepSize, 0.0);
        ratingRenderProperty->UpdateRatingScore(ratingScore);
        PaintFocusState(ratingScore);
        return true;
    }
    if (event.code == KeyCode::KEY_DPAD_RIGHT) {
        ratingScore =
            fmin(ratingScore + stepSize, GetLayoutProperty<RatingLayoutProperty>()->GetStars().value_or(
                                             GetStarNumFromTheme().value_or(OHOS::Ace::DEFAULT_RATING_STAR_NUM)));
        ratingRenderProperty->UpdateRatingScore(ratingScore);
        PaintFocusState(ratingScore);
        return true;
    }
    return false;
}

void RatingPattern::InitOnKeyEvent(const RefPtr<FocusHub>& focusHub)
{
    auto onKeyEvent = [wp = WeakClaim(this)](const KeyEvent& event) -> bool {
        auto pattern = wp.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->OnKeyEvent(event);
    };
    focusHub->SetOnKeyEventInternal(std::move(onKeyEvent));

    auto getInnerPaintRectCallback = [wp = WeakClaim(this)](RoundRect& paintRect) {
        auto pattern = wp.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->GetInnerFocusPaintRect(paintRect);
    };
    focusHub->SetInnerFocusPaintRectCallback(getInnerPaintRectCallback);
}

void RatingPattern::InitMouseEvent()
{
    CHECK_NULL_VOID(!(mouseEvent_ && hoverEvent_));
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gesture = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);
    auto eventHub = GetHost()->GetEventHub<RatingEventHub>();
    auto inputHub = eventHub->GetOrCreateInputEventHub();

    auto hoverTask = [weak = WeakClaim(this)](bool isHover) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleHoverEvent(isHover);
    };

    auto mouseTask = [weak = WeakClaim(this)](MouseInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleMouseEvent(info);
    };
    mouseEvent_ = MakeRefPtr<InputEvent>(std::move(mouseTask));
    hoverEvent_ = MakeRefPtr<InputEvent>(std::move(hoverTask));
    inputHub->AddOnHoverEvent(hoverEvent_);
    inputHub->AddOnMouseEvent(mouseEvent_);
}

void RatingPattern::HandleHoverEvent(bool isHover)
{
    isHover_ = isHover;
    state_ = isHover_ ? RatingModifier::RatingAnimationType::HOVER : RatingModifier::RatingAnimationType::NONE;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RatingPattern::HandleMouseEvent(MouseInfo& info)
{
    CHECK_NULL_VOID_NOLOG(!IsIndicator() && isHover_);

    auto ratingRenderProperty = GetPaintProperty<RatingRenderProperty>();
    CHECK_NULL_VOID(ratingRenderProperty);
    auto ratingLayoutProperty = GetLayoutProperty<RatingLayoutProperty>();
    CHECK_NULL_VOID(ratingLayoutProperty);
    // calculate the number of star the mouse moved on and trigger render update.
    auto touchStar = static_cast<int32_t>(floor(info.GetLocalLocation().GetX() / foregroundConfig_.dstRect_.Width()));
    touchStar = std::clamp(touchStar, DEFAULT_RATING_TOUCH_STAR_NUMBER,
        ratingLayoutProperty->GetStars().value_or(GetStarNumFromTheme().value_or(OHOS::Ace::DEFAULT_RATING_STAR_NUM)) -
            1);
    ratingRenderProperty->UpdateTouchStar(touchStar);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RatingPattern::LoadForeground()
{
    auto ratingLayoutProperty = GetLayoutProperty<RatingLayoutProperty>();
    CHECK_NULL_VOID(ratingLayoutProperty);
    foregroundConfig_.isSvg_ = false;
    /*
     * tips : foregroundUri loaded the default star the first time, the ForegroundImageSourceInfo will not nullopt when
     * rating create again. such as the ratingScore partical update.
     * secondaryUri, backgroundUri is the same.
     */
    if (!ratingLayoutProperty->HasForegroundImageSourceInfo()) {
        isForegroundImageInfoFromTheme_ = true;
        ratingLayoutProperty->UpdateForegroundImageSourceInfo(GetImageSourceInfoFromTheme(0b001));
    }
    ImageSourceInfo foregroundImageSourceInfo =
        ratingLayoutProperty->GetForegroundImageSourceInfo().value_or(GetImageSourceInfoFromTheme(0b001));
    UpdateInternalResource(foregroundImageSourceInfo, 0b001);
    if (foregroundImageSourceInfo.IsSvg()) {
        foregroundConfig_.isSvg_ = true;
    }
    // Recreate ImageLoadingContext only when image source info has changed.
    if (!foregroundImageLoadingCtx_ || (foregroundImageLoadingCtx_->GetSourceInfo() != foregroundImageSourceInfo)) {
        // Construct the ImageLoadingContext and register the image life cycle callback.
        LoadNotifier loadNotifierForegroundImage(
            CreateDataReadyCallback(0b001), CreateLoadSuccessCallback(0b001), CreateLoadFailCallback(0b001));
        foregroundImageLoadingCtx_ =
            AceType::MakeRefPtr<ImageLoadingContext>(foregroundImageSourceInfo, std::move(loadNotifierForegroundImage));
        foregroundImageLoadingCtx_->LoadImageData();
    }
}

void RatingPattern::LoadSecondary()
{
    auto ratingLayoutProperty = GetLayoutProperty<RatingLayoutProperty>();
    CHECK_NULL_VOID(ratingLayoutProperty);
    secondaryConfig_.isSvg_ = false;
    if (!ratingLayoutProperty->HasSecondaryImageSourceInfo()) {
        isSecondaryImageInfoFromTheme_ = true;
        ratingLayoutProperty->UpdateSecondaryImageSourceInfo(GetImageSourceInfoFromTheme(0b010));
    }
    ImageSourceInfo secondaryImageSourceInfo =
        ratingLayoutProperty->GetSecondaryImageSourceInfo().value_or(GetImageSourceInfoFromTheme(0b010));
    UpdateInternalResource(secondaryImageSourceInfo, 0b010);
    if (secondaryImageSourceInfo.IsSvg()) {
        secondaryConfig_.isSvg_ = true;
    }
    if (!secondaryImageLoadingCtx_ || secondaryImageLoadingCtx_->GetSourceInfo() != secondaryImageSourceInfo) {
        LoadNotifier loadNotifierSecondaryImage(
            CreateDataReadyCallback(0b010), CreateLoadSuccessCallback(0b010), CreateLoadFailCallback(0b010));
        secondaryImageLoadingCtx_ =
            AceType::MakeRefPtr<ImageLoadingContext>(secondaryImageSourceInfo, std::move(loadNotifierSecondaryImage));
        secondaryImageLoadingCtx_->LoadImageData();
    }
}

void RatingPattern::LoadBackground()
{
    auto ratingLayoutProperty = GetLayoutProperty<RatingLayoutProperty>();
    CHECK_NULL_VOID(ratingLayoutProperty);
    backgroundConfig_.isSvg_ = false;
    if (!ratingLayoutProperty->HasBackgroundImageSourceInfo()) {
        isBackgroundImageInfoFromTheme_ = true;
        ratingLayoutProperty->UpdateBackgroundImageSourceInfo(GetImageSourceInfoFromTheme(0b100));
    }
    ImageSourceInfo backgroundImageSourceInfo =
        ratingLayoutProperty->GetBackgroundImageSourceInfo().value_or(GetImageSourceInfoFromTheme(0b100));
    UpdateInternalResource(backgroundImageSourceInfo, 0b100);
    if (backgroundImageSourceInfo.IsSvg()) {
        backgroundConfig_.isSvg_ = true;
    }
    if (!backgroundImageLoadingCtx_ || backgroundImageLoadingCtx_->GetSourceInfo() != backgroundImageSourceInfo) {
        LoadNotifier loadNotifierBackgroundImage(
            CreateDataReadyCallback(0b100), CreateLoadSuccessCallback(0b100), CreateLoadFailCallback(0b100));
        backgroundImageLoadingCtx_ =
            AceType::MakeRefPtr<ImageLoadingContext>(backgroundImageSourceInfo, std::move(loadNotifierBackgroundImage));
        backgroundImageLoadingCtx_->LoadImageData();
    }
}

void RatingPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    // Reset image state code.
    imageReadyStateCode_ = 0;
    imageSuccessStateCode_ = 0;
    // Constrains ratingScore and starNum in case of the illegal input.
    ConstrainsRatingScore();

    LoadForeground();
    LoadSecondary();
    LoadBackground();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    // Init touch, pan, click and key event and register callback.
    InitTouchEvent(gestureHub);
    InitPanEvent(gestureHub);
    InitClickEvent(gestureHub);
    InitMouseEvent();
    auto focusHub = host->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    InitOnKeyEvent(focusHub);
}

// XTS inspector code
void RatingPattern::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    auto ratingLayoutProperty = GetLayoutProperty<RatingLayoutProperty>();
    if (isForegroundImageInfoFromTheme_) {
        json->Put("foregroundImageSourceInfo", ImageSourceInfo("").ToString().c_str());
    } else {
        auto foregroundImageSourceInfo =
            ratingLayoutProperty->GetForegroundImageSourceInfo().value_or(ImageSourceInfo(""));
        json->Put("foregroundImageSourceInfo", foregroundImageSourceInfo.ToString().c_str());
    }
    if (isSecondaryImageInfoFromTheme_) {
        json->Put("secondaryImageSourceInfo", ImageSourceInfo("").ToString().c_str());
    } else {
        auto secondaryImageSourceInfo =
            ratingLayoutProperty->GetSecondaryImageSourceInfo().value_or(ImageSourceInfo(""));
        json->Put("secondaryImageSourceInfo", secondaryImageSourceInfo.ToString().c_str());
    }
    if (isBackgroundImageInfoFromTheme_) {
        json->Put("backgroundImageSourceInfo", ImageSourceInfo("").ToString().c_str());
    } else {
        auto backgroundImageSourceInfo =
            ratingLayoutProperty->GetBackgroundImageSourceInfo().value_or(ImageSourceInfo(""));
        json->Put("backgroundImageSourceInfo", backgroundImageSourceInfo.ToString().c_str());
    }
}
} // namespace OHOS::Ace::NG
