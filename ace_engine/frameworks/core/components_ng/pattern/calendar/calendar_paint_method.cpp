/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/calendar/calendar_paint_method.h"

#include <cstdint>

#include "base/geometry/ng/rect_t.h"
#include "base/geometry/offset.h"
#include "base/geometry/point.h"
#include "base/i18n/localization.h"
#include "base/memory/referenced.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/components/calendar/calendar_theme.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/pattern/calendar/calendar_paint_property.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/components_ng/render/paint_property.h"

namespace OHOS::Ace::NG {

namespace {

const char ELLIPSIS[] = "...";
constexpr int32_t DEFAULT_WEEKS = 5;
constexpr double WEEKEND_TRANSPARENT = 0x7D;

std::unique_ptr<RSParagraph> GetTextParagraph(const std::string& text, const rosen::TextStyle& textStyle)
{
    RSParagraphStyle style;
    auto fontCollection = RSFontCollection::GetInstance(false);
    CHECK_NULL_RETURN(fontCollection, nullptr);
    std::unique_ptr<RSParagraphBuilder> builder = RSParagraphBuilder::CreateRosenBuilder(style, fontCollection);
    builder->PushStyle(textStyle);
    builder->AddText(StringUtils::Str8ToStr16(text));
    return builder->Build();
}

void DrawCalendarText(
    RSCanvas* canvas, const std::string& text, const RSTextStyle& textStyle, const Rect& boxRect, Rect& textRect)
{
    // The lunar calendar description is truncated by more than three characters.
    std::string newText { text };
    auto wText = StringUtils::ToWstring(text);
    if (wText.size() > 3) {
        wText = wText.substr(0, 2);
        newText = StringUtils::ToString(wText);
        newText += ELLIPSIS;
    }

    auto paragraph = GetTextParagraph(newText, textStyle);
    CHECK_NULL_VOID_NOLOG(paragraph);
    const auto& offset = boxRect.GetOffset();
    paragraph->Layout(boxRect.Width());
    double textWidth = paragraph->GetMaxIntrinsicWidth();
    double textHeight = paragraph->GetHeight();
    // paint text in center of item
    double textPaintOffsetX = (boxRect.Width() - textWidth) / 2.0;
    double textPaintOffsetY = (boxRect.Height() - textHeight) / 2.0;
    paragraph->Paint(canvas, offset.GetX() + textPaintOffsetX, offset.GetY() + textPaintOffsetY);
    textRect.SetRect(offset.GetX() + textPaintOffsetX, offset.GetY() + textPaintOffsetY, textWidth, textHeight);
}

void DrawCalendarText(RSCanvas* canvas, const std::string& text, const RSTextStyle& textStyle, const Rect& boxRect)
{
    Rect textRect;
    DrawCalendarText(canvas, text, textStyle, boxRect, textRect);
}

} // namespace

CanvasDrawFunction CalendarPaintMethod::GetContentDrawFunction(PaintWrapper* paintWrapper)
{
    auto paintProperty = DynamicCast<CalendarPaintProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_RETURN(paintProperty, nullptr);
    frameSize_ = paintWrapper->GetGeometryNode()->GetFrameSize();
    auto paintFunc = [weak = WeakClaim(this), paintProperty](RSCanvas& canvas) {
        auto calendar_ = weak.Upgrade();
        if (calendar_) {
            calendar_->PaintContent(canvas, paintProperty);
        }
    };
    return paintFunc;
}

void CalendarPaintMethod::PaintContent(RSCanvas& canvas, const RefPtr<CalendarPaintProperty>& paintProperty)
{
    SetCalendarTheme(paintProperty);
    DrawWeekAndDates(canvas, offset_);
}

void CalendarPaintMethod::DrawWeekAndDates(RSCanvas& canvas, Offset offset)
{
    weekNumbers_ = Localization::GetInstance()->GetWeekdays(true);
    uint32_t totalWeek = weekNumbers_.size();
    if (!obtainedMonth_.days.empty()) {
        calendarDays_.assign(obtainedMonth_.days.begin(), obtainedMonth_.days.end());
    }

    offset += { touchCircleStrokeWidth_, 0 };
    DrawWeek(canvas, offset);
    if (!calendarDays_.empty()) {
        int32_t dateNumber = 0;
        double dailyRowSpace = 0.0;
        double dayNumberStartY = topPadding_ + weekHeight_ + weekAndDayRowSpace_;

        // Set the rowCount.
        if (totalWeek != 0) {
            rowCount_ = (static_cast<int32_t>(calendarDays_.size()) / totalWeek);
        }

        // Set dailyFiveRowSpace_ for five line calendar.
        // Set dailySixRowSpace_ for six line calendar.
        dailyRowSpace = rowCount_ == DEFAULT_WEEKS ? dailyFiveRowSpace_ : dailySixRowSpace_;
        for (int32_t row = 0; row < rowCount_; row++) {
            double y = row * (dayHeight_ + dailyRowSpace) + dayNumberStartY;
            for (uint32_t column = 0; column < totalWeek; column++) {
                const auto& day = calendarDays_[dateNumber++];
                double x = textDirection_ == TextDirection::LTR ? column * (dayWidth_ + colSpace_)
                                                                : (totalWeek - column - 1) * (dayWidth_ + colSpace_);
                auto dayOffset = Offset(x, y);
                DrawCalendar(canvas, offset, dayOffset, day);
            }
        }
    }
}

void CalendarPaintMethod::DrawCalendar(
    RSCanvas& canvas, const Offset& offset, const Offset& dayOffset, const CalendarDay& day)
{
    rosen::TextStyle dateTextStyle;
    rosen::TextStyle lunarTextStyle;
    InitTextStyle(dateTextStyle, lunarTextStyle);
    dateTextStyle.locale_ = Localization::GetInstance()->GetFontLocale();
    lunarTextStyle.locale_ = Localization::GetInstance()->GetFontLocale();

    // First of all, check whether the day is current month or not, and set text style.
    SetDayTextStyle(dateTextStyle, lunarTextStyle, day);

    auto x = dayOffset.GetX();
    auto y = dayOffset.GetY();

    if (day.focused && day.month.month == currentMonth_.month) {
        if (IsToday(day)) {
            DrawTodayArea(canvas, offset, x, y);
        } else {
            DrawFocusedArea(canvas, offset, x, y);
        }
    }

    if (showLunar_ && !day.lunarDay.empty()) {
        // paint day
        Offset dateNumberOffset = offset + Offset(x, y);
        PaintDay(canvas, dateNumberOffset, day, dateTextStyle);

        // paint lunar day
        Offset lunarDayOffset = offset + Offset(x, y + gregorianCalendarHeight_);
        PaintLunarDay(canvas, lunarDayOffset, day, lunarTextStyle);
    } else {
        // when there is no lunar calendar, the date is displayed in the center
        Offset dateNumberOffset = offset + Offset(x, y + (focusedAreaRadius_ - gregorianCalendarHeight_ / 2));
        PaintDay(canvas, dateNumberOffset, day, dateTextStyle);
    }
}

void CalendarPaintMethod::DrawTodayArea(RSCanvas& canvas, const Offset& offset, double x, double y) const
{
    // draw Today background circle
    RSBrush brush;
    brush.SetAntiAlias(true);
    brush.SetColor(focusedAreaBackgroundColor_);
    canvas.AttachBrush(brush);

    Offset circleCenter = Offset(x - (focusedAreaRadius_ * 2 - dayWidth_) / 2 + focusedAreaRadius_,
        y - (1.0_vp).ConvertToPx() + focusedAreaRadius_);
    Offset bgCircleStart = offset + circleCenter;
    canvas.DrawCircle(rosen::Point(static_cast<float>(bgCircleStart.GetX()), static_cast<float>(bgCircleStart.GetY())),
        static_cast<float>(focusedAreaRadius_));
}

void CalendarPaintMethod::DrawFocusedArea(RSCanvas& canvas, const Offset& offset, double x, double y) const
{
    // draw focus background circle
    RSPen pen;
    pen.SetAntiAlias(true);
    pen.SetColor(focusedAreaBackgroundColor_);
    canvas.AttachPen(pen);

    Offset circleCenter = Offset(x - (focusedAreaRadius_ * 2 - dayWidth_) / 2 + focusedAreaRadius_,
        y - (1.0_vp).ConvertToPx() + focusedAreaRadius_);
    Offset bgCircleStart = offset + circleCenter;
    canvas.DrawCircle(rosen::Point(static_cast<float>(bgCircleStart.GetX()), static_cast<float>(bgCircleStart.GetY())),
        static_cast<float>(focusedAreaRadius_));
}

void CalendarPaintMethod::InitTextStyle(rosen::TextStyle& dateTextStyle, rosen::TextStyle& lunarTextStyle)
{
    dateTextStyle.fontSize_ = dayFontSize_;
    dateTextStyle.fontWeight_ = static_cast<rosen::FontWeight>(dayFontWeight_);

    lunarTextStyle.fontSize_ = lunarDayFontSize_;
    lunarTextStyle.fontWeight_ = static_cast<rosen::FontWeight>(lunarDayFontWeight_);
}

void CalendarPaintMethod::SetDayTextStyle(
    rosen::TextStyle& dateTextStyle, rosen::TextStyle& lunarTextStyle, const CalendarDay& day)
{
    // Set the noncurrent month style and current month style.
    if (day.month.month != currentMonth_.month) {
        dateTextStyle.color_ = nonCurrentMonthDayColor_;
        lunarTextStyle.color_ = day.markLunarDay ? RSColor(markLunarColor_.GetRed(), markLunarColor_.GetGreen(),
            markLunarColor_.GetBlue(), WEEKEND_TRANSPARENT) : nonCurrentMonthLunarColor_;
    } else {
        dateTextStyle.color_ = IsToday(day) ? focusedDayColor_ : IsOffDay(day) ? weekendDayColor_ : dayColor_;
        lunarTextStyle.color_ =
            IsToday(day) ? focusedLunarColor_
                         : (day.markLunarDay ? markLunarColor_ : (IsOffDay(day) ? weekendLunarColor_ : lunarColor_));
    }
}

void CalendarPaintMethod::PaintDay(
    RSCanvas& canvas, const Offset& offset, const CalendarDay& day, rosen::TextStyle& textStyle) const
{
    // paint day
    Rect boxRect { offset.GetX(), offset.GetY(), dayWidth_, gregorianCalendarHeight_ };
    Rect textRect;
    rosen::TextStyle workStateStyle;

    auto dayStr = std::to_string(day.day);
    dayStr = Localization::GetInstance()->NumberFormat(day.day);
    DrawCalendarText(&canvas, dayStr, textStyle, boxRect, textRect);

    if (!day.dayMark.empty() && showHoliday_) {
        workStateStyle.fontWeight_ = static_cast<rosen::FontWeight>(workStateFontWeight_);
        workStateStyle.locale_ = Localization::GetInstance()->GetFontLocale();
        boxRect = { textRect.GetOffset().GetX() + textRect.Width() - workStateHorizontalMovingDistance_,
            textRect.GetOffset().GetY() + textRect.Height() - workStateVerticalMovingDistance_, workStateWidth_,
            workStateWidth_ };
        if (day.month.month == currentMonth_.month) {
            if (day.dayMark == "work") {
                workStateStyle.fontSize_ = workDayMarkSize_;
                workStateStyle.color_ = workDayMarkColor_;
            } else if (day.dayMark == "off") {
                workStateStyle.fontSize_ = offDayMarkSize_;
                workStateStyle.color_ = offDayMarkColor_;
            }
        } else {
            if (day.dayMark == "work") {
                workStateStyle.fontSize_ = workDayMarkSize_;
                workStateStyle.color_ = RSColor(workDayMarkColor_.GetRed(), workDayMarkColor_.GetGreen(),
                    workDayMarkColor_.GetBlue(), WEEKEND_TRANSPARENT);
            } else if (day.dayMark == "off") {
                workStateStyle.fontSize_ = offDayMarkSize_;
                workStateStyle.color_ = RSColor(offDayMarkColor_.GetRed(), offDayMarkColor_.GetGreen(),
                    offDayMarkColor_.GetBlue(), WEEKEND_TRANSPARENT);
            }
            if (day.focused) {
                workStateStyle.color_ = Color::BLACK.GetValue();
            }
            if (IsToday(day) && day.touched) {
                workStateStyle.color_ = focusedDayColor_;
            }
        }
        DrawCalendarText(&canvas, day.dayMarkValue, workStateStyle, boxRect);
    }
}

void CalendarPaintMethod::PaintLunarDay(
    RSCanvas& canvas, const Offset& offset, const CalendarDay& day, const rosen::TextStyle& textStyle) const
{
    Rect boxRect = { offset.GetX(), offset.GetY(), dayWidth_, lunarHeight_ };
    DrawCalendarText(&canvas, day.lunarDay, textStyle, boxRect);
}

void CalendarPaintMethod::DrawWeek(RSCanvas& canvas, const Offset& offset) const
{
    uint32_t totalWeek = weekNumbers_.size();
    rosen::TextStyle weekTextStyle;
    weekTextStyle.color_ = weekColor_;
    weekTextStyle.fontSize_ = weekFontSize_;
    weekTextStyle.locale_ = Localization::GetInstance()->GetFontLocale();
    static const int32_t daysOfWeek = 7;

    auto startDayOfWeek = startOfWeek_;
    for (uint32_t column = 0; column < totalWeek; column++) {
        double x = textDirection_ == TextDirection::LTR ? column * (weekWidth_ + colSpace_)
                                                        : (totalWeek - column - 1) * (weekWidth_ + colSpace_);
        Offset weekNumberOffset = offset + Offset(x, topPadding_);
        Rect boxRect { weekNumberOffset.GetX(), weekNumberOffset.GetY(), weekWidth_, weekHeight_ };
        std::string newText { weekNumbers_[(startDayOfWeek + 1) % daysOfWeek] };
        auto wText = StringUtils::ToWstring(newText);
        if (wText.size() > 3) {
            wText = wText.substr(0, 3);
            newText = StringUtils::ToString(wText);
        }
        DrawCalendarText(&canvas, newText, weekTextStyle, boxRect);
        ++startDayOfWeek;
    }
}

void CalendarPaintMethod::SetCalendarTheme(const RefPtr<CalendarPaintProperty>& paintProperty)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    RefPtr<CalendarTheme> theme = pipelineContext->GetTheme<CalendarTheme>();
    CHECK_NULL_VOID(theme);

    weekColor_ = ToRSColor(paintProperty->GetWeekColor().value_or(theme->GetCalendarTheme().weekColor));
    dayColor_ = ToRSColor(paintProperty->GetDayColor().value_or(theme->GetCalendarTheme().dayColor));
    lunarColor_ = ToRSColor(paintProperty->GetLunarColor().value_or(theme->GetCalendarTheme().lunarColor));
    weekendDayColor_ =
        ToRSColor(paintProperty->GetWeekendDayColor().value_or(theme->GetCalendarTheme().weekendDayColor));
    weekendLunarColor_ =
        ToRSColor(paintProperty->GetWeekendLunarColor().value_or(theme->GetCalendarTheme().weekendLunarColor));
    nonCurrentMonthDayColor_ = ToRSColor(
        paintProperty->GetNonCurrentMonthDayColor().value_or(theme->GetCalendarTheme().nonCurrentMonthDayColor));
    nonCurrentMonthLunarColor_ = ToRSColor(
        paintProperty->GetNonCurrentMonthLunarColor().value_or(theme->GetCalendarTheme().nonCurrentMonthLunarColor));
    workDayMarkColor_ =
        ToRSColor(paintProperty->GetWorkDayMarkColor().value_or(theme->GetCalendarTheme().workDayMarkColor));
    offDayMarkColor_ =
        ToRSColor(paintProperty->GetOffDayMarkColor().value_or(theme->GetCalendarTheme().offDayMarkColor));
    nonCurrentMonthWorkDayMarkColor_ = ToRSColor(paintProperty->GetNonCurrentMonthWorkDayMarkColor().value_or(
        theme->GetCalendarTheme().nonCurrentMonthWorkDayMarkColor));
    nonCurrentMonthOffDayMarkColor_ = ToRSColor(paintProperty->GetNonCurrentMonthOffDayMarkColor().value_or(
        theme->GetCalendarTheme().nonCurrentMonthOffDayMarkColor));
    focusedDayColor_ =
        ToRSColor(paintProperty->GetFocusedDayColor().value_or(theme->GetCalendarTheme().focusedDayColor));
    focusedLunarColor_ =
        ToRSColor(paintProperty->GetFocusedLunarColor().value_or(theme->GetCalendarTheme().focusedLunarColor));
    focusedAreaBackgroundColor_ = ToRSColor(
        paintProperty->GetFocusedAreaBackgroundColor().value_or(theme->GetCalendarTheme().focusedAreaBackgroundColor));
    markLunarColor_ = ToRSColor(paintProperty->GetMarkLunarColor().value_or(theme->GetCalendarTheme().markLunarColor));

    todayDayColor_ = RSColor(theme->GetCalendarTheme().todayColor.GetValue());
    todayLunarColor_ = RSColor(theme->GetCalendarTheme().todayLunarColor.GetValue());

    dayFontWeight_ = StringUtils::StringToFontWeight(theme->GetCalendarTheme().dayFontWeight);
    lunarDayFontWeight_ = StringUtils::StringToFontWeight(theme->GetCalendarTheme().lunarDayFontWeight);
    workStateFontWeight_ = StringUtils::StringToFontWeight(theme->GetCalendarTheme().workStateFontWeight);

    topPadding_ = theme->GetCalendarTheme().topPadding.ConvertToPx();
    weekFontSize_ = paintProperty->GetWeekFontSize().value_or(theme->GetCalendarTheme().weekFontSize).ConvertToPx();
    dayFontSize_ = paintProperty->GetDayFontSize().value_or(theme->GetCalendarTheme().dayFontSize).ConvertToPx();
    lunarDayFontSize_ =
        paintProperty->GetLunarDayFontSize().value_or(theme->GetCalendarTheme().lunarDayFontSize).ConvertToPx();
    workDayMarkSize_ =
        paintProperty->GetWorkDayMarkSize().value_or(theme->GetCalendarTheme().workDayMarkSize).ConvertToPx();
    offDayMarkSize_ =
        paintProperty->GetOffDayMarkSize().value_or(theme->GetCalendarTheme().offDayMarkSize).ConvertToPx();
    focusedAreaRadius_ = paintProperty->GetFocusedAreaRadiusValue({}).ConvertToPx() <= 0
                             ? theme->GetCalendarTheme().focusedAreaRadius.ConvertToPx()
                             : paintProperty->GetFocusedAreaRadiusValue({}).ConvertToPx();

    weekHeight_ = paintProperty->GetWeekHeightValue({}).ConvertToPx() <= 0
                      ? theme->GetCalendarTheme().weekHeight.ConvertToPx()
                      : paintProperty->GetWeekHeightValue({}).ConvertToPx();
    dayHeight_ = paintProperty->GetDayHeightValue({}).ConvertToPx() <= 0
                     ? theme->GetCalendarTheme().dayHeight.ConvertToPx()
                     : paintProperty->GetDayHeightValue({}).ConvertToPx();
    weekWidth_ = paintProperty->GetWeekWidthValue({}).ConvertToPx() <= 0
                     ? theme->GetCalendarTheme().weekWidth.ConvertToPx()
                     : paintProperty->GetWeekWidthValue({}).ConvertToPx();
    dayWidth_ = paintProperty->GetDayWidthValue({}).ConvertToPx() <= 0
                    ? theme->GetCalendarTheme().dayWidth.ConvertToPx()
                    : paintProperty->GetDayWidthValue({}).ConvertToPx();
    weekAndDayRowSpace_ =
        paintProperty->GetWeekAndDayRowSpace().value_or(theme->GetCalendarTheme().weekAndDayRowSpace).ConvertToPx();
    lunarHeight_ = paintProperty->GetLunarHeight().value_or(theme->GetCalendarTheme().lunarHeight).ConvertToPx();
    touchCircleStrokeWidth_ = theme->GetCalendarTheme().touchCircleStrokeWidth.ConvertToPx();

    colSpace_ = paintProperty->GetColSpaceValue({}).ConvertToPx() <= 0
                    ? theme->GetCalendarTheme().colSpace.ConvertToPx()
                    : paintProperty->GetColSpaceValue({}).ConvertToPx();

    dailyFiveRowSpace_ = paintProperty->GetDailyFiveRowSpaceValue({}).ConvertToPx() <= 0
                             ? theme->GetCalendarTheme().dailyFiveRowSpace.ConvertToPx()
                             : paintProperty->GetDailyFiveRowSpaceValue({}).ConvertToPx();

    dailySixRowSpace_ = paintProperty->GetDailySixRowSpaceValue({}).ConvertToPx() <= 0
                             ? theme->GetCalendarTheme().dailySixRowSpace.ConvertToPx()
                             : paintProperty->GetDailySixRowSpaceValue({}).ConvertToPx();

    gregorianCalendarHeight_ = paintProperty->GetGregorianCalendarHeightValue({}).ConvertToPx() <= 0
                                   ? theme->GetCalendarTheme().gregorianCalendarHeight.ConvertToPx()
                                   : paintProperty->GetGregorianCalendarHeightValue({}).ConvertToPx();
    workStateWidth_ = paintProperty->GetWorkStateWidthValue({}).ConvertToPx() <= 0
                          ? theme->GetCalendarTheme().workStateWidth.ConvertToPx()
                          : paintProperty->GetWorkStateWidthValue({}).ConvertToPx();

    workStateHorizontalMovingDistance_ = paintProperty->GetWorkStateHorizontalMovingDistance()
                                             .value_or(theme->GetCalendarTheme().workStateHorizontalMovingDistance)
                                             .ConvertToPx();
    workStateVerticalMovingDistance_ = paintProperty->GetWorkStateVerticalMovingDistance()
                                           .value_or(theme->GetCalendarTheme().workStateVerticalMovingDistance)
                                           .ConvertToPx();
    if (paintProperty->HasShowLunar()) {
        showLunar_ = paintProperty->GetShowLunarValue();
    }
    showHoliday_ = paintProperty->GetShowHolidayValue(true);
    if (paintProperty->HasStartOfWeek()) {
        startOfWeek_ = static_cast<uint32_t>(paintProperty->GetStartOfWeekValue());
    }
    startOfWeek_ = static_cast<uint32_t>(log2(startOfWeek_));
    if (paintProperty->HasOffDays()) {
        offDays_ = paintProperty->GetOffDaysValue();
    }
    currentMonth_.month = obtainedMonth_.month;
}

bool CalendarPaintMethod::IsToday(const CalendarDay& day) const
{
    auto today = calendarDay_;
    return today.month == day.month && today.day == day.day;
}

bool CalendarPaintMethod::IsOffDay(const CalendarDay& dayInfo) const
{
    std::vector<std::string> days;
    StringUtils::StringSplitter(offDays_, ',', days);
    int daysOfWeek = 7;
    for (const auto& day : days) {
        auto num = (StringUtils::StringToInt(day) + (daysOfWeek - startOfWeek_)) % daysOfWeek;
        if ((dayInfo.index % daysOfWeek) == num) {
            return true;
        }
    }
    return false;
}

} // namespace OHOS::Ace::NG
