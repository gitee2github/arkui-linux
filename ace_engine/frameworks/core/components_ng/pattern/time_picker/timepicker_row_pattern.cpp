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

#include "core/components_ng/pattern/time_picker/timepicker_row_pattern.h"

#include <cstdint>

#include "base/geometry/ng/size_t.h"
#include "base/utils/utils.h"
#include "core/components/picker/picker_theme.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/time_picker/toss_animation_controller.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {
namespace {
// TODO timepicker style modification
constexpr int32_t CHILD_WITH_AMPM_SIZE = 3;
constexpr int32_t CHILD_WITHOUT_AMPM_SIZE = 2;
constexpr uint32_t AM_PM_HOUR_12 = 12;
constexpr uint32_t AM_PM_HOUR_11 = 11;
const int32_t AM_PM_COUNT = 3;
const Dimension PRESS_INTERVAL = 4.0_vp;
const Dimension PRESS_RADIUS = 8.0_vp;
} // namespace

void TimePickerRowPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
}

bool TimePickerRowPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    CHECK_NULL_RETURN_NOLOG(config.frameSizeChange, false);
    CHECK_NULL_RETURN(dirty, false);
    SetButtonIdeaSize();
    return true;
}

void TimePickerRowPattern::SetButtonIdeaSize()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);
    auto pickerTheme = context->GetTheme<PickerTheme>();
    CHECK_NULL_VOID(pickerTheme);
    auto children = host->GetChildren();
    auto height = pickerTheme->GetDividerSpacing();
    auto width = host->GetGeometryNode()->GetFrameSize().Width() / static_cast<float>(children.size());
    auto defaultWidth = height.ConvertToPx() * 2;
    if (width > defaultWidth) {
        width = static_cast<float>(defaultWidth);
    }
    for (const auto& child : children) {
        auto buttonNode = DynamicCast<FrameNode>(child->GetFirstChild());
        auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
        buttonLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);
        buttonLayoutProperty->UpdateType(ButtonType::NORMAL);
        buttonLayoutProperty->UpdateBorderRadius(PRESS_RADIUS);
        buttonLayoutProperty->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(width - PRESS_INTERVAL.ConvertToPx()), CalcLength(height - PRESS_INTERVAL)));
        auto buttonConfirmRenderContext = buttonNode->GetRenderContext();
        buttonConfirmRenderContext->UpdateBackgroundColor(Color::TRANSPARENT);
        buttonNode->MarkModifyDone();
        buttonNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }
}

void TimePickerRowPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    CreateAmPmNode();
    OnColumnsBuilding();
    FlushColumn();
    InitDisabled();
    SetChangeCallback([weak = WeakClaim(this)](const RefPtr<FrameNode>& tag, bool add, uint32_t index, bool notify) {
        auto refPtr = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(refPtr);
        refPtr->HandleColumnChange(tag, add, index, notify);
    });
    SetEventCallback([weak = WeakClaim(this)](bool refresh) {
        auto refPtr = weak.Upgrade();
        CHECK_NULL_VOID_NOLOG(refPtr);
        refPtr->FireChangeEvent(refresh);
    });
    auto focusHub = host->GetFocusHub();
    if (focusHub) {
        InitOnKeyEvent(focusHub);
    }
    if (HasTitleNode()) {
        auto textTitleNode = FrameNode::GetOrCreateFrameNode(
            V2::TEXT_ETS_TAG, GetTitleId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        auto str = GetDialogTitleDate();
        CHECK_NULL_VOID(textTitleNode);
        auto textLayoutProperty = textTitleNode->GetLayoutProperty<TextLayoutProperty>();
        CHECK_NULL_VOID(textLayoutProperty);
        textLayoutProperty->UpdateContent(str.ToString(false));
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void TimePickerRowPattern::InitDisabled()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto renderContext = host->GetRenderContext();
    enabled_ = eventHub->IsEnabled();
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void TimePickerRowPattern::CreateAmPmNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);
    auto pickerTheme = context->GetTheme<PickerTheme>();
    CHECK_NULL_VOID(pickerTheme);
    auto heigth = pickerTheme->GetDividerSpacing();
    if (!GetHour24() && !HasAmPmNode()) {
        auto amPmColumnNode = FrameNode::GetOrCreateFrameNode(
            V2::COLUMN_ETS_TAG, GetAmPmId(), []() { return AceType::MakeRefPtr<TimePickerColumnPattern>(); });
        CHECK_NULL_VOID(amPmColumnNode);
        for (uint32_t index = 0; index < AM_PM_COUNT; index++) {
            auto textNode = FrameNode::CreateFrameNode(
                V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
            CHECK_NULL_VOID(textNode);
            textNode->MountToParent(amPmColumnNode);
        }
        SetColumn(amPmColumnNode);
        auto stackAmPmNode = FrameNode::GetOrCreateFrameNode(V2::STACK_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<StackPattern>(); });
        auto buttonNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
        buttonNode->MountToParent(stackAmPmNode);
        auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
        amPmColumnNode->MountToParent(stackAmPmNode);
        auto layoutProperty = stackAmPmNode->GetLayoutProperty<LayoutProperty>();
        layoutProperty->UpdateAlignment(Alignment::CENTER);
        stackAmPmNode->MountToParent(host, 0);
        if (SetAmPmButtonIdeaSize() > 0) {
            auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
            buttonLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT_MAIN_AXIS);
            buttonLayoutProperty->UpdateType(ButtonType::NORMAL);
            buttonLayoutProperty->UpdateBorderRadius(PRESS_RADIUS);
            buttonLayoutProperty->UpdateUserDefinedIdealSize(
                CalcSize(CalcLength(SetAmPmButtonIdeaSize()), CalcLength(heigth - PRESS_INTERVAL)));
            auto buttonConfirmRenderContext = buttonNode->GetRenderContext();
            buttonConfirmRenderContext->UpdateBackgroundColor(Color::TRANSPARENT);
            buttonNode->MarkModifyDone();
            buttonNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        }
        host->MarkModifyDone();
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }
}

double TimePickerRowPattern::SetAmPmButtonIdeaSize()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0);
    auto children = host->GetChildren();
    float width = 0.0f;
    for (const auto& child : children) {
        auto buttonNode = DynamicCast<FrameNode>(child->GetFirstChild());
        CHECK_NULL_RETURN(buttonNode, 0);
        width = buttonNode->GetGeometryNode()->GetFrameSize().Width();
    }
    if (width > 0) {
        return width;
    }
    return 0;
}
void TimePickerRowPattern::SetEventCallback(EventCallback&& value)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto children = host->GetChildren();
    for (const auto& child : children) {
        auto stackNode = DynamicCast<FrameNode>(child);
        CHECK_NULL_VOID(stackNode);
        auto childNode = DynamicCast<FrameNode>(stackNode->GetLastChild());
        CHECK_NULL_VOID(childNode);
        auto timePickerColumnPattern = childNode->GetPattern<TimePickerColumnPattern>();
        CHECK_NULL_VOID(timePickerColumnPattern);
        timePickerColumnPattern->SetEventCallback(std::move(value));
    }
}

void TimePickerRowPattern::FireChangeEvent(bool refresh)
{
    if (refresh) {
        auto timePickerEventHub = GetEventHub<TimePickerEventHub>();
        CHECK_NULL_VOID(timePickerEventHub);
        auto str = GetSelectedObject(true);
        auto info = std::make_shared<DatePickerChangeEvent>(str);
        timePickerEventHub->FireChangeEvent(info.get());
        timePickerEventHub->FireDialogChangeEvent(str);
    }
}

std::string TimePickerRowPattern::GetSelectedObject(bool isColumnChange, int32_t status)
{
    auto time = selectedTime_;
    if (isColumnChange) {
        time = GetCurrentTime();
    }
    return time.ToString(true, hasSecond_, status);
}

PickerTime TimePickerRowPattern::GetCurrentTime()
{
    PickerTime time;
    auto allChildNode = GetAllChildNode();
    auto amPmColumn = allChildNode["amPm"];
    auto hourColumn = allChildNode["hour"];
    auto minuteColumn = allChildNode["minute"];
    CHECK_NULL_RETURN(hourColumn, time);
    CHECK_NULL_RETURN(minuteColumn, time);
    auto hourPickerColumnPattern = hourColumn->GetPattern<TimePickerColumnPattern>();
    CHECK_NULL_RETURN(hourPickerColumnPattern, time);
    auto minutePickerColumnPattern = minuteColumn->GetPattern<TimePickerColumnPattern>();
    CHECK_NULL_RETURN(minutePickerColumnPattern, time);

    if (hour24_) {
        time.SetHour(hourPickerColumnPattern->GetCurrentIndex()); // hour from 0 to 23, index from 0 to 23
    } else if (amPmColumn) {
        auto amPmPickerColumnPattern = amPmColumn->GetPattern<TimePickerColumnPattern>();
        CHECK_NULL_RETURN(amPmPickerColumnPattern, time);
        time.SetHour(GetHourFromAmPm(
            amPmPickerColumnPattern->GetCurrentIndex() == 0, hourPickerColumnPattern->GetCurrentIndex() + 1));
    } else {
        LOGE("AM PM column is null.");
    }

    time.SetMinute(minutePickerColumnPattern->GetCurrentIndex()); // minute from 0 to 59, index from 0 to 59
    return time;
}

uint32_t TimePickerRowPattern::GetHourFromAmPm(bool isAm, uint32_t amPmhour) const
{
    if (isAm) {
        if (amPmhour == AM_PM_HOUR_12) { // AM 12:00 means 00:00
            return 0;
        }
        return amPmhour;
    }
    if (amPmhour == AM_PM_HOUR_12) { // PM 12 means 12:00
        return AM_PM_HOUR_12;
    }
    return amPmhour + AM_PM_HOUR_12; // need add 12 hour to 24 hours style
}

void TimePickerRowPattern::HandleColumnChange(const RefPtr<FrameNode>& tag, bool isAdd, uint32_t index, bool needNotify)
{
    std::vector<RefPtr<FrameNode>> tags;
    OnDataLinking(tag, isAdd, index, tags);
    for (const auto& tag : tags) {
        auto iter = std::find_if(timePickerColumns_.begin(), timePickerColumns_.end(),
            [&tag](const RefPtr<FrameNode>& column) { return column->GetId() == tag->GetId(); });
        if (iter != timePickerColumns_.end()) {
            auto timePickerColumnPattern = (*iter)->GetPattern<TimePickerColumnPattern>();
            CHECK_NULL_VOID(timePickerColumnPattern);
            timePickerColumnPattern->FlushCurrentOptions();
        }
    }
}

void TimePickerRowPattern::SetChangeCallback(ColumnChangeCallback&& value)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto children = host->GetChildren();
    for (const auto& child : children) {
        auto stackNode = DynamicCast<FrameNode>(child);
        CHECK_NULL_VOID(stackNode);
        auto childNode = DynamicCast<FrameNode>(stackNode->GetLastChild());
        CHECK_NULL_VOID(childNode);
        auto timePickerColumnPattern = childNode->GetPattern<TimePickerColumnPattern>();
        CHECK_NULL_VOID(timePickerColumnPattern);
        timePickerColumnPattern->SetChangeCallback(std::move(value));
    }
}

void TimePickerRowPattern::FlushColumn()
{
    auto allChildNode = GetAllChildNode();
    auto amPmColumn = allChildNode["amPm"];
    auto hourColumn = allChildNode["hour"];
    if (GetHour24()) {
        CHECK_NULL_VOID(hourColumn);
        auto hourColumnPattern = hourColumn->GetPattern<TimePickerColumnPattern>();
        CHECK_NULL_VOID(hourColumnPattern);
        hourColumnPattern->SetOptions(GetOptions());
        hourColumnPattern->SetShowCount(GetShowCount());
        hourColumnPattern->FlushCurrentOptions();
    } else if (amPmColumn) {
        auto amPmColumnPattern = amPmColumn->GetPattern<TimePickerColumnPattern>();
        CHECK_NULL_VOID(amPmColumnPattern);
        amPmColumnPattern->SetShowCount(AM_PM_COUNT);
        amPmColumnPattern->FlushCurrentOptions();

        CHECK_NULL_VOID(hourColumn);
        auto hourColumnPattern = hourColumn->GetPattern<TimePickerColumnPattern>();
        CHECK_NULL_VOID(hourColumnPattern);
        hourColumnPattern->SetOptions(GetOptions());
        hourColumnPattern->SetShowCount(GetShowCount());
        hourColumnPattern->FlushCurrentOptions();
    }

    auto minuteColumn = allChildNode["minute"];
    CHECK_NULL_VOID(minuteColumn);
    auto minuteColumnPattern = minuteColumn->GetPattern<TimePickerColumnPattern>();
    CHECK_NULL_VOID(minuteColumnPattern);
    minuteColumnPattern->SetShowCount(GetShowCount());
    minuteColumnPattern->FlushCurrentOptions();
}

void TimePickerRowPattern::OnDataLinking(
    const RefPtr<FrameNode>& tag, bool isAdd, uint32_t index, std::vector<RefPtr<FrameNode>>& resultTags)
{
    CHECK_NULL_VOID(tag);
    auto hourNode = GetAllChildNode()["hour"];
    CHECK_NULL_VOID(hourNode);
    if (tag->GetId() != hourNode->GetId()) {
        return;
    }

    if (!hour24_) {
        HandleHour12Change(isAdd, index, resultTags);
    }
}

void TimePickerRowPattern::OnColumnsBuilding()
{
    HandleHourColumnBuilding();

    auto allChildNode = GetAllChildNode();
    auto minuteColumn = allChildNode["minute"];
    CHECK_NULL_VOID(minuteColumn);
    auto minuteColumnPattern = minuteColumn->GetPattern<TimePickerColumnPattern>();
    CHECK_NULL_VOID(minuteColumnPattern);
    options_[minuteColumn].clear();

    for (uint32_t minute = 0; minute <= 59; ++minute) { // time's minute from 0 to 59
        if (minute == selectedTime_.GetMinute()) {
            minuteColumnPattern->SetCurrentIndex(options_[minuteColumn].size());
        }
        auto minuteFormatStringValue = GetMinuteFormatString(minute);
        options_[minuteColumn].emplace_back(minuteFormatStringValue);
    }
    minuteColumnPattern->SetOptions(GetOptions());
}

void TimePickerRowPattern::HandleHourColumnBuilding()
{
    auto allChildNode = GetAllChildNode();
    auto amPmColumn = allChildNode["amPm"];
    auto hourColumn = allChildNode["hour"];
    options_[hourColumn].clear();
    if (GetHour24()) {
        CHECK_NULL_VOID(hourColumn);
        auto hourColumnPattern = hourColumn->GetPattern<TimePickerColumnPattern>();
        CHECK_NULL_VOID(hourColumnPattern);
        for (uint32_t hour = 0; hour <= 23; ++hour) { // time's hour from 0 to 23.
            if (hour == selectedTime_.GetHour()) {
                hourColumnPattern->SetCurrentIndex(options_[hourColumn].size());
            }
            auto hourFormatStringValue = GetHourFormatString(hour);
            options_[hourColumn].emplace_back(hourFormatStringValue);
        }
        hourColumnPattern->SetOptions(GetOptions());
        hourColumn->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    } else if (amPmColumn) {
        CHECK_NULL_VOID(amPmColumn);
        CHECK_NULL_VOID(hourColumn);
        auto amPmColumnPattern = amPmColumn->GetPattern<TimePickerColumnPattern>();
        CHECK_NULL_VOID(amPmColumnPattern);
        auto hourColumnPattern = hourColumn->GetPattern<TimePickerColumnPattern>();
        CHECK_NULL_VOID(hourColumnPattern);
        options_[amPmColumn].clear();
        options_[amPmColumn].emplace_back(GetAmFormatString());
        options_[amPmColumn].emplace_back(GetPmFormatString());

        if (IsAmHour(selectedTime_.GetHour())) {
            amPmColumnPattern->SetCurrentIndex(0); // AM's index
        } else {
            amPmColumnPattern->SetCurrentIndex(1); // PM's index
        }
        auto selectedHour = GetAmPmHour(selectedTime_.GetHour());
        for (uint32_t hour = 1; hour <= AM_PM_HOUR_12; ++hour) { // hour start from 1 to 12
            if (hour == selectedHour) {
                hourColumnPattern->SetCurrentIndex(options_[hourColumn].size());
            }
            auto hourFormatStringValue = GetHourFormatString(hour);
            options_[hourColumn].emplace_back(hourFormatStringValue);
        }
        amPmColumnPattern->SetOptions(GetOptions());
        hourColumnPattern->SetOptions(GetOptions());
    } else {
        LOGE("AM PM column is null.");
    }
}

std::unordered_map<std::string, RefPtr<FrameNode>> TimePickerRowPattern::GetAllChildNode()
{
    std::unordered_map<std::string, RefPtr<FrameNode>> allChildNode;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, allChildNode);
    if (GetHour24() && host->GetChildren().size() == CHILD_WITH_AMPM_SIZE) {
        host->RemoveChildAtIndex(0);
        amPmId_.reset();
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        host->MarkModifyDone();
    } else if (!GetHour24() && host->GetChildren().size() == CHILD_WITHOUT_AMPM_SIZE) {
        CreateAmPmNode();
    }
    if (GetHour24() && host->GetChildren().size() != CHILD_WITHOUT_AMPM_SIZE) {
        return allChildNode;
    }

    if (!GetHour24() && host->GetChildren().size() != CHILD_WITH_AMPM_SIZE) {
        return allChildNode;
    }
    auto children = host->GetChildren();
    auto iter = children.begin();
    CHECK_NULL_RETURN(*iter, allChildNode);
    if (!GetHour24()) {
        auto amPm = (*iter);
        CHECK_NULL_RETURN(amPm, allChildNode);
        iter++;
        auto hour = *iter;
        CHECK_NULL_RETURN(hour, allChildNode);
        iter++;
        auto minute = *iter;
        CHECK_NULL_RETURN(minute, allChildNode);
        auto amPmStackNode = DynamicCast<FrameNode>(amPm);
        auto amPmNode = DynamicCast<FrameNode>(amPmStackNode->GetLastChild());
        auto hourStackNode = DynamicCast<FrameNode>(hour);
        auto hourNode = DynamicCast<FrameNode>(hourStackNode->GetLastChild());
        auto minuteStackNode = DynamicCast<FrameNode>(minute);
        auto minuteNode = DynamicCast<FrameNode>(minuteStackNode->GetLastChild());
        CHECK_NULL_RETURN(amPmNode, allChildNode);
        CHECK_NULL_RETURN(hourNode, allChildNode);
        CHECK_NULL_RETURN(minuteNode, allChildNode);
        allChildNode["amPm"] = amPmNode;
        allChildNode["hour"] = hourNode;
        allChildNode["minute"] = minuteNode;
    } else {
        auto hour = *iter;
        CHECK_NULL_RETURN(hour, allChildNode);
        iter++;
        auto minute = *iter;
        CHECK_NULL_RETURN(minute, allChildNode);
        auto hourStackNode = DynamicCast<FrameNode>(hour);
        auto hourNode = DynamicCast<FrameNode>(hourStackNode->GetLastChild());
        auto minuteStackNode = DynamicCast<FrameNode>(minute);
        auto minuteNode = DynamicCast<FrameNode>(minuteStackNode->GetLastChild());
        CHECK_NULL_RETURN(hourNode, allChildNode);
        CHECK_NULL_RETURN(minuteNode, allChildNode);
        allChildNode["hour"] = hourNode;
        allChildNode["minute"] = minuteNode;
    }
    return allChildNode;
}

void TimePickerRowPattern::HandleHour12Change(bool isAdd, uint32_t index, std::vector<RefPtr<FrameNode>>& resultTags)
{
    auto allChildNode = GetAllChildNode();
    auto amPm = allChildNode["amPm"];
    CHECK_NULL_VOID(amPm);
    auto amPmPickerColumnPattern = amPm->GetPattern<TimePickerColumnPattern>();

    if (amPmPickerColumnPattern->GetCurrentIndex() == 0 && isAdd && index == 11) { // hour index start from 0 to 11
        amPmPickerColumnPattern->SetCurrentIndex(1);                               // add to PM's index
        resultTags.emplace_back(amPm);
        return;
    }
    if (amPmPickerColumnPattern->GetCurrentIndex() == 1 && !isAdd && index == 10) { // reduce to 11 hour (index is 10)
        amPmPickerColumnPattern->SetCurrentIndex(0);                                // change to AM whose index is 0
        resultTags.emplace_back(amPm);
        return;
    }
    if (amPmPickerColumnPattern->GetCurrentIndex() == 1 && isAdd && index == 11) {
        amPmPickerColumnPattern->SetCurrentIndex(0); // is PM (index is 1) and last hour (index is 11)
        resultTags.emplace_back(amPm);               // change to PM (index is 0)
        return;
    }
    if (amPmPickerColumnPattern->GetCurrentIndex() == 0 && !isAdd && index == 10) { // reduce to 11 hour(index is 10)
        amPmPickerColumnPattern->SetCurrentIndex(1);                                // change to PM
        resultTags.emplace_back(amPm);
        return;
    }
}

uint32_t TimePickerRowPattern::GetAmPmHour(uint32_t hourOf24) const
{
    if (hourOf24 == 0) {
        return AM_PM_HOUR_12; // AM 12:00 means 00:00 in 24 hour style
    }
    if (1 <= hourOf24 && hourOf24 <= AM_PM_HOUR_11) { // 00:00 to 11:00 is the same for any hour style
        return hourOf24;
    }
    if (hourOf24 == AM_PM_HOUR_12) { // 12:00 means PM start hour
        return AM_PM_HOUR_12;        // 12 PM
    }                                // hour from 13 to 23
    return hourOf24 - AM_PM_HOUR_12; // need reduce 12 to 12 hours style
}

bool TimePickerRowPattern::IsAmHour(uint32_t hourOf24) const
{
    return (0 <= hourOf24 && hourOf24 <= AM_PM_HOUR_11); // 00:00 to 11:00 is AM hour
}

std::string TimePickerRowPattern::GetAmFormatString() const
{
    if (vecAmPm_.empty()) {
        return "AM";
    }
    return vecAmPm_[0]; // first index is AM
}

std::string TimePickerRowPattern::GetPmFormatString() const
{
    if (vecAmPm_.size() < 2) { // size need to be 2 for AM and PM
        return "PM";
    }
    return vecAmPm_[1]; // second index is PM
}

std::string TimePickerRowPattern::GetHourFormatString(uint32_t hour) const
{
    DateTime time;
    time.minute = hour; // minute range [0, 59], hour range [0, 23]; hour range is in minute range.
    if (Localization::GetInstance()->HasZeroHour()) {
        return AddZeroPrefix(Localization::GetInstance()->FormatDateTime(time, "m"));
    }

    return Localization::GetInstance()->FormatDateTime(time, "m");
}

std::string TimePickerRowPattern::GetMinuteFormatString(uint32_t minute) const
{
    DateTime time;
    time.minute = minute;
    return AddZeroPrefix(Localization::GetInstance()->FormatDateTime(time, "m"));
}

std::string TimePickerRowPattern::AddZeroPrefix(const std::string& value) const
{
    if (value.size() == 1 && '0' <= value[0] && value[0] <= '9') { // value is number in range [0, 9]
        return std::string("0") + value;                           // add prefix '0'
    }
    return value;
}

void TimePickerRowPattern::InitOnKeyEvent(const RefPtr<FocusHub>& focusHub)
{
    auto onKeyEvent = [wp = WeakClaim(this)](const KeyEvent& event) -> bool {
        auto pattern = wp.Upgrade();
        if (pattern) {
            return pattern->OnKeyEvent(event);
        }
        return false;
    };
    focusHub->SetOnKeyEventInternal(std::move(onKeyEvent));

    auto getInnerPaintRectCallback = [wp = WeakClaim(this)](RoundRect& paintRect) {
        auto pattern = wp.Upgrade();
        if (pattern) {
            pattern->GetInnerFocusPaintRect(paintRect);
        }
    };
    focusHub->SetInnerFocusPaintRectCallback(getInnerPaintRectCallback);
}

void TimePickerRowPattern::PaintFocusState()
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

void TimePickerRowPattern::GetInnerFocusPaintRect(RoundRect& paintRect)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto childSize = host->GetChildren().size();
    auto stackChild = DynamicCast<FrameNode>(host->GetChildAtIndex(focusKeyID_));
    CHECK_NULL_VOID(stackChild);
    auto pickerChild = DynamicCast<FrameNode>(stackChild->GetLastChild());
    CHECK_NULL_VOID(pickerChild);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto pickerTheme = pipeline->GetTheme<PickerTheme>();
    CHECK_NULL_VOID(pickerTheme);
    auto frameWidth = host->GetGeometryNode()->GetFrameSize().Width();
    auto dividerSpacing = pipeline->NormalizeToPx(pickerTheme->GetDividerSpacing());
    auto pickerThemeWidth = dividerSpacing * 2;

    auto centerX = (frameWidth / childSize - pickerThemeWidth) / 2 +
                   pickerChild->GetGeometryNode()->GetFrameRect().Width() * focusKeyID_ +
                   PRESS_INTERVAL.ConvertToPx() * 2;
    auto centerY =
        (host->GetGeometryNode()->GetFrameSize().Height() - dividerSpacing) / 2 + PRESS_INTERVAL.ConvertToPx();

    paintRect.SetRect(RectF(centerX, centerY, (dividerSpacing - PRESS_INTERVAL.ConvertToPx()) * 2,
        dividerSpacing - PRESS_INTERVAL.ConvertToPx() * 2));

    paintRect.SetCornerRadius(RoundRect::CornerPos::TOP_LEFT_POS, static_cast<RSScalar>(PRESS_RADIUS.ConvertToPx()),
        static_cast<RSScalar>(PRESS_RADIUS.ConvertToPx()));
    paintRect.SetCornerRadius(RoundRect::CornerPos::TOP_RIGHT_POS, static_cast<RSScalar>(PRESS_RADIUS.ConvertToPx()),
        static_cast<RSScalar>(PRESS_RADIUS.ConvertToPx()));
    paintRect.SetCornerRadius(RoundRect::CornerPos::BOTTOM_LEFT_POS, static_cast<RSScalar>(PRESS_RADIUS.ConvertToPx()),
        static_cast<RSScalar>(PRESS_RADIUS.ConvertToPx()));
    paintRect.SetCornerRadius(RoundRect::CornerPos::BOTTOM_RIGHT_POS, static_cast<RSScalar>(PRESS_RADIUS.ConvertToPx()),
        static_cast<RSScalar>(PRESS_RADIUS.ConvertToPx()));
}

bool TimePickerRowPattern::OnKeyEvent(const KeyEvent& event)
{
    if (event.action != KeyAction::DOWN) {
        return false;
    }
    if (event.code == KeyCode::KEY_DPAD_UP || event.code == KeyCode::KEY_DPAD_DOWN ||
        event.code == KeyCode::KEY_DPAD_LEFT || event.code == KeyCode::KEY_DPAD_RIGHT) {
        HandleDirectionKey(event.code);
        return true;
    }
    return false;
}

bool TimePickerRowPattern::HandleDirectionKey(KeyCode code)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);

    auto stackChild = DynamicCast<FrameNode>(host->GetChildAtIndex(focusKeyID_));
    auto childSize = host->GetChildren().size();
    auto pickerChild = DynamicCast<FrameNode>(stackChild->GetLastChild());
    auto pattern = pickerChild->GetPattern<TimePickerColumnPattern>();
    auto currernIndex = pattern->GetCurrentIndex();
    auto totalOptionCount = GetOptionCount(pickerChild);
    if (totalOptionCount == 0) {
        return false;
    }
    if (code == KeyCode::KEY_DPAD_UP) {
        pattern->SetCurrentIndex((totalOptionCount + currernIndex - 1) % totalOptionCount);
        pattern->FlushCurrentOptions();
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        return true;
    }
    if (code == KeyCode::KEY_DPAD_DOWN) {
        pattern->SetCurrentIndex((totalOptionCount + currernIndex + 1) % totalOptionCount);
        pattern->FlushCurrentOptions();
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        return true;
    }
    if (code == KeyCode::KEY_DPAD_LEFT) {
        focusKeyID_ -= 1;
        if (focusKeyID_ < 0) {
            focusKeyID_ = 0;
        }
        PaintFocusState();
        return true;
    }
    if (code == KeyCode::KEY_DPAD_RIGHT) {
        focusKeyID_ += 1;
        if (focusKeyID_ > static_cast<int32_t>(childSize) - 1) {
            focusKeyID_ = static_cast<int32_t>(childSize) - 1;
        }
        PaintFocusState();
        return true;
    }
    return false;
}

} // namespace OHOS::Ace::NG
