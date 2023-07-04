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

#include "frameworks/core/components_ng/svg/parse/svg_fe_composite.h"

#include "include/effects/SkArithmeticImageFilter.h"

#include "base/utils/utils.h"
#include "frameworks/core/components/declaration/svg/svg_fe_composite_declaration.h"

namespace OHOS::Ace::NG {

RefPtr<SvgNode> SvgFeComposite::Create()
{
    return AceType::MakeRefPtr<SvgFeComposite>();
}

SvgFeComposite::SvgFeComposite() : SvgFe()
{
    declaration_ = AceType::MakeRefPtr<SvgFeCompositeDeclaration>();
    declaration_->Init();
    declaration_->InitializeStyle();
}

void SvgFeComposite::OnAsImageFilter(sk_sp<SkImageFilter>& imageFilter,
    const ColorInterpolationType& srcColor, ColorInterpolationType& currentColor) const
{
    auto declaration = AceType::DynamicCast<SvgFeCompositeDeclaration>(declaration_);
    CHECK_NULL_VOID_NOLOG(declaration);
    if (declaration->GetOperatorType() != FeOperatorType::FE_ARITHMETIC) {
        // this version skia not support SkBlendImageFilters
        return;
    }

    auto foreImageFilter = MakeImageFilter(declaration->GetIn(), imageFilter);
    auto backImageFilter = MakeImageFilter(declaration->GetIn2(), imageFilter);
    ConverImageFilterColor(foreImageFilter, srcColor, currentColor);
    ConverImageFilterColor(backImageFilter, srcColor, currentColor);

    imageFilter = SkArithmeticImageFilter::Make(
        declaration->GetK1(), declaration->GetK2(), declaration->GetK3(), declaration->GetK4(),
        true, backImageFilter, foreImageFilter, nullptr);

    ConverImageFilterColor(imageFilter, srcColor, currentColor);
}

} // namespace OHOS::Ace::NG
