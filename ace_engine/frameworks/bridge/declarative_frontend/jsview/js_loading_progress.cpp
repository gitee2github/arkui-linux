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

#include "bridge/declarative_frontend/jsview/js_loading_progress.h"

#include "bridge/declarative_frontend/jsview/models/loading_progress_model_impl.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/pattern/loading_progress/loading_progress_model.h"
#include "core/components_ng/pattern/loading_progress/loading_progress_model_ng.h"

namespace OHOS::Ace {
std::unique_ptr<LoadingProgressModel> LoadingProgressModel::instance_ = nullptr;

LoadingProgressModel* LoadingProgressModel::GetInstance()
{
    if (!instance_) {
#ifdef NG_BUILD
        instance_.reset(new NG::LoadingProgressModelNG());
#else
        if (Container::IsCurrentUseNewPipeline()) {
            instance_.reset(new NG::LoadingProgressModelNG());
        } else {
            instance_.reset(new Framework::LoadingProgressModelImpl());
        }
#endif
    }
    return instance_.get();
}

} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {
void JSLoadingProgress::JSBind(BindingTarget globalObj)
{
    JSClass<JSLoadingProgress>::Declare("LoadingProgress");
    MethodOptions opt = MethodOptions::NONE;

    JSClass<JSLoadingProgress>::StaticMethod("create", &JSLoadingProgress::Create, opt);
    JSClass<JSLoadingProgress>::StaticMethod("color", &JSLoadingProgress::SetColor, opt);
    JSClass<JSLoadingProgress>::Inherit<JSViewAbstract>();
    JSClass<JSLoadingProgress>::Bind(globalObj);
}

void JSLoadingProgress::Create()
{
    LoadingProgressModel::GetInstance()->Create();
}

void JSLoadingProgress::SetColor(const JSCallbackInfo& info)
{
    Color progressColor;
    if (!ParseJsColor(info[0], progressColor)) {
        return;
    }

    LoadingProgressModel::GetInstance()->SetColor(progressColor);
}
}; // namespace OHOS::Ace::Framework
