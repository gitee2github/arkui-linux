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

#include "core/components_ng/pattern/select_overlay/select_overlay_pattern.h"

#include <algorithm>

#include "base/geometry/dimension.h"
#include "base/geometry/dimension_rect.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/point_t.h"
#include "base/geometry/offset.h"
#include "base/utils/utils.h"
#include "core/components/menu/menu_component.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_node.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_property.h"
#include "core/components_ng/property/property.h"
#include "core/gestures/gesture_info.h"
#include "core/pipeline/base/constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
void SelectOverlayPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
    host->GetLayoutProperty()->UpdateAlignment(Alignment::TOP_LEFT);

    UpdateHandleHotZone();
    auto gesture = host->GetOrCreateGestureEventHub();
    gesture->SetHitTestMode(info_->hitTestMode);

    clickEvent_ = MakeRefPtr<ClickEvent>([weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleOnClick(info);
    });
    if (info_->isSingleHandle) {
        gesture->AddClickEvent(clickEvent_);
    }
    auto panStart = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandlePanStart(info);
    };
    auto panUpdate = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandlePanMove(info);
    };
    auto panEnd = [weak = WeakClaim(this)](GestureEvent& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandlePanEnd(info);
    };
    auto panCancel = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandlePanCancel();
    };
    panEvent_ =
        MakeRefPtr<PanEvent>(std::move(panStart), std::move(panUpdate), std::move(panEnd), std::move(panCancel));
    gesture->SetPanEvent(panEvent_, { PanDirection::ALL }, 1, DEFAULT_PAN_DISTANCE);

    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleTouchEvent(info);
        }
    };
    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gesture->AddTouchEvent(touchEvent_);
}

void SelectOverlayPattern::OnDetachFromFrameNode(FrameNode* /*frameNode*/)
{
    if (info_->onClose) {
        info_->onClose();
    }
}

void SelectOverlayPattern::UpdateHandleHotZone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<TextOverlayTheme>();
    CHECK_NULL_VOID(theme);
    auto hotZone = theme->GetHandleHotZoneRadius().ConvertToPx();
    firstHandleRegion_.SetSize({ hotZone * 2, hotZone * 2 });
    auto firstHandleOffsetX = (info_->firstHandle.paintRect.Left() + info_->firstHandle.paintRect.Right()) / 2;
    secondHandleRegion_.SetSize({ hotZone * 2, hotZone * 2 });
    auto secondHandleOffsetX = (info_->secondHandle.paintRect.Left() + info_->secondHandle.paintRect.Right()) / 2;
    std::vector<DimensionRect> responseRegion;
    if (info_->isSingleHandle) {
        if (!info_->firstHandle.isShow && info_->secondHandle.isShow) {
            // Use the second handle to make a single handle.
            auto secondHandleOffsetY = info_->secondHandle.paintRect.Bottom();
            secondHandleRegion_.SetOffset({ secondHandleOffsetX - hotZone, secondHandleOffsetY });
            DimensionRect secondHandleRegion;
            secondHandleRegion.SetSize({ Dimension(secondHandleRegion_.GetSize().Width()),
                Dimension(secondHandleRegion_.GetSize().Height()) });
            secondHandleRegion.SetOffset(DimensionOffset(
                Offset(secondHandleRegion_.GetOffset().GetX(), secondHandleRegion_.GetOffset().GetY())));
            responseRegion.emplace_back(secondHandleRegion);
            host->GetOrCreateGestureEventHub()->SetResponseRegion(responseRegion);
        } else {
            // Use the first handle to make a single handle.
            auto firstHandleOffsetY = info_->firstHandle.paintRect.Bottom();
            firstHandleRegion_.SetOffset({ firstHandleOffsetX - hotZone, firstHandleOffsetY });
            DimensionRect firstHandleRegion;
            firstHandleRegion.SetSize(
                { Dimension(firstHandleRegion_.GetSize().Width()), Dimension(firstHandleRegion_.GetSize().Height()) });
            firstHandleRegion.SetOffset(
                DimensionOffset(Offset(firstHandleRegion_.GetOffset().GetX(), firstHandleRegion_.GetOffset().GetY())));
            responseRegion.emplace_back(firstHandleRegion);
            host->GetOrCreateGestureEventHub()->SetResponseRegion(responseRegion);
        }
        return;
    }
    if (info_->handleReverse) {
        auto firstHandleOffsetY = info_->firstHandle.paintRect.Bottom();
        firstHandleRegion_.SetOffset({ firstHandleOffsetX - hotZone, firstHandleOffsetY });
        auto secondHandleOffsetY = info_->secondHandle.paintRect.Top();
        secondHandleRegion_.SetOffset({ secondHandleOffsetX - hotZone, secondHandleOffsetY - hotZone * 2 });
    } else {
        auto firstHandleOffsetY = info_->firstHandle.paintRect.Top();
        firstHandleRegion_.SetOffset({ firstHandleOffsetX - hotZone, firstHandleOffsetY - hotZone * 2 });
        auto secondHandleOffsetY = info_->secondHandle.paintRect.Bottom();
        secondHandleRegion_.SetOffset({ secondHandleOffsetX - hotZone, secondHandleOffsetY });
    }
    DimensionRect firstHandleRegion;
    firstHandleRegion.SetSize(
        { Dimension(firstHandleRegion_.GetSize().Width()), Dimension(firstHandleRegion_.GetSize().Height()) });
    firstHandleRegion.SetOffset(
        DimensionOffset(Offset(firstHandleRegion_.GetOffset().GetX(), firstHandleRegion_.GetOffset().GetY())));
    responseRegion.emplace_back(firstHandleRegion);
    DimensionRect secondHandleRegion;
    secondHandleRegion.SetSize(
        { Dimension(secondHandleRegion_.GetSize().Width()), Dimension(secondHandleRegion_.GetSize().Height()) });
    secondHandleRegion.SetOffset(
        DimensionOffset(Offset(secondHandleRegion_.GetOffset().GetX(), secondHandleRegion_.GetOffset().GetY())));
    responseRegion.emplace_back(secondHandleRegion);
    host->GetOrCreateGestureEventHub()->SetResponseRegion(responseRegion);
}

void SelectOverlayPattern::HandleOnClick(GestureEvent& /*info*/)
{
    if (!info_->isSingleHandle) {
        return;
    }
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    if (!info_->menuInfo.menuDisable) {
        if (!info_->menuInfo.menuIsShow) {
            info_->menuInfo.menuIsShow = true;
            host->UpdateToolBar(false);
        }
    }
}

void SelectOverlayPattern::HandleTouchEvent(const TouchEventInfo& info)
{
    const auto& changedPoint = info.GetChangedTouches().front();
    if (info_->onTouchDown && changedPoint.GetTouchType() == TouchType::DOWN) {
        info_->onTouchDown(info);
    } else if (info_->onTouchDown && changedPoint.GetTouchType() == TouchType::UP) {
        info_->onTouchUp(info);
    } else if (info_->onTouchMove && changedPoint.GetTouchType() == TouchType::MOVE) {
        info_->onTouchMove(info);
    }
}

void SelectOverlayPattern::HandlePanStart(GestureEvent& info)
{
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    orignMenuIsShow_ = info_->menuInfo.menuIsShow;
    if (info_->menuInfo.menuIsShow) {
        info_->menuInfo.menuIsShow = false;
        host->UpdateToolBar(false);
    }
    PointF point = { info.GetLocalLocation().GetX(), info.GetLocalLocation().GetY() };
    if (firstHandleRegion_.IsInRegion(point)) {
        firstHandleDrag_ = true;
        secondHandleDrag_ = false;
        if (info_->onHandleMoveStart) {
            info_->onHandleMoveStart(firstHandleDrag_);
        }
    } else if (secondHandleRegion_.IsInRegion(point)) {
        firstHandleDrag_ = false;
        secondHandleDrag_ = true;
        if (info_->onHandleMoveStart) {
            info_->onHandleMoveStart(firstHandleDrag_);
        }
    } else {
        LOGW("the point is not in drag area");
    }
}

void SelectOverlayPattern::HandlePanMove(GestureEvent& info)
{
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    const auto& offset = OffsetF(info.GetDelta().GetX(), info.GetDelta().GetY());
    if (firstHandleDrag_) {
        firstHandleRegion_ += offset;
        info_->firstHandle.paintRect += offset;
        CheckHandleReverse();
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        if (info_->onHandleMove) {
            info_->onHandleMove(info_->firstHandle.paintRect, true);
        }
    } else if (secondHandleDrag_) {
        secondHandleRegion_ += offset;
        info_->secondHandle.paintRect += offset;
        CheckHandleReverse();
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        if (info_->onHandleMove) {
            info_->onHandleMove(info_->secondHandle.paintRect, false);
        }
    } else {
        LOGW("the move point is not in drag area");
    }
}

void SelectOverlayPattern::HandlePanEnd(GestureEvent& /*info*/)
{
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    if (!info_->menuInfo.menuIsShow) {
        info_->menuInfo.menuIsShow = orignMenuIsShow_;
        host->UpdateToolBar(false);
    }
    if (firstHandleDrag_) {
        if (info_->onHandleMoveDone) {
            info_->onHandleMoveDone(info_->firstHandle.paintRect, true);
        }
        firstHandleDrag_ = false;
        return;
    }
    if (secondHandleDrag_) {
        if (info_->onHandleMoveDone) {
            info_->onHandleMoveDone(info_->secondHandle.paintRect, false);
        }
        secondHandleDrag_ = false;
    }
}

void SelectOverlayPattern::HandlePanCancel()
{
    GestureEvent info;
    HandlePanEnd(info);
}

void SelectOverlayPattern::CheckHandleReverse()
{
    bool handleReverseChanged = false;
    double epsilon = std::max(info_->firstHandle.paintRect.Height(), info_->secondHandle.paintRect.Height());
    epsilon = std::max(static_cast<double>(info_->singleLineHeight), epsilon);
    if (NearEqual(info_->firstHandle.paintRect.Top(), info_->secondHandle.paintRect.Top(), epsilon)) {
        if (info_->firstHandle.paintRect.Left() > info_->secondHandle.paintRect.Left()) {
            if (!info_->handleReverse) {
                info_->handleReverse = true;
                handleReverseChanged = true;
            }
        } else {
            if (info_->handleReverse) {
                info_->handleReverse = false;
                handleReverseChanged = true;
            }
        }
    } else if (GreatNotEqual(info_->firstHandle.paintRect.Top(), info_->secondHandle.paintRect.Top())) {
        if (!info_->handleReverse) {
            info_->handleReverse = true;
            handleReverseChanged = true;
        }
    } else {
        if (info_->handleReverse) {
            info_->handleReverse = false;
            handleReverseChanged = true;
        }
    }
    if (handleReverseChanged && info_->onHandleReverse) {
        info_->onHandleReverse(info_->handleReverse);
    }
}

void SelectOverlayPattern::UpdateFirstSelectHandleInfo(const SelectHandleInfo& info)
{
    if (info_->firstHandle == info) {
        return;
    }
    info_->firstHandle = info;
    CheckHandleReverse();
    UpdateHandleHotZone();
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SelectOverlayPattern::UpdateSecondSelectHandleInfo(const SelectHandleInfo& info)
{
    if (info_->secondHandle == info) {
        return;
    }
    info_->secondHandle = info;
    CheckHandleReverse();
    UpdateHandleHotZone();
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void SelectOverlayPattern::UpdateFirstAndSecondHandleInfo(
    const SelectHandleInfo& firstInfo, const SelectHandleInfo& secondInfo)
{
    if (info_->firstHandle == firstInfo && info_->secondHandle == secondInfo) {
        return;
    }
    if (info_->firstHandle != firstInfo) {
        info_->firstHandle = firstInfo;
    }
    if (info_->secondHandle != secondInfo) {
        info_->secondHandle = secondInfo;
    }
    CheckHandleReverse();
    UpdateHandleHotZone();
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    host->UpdateToolBar(false);
}

void SelectOverlayPattern::UpdateSelectMenuInfo(const SelectMenuInfo& info)
{
    auto host = DynamicCast<SelectOverlayNode>(GetHost());
    CHECK_NULL_VOID(host);
    auto itemChanged = info_->menuInfo.IsIconChanged(info);
    info_->menuInfo = info;
    host->UpdateToolBar(itemChanged);
}

void SelectOverlayPattern::UpdateShowArea(const RectF& area)
{
    if (info_->showArea != area) {
        info_->showArea = area;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_LAYOUT);
}

bool SelectOverlayPattern::OnDirtyLayoutWrapperSwap(
    const RefPtr<LayoutWrapper>& /*dirty*/, const DirtySwapConfig& /*config*/)
{
    return true;
}

} // namespace OHOS::Ace::NG