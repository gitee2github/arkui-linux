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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_SWIPER_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_SWIPER_MODEL_H

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/memory/referenced.h"
#include "base/utils/macros.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/scroll_bar.h"
#include "core/components/declaration/swiper/swiper_declaration.h"
#include "core/components_ng/base/view_abstract_model.h"
#include "core/components_ng/pattern/swiper/swiper_event_hub.h"
#include "core/components_v2/inspector/inspector_composed_component.h"

namespace OHOS::Ace {

struct SwiperParameters {
    std::optional<Dimension> dimLeft;
    std::optional<Dimension> dimTop;
    std::optional<Dimension> dimRight;
    std::optional<Dimension> dimBottom;
    std::optional<Dimension> dimSize;
    std::optional<bool> maskValue;
    std::optional<Color> colorVal;
    std::optional<Color> selectedColorVal;
};

class ACE_EXPORT SwiperModel {
public:
    static SwiperModel* GetInstance();
    virtual ~SwiperModel() = default;

    virtual RefPtr<SwiperController> Create();
    virtual void SetDirection(Axis axis);
    virtual void SetIndex(uint32_t index);
    virtual void SetAutoPlay(bool autoPlay);
    virtual void SetAutoPlayInterval(uint32_t interval);
    virtual void SetDuration(uint32_t duration);
    virtual void SetCurve(const RefPtr<Curve>& curve);
    virtual void SetLoop(bool loop);
    virtual void SetEnabled(bool enabled);
    virtual void SetDisableSwipe(bool disableSwipe);
    virtual void SetEdgeEffect(EdgeEffect EdgeEffect);
    virtual void SetDisplayMode(SwiperDisplayMode displayMode);
    virtual void SetDisplayCount(int32_t displayCount);
    virtual void SetShowIndicator(bool showIndicator);
    virtual void SetItemSpace(const Dimension& itemSpace);
    virtual void SetCachedCount(int32_t cachedCount);
    virtual void SetOnChange(std::function<void(const BaseEventInfo* info)>&& onChange);
    virtual void SetOnAnimationStart(std::function<void(const BaseEventInfo* info)>&& onAnimationStart);
    virtual void SetOnAnimationEnd(std::function<void(const BaseEventInfo* info)>&& onAnimationEnd);

    virtual void SetRemoteMessageEventId(RemoteCallback&& remoteCallback);
    virtual void SetOnClick(
        std::function<void(const BaseEventInfo* info, const RefPtr<V2::InspectorFunctionImpl>& impl)>&& value);
    virtual void SetMainSwiperSizeWidth();
    virtual void SetMainSwiperSizeHeight();
    virtual void SetIndicatorStyle(const SwiperParameters& swiperParameters);

private:
    static std::unique_ptr<SwiperModel> instance_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SWIPER_SWIPER_MODEL_H
