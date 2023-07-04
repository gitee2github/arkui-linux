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

#include "core/components_ng/pattern/loading_progress/loading_progress_modifier.h"
#include <algorithm>

#include "base/geometry/dimension.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "bridge/common/dom/dom_type.h"
#include "core/components/common/properties/animation_option.h"
#include "core/components_ng/base/modifier.h"
#include "core/components_ng/pattern/loading_progress/loading_progress_utill.h"
#include "core/components_ng/pattern/refresh/refresh_animation_state.h"
#include "core/components_ng/render/animation_utils.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/components_ng/render/paint.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float TOTAL_ANGLE = 360.0f;
constexpr float ROTATEX = -116.0f;
constexpr float ROTATEY = 30.0f;
constexpr float ROTATEZ = 22.0f;
constexpr float COUNT = 50.0f;
constexpr float HALF = 0.5f;
constexpr float DOUBLE = 2.0f;
constexpr int32_t RING_ALPHA = 200;
constexpr int32_t TOTAL_POINTS_COUNT = 20;
constexpr int32_t TAIL_ANIAMTION_DURATION = 400;
constexpr int32_t TRANS_DURATION = 100;
constexpr float TOTAL_TAIL_LENGTH = 60.0f;
constexpr float TAIL_ALPHA_RATIO = 0.82f;
constexpr float INITIAL_SIZE_SCALE = 0.825f;
constexpr float INITIAL_OPACITY_SCALE = 0.7f;
constexpr float COMET_TAIL_ANGLE = 3.0f;
constexpr int32_t LOADING_DURATION = 1200;
constexpr float FOLLOW_START = 72.0f;
constexpr float FOLLOW_SPAN = 10.0f;
constexpr float FULL_COUNT = 100.0f;
constexpr float STAGE1 = 0.25f;
constexpr float STAGE2 = 0.65f;
constexpr float STAGE3 = 0.75f;
constexpr float STAGE4 = 0.85f;
constexpr float STAGE5 = 1.0f;
constexpr float OPACITY1 = 0.2f;
constexpr float OPACITY2 = 0.7f;
constexpr float OPACITY3 = 1.0f;
constexpr float SIZE_SCALE1 = 0.65f;
constexpr float SIZE_SCALE2 = 0.825f;
constexpr float SIZE_SCALE3 = 0.93f;
constexpr float MOVE_STEP = 0.06f;
constexpr float TRANS_OPACITY_SPAN = 0.3f;
constexpr float FULL_OPACITY = 255.0f;
constexpr float TWO = 2.0f;
} // namespace
LoadingProgressModifier::LoadingProgressModifier(LoadingProgressOwner loadingProgressOwner)
    : date_(AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f)),
      color_(AceType::MakeRefPtr<AnimatablePropertyColor>(LinearColor::BLUE)),
      centerDeviation_(AceType::MakeRefPtr<AnimatablePropertyFloat>(0.0f)),
      cometOpacity_(AceType::MakeRefPtr<AnimatablePropertyFloat>(INITIAL_OPACITY_SCALE)),
      cometSizeScale_(AceType::MakeRefPtr<AnimatablePropertyFloat>(INITIAL_SIZE_SCALE)),
      cometTailLen_(AceType::MakeRefPtr<AnimatablePropertyFloat>(TOTAL_TAIL_LENGTH)),
      sizeScale_(AceType::MakeRefPtr<AnimatablePropertyFloat>(1.0f)),
      loadingProgressOwner_(loadingProgressOwner)
{
    AttachProperty(date_);
    AttachProperty(color_);
    AttachProperty(centerDeviation_);
    AttachProperty(cometOpacity_);
    AttachProperty(cometSizeScale_);
    AttachProperty(cometTailLen_);
    AttachProperty(sizeScale_);
};

void LoadingProgressModifier::onDraw(DrawingContext& context)
{
    float date = date_->Get();
    auto diameter = std::min(context.width, context.height);
    RingParam ringParam;
    ringParam.strokeWidth = LoadingProgressUtill::GetRingStrokeWidth(diameter) * sizeScale_->Get();
    ringParam.radius = LoadingProgressUtill::GetRingRadius(diameter) * sizeScale_->Get();
    ringParam.movement =
        (ringParam.radius * DOUBLE + ringParam.strokeWidth) * centerDeviation_->Get() * sizeScale_->Get();

    CometParam cometParam;
    cometParam.radius = LoadingProgressUtill::GetCometRadius(diameter) * sizeScale_->Get();
    cometParam.alphaScale = cometOpacity_->Get();
    cometParam.sizeScale = cometSizeScale_->Get();
    cometParam.pointCount = GetCometNumber();

    auto orbitRadius = LoadingProgressUtill::GetOrbitRadius(diameter) * sizeScale_->Get();
    if (date > COUNT) {
        DrawRing(context, ringParam);
        DrawOrbit(context, cometParam, orbitRadius, date);
    } else {
        DrawOrbit(context, cometParam, orbitRadius, date);
        DrawRing(context, ringParam);
    }
}

void LoadingProgressModifier::DrawRing(DrawingContext& context, const RingParam& ringParam)
{
    auto& canvas = context.canvas;
    canvas.Save();
    RSPen pen;
    auto ringColor = color_->Get();
    pen.SetColor(ToRSColor(Color::FromARGB(RING_ALPHA, ringColor.GetRed(), ringColor.GetGreen(), ringColor.GetBlue())));
    pen.SetWidth(ringParam.strokeWidth);
    pen.SetAntiAlias(true);
    canvas.AttachPen(pen);
    canvas.DrawCircle({ context.width * HALF, context.height * HALF + ringParam.movement }, ringParam.radius);
    canvas.DetachPen();
    canvas.Restore();
}

void LoadingProgressModifier::DrawOrbit(
    DrawingContext& context, const CometParam& cometParam, float orbitRadius, float date)
{
    auto pointCounts = cometParam.pointCount;
    auto& canvas = context.canvas;
    float width_ = context.width;
    float height_ = context.height;
    double angle = TOTAL_ANGLE * date / FULL_COUNT;
    auto* camera_ = new RSCamera3D();
    camera_->Save();
    camera_->RotateYDegrees(ROTATEY);
    camera_->RotateXDegrees(ROTATEX);
    camera_->RotateZDegrees(ROTATEZ);
    RSMatrix matrix;
    camera_->ApplyToMatrix(matrix);
    camera_->Restore();
    auto center = RSPoint(width_ / 2, height_ / 2);
    RSBrush brush;
    brush.SetAntiAlias(true);
    canvas.Save();
    canvas.Translate(center.GetX(), center.GetY());
    std::vector<RSPoint> points;
    for (uint32_t i = 0; i < pointCounts; i++) {
        RSPoint point;
        float cometAngal = GetCurentCometAngle(angle, pointCounts - i, pointCounts);
        float rad = cometAngal * PI_NUM / (TOTAL_ANGLE * HALF);
        point.SetX(std::cos(rad) * orbitRadius);
        point.SetY(-std::sin(rad) * orbitRadius);
        points.push_back(point);
    }
    std::vector<RSPoint> distPoints(points.size());
    matrix.MapPoints(distPoints, points, points.size());
    auto cometColor = color_->Get();
    float colorAlpha = cometColor.GetAlpha() / FULL_OPACITY;
    auto baseAlpha = colorAlpha * cometParam.alphaScale;
    for (uint32_t i = 0; i < distPoints.size(); i++) {
        RSPoint pointCenter = distPoints[i];
        float setAlpha = GetCurentCometOpacity(baseAlpha, distPoints.size() - i, distPoints.size());
        if (NearZero(setAlpha)) {
            continue;
        }
        brush.SetColor(
            ToRSColor(Color::FromRGBO(cometColor.GetRed(), cometColor.GetGreen(), cometColor.GetBlue(), setAlpha)));
        canvas.AttachBrush(brush);
        canvas.DrawCircle(pointCenter, cometParam.radius * cometParam.sizeScale);
    }
    canvas.DetachBrush();
    canvas.Restore();
}

void LoadingProgressModifier::StartRecycleRingAnimation()
{
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto previousStageCurve = AceType::MakeRefPtr<CubicCurve>(0.0f, 0.0f, 0.67f, 1.0f);
    AnimationOption option;
    option.SetDuration(LOADING_DURATION);
    option.SetCurve(previousStageCurve);
    if (context->IsFormRender()) {
        LOGI("LoadingProgress is restricted at runtime when form render");
        option.SetIteration(1);
    } else {
        option.SetIteration(-1);
    }
    AnimationUtils::OpenImplicitAnimation(option, previousStageCurve, nullptr);
    auto middleStageCurve = AceType::MakeRefPtr<CubicCurve>(0.33f, 0.0f, 0.67f, 1.0f);
    AnimationUtils::AddKeyFrame(STAGE1, middleStageCurve, [&]() { centerDeviation_->Set(-1 * MOVE_STEP); });
    auto latterStageCurve = AceType::MakeRefPtr<CubicCurve>(0.33f, 0.0f, 1.0f, 1.0f);
    AnimationUtils::AddKeyFrame(STAGE3, latterStageCurve, [&]() { centerDeviation_->Set(MOVE_STEP); });
    AnimationUtils::AddKeyFrame(STAGE5, latterStageCurve, [&]() { centerDeviation_->Set(0.0f); });
    AnimationUtils::CloseImplicitAnimation();
}

void LoadingProgressModifier::StartRecycleCometAnimation()
{
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto curve = AceType::MakeRefPtr<LinearCurve>();
    AnimationOption option;
    option.SetDuration(LOADING_DURATION);
    option.SetCurve(curve);
    if (context->IsFormRender()) {
        LOGI("LoadingProgress is restricted at runtime when form render");
        option.SetIteration(1);
    } else {
        option.SetIteration(-1);
    }

    cometOpacity_->Set(OPACITY2);
    AnimationUtils::OpenImplicitAnimation(option, curve, nullptr);
    AnimationUtils::AddKeyFrame(STAGE1, curve, [&]() {
        cometOpacity_->Set(OPACITY1);
        cometSizeScale_->Set(SIZE_SCALE1);
    });
    AnimationUtils::AddKeyFrame(STAGE2, curve, [&]() {
        cometOpacity_->Set(OPACITY3);
        cometSizeScale_->Set(SIZE_SCALE3);
    });
    AnimationUtils::AddKeyFrame(STAGE3, curve, [&]() {
        cometOpacity_->Set(OPACITY3);
        cometSizeScale_->Set(1.0f);
    });
    AnimationUtils::AddKeyFrame(STAGE4, curve, [&]() {
        cometOpacity_->Set(OPACITY3);
        cometSizeScale_->Set(SIZE_SCALE3);
    });
    AnimationUtils::AddKeyFrame(STAGE5, curve, [&]() {
        cometOpacity_->Set(OPACITY2);
        cometSizeScale_->Set(SIZE_SCALE2);
    });
    AnimationUtils::CloseImplicitAnimation();
}

void LoadingProgressModifier::StartCometTailAnimation()
{
    auto curve = AceType::MakeRefPtr<LinearCurve>();
    AnimationOption option;
    option.SetDuration(TAIL_ANIAMTION_DURATION);
    option.SetIteration(1);
    option.SetCurve(curve);
    AnimationUtils::Animate(option, [&]() { cometTailLen_->Set(TOTAL_TAIL_LENGTH); });
}

float LoadingProgressModifier::GetCurentCometOpacity(float baseOpacity, uint32_t index, uint32_t totalNumber)
{
    return baseOpacity * std::pow(TAIL_ALPHA_RATIO, std::clamp(index, 1u, totalNumber) - 1);
}

float LoadingProgressModifier::GetCurentCometAngle(float baseAngle, uint32_t index, uint32_t totalNumber)
{
    return std::fmod((baseAngle - (std::clamp(index, 1u, totalNumber) - 1) * COMET_TAIL_ANGLE), TOTAL_ANGLE);
}

uint32_t LoadingProgressModifier::GetCometNumber()
{
    CHECK_NULL_RETURN(cometTailLen_, TOTAL_POINTS_COUNT);
    return static_cast<uint32_t>(cometTailLen_->Get() / COMET_TAIL_ANGLE);
}

void LoadingProgressModifier::StartRecycle()
{
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    if (isLoading_) {
        return;
    }
    sizeScale_->Set(1.0f);
    if (date_) {
        isLoading_ = true;
        date_->Set(0.0f);
        AnimationOption option = AnimationOption();
        RefPtr<Curve> curve = AceType::MakeRefPtr<LinearCurve>();
        option.SetDuration(LOADING_DURATION);
        option.SetDelay(0);
        option.SetCurve(curve);
        if (context->IsFormRender()) {
            LOGI("LoadingProgress is restricted at runtime when form render");
            option.SetIteration(1);
        } else {
            option.SetIteration(-1);
        }

        AnimationUtils::Animate(option, [&]() { date_->Set(FULL_COUNT); });
    }
    cometOpacity_->Set(INITIAL_OPACITY_SCALE);
    cometSizeScale_->Set(INITIAL_SIZE_SCALE);
    StartRecycleRingAnimation();
    StartRecycleCometAnimation();
}

void LoadingProgressModifier::StartTransToRecycleAnimation()
{
    auto curve = AceType::MakeRefPtr<CubicCurve>(0.6f, 0.2f, 1.0f, 1.0f);
    AnimationOption option;
    option.SetDuration(TRANS_DURATION);
    option.SetIteration(1);
    option.SetCurve(curve);
    AnimationUtils::Animate(
        option,
        [&]() {
            date_->Set(FULL_COUNT);
            cometOpacity_->Set(1.0 - TRANS_OPACITY_SPAN);
            cometSizeScale_->Set(INITIAL_SIZE_SCALE);
        },
        [&]() { StartRecycle(); });
    StartCometTailAnimation();
}

void LoadingProgressModifier::ChangeRefreshFollowData(float refreshFollowRatio)
{
    auto ratio = CorrectNormalize(refreshFollowRatio);
    sizeScale_->Set(std::sqrt(TWO) * HALF + (1.0 - std::sqrt(TWO) * HALF) * ratio);
    if (isLoading_) {
        return;
    }
    CHECK_NULL_VOID(date_);
    date_->Set(FOLLOW_START + FOLLOW_SPAN * ratio);
    cometTailLen_->Set(COMET_TAIL_ANGLE);
    cometOpacity_->Set(1.0f);
    cometSizeScale_->Set(1.0f);
}

float LoadingProgressModifier::CorrectNormalize(float originData)
{
    auto ratio = originData;
    if (ratio < 0.0f) {
        ratio = 0.0f;
    }
    if (ratio > 1.0f) {
        ratio = 1.0f;
    };
    return ratio;
}
} // namespace OHOS::Ace::NG
