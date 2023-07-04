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

#include <cstdint>
#include <optional>

#include "gtest/gtest.h"

#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/flex/flex_layout_algorithm.h"
#include "core/components_ng/pattern/flex/flex_layout_pattern.h"
#include "core/components_ng/pattern/flex/flex_layout_property.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/constants.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

namespace {

const int START_INDEX = 0;
const int THREE_ITEM_SIZE = 3;
const int FOUR_ITEM_SIZE = 4;
const int DISPLAYPRIORITY_TWO = 2; 

const float RK356_WIDTH = 720.0f;
const float RK356_HEIGHT = 1136.0f;
const float ZERO = 0.0f;
const float LAYOUT_WEIGHT_ONE = 1.0f;

const float SMALLER_ITEM_HEIGHT = 45.0f;
const float SMALL_ITEM_WIDTH = 150.0f;
const float SMALL_ITEM_HEIGHT = 60.0f;

const float BIG_ITEM_WIDTH = 180.0f;
const float BIG_ITEM_HEIGHT = 75.0f;

const float ROW_HEIGHT = 120.0f;
const float COLUMN_HEIGHT = 150.0f;

const float TWENTY_PERCENT_WIDTH = 0.2f * RK356_WIDTH;
const float TWENTY_PERCENT_COLUMN_HEIGHT = 0.2f * COLUMN_HEIGHT;
const float HALF_PERCENT_WIDTH = 0.5f * RK356_WIDTH;
const float SIXTY_PERCENT_WIDTH = 0.6f * RK356_WIDTH;

const SizeF CONTAINER_SIZE(RK356_WIDTH, RK356_HEIGHT);
const SizeF SMALL_ITEM_SIZE(SMALL_ITEM_WIDTH, SMALL_ITEM_HEIGHT);
const SizeF BIG_ITEM_SIZE(BIG_ITEM_WIDTH, BIG_ITEM_HEIGHT);

const OffsetF OFFSET_TOP_LEFT = OffsetF(ZERO, ZERO);
const OffsetF OFFSET_TOP_MIDDLE = OffsetF(RK356_WIDTH / 2.0f, ZERO);
const OffsetF OFFSET_TOP_RIGHT = OffsetF(RK356_WIDTH, ZERO);

const OffsetF OFFSET_CENTER_LEFT = OffsetF(ZERO, RK356_HEIGHT / 2.0f);
const OffsetF OFFSET_CENTER_MIDDLE = OffsetF(RK356_WIDTH / 2.0f, RK356_HEIGHT / 2.0f);
const OffsetF OFFSET_CENTER_RIGHT = OffsetF(RK356_WIDTH, RK356_HEIGHT / 2.0f);

const OffsetF OFFSET_BOTTOM_LEFT = OffsetF(ZERO, RK356_HEIGHT);
const OffsetF OFFSET_BOTTOM_MIDDLE = OffsetF(RK356_WIDTH / 2.0f, RK356_HEIGHT);
const OffsetF OFFSET_BOTTOM_RIGHT = OffsetF(RK356_WIDTH, RK356_HEIGHT);

const float NOPADDING = 0.0f;

} // namespace

class FlexLayoutTestNg : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
};

void FlexLayoutTestNg::SetUp() {}

void FlexLayoutTestNg::TearDown() {}
/**
 * @tc.name: FlexRowLayoutTest001
 * @tc.desc: Set two texts with size in row and check them.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest001, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Center})
          Text("Text 1")
            .width(100)
            .height(40)
            .backgroundColor(0xF5DEB3)
            .textAlign(TextAlign.Center)
          Text("Text 2")
            .width(100)
            .height(40)
            .backgroundColor(0xD2B48C)
            .textAlign(TextAlign.Center)
        }.height(80)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    auto firstFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    firstGeometryNode->Reset();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(firstFrameNode, firstGeometryNode, firstFrameNode->GetLayoutProperty());
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    firstLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    auto boxLayoutAlgorithm = firstFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(firstFrameNode);
    layoutWrapper->AppendChild(firstLayoutWrapper);

    auto secondFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> secondGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    secondGeometryNode->Reset();
    RefPtr<LayoutWrapper> secondLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(secondFrameNode, secondGeometryNode, secondFrameNode->GetLayoutProperty());
    secondLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    secondLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    secondLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    secondLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(secondFrameNode);
    layoutWrapper->AppendChild(secondLayoutWrapper);
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));

    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(
        firstLayoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF(0.0f, (ROW_HEIGHT - SMALL_ITEM_HEIGHT) / 2));
    EXPECT_EQ(secondLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(secondLayoutWrapper->GetGeometryNode()->GetFrameOffset(),
        OffsetF(SMALL_ITEM_WIDTH, (ROW_HEIGHT - SMALL_ITEM_HEIGHT) / 2));
}

/**
 * @tc.name: FlexRowLayoutTest002
 * @tc.desc: Set two items with different constraint size.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest002, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;
    parentLayoutConstraint.selfIdealSize.SetSize(SizeF(RK356_WIDTH, ROW_HEIGHT));

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Start}) {
            Text("Text 1")
            .backgroundColor(0xF5DEB3)
            .textAlign(TextAlign.Center)
            .constraintSize({minWidth : 120, minHeight: 50})
            }
            Text("Text 2")
            .width(120)
            .height(50)
            .backgroundColor(0xD2B48C)
            .textAlign(TextAlign.Center)
            .constraintSize({maxWidth : 100, maxHeight: 40})
        }
        .height(80)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    auto firstFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    firstGeometryNode->Reset();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(firstFrameNode, firstGeometryNode, firstFrameNode->GetLayoutProperty());
    auto firstLayoutConstraint = childLayoutConstraint;
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(firstLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    firstLayoutWrapper->GetLayoutProperty()->UpdateCalcMinSize(
        CalcSize(CalcLength(BIG_ITEM_WIDTH), CalcLength(BIG_ITEM_HEIGHT)));

    auto boxLayoutAlgorithm = firstFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(firstFrameNode);
    layoutWrapper->AppendChild(firstLayoutWrapper);

    auto secondFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> secondGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    secondGeometryNode->Reset();
    RefPtr<LayoutWrapper> secondLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(secondFrameNode, secondGeometryNode, secondFrameNode->GetLayoutProperty());
    auto secondLayoutConstraint = childLayoutConstraint;
    secondLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(secondLayoutConstraint);
    secondLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(BIG_ITEM_WIDTH), CalcLength(BIG_ITEM_HEIGHT)));
    secondLayoutWrapper->GetLayoutProperty()->UpdateCalcMaxSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    secondLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    secondLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(secondFrameNode);
    layoutWrapper->AppendChild(secondLayoutWrapper);
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));

    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameSize(), BIG_ITEM_SIZE);
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);
    EXPECT_EQ(secondLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(secondLayoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF(BIG_ITEM_WIDTH, 0.0f));
}

/**
 * @tc.name: FlexRowLayoutTest003
 * @tc.desc: Set four texts with 20% of container size in row direction, container has no padding.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest003, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW_REVERSE);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);

    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(RK356_WIDTH, ROW_HEIGHT);
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Reverse, alignItems: ItemAlign.Start}) {
            Text('1').width('20%').height(40).backgroundColor(0xF5DEB3)
            Text('2').width('20%').height(40).backgroundColor(0xD2B48C)
            Text('3').width('20%').height(40).backgroundColor(0xF5DEB3)
            Text('4').width('20%').height(40).backgroundColor(0xD2B48C)
        }
        .height(80)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < FOUR_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    for (int32_t i = START_INDEX; i < FOUR_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(i);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
        EXPECT_EQ(childOffset, OffsetF(RK356_WIDTH - (i + 1) * TWENTY_PERCENT_WIDTH, 0.0f));
    }
}

/**
 * @tc.name: FlexColumnLayoutTest001
 * @tc.desc: Set two texts with size in column and check them.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexColumnLayoutTest001, TestSize.Level1)
{
    auto columnFrameNode =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(columnFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(columnFrameNode, geometryNode, columnFrameNode->GetLayoutProperty());

    auto columnLayoutPattern = columnFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(columnLayoutPattern == nullptr);
    auto columnLayoutProperty = columnLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(columnLayoutProperty == nullptr);
    columnLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
    columnLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    auto columnLayoutAlgorithm = columnLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(columnLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(columnLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Column, alignItems: ItemAlign.Center}){
          Text("Text 1")
            .width(100)
            .height(40)
            .backgroundColor(0xF5DEB3)
            .textAlign(TextAlign.Center)
          Text("Text 2")
            .width(100)
            .height(40)
            .backgroundColor(0xD2B48C)
            .textAlign(TextAlign.Center)
        }.height(80)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    auto firstFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    firstGeometryNode->Reset();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(firstFrameNode, firstGeometryNode, firstFrameNode->GetLayoutProperty());
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    firstLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    auto boxLayoutAlgorithm = firstFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    columnFrameNode->AddChild(firstFrameNode);
    layoutWrapper->AppendChild(firstLayoutWrapper);

    auto secondFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> secondGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    secondGeometryNode->Reset();
    RefPtr<LayoutWrapper> secondLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(secondFrameNode, secondGeometryNode, secondFrameNode->GetLayoutProperty());
    secondLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    secondLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    secondLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    secondLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    columnFrameNode->AddChild(secondFrameNode);
    layoutWrapper->AppendChild(secondLayoutWrapper);
    columnLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    columnLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));

    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(
        firstLayoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF((RK356_WIDTH - SMALL_ITEM_WIDTH) / 2, 0.0f));
    EXPECT_EQ(secondLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(secondLayoutWrapper->GetGeometryNode()->GetFrameOffset(),
        OffsetF((RK356_WIDTH - SMALL_ITEM_WIDTH) / 2, SMALL_ITEM_HEIGHT));
}

/**
 * @tc.name: FlexColumnLayoutTest002
 * @tc.desc: Set two items in column with different constraint size.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexColumnLayoutTest002, TestSize.Level1)
{
    auto columnFrameNode =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(columnFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(columnFrameNode, geometryNode, columnFrameNode->GetLayoutProperty());

    auto columnLayoutPattern = columnFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(columnLayoutPattern == nullptr);
    auto columnLayoutProperty = columnLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(columnLayoutProperty == nullptr);
    columnLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
    columnLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));
    auto columnLayoutAlgorithm = columnLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(columnLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(columnLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;
    parentLayoutConstraint.selfIdealSize.SetSize(SizeF(RK356_WIDTH, COLUMN_HEIGHT));

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Column, alignItems: ItemAlign.Start}) {
            Text("Text 1")
            .backgroundColor(0xF5DEB3)
            .textAlign(TextAlign.Center)
            .constraintSize({minWidth : 120, minHeight: 50})
            }
            Text("Text 2")
            .width(120)
            .height(50)
            .backgroundColor(0xD2B48C)
            .textAlign(TextAlign.Center)
            .constraintSize({maxWidth : 100, maxHeight: 40})
        }
        .height(100)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    auto firstFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    firstGeometryNode->Reset();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(firstFrameNode, firstGeometryNode, firstFrameNode->GetLayoutProperty());
    auto firstLayoutConstraint = childLayoutConstraint;
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(firstLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    firstLayoutWrapper->GetLayoutProperty()->UpdateCalcMinSize(
        CalcSize(CalcLength(BIG_ITEM_WIDTH), CalcLength(BIG_ITEM_HEIGHT)));

    auto boxLayoutAlgorithm = firstFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    columnFrameNode->AddChild(firstFrameNode);
    layoutWrapper->AppendChild(firstLayoutWrapper);

    auto secondFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> secondGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    secondGeometryNode->Reset();
    RefPtr<LayoutWrapper> secondLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(secondFrameNode, secondGeometryNode, secondFrameNode->GetLayoutProperty());
    auto secondLayoutConstraint = childLayoutConstraint;
    secondLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(secondLayoutConstraint);
    secondLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(BIG_ITEM_WIDTH), CalcLength(BIG_ITEM_HEIGHT)));
    secondLayoutWrapper->GetLayoutProperty()->UpdateCalcMaxSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    secondLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    secondLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    columnFrameNode->AddChild(secondFrameNode);
    layoutWrapper->AppendChild(secondLayoutWrapper);
    columnLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    columnLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));

    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameSize(), BIG_ITEM_SIZE);
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);
    EXPECT_EQ(secondLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(secondLayoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF(0.0f, BIG_ITEM_HEIGHT));
}

/**
 * @tc.name: FlexColumnLayoutTest003
 * @tc.desc: Set four texts with 20% of container size in column direction, container has no padding.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexColumnLayoutTest003, TestSize.Level1)
{
    auto columnFrameNode =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(columnFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(columnFrameNode, geometryNode, columnFrameNode->GetLayoutProperty());

    auto columnLayoutPattern = columnFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(columnLayoutPattern == nullptr);
    auto columnLayoutProperty = columnLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(columnLayoutProperty == nullptr);
    columnLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN_REVERSE);
    columnLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));
    auto columnLayoutAlgorithm = columnLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(columnLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(columnLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);

    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(RK356_WIDTH, ROW_HEIGHT);
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.ColumnReverse, alignItems: ItemAlign.Start}) {
            Text('1').width('20%').height('20%').backgroundColor(0xF5DEB3)
            Text('2').width('20%').height('20%').backgroundColor(0xD2B48C)
            Text('3').width('20%').height('20%').backgroundColor(0xF5DEB3)
            Text('4').width('20%').height('20%').backgroundColor(0xD2B48C)
        }
        .height(100)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < FOUR_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(TWENTY_PERCENT_COLUMN_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        columnFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    columnLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    columnLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    for (int32_t i = START_INDEX; i < FOUR_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(i);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, TWENTY_PERCENT_COLUMN_HEIGHT));
        EXPECT_EQ(childOffset, OffsetF(0.0f, COLUMN_HEIGHT - (i + 1) * TWENTY_PERCENT_COLUMN_HEIGHT));
    }
}

/**
 * @tc.name: FlexRowLayoutTest005
 * @tc.desc: Set three texts with size in row and check the alignItems is Start and justifyContent is Start.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest005, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateMainAxisAlign(FlexAlign::FLEX_START);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Start, justifyContent: FlexAlign.Start}) {
            Text('1').width('20%').height(40).backgroundColor(0xDDDDDD)
            Text('2').width('20%').height(40).backgroundColor(0xDFFFFF)
            Text('3').width('20%').height(40).backgroundColor(0xF5DEB3)
        }
        .height(80)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(i);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
        EXPECT_EQ(childOffset, OffsetF(i * TWENTY_PERCENT_WIDTH, 0.0f));
    }
}

/**
 * @tc.name: FlexRowLayoutTest006
 * @tc.desc: Set three texts with size in row and check the alignItems is Center and justifyContent is Center.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest006, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateMainAxisAlign(FlexAlign::CENTER);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center}) {
            Text('1').width('20%').height(40).backgroundColor(0xDDDDDD)
            Text('2').width('20%').height(40).backgroundColor(0xDFFFFF)
            Text('3').width('20%').height(40).backgroundColor(0xF5DEB3)
        }
        .height(80)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto horizontalRemaining = RK356_WIDTH - THREE_ITEM_SIZE * TWENTY_PERCENT_WIDTH;
    auto verticalRemaining = ROW_HEIGHT - SMALL_ITEM_HEIGHT;
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(i);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
        EXPECT_EQ(childOffset, OffsetF(horizontalRemaining / 2 + i * TWENTY_PERCENT_WIDTH, verticalRemaining / 2));
    }
}

/**
 * @tc.name: FlexRowLayoutTest007
 * @tc.desc: Set three texts with size in row and check the alignItems is End and justifyContent is End.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest007, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateMainAxisAlign(FlexAlign::FLEX_END);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_END);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.End, justifyContent: FlexAlign.End}) {
            Text('1').width('20%').height(40).backgroundColor(0xDDDDDD)
            Text('2').width('20%').height(40).backgroundColor(0xDFFFFF)
            Text('3').width('20%').height(40).backgroundColor(0xF5DEB3)
        }
        .height(80)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto horizontalRemaining = RK356_WIDTH - THREE_ITEM_SIZE * TWENTY_PERCENT_WIDTH;
    auto verticalRemaining = ROW_HEIGHT - SMALL_ITEM_HEIGHT;
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(i);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
        EXPECT_EQ(childOffset, OffsetF(horizontalRemaining + i * TWENTY_PERCENT_WIDTH, verticalRemaining));
    }
}

/**
 * @tc.name: FlexRowLayoutTest008
 * @tc.desc: Set three texts with size in row and check the alignItems is Stretch and justifyContent is SpaceBetween.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest008, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateMainAxisAlign(FlexAlign::SPACE_BETWEEN);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::STRETCH);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Stretch, justifyContent: FlexAlign.SpaceBetween}) {
            Text('1').width('20%').height(40).backgroundColor(0xDDDDDD)
            Text('2').width('20%').height(40).backgroundColor(0xDFFFFF)
            Text('3').width('20%').height(40).backgroundColor(0xF5DEB3)
        }
        .height(80)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto horizontalRemaining = RK356_WIDTH - THREE_ITEM_SIZE * TWENTY_PERCENT_WIDTH;
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(i);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, ROW_HEIGHT));
        EXPECT_EQ(childOffset, OffsetF(i * (TWENTY_PERCENT_WIDTH + horizontalRemaining / 2), 0.0f));
    }
}

/**
 * @tc.name: FlexRowLayoutTest009
 * @tc.desc: Set three texts with size in row and check the alignItems is Baseline and justifyContent is SpaceEvenly.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest009, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateMainAxisAlign(FlexAlign::SPACE_EVENLY);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::BASELINE);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, ItemAlign.Baseline, justifyContent: FlexAlign.SpaceEvenly}) {
            Text('1').width('20%').height(40).backgroundColor(0xDDDDDD)
            Text('2').width('20%').height(40).backgroundColor(0xDFFFFF)
            Text('3').width('20%').height(40).backgroundColor(0xF5DEB3)
        }
        .height(80)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto horizontalRemaining = RK356_WIDTH - THREE_ITEM_SIZE * TWENTY_PERCENT_WIDTH;
    horizontalRemaining = horizontalRemaining / (THREE_ITEM_SIZE + 1);
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(i);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
        EXPECT_EQ(childOffset, OffsetF(i * TWENTY_PERCENT_WIDTH + (i + 1) * horizontalRemaining, 0.0f));
    }
}

/**
 * @tc.name: FlexRowLayoutTest010
 * @tc.desc: Set three texts with size in row and check the alignItems is Start and justifyContent is SpaceAround.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest010, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateMainAxisAlign(FlexAlign::SPACE_AROUND);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, ItemAlign.Start, justifyContent: FlexAlign.SpaceAround}) {
            Text('1').width('20%').height(40).backgroundColor(0xDDDDDD)
            Text('2').width('20%').height(40).backgroundColor(0xDFFFFF)
            Text('3').width('20%').height(40).backgroundColor(0xF5DEB3)
        }
        .height(80)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto horizontalRemaining = RK356_WIDTH - THREE_ITEM_SIZE * TWENTY_PERCENT_WIDTH;
    horizontalRemaining = horizontalRemaining / THREE_ITEM_SIZE;

    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(firstChildSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OffsetF(horizontalRemaining / 2, 0.0f));
    for (int32_t i = START_INDEX + 1; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(i);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
        EXPECT_EQ(
            childOffset, OffsetF(horizontalRemaining / 2 + i * (TWENTY_PERCENT_WIDTH + horizontalRemaining), 0.0f));
    }
}

/**
 * @tc.name: wrapRowLayoutTest001
 * @tc.desc: Test layout of flex wrap.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, wrapRowLayoutTest001, TestSize.Level1)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);
    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::HORIZONTAL);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::START);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::START);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({ direction: FlexDirection.Row, wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Start, alignItems:
       ItemAlign.Start, alignContent: FlexAlign.Start }) { Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
            Text('2').width('50%').height(40).backgroundColor(0xCCCCCC)
            Text('3').width('50%').height(40).backgroundColor(0x000000)
        }
        .size({ width: '100%', height: 100})
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        wrapFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(firstChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OFFSET_TOP_LEFT);

    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(secondChildOffset, OffsetF(HALF_PERCENT_WIDTH, 0.0f));

    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(thirdChildOffset, OffsetF(0.0f, SMALL_ITEM_HEIGHT));
}

/**
 * @tc.name: wrapRowLayoutTest002
 * @tc.desc: Test layout of different flex wrap settings.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, wrapRowLayoutTest002, TestSize.Level1)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);
    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::HORIZONTAL_REVERSE);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::CENTER);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::CENTER);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::CENTER);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({ direction: FlexDirection.Row, wrap: FlexWrap.WrapReverse, justifyContent: FlexAlign.Center, alignItems:
       ItemAlign.Center, alignContent: FlexAlign.Center }) { Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
            Text('2').width('50%').height(40).backgroundColor(0xCCCCCC)
            Text('3').width('50%').height(40).backgroundColor(0x000000)
        }
        .size({ width: '100%', height: 100})
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        wrapFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto verticalRemaining = COLUMN_HEIGHT - 2 * SMALL_ITEM_HEIGHT;
    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(firstChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OffsetF(HALF_PERCENT_WIDTH, verticalRemaining / 2));

    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(secondChildOffset, OffsetF(0.0f, verticalRemaining / 2));

    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(thirdChildOffset, OffsetF(HALF_PERCENT_WIDTH / 2, verticalRemaining / 2 + SMALL_ITEM_HEIGHT));
}

/**
 * @tc.name: wrapRowLayoutTest003
 * @tc.desc: Test layout of different flex wrap settings.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, wrapRowLayoutTest003, TestSize.Level1)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);
    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::VERTICAL);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::END);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::END);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::END);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({ direction: FlexDirection.Column, wrap: FlexWrap.Wrap, justifyContent: FlexAlign.End, alignItems:
       ItemAlign.End, alignContent: FlexAlign.End }) { Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
            Text('2').width('50%').height(40).backgroundColor(0xCCCCCC)
            Text('3').width('50%').height(40).backgroundColor(0x000000)
        }
        .size({ width: '100%', height: 100})
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        wrapFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(firstChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OffsetF(0.0f, COLUMN_HEIGHT - 2 * SMALL_ITEM_HEIGHT));

    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(secondChildOffset, OffsetF(0.0f, COLUMN_HEIGHT - SMALL_ITEM_HEIGHT));

    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(thirdChildOffset, OffsetF(HALF_PERCENT_WIDTH, COLUMN_HEIGHT - SMALL_ITEM_HEIGHT));
}

/**
 * @tc.name: wrapRowLayoutTest004
 * @tc.desc: Test layout of different flex wrap settings.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, wrapRowLayoutTest004, TestSize.Level1)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);
    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::VERTICAL_REVERSE);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::SPACE_AROUND);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::SPACE_AROUND);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::STRETCH);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({ direction: FlexDirection.Column, wrap: FlexWrap.WrapReverse, justifyContent: FlexAlign.SpaceAround,
       alignItems: ItemAlign.Stretch, alignContent: FlexAlign.SpaceAround }) {
            Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
            Text('2').width('50%').height(40).backgroundColor(0xCCCCCC)
            Text('3').width('50%').height(40).backgroundColor(0x000000)
        }
        .size({ width: '100%', height: 100})
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        wrapFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    auto verticalRemaining = COLUMN_HEIGHT - 2 * SMALL_ITEM_HEIGHT;
    EXPECT_EQ(firstChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OffsetF(0.0f, COLUMN_HEIGHT - verticalRemaining / 4 - SMALL_ITEM_HEIGHT));

    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(secondChildOffset, OffsetF(0.0f, verticalRemaining / 4));

    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(thirdChildOffset, OffsetF(HALF_PERCENT_WIDTH, (COLUMN_HEIGHT - SMALL_ITEM_HEIGHT) / 2));
}

/**
 * @tc.name: wrapRowLayoutTest005
 * @tc.desc: Test layout of different flex wrap settings.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, wrapRowLayoutTest005, TestSize.Level1)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);
    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::HORIZONTAL);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::SPACE_BETWEEN);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::SPACE_BETWEEN);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::BASELINE);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({ direction: FlexDirection.Row, wrap: FlexWrap.Wrap, justifyContent: FlexAlign.SpaceBetween, alignItems:
       ItemAlign.Baseline, alignContent: FlexAlign.SpaceBetween }) {
            Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
            Text('2').width('50%').height(40).backgroundColor(0xCCCCCC)
            Text('3').width('50%').height(40).backgroundColor(0x000000)
        }
        .size({ width: '100%', height: 100})
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        wrapFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(firstChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OFFSET_TOP_LEFT);

    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(secondChildOffset, OffsetF(HALF_PERCENT_WIDTH, 0.0f));

    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(thirdChildOffset, OffsetF(0.0f, COLUMN_HEIGHT - SMALL_ITEM_HEIGHT));
}

/**
 * @tc.name: wrapRowLayoutTest006
 * @tc.desc: Test layout of different flex wrap settings.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, wrapRowLayoutTest006, TestSize.Level1)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);
    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::HORIZONTAL);
    wrapLayoutProperty->UpdateAlignment(WrapAlignment::SPACE_EVENLY);
    wrapLayoutProperty->UpdateMainAlignment(WrapAlignment::SPACE_EVENLY);
    wrapLayoutProperty->UpdateCrossAlignment(WrapAlignment::START);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({ direction: FlexDirection.Row, wrap: FlexWrap.Wrap, justifyContent: FlexAlign.SpaceEvenly, alignItems:
       ItemAlign.Start, alignContent: FlexAlign.SpaceEvenly }) {
            Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
            Text('2').width('50%').height(40).backgroundColor(0xCCCCCC)
            Text('3').width('50%').height(40).backgroundColor(0x000000)
        }
        .size({ width: '100%', height: 100})
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        wrapFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    auto verticalRemaining = COLUMN_HEIGHT - 2 * SMALL_ITEM_HEIGHT;
    EXPECT_EQ(firstChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OffsetF(0.0f, verticalRemaining / 3));

    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(secondChildOffset, OffsetF(HALF_PERCENT_WIDTH, verticalRemaining / 3));

    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(
        thirdChildOffset, OffsetF(HALF_PERCENT_WIDTH / 2, COLUMN_HEIGHT - SMALL_ITEM_HEIGHT - verticalRemaining / 3));
}

/**
 * @tc.name: wrapRowLayoutTest007
 * @tc.desc: Test the sum of width is bigger than flex size
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, wrapRowLayoutTest007, TestSize.Level1)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);
    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::HORIZONTAL);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({ direction: FlexDirection.Row, wrap: FlexWrap.Wrap}) {
            Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
            Text('2').width('60%').height(30).backgroundColor(0xCCCCCC)
            Text('3').width('50%').height(40).backgroundColor(0x000000)
        }
        .size({ width: '100%', height: 100})
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        wrapFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    secondChildWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SIXTY_PERCENT_WIDTH), CalcLength(SMALLER_ITEM_HEIGHT)));
    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(firstChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OFFSET_TOP_LEFT);

    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(SIXTY_PERCENT_WIDTH, SMALLER_ITEM_HEIGHT));
    EXPECT_EQ(secondChildOffset, OffsetF(0.0f, SMALL_ITEM_HEIGHT));

    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(thirdChildOffset, OffsetF(0.0f, SMALL_ITEM_HEIGHT + SMALLER_ITEM_HEIGHT));
}

/**
 * @tc.name: wrapRowLayoutTest008
 * @tc.desc: Test the sum of width is bigger than flex width in column
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, wrapRowLayoutTest008, TestSize.Level1)
{
    auto wrapFrameNode = FrameNode::CreateFrameNode(V2::FLEX_ETS_TAG, 0, AceType::MakeRefPtr<FlexLayoutPattern>(true));
    EXPECT_FALSE(wrapFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(wrapFrameNode, geometryNode, wrapFrameNode->GetLayoutProperty());

    auto wrapLayoutPattern = wrapFrameNode->GetPattern<FlexLayoutPattern>();
    EXPECT_FALSE(wrapLayoutPattern == nullptr);
    auto wrapLayoutProperty = wrapLayoutPattern->GetLayoutProperty<FlexLayoutProperty>();
    EXPECT_FALSE(wrapLayoutProperty == nullptr);
    wrapLayoutProperty->UpdateWrapDirection(WrapDirection::VERTICAL);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));

    auto wrapLayoutAlgorithm = AccessibilityManager::MakeRefPtr<WrapLayoutAlgorithm>(false);
    EXPECT_FALSE(wrapLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(wrapLayoutAlgorithm));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Column, wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Start, alignItems:
       ItemAlign.Start, alignContent: FlexAlign.Start}) { Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
            Text('2').width('60%').height(30).backgroundColor(0xCCCCCC)
            Text('3').width('50%').height(40).backgroundColor(0x000000)
        }
        .size({ width: '100%', height: 100})
        .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        wrapFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    secondChildWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SIXTY_PERCENT_WIDTH), CalcLength(SMALLER_ITEM_HEIGHT)));
    wrapLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    wrapLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(firstChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OFFSET_TOP_LEFT);

    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(SIXTY_PERCENT_WIDTH, SMALLER_ITEM_HEIGHT));
    EXPECT_EQ(secondChildOffset, OffsetF(0.0f, SMALL_ITEM_HEIGHT));

    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(HALF_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(thirdChildOffset, OffsetF(SIXTY_PERCENT_WIDTH, 0.0f));
}

/**
 * @tc.name: FlexRowLayoutTest015
 * @tc.desc: Set 6 texts with 20% of container size in Row direction, container has no padding, the 4,5 has 1
 * layoutweight, the 6 has 2 displayPriority and 1 layoutweight.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest015, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);

    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(RK356_WIDTH, ROW_HEIGHT);
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Center}) {
        Text('1').width('20%').height(40).backgroundColor(0xFFFFFF)
        Text('2').width('20%').height(40).backgroundColor(0xEEEEEE)
        Text('3').width('20%').height(40).backgroundColor(0xDDDDDD)
        Text('4').width('20%').height(40).backgroundColor(0xCCCCCC).layoutWeight(1)
        Text('5').width('20%').height(40).backgroundColor(0xBBBBBB).layoutWeight(1)
        Text('6').width('20%').height(40).backgroundColor(0xAAAAAA).displayPriority(2).layoutWeight(1)
      }
      .height(80)
      .width('100%')
      .backgroundColor(0xAFEEEE)
    */
    int32_t index = 1;
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
        index++;
    }

    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE - 1; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutWeight(LAYOUT_WEIGHT_ONE);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
        index++;
    }

    auto sixthFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> sixthGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    sixthGeometryNode->Reset();
    RefPtr<LayoutWrapper> sixthLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(sixthFrameNode, sixthGeometryNode, sixthFrameNode->GetLayoutProperty());
    sixthLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    sixthLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateDisplayIndex(DISPLAYPRIORITY_TWO);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateLayoutWeight(LAYOUT_WEIGHT_ONE);
    auto boxLayoutAlgorithm = sixthFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    sixthLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(sixthFrameNode);
    layoutWrapper->AppendChild(sixthLayoutWrapper);

    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    index = 0;
    auto verticalRemaning = ROW_HEIGHT - SMALL_ITEM_HEIGHT;
    auto horizontalRemaining = RK356_WIDTH - THREE_ITEM_SIZE * TWENTY_PERCENT_WIDTH;
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
        EXPECT_EQ(childOffset, OffsetF(i * TWENTY_PERCENT_WIDTH, verticalRemaning / 2.0f));
        index++;
    }
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(horizontalRemaining / 3, SMALL_ITEM_HEIGHT));
        EXPECT_EQ(childOffset,
            OffsetF(THREE_ITEM_SIZE * TWENTY_PERCENT_WIDTH + i * horizontalRemaining / 3, verticalRemaning / 2.0f));
        index++;
    }
}
/**
 * @tc.name: FlexRowLayoutTest016
 * @tc.desc: Set former 3 texts with 40% of container size in Row direction, container has no padding, the latter 3
 * texts with 20% width, and 4,5 has 1 layoutweight, the 6 has 2 displayPriority and 1 layoutweight.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest016, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW_REVERSE);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);

    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(RK356_WIDTH, ROW_HEIGHT);
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.RowReverse, alignItems: ItemAlign.Center}) {
        Text('1').width('40%').height(40).backgroundColor(0xFFFFFF)
        Text('2').width('40%').height(40).backgroundColor(0xEEEEEE)
        Text('3').width('40%').height(40).backgroundColor(0xDDDDDD)
        Text('4').width('20%').height(40).backgroundColor(0xCCCCCC).layoutWeight(1)
        Text('5').width('20%').height(40).backgroundColor(0xBBBBBB).layoutWeight(1)
        Text('6').width('20%').height(40).backgroundColor(0xAAAAAA).displayPriority(2).layoutWeight(1)
      }
      .height(80)
      .width('100%')
      .backgroundColor(0xAFEEEE)
    */
    int32_t index = 1;
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(2 * TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
        index++;
    }

    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE - 1; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutWeight(LAYOUT_WEIGHT_ONE);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
        index++;
    }

    auto sixthFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> sixthGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    sixthGeometryNode->Reset();
    RefPtr<LayoutWrapper> sixthLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(sixthFrameNode, sixthGeometryNode, sixthFrameNode->GetLayoutProperty());
    sixthLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    sixthLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateDisplayIndex(DISPLAYPRIORITY_TWO);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateLayoutWeight(LAYOUT_WEIGHT_ONE);
    auto boxLayoutAlgorithm = sixthFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    sixthLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(sixthFrameNode);
    layoutWrapper->AppendChild(sixthLayoutWrapper);

    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    index = 0;
    for (int32_t i = START_INDEX; i < 2 * THREE_ITEM_SIZE - 1; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(0.0f, 0.0f));
        EXPECT_EQ(childOffset, OffsetF(RK356_WIDTH, ROW_HEIGHT / 2.0f));
        index++;
    }
    auto verticalRemaning = ROW_HEIGHT - SMALL_ITEM_HEIGHT;
    auto sixthChildWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    auto sixthChildSize = sixthChildWrapper->GetGeometryNode()->GetFrameSize();
    auto sixthChildOffset = sixthChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(sixthChildSize, SizeF(RK356_WIDTH / THREE_ITEM_SIZE, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(sixthChildOffset, OffsetF(RK356_WIDTH - RK356_WIDTH / 3, verticalRemaning / 2));
}

/**
 * @tc.name: FlexRowLayoutTest017
 * @tc.desc: Set three texts with different flexBasis, flexGrows, flexShrink.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest017, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateMainAxisAlign(FlexAlign::FLEX_START);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Start}) {
            Text('1').height(40).backgroundColor(0xDDDDDD).flexBasis(96)
            Text('2').width('20%').height(40).backgroundColor(0xDFFFFF).flexGrow(2)
            Text('3').width('20%').height(40).backgroundColor(0xF5DEB3).flexShrink(1)
        }
        .height(80)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    auto firstItemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> firstItemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    firstItemGeometryNode->Reset();
    RefPtr<LayoutWrapper> firstItemLayoutWrapper = AceType::MakeRefPtr<LayoutWrapper>(
        firstItemFrameNode, firstItemGeometryNode, firstItemFrameNode->GetLayoutProperty());
    firstItemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    firstItemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(), CalcLength(SMALL_ITEM_HEIGHT)));
    firstItemLayoutWrapper->GetLayoutProperty()->UpdateFlexBasis(Dimension(TWENTY_PERCENT_WIDTH));
    firstItemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    auto boxLayoutAlgorithm = firstItemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    firstItemLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(firstItemFrameNode);
    layoutWrapper->AppendChild(firstItemLayoutWrapper);

    auto secondItemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> secondItemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    secondItemGeometryNode->Reset();
    RefPtr<LayoutWrapper> secondItemLayoutWrapper = AceType::MakeRefPtr<LayoutWrapper>(
        secondItemFrameNode, secondItemGeometryNode, secondItemFrameNode->GetLayoutProperty());
    secondItemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    secondItemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    secondItemLayoutWrapper->GetLayoutProperty()->UpdateFlexGrow(2);
    secondItemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    secondItemLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(secondItemFrameNode);
    layoutWrapper->AppendChild(secondItemLayoutWrapper);

    auto thirdItemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 3, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> thirdItemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    thirdItemGeometryNode->Reset();
    RefPtr<LayoutWrapper> thirdItemLayoutWrapper = AceType::MakeRefPtr<LayoutWrapper>(
        thirdItemFrameNode, thirdItemGeometryNode, thirdItemFrameNode->GetLayoutProperty());
    thirdItemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    thirdItemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    thirdItemLayoutWrapper->GetLayoutProperty()->UpdateFlexShrink(1);
    thirdItemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    thirdItemLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(thirdItemFrameNode);
    layoutWrapper->AppendChild(thirdItemLayoutWrapper);

    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(firstChildSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(firstChildOffset, OFFSET_TOP_LEFT);
    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(SIXTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(secondChildOffset, OffsetF(TWENTY_PERCENT_WIDTH, 0.0f));
    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
    EXPECT_EQ(thirdChildOffset, OffsetF(SIXTY_PERCENT_WIDTH + TWENTY_PERCENT_WIDTH, 0.0f));
}

/**
 * @tc.name: FlexColumnLayoutTest004
 * @tc.desc: Set 6 texts with 20% of container size in Column direction, container has no padding, the 4,5 has 1
 * layoutweight, the 6 has 2 displayPriority and 1 layoutweight.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexColumnLayoutTest004, TestSize.Level1)
{
    auto columnFrameNode =
        FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(true));
    EXPECT_FALSE(columnFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(columnFrameNode, geometryNode, columnFrameNode->GetLayoutProperty());

    auto columnLayoutPattern = columnFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(columnLayoutPattern == nullptr);
    auto columnLayoutProperty = columnLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(columnLayoutProperty == nullptr);
    columnLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
    columnLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));
    auto columnLayoutAlgorithm = columnLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(columnLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(columnLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);

    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(RK356_WIDTH, ROW_HEIGHT);
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Column, alignItems: ItemAlign.Start}) {
        Text('1').width('20%').height(30).backgroundColor(0xFFFFFF)
        Text('2').width('20%').height(30).backgroundColor(0xEEEEEE)
        Text('3').width('20%').height(30).backgroundColor(0xDDDDDD)
        Text('4').width('20%').height(30).backgroundColor(0xCCCCCC).layoutWeight(1)
        Text('5').width('20%').height(30).backgroundColor(0xBBBBBB).layoutWeight(1)
        Text('6').width('20%').height(30).backgroundColor(0xAAAAAA).displayPriority(2).layoutWeight(1)
      }
      .height(100)
      .width('100%')
      .backgroundColor(0xAFEEEE)
    */
    int32_t index = 1;
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALLER_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        columnFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
        index++;
    }
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE - 1; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALLER_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutWeight(LAYOUT_WEIGHT_ONE);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        columnFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
        index++;
    }

    auto sixthFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> sixthGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    sixthGeometryNode->Reset();
    RefPtr<LayoutWrapper> sixthLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(sixthFrameNode, sixthGeometryNode, sixthFrameNode->GetLayoutProperty());
    sixthLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALLER_ITEM_HEIGHT)));
    sixthLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateDisplayIndex(DISPLAYPRIORITY_TWO);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateLayoutWeight(LAYOUT_WEIGHT_ONE);
    auto boxLayoutAlgorithm = sixthFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    sixthLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    columnFrameNode->AddChild(sixthFrameNode);
    layoutWrapper->AppendChild(sixthLayoutWrapper);

    columnLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    columnLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    index = 0;
    auto verticalRemaning = COLUMN_HEIGHT - 3 * SMALLER_ITEM_HEIGHT;
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, SMALLER_ITEM_HEIGHT));
        EXPECT_EQ(childOffset, OffsetF(0.0f, i * SMALLER_ITEM_HEIGHT));
        index++;
    }
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, verticalRemaning / 3));
        EXPECT_EQ(childOffset, OffsetF(0.0f, THREE_ITEM_SIZE * SMALLER_ITEM_HEIGHT + i * verticalRemaning / 3));
        index++;
    }
}
/**
 * @tc.name: FlexColumnLayoutTest005
 * @tc.desc: Set former 3 texts with 40% of container size in Row direction, container has no padding, the latter 3
 * texts with 20% width, and 4,5 has 1 layoutweight, the 6 has 2 displayPriority and 1 layoutweight.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexColumnLayoutTest005, TestSize.Level1)
{
    auto columnFrameNode =
        FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(true));
    EXPECT_FALSE(columnFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(columnFrameNode, geometryNode, columnFrameNode->GetLayoutProperty());

    auto columnLayoutPattern = columnFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(columnLayoutPattern == nullptr);
    auto columnLayoutProperty = columnLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(columnLayoutProperty == nullptr);
    columnLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN_REVERSE);
    columnLayoutProperty->UpdateMainAxisAlign(FlexAlign::FLEX_START);
    columnLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));
    auto columnLayoutAlgorithm = columnLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(columnLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(columnLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);

    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(RK356_WIDTH, ROW_HEIGHT);
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.ColumnReverse, alignItems: ItemAlign.Start}) {
        Text('1').width('20%').height(40).backgroundColor(0xFFFFFF)
        Text('2').width('20%').height(40).backgroundColor(0xEEEEEE)
        Text('3').width('20%').height(40).backgroundColor(0xDDDDDD)
        Text('4').width('20%').height(40).backgroundColor(0xCCCCCC).layoutWeight(1)
        Text('5').width('20%').height(40).backgroundColor(0xBBBBBB).layoutWeight(1)
        Text('6').width('20%').height(40).backgroundColor(0xAAAAAA).displayPriority(2).layoutWeight(1)
      }
      .height(100)
      .width('100%')
      .backgroundColor(0xAFEEEE)
    */
    int32_t index = 1;
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        columnFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
        index++;
    }

    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE - 1; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        itemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutWeight(LAYOUT_WEIGHT_ONE);
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        columnFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
        index++;
    }

    auto sixthFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, index, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> sixthGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    sixthGeometryNode->Reset();
    RefPtr<LayoutWrapper> sixthLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(sixthFrameNode, sixthGeometryNode, sixthFrameNode->GetLayoutProperty());
    sixthLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    sixthLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateDisplayIndex(DISPLAYPRIORITY_TWO);
    sixthLayoutWrapper->GetLayoutProperty()->UpdateLayoutWeight(LAYOUT_WEIGHT_ONE);
    auto boxLayoutAlgorithm = sixthFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    sixthLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    columnFrameNode->AddChild(sixthFrameNode);
    layoutWrapper->AppendChild(sixthLayoutWrapper);

    columnLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    columnLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    index = 0;
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(0.0f, 0.0f));
        EXPECT_EQ(childOffset, OffsetF(0.0f, COLUMN_HEIGHT));
        index++;
    }

    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE - 1; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(0.0f, 0.0f));
        EXPECT_EQ(childOffset, OffsetF(0.0f, COLUMN_HEIGHT));
        index++;
    }
    auto sixthChildWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    auto sixthChildSize = sixthChildWrapper->GetGeometryNode()->GetFrameSize();
    auto sixthChildOffset = sixthChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(sixthChildSize, SizeF(TWENTY_PERCENT_WIDTH, COLUMN_HEIGHT / 3));
    EXPECT_EQ(sixthChildOffset, OffsetF(0.0f, 2 * COLUMN_HEIGHT / 3));
}

/**
 * @tc.name: FlexColumnLayoutTest006
 * @tc.desc: Set three texts with different flexBasis, flexGrows, flexShrink.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexColumnLayoutTest006, TestSize.Level1)
{
    auto columnFrameNode =
        FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(true));
    EXPECT_FALSE(columnFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(columnFrameNode, geometryNode, columnFrameNode->GetLayoutProperty());

    auto columnLayoutPattern = columnFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(columnLayoutPattern == nullptr);
    auto columnLayoutProperty = columnLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(columnLayoutProperty == nullptr);
    columnLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
    columnLayoutProperty->UpdateMainAxisAlign(FlexAlign::FLEX_START);
    columnLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    auto columnLayoutAlgorithm = columnLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(columnLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(columnLayoutAlgorithm));
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;

    PaddingProperty noPadding;
    noPadding.left = CalcLength(NOPADDING);
    noPadding.right = CalcLength(NOPADDING);
    noPadding.top = CalcLength(NOPADDING);
    noPadding.bottom = CalcLength(NOPADDING);
    layoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = CONTAINER_SIZE;
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Column, alignItems: ItemAlign.Start}) {
            Text('1').width('20%').backgroundColor(0xDDDDDD).flexBasis(20)
            Text('2').width('20%').height('20%').backgroundColor(0xDFFFFF).flexGrow(2)
            Text('3').width('20%').height('20%').backgroundColor(0xF5DEB3).flexShrink(1)
        }
        .height(100)
        .width('100%')
        .backgroundColor(0xAFEEEE)
    */
    auto firstItemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> firstItemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    firstItemGeometryNode->Reset();
    RefPtr<LayoutWrapper> firstItemLayoutWrapper = AceType::MakeRefPtr<LayoutWrapper>(
        firstItemFrameNode, firstItemGeometryNode, firstItemFrameNode->GetLayoutProperty());
    firstItemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    firstItemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength()));
    firstItemLayoutWrapper->GetLayoutProperty()->UpdateFlexBasis(Dimension(SMALL_ITEM_HEIGHT / 2));
    firstItemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    auto boxLayoutAlgorithm = firstItemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    firstItemLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    columnFrameNode->AddChild(firstItemFrameNode);
    layoutWrapper->AppendChild(firstItemLayoutWrapper);

    auto secondItemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> secondItemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    secondItemGeometryNode->Reset();
    RefPtr<LayoutWrapper> secondItemLayoutWrapper = AceType::MakeRefPtr<LayoutWrapper>(
        secondItemFrameNode, secondItemGeometryNode, secondItemFrameNode->GetLayoutProperty());
    secondItemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    secondItemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT / 2)));
    secondItemLayoutWrapper->GetLayoutProperty()->UpdateFlexGrow(2);
    secondItemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    secondItemLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    columnFrameNode->AddChild(secondItemFrameNode);
    layoutWrapper->AppendChild(secondItemLayoutWrapper);

    auto thirdItemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 3, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> thirdItemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    thirdItemGeometryNode->Reset();
    RefPtr<LayoutWrapper> thirdItemLayoutWrapper = AceType::MakeRefPtr<LayoutWrapper>(
        thirdItemFrameNode, thirdItemGeometryNode, thirdItemFrameNode->GetLayoutProperty());
    thirdItemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
    thirdItemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT / 2)));
    thirdItemLayoutWrapper->GetLayoutProperty()->UpdateFlexShrink(1);
    thirdItemLayoutWrapper->GetLayoutProperty()->UpdatePadding(noPadding);
    thirdItemLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    columnFrameNode->AddChild(thirdItemFrameNode);
    layoutWrapper->AppendChild(thirdItemLayoutWrapper);

    columnLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    columnLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    auto firstChildWrapper = layoutWrapper->GetOrCreateChildByIndex(0);
    auto firstChildSize = firstChildWrapper->GetGeometryNode()->GetFrameSize();
    auto firstChildOffset = firstChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(firstChildSize, SizeF(TWENTY_PERCENT_WIDTH, ROW_HEIGHT / 4));
    EXPECT_EQ(firstChildOffset, OFFSET_TOP_LEFT);
    auto secondChildWrapper = layoutWrapper->GetOrCreateChildByIndex(1);
    auto secondChildSize = secondChildWrapper->GetGeometryNode()->GetFrameSize();
    auto secondChildOffset = secondChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(secondChildSize, SizeF(TWENTY_PERCENT_WIDTH, ROW_HEIGHT / 4 * 3));
    EXPECT_EQ(secondChildOffset, OffsetF(0.0f, ROW_HEIGHT / 4));
    auto thirdChildWrapper = layoutWrapper->GetOrCreateChildByIndex(2);
    auto thirdChildSize = thirdChildWrapper->GetGeometryNode()->GetFrameSize();
    auto thirdChildOffset = thirdChildWrapper->GetGeometryNode()->GetFrameOffset();
    EXPECT_EQ(thirdChildSize, SizeF(TWENTY_PERCENT_WIDTH, ROW_HEIGHT / 4));
    EXPECT_EQ(thirdChildOffset, OffsetF(0.0f, COLUMN_HEIGHT - ROW_HEIGHT / 4));
}

/**
 * @tc.name: FlexRowLayoutTest018
 * @tc.desc: Set 6 texts in Row direction, container has no padding, the former 3 has 50% width
 * the latter 3 has 2 displayPriority with 20% width.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest018, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));
    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(RK356_WIDTH, ROW_HEIGHT);
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Center}) {
        Text('1').width('50%').height(40).backgroundColor(0xFFFFFF).flexGrow(2)
        Text('2').width('50%').height(40).backgroundColor(0xEEEEEE).flexGrow(0)
        Text('3').width('50%').height(40).backgroundColor(0xDDDDDD).flexShrink(1)
        Text('4').width('20%').height(40).backgroundColor(0xCCCCCC).displayPriority(2).flexGrow(2)
        Text('5').width('20%').height(40).backgroundColor(0xBBBBBB).displayPriority(2).flexGrow(0)
        Text('6').width('20%').height(40).backgroundColor(0xAAAAAA).displayPriority(2).flexShrink(1)
      }
      .height(80)
      .width('100%')
      .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < 2 * THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        if (i >= THREE_ITEM_SIZE) {
            itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
                CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
            itemLayoutWrapper->GetLayoutProperty()->UpdateDisplayIndex(DISPLAYPRIORITY_TWO);
        }
        if (i == 0 || i == 3) {
            itemLayoutWrapper->GetLayoutProperty()->UpdateFlexGrow(2);
        } else if (i == 1 || i == 4) {
            itemLayoutWrapper->GetLayoutProperty()->UpdateFlexGrow(0);
        } else {
            itemLayoutWrapper->GetLayoutProperty()->UpdateFlexShrink(1);
        }
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(i + THREE_ITEM_SIZE);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        if (i == 0) {
            EXPECT_EQ(childSize, SizeF(THREE_ITEM_SIZE * TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
            EXPECT_EQ(childOffset, OffsetF(i * TWENTY_PERCENT_WIDTH, (ROW_HEIGHT - SMALL_ITEM_HEIGHT) / 2.0f));
        } else {
            EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
            EXPECT_EQ(childOffset, OffsetF((i + 2) * TWENTY_PERCENT_WIDTH, (ROW_HEIGHT - SMALL_ITEM_HEIGHT) / 2.0f));
        }
    }
}

/**
 * @tc.name: FlexRowLayoutTest019
 * @tc.desc: Set 6 texts in RowReverse direction, container has no padding, the former 3 has 50% width
 * the latter 3 has 2 displayPriority with 20% width.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest019, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW_REVERSE);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(ROW_HEIGHT)));
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(RK356_WIDTH, ROW_HEIGHT);
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.RowReverse, alignItems: ItemAlign.Center}) {
        Text('1').width('50%').height(40).backgroundColor(0xFFFFFF)
        Text('2').width('50%').height(40).backgroundColor(0xEEEEEE)
        Text('3').width('50%').height(40).backgroundColor(0xDDDDDD)
        Text('4').width('20%').height(40).backgroundColor(0xCCCCCC).displayPriority(2)
        Text('5').width('20%').height(40).backgroundColor(0xBBBBBB).displayPriority(2)
        Text('6').width('20%').height(40).backgroundColor(0xAAAAAA).displayPriority(2)
      }
      .height(80)
      .width('100%')
      .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < 2 * THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
        if (i >= THREE_ITEM_SIZE) {
            itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
                CalcSize(CalcLength(TWENTY_PERCENT_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
            itemLayoutWrapper->GetLayoutProperty()->UpdateDisplayIndex(DISPLAYPRIORITY_TWO);
        }
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, ROW_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);
    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(i + THREE_ITEM_SIZE);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(TWENTY_PERCENT_WIDTH, SMALL_ITEM_HEIGHT));
        EXPECT_EQ(childOffset,
            OffsetF(RK356_WIDTH - (i + 1) * TWENTY_PERCENT_WIDTH, (ROW_HEIGHT - SMALL_ITEM_HEIGHT) / 2.0f));
    }
}

/**
 * @tc.name: FlexColumnLayoutTest007
 * @tc.desc: Set 6 texts in Column direction, container has no padding, the former 3 has 50 height
 * the latter 3 has 2 displayPriority with 20 height.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexColumnLayoutTest007, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(true));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());

    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(RK356_WIDTH), CalcLength(COLUMN_HEIGHT)));
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.percentReference = CONTAINER_SIZE;
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();

    auto childLayoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    childLayoutConstraint.maxSize = SizeF(RK356_WIDTH, ROW_HEIGHT);
    childLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    /* corresponding ets code:
        Flex({direction: FlexDirection.Column, alignItems: ItemAlign.Start}) {
        Text('1').width('50%').height(50).backgroundColor(0xFFFFFF)
        Text('2').width('50%').height(50).backgroundColor(0xEEEEEE)
        Text('3').width('50%').height(50).backgroundColor(0xDDDDDD)
        Text('4').width('50%').height(30).backgroundColor(0xCCCCCC).displayPriority(2)
        Text('5').width('50%').height(30).backgroundColor(0xBBBBBB).displayPriority(2)
        Text('6').width('50%').height(30).backgroundColor(0xAAAAAA).displayPriority(2)
      }
      .height(100)
      .width('100%')
      .backgroundColor(0xAFEEEE)
    */
    for (int32_t i = START_INDEX; i < 2 * THREE_ITEM_SIZE; i++) {
        auto itemFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, i + 1, AceType::MakeRefPtr<Pattern>());
        RefPtr<GeometryNode> itemGeometryNode = AceType::MakeRefPtr<GeometryNode>();
        itemGeometryNode->Reset();
        RefPtr<LayoutWrapper> itemLayoutWrapper =
            AceType::MakeRefPtr<LayoutWrapper>(itemFrameNode, itemGeometryNode, itemFrameNode->GetLayoutProperty());
        itemLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(childLayoutConstraint);
        itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(BIG_ITEM_HEIGHT)));
        if (i >= THREE_ITEM_SIZE) {
            itemLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
                CalcSize(CalcLength(HALF_PERCENT_WIDTH), CalcLength(SMALLER_ITEM_HEIGHT)));
            itemLayoutWrapper->GetLayoutProperty()->UpdateDisplayIndex(DISPLAYPRIORITY_TWO);
        }
        auto boxLayoutAlgorithm = itemFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
        EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
        itemLayoutWrapper->SetLayoutAlgorithm(
            AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
        rowFrameNode->AddChild(itemFrameNode);
        layoutWrapper->AppendChild(itemLayoutWrapper);
    }
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(RK356_WIDTH, COLUMN_HEIGHT));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OFFSET_TOP_LEFT);

    for (int32_t i = START_INDEX; i < THREE_ITEM_SIZE; i++) {
        auto childWrapper = layoutWrapper->GetOrCreateChildByIndex(i + THREE_ITEM_SIZE);
        auto childSize = childWrapper->GetGeometryNode()->GetFrameSize();
        auto childOffset = childWrapper->GetGeometryNode()->GetFrameOffset();
        EXPECT_EQ(childSize, SizeF(HALF_PERCENT_WIDTH, SMALLER_ITEM_HEIGHT));
        EXPECT_EQ(childOffset, OffsetF(0.0f, i * SMALLER_ITEM_HEIGHT));
    }
}

/**
 * @tc.name: FlexRowLayoutTest020
 * @tc.desc: Set one child in Row direction and no constraint
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest020, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());
    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Start}) {
            Text('1').width(100).height(40).backgroundColor(0xFFFFFF)
      }
      .backgroundColor(0xAFEEEE)
    */
    auto firstFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    firstGeometryNode->Reset();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(firstFrameNode, firstGeometryNode, firstFrameNode->GetLayoutProperty());
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    auto boxLayoutAlgorithm = firstFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(firstFrameNode);
    layoutWrapper->AppendChild(firstLayoutWrapper);

    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF());
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF());
}

/**
 * @tc.name: FlexRowLayoutTest021
 * @tc.desc: Set one child in RowReverse direction and no constraint
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest021, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());
    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW_REVERSE);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Start}) {
            Text('1').width(100).height(40).backgroundColor(0xFFFFFF)
      }
      .backgroundColor(0xAFEEEE)
    */
    auto firstFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    firstGeometryNode->Reset();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(firstFrameNode, firstGeometryNode, firstFrameNode->GetLayoutProperty());
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    auto boxLayoutAlgorithm = firstFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(firstFrameNode);
    layoutWrapper->AppendChild(firstLayoutWrapper);

    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF());
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF());
}

/**
 * @tc.name: FlexRowLayoutTest022
 * @tc.desc: Set no child in Row direction and no constraint
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexRowLayoutTest022, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());
    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.minSize = SizeF(ZERO, ZERO);

    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Start}) {

      }
      .backgroundColor(0xAFEEEE)
    */

    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));
    EXPECT_EQ(layoutWrapper->GetAllChildrenWithBuild().size(), 0);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SizeF(-1.0f, -1.0f));
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF());
}

/**
 * @tc.name: FlexColumnLayoutTest008
 * @tc.desc: Set one child in Column direction and no constraint
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTestNg, FlexColumnLayoutTest008, TestSize.Level1)
{
    auto rowFrameNode = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    EXPECT_FALSE(rowFrameNode == nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    RefPtr<LayoutWrapper> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(rowFrameNode, geometryNode, rowFrameNode->GetLayoutProperty());
    auto rowLayoutPattern = rowFrameNode->GetPattern<LinearLayoutPattern>();
    EXPECT_FALSE(rowLayoutPattern == nullptr);
    auto rowLayoutProperty = rowLayoutPattern->GetLayoutProperty<LinearLayoutProperty>();
    EXPECT_FALSE(rowLayoutProperty == nullptr);
    rowLayoutProperty->UpdateFlexDirection(FlexDirection::COLUMN);
    rowLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_START);
    auto rowLayoutAlgorithm = rowLayoutPattern->CreateLayoutAlgorithm();
    EXPECT_FALSE(rowLayoutAlgorithm == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(rowLayoutAlgorithm));

    LayoutConstraintF parentLayoutConstraint;
    parentLayoutConstraint.maxSize = CONTAINER_SIZE;
    parentLayoutConstraint.minSize = SizeF(ZERO, ZERO);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    /* corresponding ets code:
        Flex({direction: FlexDirection.Row, alignItems: ItemAlign.Start}) {
            Text('1').width(100).height(40).backgroundColor(0xFFFFFF)
      }
      .backgroundColor(0xAFEEEE)
    */
    auto firstFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> firstGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    firstGeometryNode->Reset();
    RefPtr<LayoutWrapper> firstLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(firstFrameNode, firstGeometryNode, firstFrameNode->GetLayoutProperty());
    firstLayoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(parentLayoutConstraint);
    firstLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SMALL_ITEM_WIDTH), CalcLength(SMALL_ITEM_HEIGHT)));
    auto boxLayoutAlgorithm = firstFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    EXPECT_FALSE(boxLayoutAlgorithm == nullptr);
    firstLayoutWrapper->SetLayoutAlgorithm(
        AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    rowFrameNode->AddChild(firstFrameNode);
    layoutWrapper->AppendChild(firstLayoutWrapper);
    rowLayoutAlgorithm->Measure(AccessibilityManager::RawPtr(layoutWrapper));
    rowLayoutAlgorithm->Layout(AccessibilityManager::RawPtr(layoutWrapper));

    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF());
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameSize(), SMALL_ITEM_SIZE);
    EXPECT_EQ(firstLayoutWrapper->GetGeometryNode()->GetFrameOffset(), OffsetF());
}

} // namespace OHOS::Ace::NG