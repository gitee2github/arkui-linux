/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_COUNTER_COUNTER_THEME_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_COUNTER_COUNTER_THEME_H

#include "core/components/common/properties/color.h"
#include "core/components/common/properties/edge.h"
#include "core/components/common/properties/radius.h"
#include "core/components/common/properties/text_style.h"
#include "core/components/theme/theme.h"
#include "core/components/theme/theme_constants.h"
#include "core/components/theme/theme_constants_defines.h"
#include "core/components_ng/property/border_property.h"

namespace OHOS::Ace {

class CounterTheme : public virtual Theme {
    DECLARE_ACE_TYPE(CounterTheme, Theme);

public:
    class Builder {
    public:
        Builder() = default;
        ~Builder() = default;

        RefPtr<CounterTheme> Build(const RefPtr<ThemeConstants>& themeConstants) const
        {
            RefPtr<CounterTheme> theme = AceType::MakeRefPtr<CounterTheme>();
            if (!themeConstants) {
                return theme;
            }

            // init theme from global data
            theme->contentTextStyle_.SetFontSize(themeConstants->GetDimension(THEME_COUNTER_TITLE_FONTSIZE));
            theme->contentTextStyle_.SetTextColor(themeConstants->GetColor(THEME_COUNTER_FONTCOLOR));
            theme->backgroundColor_ = themeConstants->GetColor(THEME_COUNTER_BACKGROUND_COLOR);
            return theme;
        }
    };

    ~CounterTheme() override = default;

    const TextStyle& GetContentTextStyle() const
    {
        return contentTextStyle_;
    }

    const Color& GetBackGroundColor() const
    {
        return backgroundColor_;
    }

    const Dimension& GetHeight() const
    {
        return height_;
    }

    const Dimension& GetWidth() const
    {
        return width_;
    }

    const Dimension& GetControlWidth() const
    {
        return controlWidth_;
    }

    const Dimension& GetContentWidth() const
    {
        return contentWidth_;
    }

    const NG::BorderRadiusProperty& GetBorderRadius() const
    {
        return borderRadius_;
    }

    const NG::BorderWidthProperty& GetBorderWidth() const
    {
        return borderWidth_;
    }

    const NG::BorderColorProperty& GetBorderColor() const
    {
        return borderColor_;
    }

    const NG::BorderStyleProperty& GetBorderStyle() const
    {
        return borderStyle_;
    }

private:
    TextStyle contentTextStyle_;
    Color backgroundColor_ = Color(0xff191919);
    Dimension height_ = 32.0_vp;
    Dimension width_ = 100.0_vp;
    Dimension controlWidth_ = 32.0_vp;
    Dimension contentWidth_ = 36.0_vp;
    NG::BorderRadiusProperty borderRadius_ = { 4.0_vp, 4.0_vp, 4.0_vp, 4.0_vp };
    NG::BorderWidthProperty borderWidth_ = { 1.0_vp, 1.0_vp, 1.0_vp, 1.0_vp };
    NG::BorderColorProperty borderColor_ = { Color::GRAY, Color::GRAY, Color::GRAY, Color::GRAY };
    NG::BorderStyleProperty borderStyle_ = { BorderStyle::SOLID, BorderStyle::SOLID, BorderStyle::SOLID,
        BorderStyle::SOLID };
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_COUNTER_COUNTER_THEME_H
