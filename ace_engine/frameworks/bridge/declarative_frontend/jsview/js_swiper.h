/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_SWIPER_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_SWIPER_H

#include "frameworks/bridge/declarative_frontend/jsview/js_container_base.h"

namespace OHOS::Ace::Framework {

class JSSwiper : public JSContainerBase {
public:
    static void JSBind(BindingTarget globalObj);
    static void Create(const JSCallbackInfo& info);

    static void SetAutoPlay(bool autoPlay);
    static void SetDuration(int32_t duration);
    static void SetIndex(int32_t index);
    static void SetInterval(int32_t interval);
    static void SetLoop(bool loop);
    static void SetVertical(bool isVertical);
    static void SetIndicator(bool showIndicator);
    static void SetWidth(const JSCallbackInfo& info);
    static void SetHeight(const JSCallbackInfo& info);
    static void SetWidth(const JSRef<JSVal>& jsValue);
    static void SetHeight(const JSRef<JSVal>& jsValue);
    static void SetSize(const JSCallbackInfo& info);
    static void SetIndicatorStyle(const JSCallbackInfo& info);
    static void SetItemSpace(const JSCallbackInfo& info);
    static void SetDisplayMode(int32_t index);
    static void SetEffectMode(const JSCallbackInfo& info);
    static void SetDisplayCount(const JSCallbackInfo& info);
    static void SetCachedCount(int32_t cachedCount);
    static void SetEnabled(const JSCallbackInfo& info);
    static void SetDisableSwipe(bool disableSwipe);
    static void SetCurve(const std::string& curveStr);
    static void SetOnChange(const JSCallbackInfo& info);
    static void SetOnAnimationStart(const JSCallbackInfo& info);
    static void SetOnAnimationEnd(const JSCallbackInfo& info);
    static void SetOnClick(const JSCallbackInfo& info);
    static void JsRemoteMessage(const JSCallbackInfo& info);
};

class JSSwiperController final : public Referenced {
public:
    JSSwiperController() = default;
    ~JSSwiperController() override = default;

    static void JSBind(BindingTarget globalObj);
    static void Constructor(const JSCallbackInfo& args);
    static void Destructor(JSSwiperController* scroller);

    void SwipeTo(const JSCallbackInfo& args)
    {
        if (args.Length() < 1 || !args[0]->IsNumber()) {
            LOGE("Param is not valid");
            return;
        }
        if (controller_) {
            controller_->SwipeTo(args[0]->ToNumber<int32_t>());
        }
    }

    void ShowNext(const JSCallbackInfo& args)
    {
        if (controller_) {
            controller_->ShowNext();
        }
    }

    void ShowPrevious(const JSCallbackInfo& args)
    {
        if (controller_) {
            controller_->ShowPrevious();
        }
    }

    void FinishAnimation(const JSCallbackInfo& args);

    void SetController(const RefPtr<SwiperController>& controller)
    {
        controller_ = controller;
    }

private:
    RefPtr<SwiperController> controller_;

    ACE_DISALLOW_COPY_AND_MOVE(JSSwiperController);
};

} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_SWIPER_H
