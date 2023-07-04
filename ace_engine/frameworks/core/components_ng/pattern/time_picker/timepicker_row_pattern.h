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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TIME_PICKER_TIME_PICKER_ROW_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TIME_PICKER_TIME_PICKER_ROW_PATTERN_H

#include <optional>

#include "base/i18n/localization.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/time_picker/timepicker_column_pattern.h"
#include "core/components_ng/pattern/time_picker/timepicker_event_hub.h"
#include "core/components_ng/pattern/time_picker/timepicker_paint_method.h"
#include "core/components_v2/inspector/utils.h"

namespace OHOS::Ace::NG {
namespace {
const Dimension TIME_FOCUS_PAINT_WIDTH = 2.0_vp;
}

class TimePickerRowPattern : public LinearLayoutPattern {
    DECLARE_ACE_TYPE(TimePickerRowPattern, LinearLayoutPattern);

public:
    TimePickerRowPattern() : LinearLayoutPattern(false) {};

    ~TimePickerRowPattern() override = default;

    bool IsAtomicNode() const override
    {
        return true;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<TimePickerEventHub>();
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        auto paintMethod = MakeRefPtr<TimePickerPaintMethod>();
        paintMethod->SetEnabled(enabled_);
        return paintMethod;
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<LinearLayoutAlgorithm>();
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<LinearLayoutProperty>(isVertical_);
    }

    void OnColumnsBuilding();

    std::unordered_map<std::string, RefPtr<FrameNode>> GetAllChildNode();

    void HandleHourColumnBuilding();

    void FlushColumn();

    void OnDataLinking(
        const RefPtr<FrameNode>& tag, bool isAdd, uint32_t index, std::vector<RefPtr<FrameNode>>& resultTags);

    void HandleHour12Change(bool isAdd, uint32_t index, std::vector<RefPtr<FrameNode>>& resultTags);

    void SetChangeCallback(ColumnChangeCallback&& value);

    void HandleColumnChange(const RefPtr<FrameNode>& tag, bool isAdd, uint32_t index, bool needNotify);

    void SetEventCallback(EventCallback&& value);

    void FireChangeEvent(bool refresh);

    std::string GetSelectedObject(bool isColumnChange, int32_t status = -1);

    PickerTime GetCurrentTime();

    uint32_t GetHourFromAmPm(bool isAm, uint32_t amPmhour) const;

    bool HasTitleNode() const
    {
        return titleId_.has_value();
    }

    int32_t GetTitleId()
    {
        if (!titleId_.has_value()) {
            titleId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return titleId_.value();
    }

    uint32_t GetShowCount() const
    {
        return showCount_;
    }

    void SetShowCount(uint32_t showCount)
    {
        showCount_ = showCount;
    }

    uint32_t GetOptionCount(const RefPtr<FrameNode>& frmeNode)
    {
        return options_[frmeNode].size();
    }

    std::string GetOptionValue(const RefPtr<FrameNode>& frmeNode, uint32_t index)
    {
        if (index >= GetOptionCount(frmeNode)) {
            LOGE("index out of range.");
            return nullptr;
        }
        return options_[frmeNode][index];
    }

    bool HasDividerNode() const
    {
        return DividerId_.has_value();
    }

    int32_t GetDividerId()
    {
        if (!DividerId_.has_value()) {
            DividerId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return DividerId_.value();
    }

    const std::vector<std::string>& GetAllOptions(const RefPtr<FrameNode>& frmeNode)
    {
        return options_[frmeNode];
    }

    const std::map<RefPtr<FrameNode>, std::vector<std::string>>& GetOptions() const
    {
        return options_;
    }

    void SetHour24(bool value)
    {
        hour24_ = value;
    }

    bool GetHour24() const
    {
        return hour24_;
    }

    void SetSelectedTime(const PickerTime& value)
    {
        selectedTime_ = value;
    }

    const PickerTime& GetSelectedTime()
    {
        return selectedTime_;
    }

    void SetDialogTitleDate(const PickerDate& value)
    {
        dialogTitleDate_ = value;
    }

    const PickerDate& GetDialogTitleDate()
    {
        return dialogTitleDate_;
    }

    bool HasAmPmNode() const
    {
        return amPmId_.has_value();
    }

    int32_t GetAmPmId()
    {
        if (!amPmId_.has_value()) {
            amPmId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return amPmId_.value();
    }

    bool HasHourNode() const
    {
        return hourId_.has_value();
    }

    int32_t GetHourId()
    {
        if (!hourId_.has_value()) {
            hourId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return hourId_.value();
    }

    bool HasMinuteNode() const
    {
        return minuteId_.has_value();
    }

    int32_t GetMinuteId()
    {
        if (!minuteId_.has_value()) {
            minuteId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return minuteId_.value();
    }

    bool GetHasSecond() const
    {
        return hasSecond_;
    }
    void SetHasSecond(bool value)
    {
        hasSecond_ = value;
    }

    RefPtr<FrameNode> GetColumn(int32_t tag) const
    {
        auto iter = std::find_if(timePickerColumns_.begin(), timePickerColumns_.end(),
            [tag](const RefPtr<FrameNode>& column) { return column->GetId() == tag; });
        return (iter == timePickerColumns_.end()) ? nullptr : *iter;
    }

    void SetColumn(const RefPtr<FrameNode>& value)
    {
        timePickerColumns_.emplace_back(value);
    }

    bool IsAmHour(uint32_t hourOf24) const;

    uint32_t GetAmPmHour(uint32_t hourOf24) const;

    std::string GetAmFormatString() const;

    std::string GetPmFormatString() const;

    std::string AddZeroPrefix(const std::string& value) const;

    std::string GetHourFormatString(uint32_t hour) const;

    std::string GetMinuteFormatString(uint32_t minute) const;

    FocusPattern GetFocusPattern() const override
    {
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_RETURN(pipeline, FocusPattern());
        auto pickerTheme = pipeline->GetTheme<PickerTheme>();
        CHECK_NULL_RETURN(pickerTheme, FocusPattern());
        auto focusColor = pickerTheme->GetFocusColor();
        FocusPaintParam focusPaintParams;
        focusPaintParams.SetPaintColor(focusColor);
        focusPaintParams.SetPaintWidth(TIME_FOCUS_PAINT_WIDTH);
        return { FocusType::NODE, true, FocusStyleType::CUSTOM_REGION, focusPaintParams };
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override
    {
        json->Put("selected", selectedTime_.ToString(false, false).c_str());
        json->Put("useMilitaryTime", V2::ConvertBoolToString(hour24_).c_str());
    }

    void CreateAmPmNode();

private:
    void OnModifyDone() override;
    void OnAttachToFrameNode() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    void InitOnKeyEvent(const RefPtr<FocusHub>& focusHub);
    bool OnKeyEvent(const KeyEvent& event);
    bool HandleDirectionKey(KeyCode code);
    void InitDisabled();
    void GetInnerFocusPaintRect(RoundRect& paintRect);
    void PaintFocusState();
    void SetButtonIdeaSize();
    double SetAmPmButtonIdeaSize();

    RefPtr<ClickEvent> clickEventListener_;
    bool enabled_ = true;
    int32_t focusKeyID_ = 0;
    std::map<RefPtr<FrameNode>, std::vector<std::string>> options_;
    uint32_t showCount_ = 0;
    // true, use 24 hours style; false, use 12 hours style.
    bool hour24_ = !Localization::GetInstance()->IsAmPmHour();
    PickerTime selectedTime_ = PickerTime::Current();
    PickerDate dialogTitleDate_ = PickerDate::Current();
    std::optional<int32_t> amPmId_;
    std::optional<int32_t> hourId_;
    std::optional<int32_t> minuteId_;
    std::optional<int32_t> titleId_;
    std::optional<int32_t> ButtonTitleId_;
    std::optional<int32_t> DividerId_;

    bool hasSecond_ = false;
    std::vector<RefPtr<FrameNode>> timePickerColumns_;
    std::vector<std::string> vecAmPm_ = Localization::GetInstance()->GetAmPmStrings();
    bool isVertical_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(TimePickerRowPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TIME_PICKER_TIME_PICKER_ROW_PATTERN_H
