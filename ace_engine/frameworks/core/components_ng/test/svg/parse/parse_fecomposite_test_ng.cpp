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

#include <string>
#define private public
#define protected public

#include "gtest/gtest.h"

#include "core/components/common/properties/color.h"
#include "core/components/declaration/svg/svg_fe_composite_declaration.h"
#include "core/components_ng/svg/parse/svg_fe_composite.h"
#include "core/components_ng/svg/parse/svg_filter.h"
#include "core/components_ng/svg/parse/svg_svg.h"
#include "core/components_ng/svg/svg_dom.h"
#include "core/components_ng/test/svg/parse/svg_const.h"
#include "core/components/common/layout/constants.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::string SVG_LABEL =
    "<svg height=\"900\" width=\"900\"><filter id=\"composite\" y=\"0\" x=\"0\" width=\"100%\" "
    "height=\"100%\"><feComposite in2=\"B\" "
    "in=\"SourceGraphic\" operator=\"in\" /></filter><ellipse cx=\"100\" cy=\"87\" rx=\"75\" ry=\"87\" fill=\"red\" "
    "filter=\"url(#composite)\"/></svg>";
} // namespace
class ParseFeCompositeTestNg : public testing::Test {};

/**
 * @tc.name: ParseTest001
 * @tc.desc: parse FeComposite label
 * @tc.type: FUNC
 */

HWTEST_F(ParseFeCompositeTestNg, ParseTest001, TestSize.Level1)
{
    auto svgStream = SkMemoryStream::MakeCopy(SVG_LABEL.c_str(), SVG_LABEL.length());
    EXPECT_NE(svgStream, nullptr);
    auto svgDom = SvgDom::CreateSvgDom(*svgStream, Color::BLACK);
    auto svg = AceType::DynamicCast<SvgSvg>(svgDom->root_);
    EXPECT_GT(svg->children_.size(), 0);
    auto svgFilter = AceType::DynamicCast<SvgFilter>(svg->children_.at(0));
    EXPECT_NE(svgFilter, nullptr);
    auto svgFeComposite = AceType::DynamicCast<SvgFeComposite>(svgFilter->children_.at(0));
    EXPECT_NE(svgFeComposite, nullptr);
    auto feCompositsDeclaration = AceType::DynamicCast<SvgFeCompositeDeclaration>(svgFeComposite->declaration_);
    EXPECT_NE(feCompositsDeclaration, nullptr);
    RSCanvas rSCanvas;
    svgDom->DrawImage(rSCanvas, ImageFit::FILL, Size(IMAGE_COPONENT_WIDTH, IMAGE_COPONENT_HEIGHT), Color::RED);
    EXPECT_EQ(svgDom->svgSize_.IsValid(), true);
    EXPECT_EQ(svgDom->viewBox_.IsValid(), false);
}
} // namespace OHOS::Ace::NG