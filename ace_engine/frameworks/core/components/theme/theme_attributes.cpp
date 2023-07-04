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

#include "core/components/theme/theme_attributes.h"

namespace OHOS::Ace {

// theme attributes
const char THEME_ATTR_BG_COLOR[] = "bg_color";

// component patterns
const char THEME_PATTERN_BUTTON[] = "button_pattern";
const char THEME_PATTERN_CHECKBOX[] = "checkbox_pattern";
const char THEME_PATTERN_DATA_PANEL[] = "data_panel_pattern";
const char THEME_PATTERN_RADIO[] = "radio_pattern";
const char THEME_PATTERN_SWIPER[] = "swiper_pattern";
const char THEME_PATTERN_SWITCH[] = "switch_pattern";
const char THEME_PATTERN_TOOLBAR[] = "toolbar_pattern";
const char THEME_PATTERN_TOGGLE[] = "toggle_pattern";
const char THEME_PATTERN_TOAST[] = "toast_pattern";
const char THEME_PATTERN_DIALOG[] = "dialog_pattern";
const char THEME_PATTERN_DRAG_BAR[] = "drag_bar_pattern";
const char THEME_PATTERN_SEMI_MODAL[] = "semi_modal_pattern";
const char THEME_PATTERN_BADGE[] = "badge_pattern";
const char THEME_PATTERN_CALENDAR[] = "calendar_pattern";
const char THEME_PATTERN_CAMERA[] = "camera_pattern";
const char THEME_PATTERN_CLOCK[] = "clock_pattern";
const char THEME_PATTERN_COUNTER[] = "counter_pattern";
const char THEME_PATTERN_DIVIDER[] = "divider_pattern";
const char THEME_PATTERN_FOCUS_ANIMATION[] = "focus_animation_pattern";
const char THEME_PATTERN_GRID[] = "grid_pattern";
const char THEME_PATTERN_ICON[] = "icon_pattern";
const char THEME_PATTERN_IMAGE[] = "image_pattern";
const char THEME_PATTERN_LIST[] = "list_pattern";
const char THEME_PATTERN_LIST_ITEM[] = "list_item_pattern";
const char THEME_PATTERN_MARQUEE[] = "marquee_pattern";
const char THEME_PATTERN_NAVIGATION_BAR[] = "navigation_bar_pattern";
const char THEME_PATTERN_PICKER[] = "picker_pattern";
const char THEME_PATTERN_PIECE[] = "piece_pattern";
const char THEME_PATTERN_POPUP[] = "popup_pattern";
const char THEME_PATTERN_PROGRESS[] = "progress_pattern";
const char THEME_PATTERN_QRCODE[] = "qrcode_pattern";
const char THEME_PATTERN_RATING[] = "rating_pattern";
const char THEME_PATTERN_REFRESH[] = "refresh_pattern";
const char THEME_PATTERN_SCROLL_BAR[] = "scroll_bar_pattern";
const char THEME_PATTERN_SEARCH[] = "search_pattern";
const char THEME_PATTERN_STEPPER[] = "stepper_pattern";
const char THEME_PATTERN_TAB[] = "tab_pattern";
const char THEME_PATTERN_SELECT[] = "select_pattern";
const char THEME_PATTERN_SLIDER[] = "slider_pattern";
const char THEME_PATTERN_TEXT[] = "text_pattern";
const char THEME_PATTERN_TEXTFIELD[] = "textfield_pattern";
const char THEME_PATTERN_TEXT_OVERLAY[] = "text_overlay_pattern";
const char THEME_PATTERN_VIDEO[] = "video_pattern";
const char THEME_PATTERN_INDEXER[] = "indexer_pattern";
const char THEME_PATTERN_APP_BAR[] = "app_bar_pattern";

// pattern general attributes
const char PATTERN_FG_COLOR[] = "fg_color";
const char PATTERN_BG_COLOR[] = "bg_color";
const char PATTERN_BG_COLOR_HOVERED[] = "bg_color_hovered";
const char PATTERN_BG_COLOR_CLICKED[] = "bg_color_clicked";
const char PATTERN_BG_COLOR_PRESSED[] = "bg_color_pressed";
const char PATTERN_BG_COLOR_SELECTED[] = "bg_color_selected";
const char PATTERN_BG_COLOR_FOCUSED[] = "bg_color_focused";
const char PATTERN_BG_COLOR_DISABLED_ALPHA[] = "bg_color_disabled_alpha";
const char PATTERN_BG_COLOR_ALPHA[] = "bg_color_alpha";
const char PATTERN_TEXT_COLOR[] = "text_color";
const char PATTERN_TEXT_COLOR_ALPHA[] = "text_color_alpha";
const char PATTERN_TEXT_COLOR_FOCUSED[] = "text_color_focused";
const char PATTERN_TEXT_COLOR_SELECTED[] = "text_color_selected";
const char PATTERN_TEXT_COLOR_DISABLED[] = "text_color_disabled";
const char PATTERN_TEXT_SIZE[] = "text_font_size";
const char PATTERN_WIDTH[] = "width";
const char PATTERN_HEIGHT[] = "height";

// pattern attributes for checkable
const char SWITCH_WIDTH[] = "switch_width";
const char SWITCH_HEIGHT[] = "switch_height";
const char SWITCH_SHADOW_WIDTH[] = "switch_shadow_width";
const char RADIO_WIDTH[] = "radio_width";
const char SWITCH_HORIZONTAL_PADDING[] = "switch_horizontal_padding";
const char SWITCH_VERTICAL_PADDING[] = "switch_vertical_padding";
const char RADIO_PADDING[] = "radio_padding";

// data panel attributes
const char DATA_PANEL_COLOR_1_START[] = "rainbow_fg_color_1_start";
const char DATA_PANEL_COLOR_2_START[] = "rainbow_fg_color_2_start";
const char DATA_PANEL_COLOR_3_START[] = "rainbow_fg_color_3_start";
const char DATA_PANEL_COLOR_4_START[] = "rainbow_fg_color_4_start";
const char DATA_PANEL_COLOR_5_START[] = "rainbow_fg_color_5_start";
const char DATA_PANEL_COLOR_6_START[] = "rainbow_fg_color_6_start";
const char DATA_PANEL_COLOR_7_START[] = "rainbow_fg_color_7_start";
const char DATA_PANEL_COLOR_8_START[] = "rainbow_fg_color_8_start";
const char DATA_PANEL_COLOR_9_START[] = "rainbow_fg_color_9_start";
const char DATA_PANEL_COLOR_1_END[] = "rainbow_fg_color_1_end";
const char DATA_PANEL_COLOR_2_END[] = "rainbow_fg_color_2_end";
const char DATA_PANEL_COLOR_3_END[] = "rainbow_fg_color_3_end";
const char DATA_PANEL_COLOR_4_END[] = "rainbow_fg_color_4_end";
const char DATA_PANEL_COLOR_5_END[] = "rainbow_fg_color_5_end";
const char DATA_PANEL_COLOR_6_END[] = "rainbow_fg_color_6_end";
const char DATA_PANEL_COLOR_7_END[] = "rainbow_fg_color_7_end";
const char DATA_PANEL_COLOR_8_END[] = "rainbow_fg_color_8_end";
const char DATA_PANEL_COLOR_9_END[] = "rainbow_fg_color_9_end";
const char DATA_PANEL_LOADING_COLOR_START[] = "loading_fg_color_start";
const char DATA_PANEL_LOADING_COLOR_END[] = "loading_fg_color_end";
const char DATA_PANEL_PROGRESS_COLOR_START[] = "progress_fg_color_start";
const char DATA_PANEL_PROGRESS_COLOR_END[] = "progress_fg_color_end";

// pattern attributes for toggle
const char DIALOG_CONTENT_TOP_PADDING[] = "dialog_content_top_padding";
const char DIALOG_TITLE_TOP_PADDING[] = "dialog_title_top_padding";
const char DIALOG_BUTTON_HEIGHT[] = "dialog_button_height";
const char DIALOG_TITLE_MAX_LINES[] = "dialog_title_max_lines";
const char DIALOG_COMMON_BUTTON_TEXT_COLOR[] = "dialog_common_button_text_color";
const char DIALOG_MIN_BUTTON_TEXT_SIZE[] = "dialog_min_button_text_size";
const char DIALOG_MAX_BUTTON_WIDTH[] = "dialog_max_button_width";
const char DIALOG_MIN_BUTTON_WIDTH[] = "dialog_min_button_width";
const char DIALOG_ACTIONS_TOP_PADDING[] = "dialog_actions_top_padding";
const char DIALOG_MASK_COLOR_END[] = "dialog_mask_color_end";

// pattern attributes for drag bar
const char DRAG_BAR_COLOR[] = "drag_bar_color";

// pattern attributes for popup
const char POPUP_BORDER_RADIUS[] = "popup_border_radius";
const char POPUP_HORIZONTAL_PADDING[] = "popup_horizontal_padding";
const char POPUP_VERTICAL_PADDING[] = "popup_vertical_padding";
const char POPUP_BUTTON_TEXT_FONT_SIZE[] = "popup_button_text_font_size";
} // namespace OHOS::Ace
