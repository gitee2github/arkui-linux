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

#include <optional>
#include <utility>

#include "gtest/gtest.h"

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"

#define private public
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/syntax/lazy_for_each_model_ng.h"
#include "core/components_ng/syntax/lazy_layout_wrapper_builder.h"
#include "core/components_ng/test/mock/syntax/mock_lazy_for_each_actuator.h"
#include "core/components_ng/test/mock/syntax/mock_lazy_for_each_builder.h"
#include "core/pipeline_ng/test/mock/mock_pipeline_base.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::list<std::optional<std::string>> LAZY_FOR_EACH_NODE_IDS = { "0", "1", "2", "3", "4", "5", "6" };
const std::list<std::optional<int32_t>> LAZY_FOR_EACH_NODE_IDS_INT = { 0, 1, 2, 3, 4, 5, 6 };
const std::unordered_map<int32_t, std::optional<std::string>> LAZY_FOR_EACH_CACHED_ITEMS = { { 0, "0" }, { 1, "1" } };
const std::list<int32_t> LAZY_FOR_EACH_ITEMS = { 0, 1, 2, 3, 4, 5 };

constexpr bool IS_ATOMIC_NODE = false;
constexpr int32_t NEW_START_ID = 0;
constexpr int32_t NEW_END_ID = 6;
constexpr int32_t NEW_END_ID_ADD = 9;
constexpr int32_t NEW_END_ID_END = 4;
constexpr int32_t INDEX_GREATER_THAN_END_INDEX = 20;
constexpr int32_t INDEX_LESS_THAN_START_INDEX = -1;
constexpr int32_t INDEX_EQUAL_WITH_START_INDEX = 1;
constexpr int32_t INDEX_EQUAL_WITH_START_INDEX_DELETED = -1;
constexpr int32_t INDEX_MIDDLE = 3;
constexpr int32_t INDEX_MIDDLE_2 = 4;
} // namespace

class LazyForEachSyntaxTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;

    static void UpdateItems(
        const RefPtr<LazyForEachNode>& lazyForEachNode, const RefPtr<LazyForEachActuator>& mockLazyForEachActuator)
    {
        /**
         * @tc.steps: step1. Add child found in generatedItem_.
         */
        auto ids = LAZY_FOR_EACH_NODE_IDS;
        auto builder = AceType::DynamicCast<LazyForEachBuilder>(mockLazyForEachActuator);
        for (auto iter : LAZY_FOR_EACH_NODE_IDS_INT) {
            builder->CreateChildByIndex(iter.value_or(0));
        }

        /**
         * @tc.steps: step2. Update item found in generatedItem_.
         */
        auto cacheItems = LAZY_FOR_EACH_CACHED_ITEMS;
        lazyForEachNode->UpdateLazyForEachItems(NEW_START_ID, NEW_END_ID, std::move(ids), std::move(cacheItems));
    }
};

void LazyForEachSyntaxTestNg::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "LazyForEachSyntaxTestNg SetUpTestCase";
}

void LazyForEachSyntaxTestNg::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "LazyForEachSyntaxTestNg TearDownTestCase";
}

void LazyForEachSyntaxTestNg::SetUp()
{
    MockPipelineBase::SetUp();
}

void LazyForEachSyntaxTestNg::TearDown()
{
    MockPipelineBase::TearDown();
}

/**
 * @tc.name: LazyForEachSyntaxCreateTest001
 * @tc.desc: Create LazyForEach.
 * @tc.type: FUNC
 */
HWTEST_F(LazyForEachSyntaxTestNg, LazyForEachSyntaxCreateTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Text and push it to view stack processor.
     * @tc.expected: Make Text as LazyForEach parent.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ViewStackProcessor::GetInstance()->Push(frameNode);

    /**
     * @tc.steps: step2. Invoke lazyForEach Create function.
     * @tc.expected: Create LazyForEachNode and can be pop from ViewStackProcessor.
     */
    LazyForEachModelNG lazyForEach;
    const RefPtr<LazyForEachActuator> mockLazyForEachActuator =
        AceType::MakeRefPtr<OHOS::Ace::Framework::MockLazyForEachBuilder>();
    lazyForEach.Create(mockLazyForEachActuator);
    auto lazyForEachNode = AceType::DynamicCast<LazyForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(lazyForEachNode != nullptr && lazyForEachNode->GetTag() == V2::JS_LAZY_FOR_EACH_ETS_TAG);

    EXPECT_EQ(lazyForEachNode->IsAtomicNode(), IS_ATOMIC_NODE);
    EXPECT_EQ(lazyForEachNode->FrameCount(), 0);
}

/**
 * @tc.name: ForEachSyntaxCreateTest002
 * @tc.desc: Create LazyForEach and its parent node is Tabs.
 * @tc.type: FUNC
 */
HWTEST_F(LazyForEachSyntaxTestNg, ForEachSyntaxCreateTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Tabs and push it to view stack processor.
     * @tc.expected: Make Tabs as LazyForEach parent.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TABS_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ViewStackProcessor::GetInstance()->Push(frameNode);

    /**
     * @tc.steps: step2. Invoke lazyForEach Create function.
     * @tc.expected: LazyForEachNode will not be created when its parent is Tabs.
     */
    LazyForEachModelNG lazyForEach;
    const RefPtr<LazyForEachActuator> mockLazyForEachActuator =
        AceType::MakeRefPtr<OHOS::Ace::Framework::MockLazyForEachBuilder>();
    lazyForEach.Create(mockLazyForEachActuator);
    auto lazyForEachNode = AceType::DynamicCast<LazyForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(lazyForEachNode == nullptr);
}

/**
 * @tc.name: ForEachSyntaxUpdateTest003
 * @tc.desc: Create LazyForEach and its Update its Items.
 * @tc.type: FUNC
 */
HWTEST_F(LazyForEachSyntaxTestNg, ForEachSyntaxUpdateTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Text and push it to view stack processor.
     * @tc.expected: Make Text as LazyForEach parent.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ViewStackProcessor::GetInstance()->Push(frameNode);

    /**
     * @tc.steps: step2. Invoke lazyForEach Create function.
     * @tc.expected: Create LazyForEachNode and can be pop from ViewStackProcessor.
     */
    LazyForEachModelNG lazyForEach;
    const RefPtr<LazyForEachActuator> mockLazyForEachActuator =
        AceType::MakeRefPtr<OHOS::Ace::Framework::MockLazyForEachBuilder>();
    lazyForEach.Create(mockLazyForEachActuator);
    auto lazyForEachNode = AceType::DynamicCast<LazyForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(lazyForEachNode != nullptr && lazyForEachNode->GetTag() == V2::JS_LAZY_FOR_EACH_ETS_TAG);

    /**
     * @tc.steps: step3. Invoke UpdateLazyForEachItems with empty nodeIds.
     * @tc.expected: lazyForEachNode ids_ is empty.
     */
    auto cacheItems = LAZY_FOR_EACH_CACHED_ITEMS;
    lazyForEachNode->UpdateLazyForEachItems(NEW_START_ID, NEW_END_ID, {}, std::move(cacheItems));
    EXPECT_TRUE(lazyForEachNode->ids_.empty());

    /**
     * @tc.steps: step4. Update lazyForEachNode items.
     * @tc.expected: lazyForEachNode ids_ and children_ are empty.
     */
    UpdateItems(lazyForEachNode, mockLazyForEachActuator);

    auto items = LAZY_FOR_EACH_ITEMS;
    lazyForEachNode->PostIdleTask(std::move(items));
    EXPECT_EQ(lazyForEachNode->ids_.size(), LAZY_FOR_EACH_NODE_IDS.size());
    EXPECT_EQ(lazyForEachNode->GetChildren().size(), LAZY_FOR_EACH_NODE_IDS.size());
}

/**
 * @tc.name: ForEachSyntaxAddDataFunctionTest004
 * @tc.desc: Create LazyForEach, update its Items and invoke OnDataAdded function.
 * @tc.type: FUNC
 */
HWTEST_F(LazyForEachSyntaxTestNg, ForEachSyntaxFunctionTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Text and push it to view stack processor.
     * @tc.expected: Make Text as LazyForEach parent.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ViewStackProcessor::GetInstance()->Push(frameNode);

    /**
     * @tc.steps: step2. Invoke lazyForEach Create function.
     * @tc.expected: Create LazyForEachNode and can be pop from ViewStackProcessor.
     */
    LazyForEachModelNG lazyForEach;
    const RefPtr<LazyForEachActuator> mockLazyForEachActuator =
        AceType::MakeRefPtr<OHOS::Ace::Framework::MockLazyForEachBuilder>();
    lazyForEach.Create(mockLazyForEachActuator);
    auto lazyForEachNode = AceType::DynamicCast<LazyForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(lazyForEachNode != nullptr && lazyForEachNode->GetTag() == V2::JS_LAZY_FOR_EACH_ETS_TAG);

    UpdateItems(lazyForEachNode, mockLazyForEachActuator);

    /**
     * @tc.steps: step3. Add index which is less than start index.
     * @tc.expected: LazyForEachNode ids_ will not be added.
     */
    lazyForEachNode->OnDataAdded(INDEX_LESS_THAN_START_INDEX);
    EXPECT_EQ(lazyForEachNode->ids_.size(), LAZY_FOR_EACH_NODE_IDS.size());

    /**
     * @tc.steps: step4. Add index which is greater than end index.
     * @tc.expected: LazyForEachNode ids_ will not be added.
     */
    lazyForEachNode->OnDataAdded(INDEX_GREATER_THAN_END_INDEX);
    EXPECT_EQ(lazyForEachNode->ids_.size(), LAZY_FOR_EACH_NODE_IDS.size());

    /**
     * @tc.steps: step5. Add index which is equal with start index.
     * @tc.expected: LazyForEachNode ids_ will be added the item.
     */
    lazyForEachNode->OnDataAdded(INDEX_EQUAL_WITH_START_INDEX);
    auto newIdsSize = LAZY_FOR_EACH_NODE_IDS.size() + 1;
    EXPECT_EQ(lazyForEachNode->ids_.size(), newIdsSize);

    /**
     * @tc.steps: step6. Add index which is equal with end index + 1.
     * @tc.expected: LazyForEachNode ids_ will be added the item.
     */
    lazyForEachNode->OnDataAdded(NEW_END_ID_ADD);
    newIdsSize++;
    EXPECT_EQ(lazyForEachNode->ids_.size(), newIdsSize);

    /**
     * @tc.steps: step7. Add index which is in the middle with start and end.
     * @tc.expected: LazyForEachNode ids_ will be added the item.
     */
    lazyForEachNode->OnDataAdded(INDEX_MIDDLE);
    newIdsSize++;
    EXPECT_EQ(lazyForEachNode->ids_.size(), newIdsSize);
}

/**
 * @tc.name: ForEachSyntaxReloadDataFunctionTest005
 * @tc.desc: Create LazyForEach, update its Items and invoke OnDataReloaded function.
 * @tc.type: FUNC
 */
HWTEST_F(LazyForEachSyntaxTestNg, ForEachSyntaxReloadDataFunctionTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Text and push it to view stack processor.
     * @tc.expected: Make Text as LazyForEach parent.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ViewStackProcessor::GetInstance()->Push(frameNode);

    /**
     * @tc.steps: step2. Invoke lazyForEach Create function.
     * @tc.expected: Create LazyForEachNode and can be pop from ViewStackProcessor.
     */
    LazyForEachModelNG lazyForEach;
    const RefPtr<LazyForEachActuator> mockLazyForEachActuator =
        AceType::MakeRefPtr<OHOS::Ace::Framework::MockLazyForEachBuilder>();
    lazyForEach.Create(mockLazyForEachActuator);
    auto lazyForEachNode = AceType::DynamicCast<LazyForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(lazyForEachNode != nullptr && lazyForEachNode->GetTag() == V2::JS_LAZY_FOR_EACH_ETS_TAG);

    UpdateItems(lazyForEachNode, mockLazyForEachActuator);

    /**
     * @tc.steps: step3. Invoke OnDataReloaded.
     * @tc.expected: LazyForEachNode ids_ will be cleared.
     */
    lazyForEachNode->OnDataReloaded();
    EXPECT_TRUE(lazyForEachNode->ids_.empty());
}

/**
 * @tc.name: ForEachSyntaxDeleteDataFunctionTest006
 * @tc.desc: Create LazyForEach, update its Items and invoke OnDataDeleted function.
 * @tc.type: FUNC
 */
HWTEST_F(LazyForEachSyntaxTestNg, ForEachSyntaxDeleteDataFunctionTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Text and push it to view stack processor.
     * @tc.expected: Make Text as LazyForEach parent.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ViewStackProcessor::GetInstance()->Push(frameNode);

    /**
     * @tc.steps: step2. Invoke lazyForEach Create function.
     * @tc.expected: Create LazyForEachNode and can be pop from ViewStackProcessor.
     */
    LazyForEachModelNG lazyForEach;
    const RefPtr<LazyForEachActuator> mockLazyForEachActuator =
        AceType::MakeRefPtr<OHOS::Ace::Framework::MockLazyForEachBuilder>();
    lazyForEach.Create(mockLazyForEachActuator);
    auto lazyForEachNode = AceType::DynamicCast<LazyForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(lazyForEachNode != nullptr && lazyForEachNode->GetTag() == V2::JS_LAZY_FOR_EACH_ETS_TAG);

    UpdateItems(lazyForEachNode, mockLazyForEachActuator);

    /**
     * @tc.steps: step3. Delete index which is greater than end index.
     * @tc.expected: LazyForEachNode ids_ will not be deleted.
     */
    lazyForEachNode->OnDataDeleted(INDEX_GREATER_THAN_END_INDEX);
    EXPECT_EQ(lazyForEachNode->ids_.size(), LAZY_FOR_EACH_NODE_IDS.size());

    /**
     * @tc.steps: step4. Delete index which is less than start index.
     * @tc.expected: LazyForEachNode ids_ will not be deleted.
     */
    lazyForEachNode->OnDataDeleted(INDEX_LESS_THAN_START_INDEX);
    EXPECT_EQ(lazyForEachNode->ids_.size(), LAZY_FOR_EACH_NODE_IDS.size());

    /**
     * @tc.steps: step5. Delete index which is equal with start index.
     * @tc.expected: LazyForEachNode ids_ will be deleted the item.
     */
    lazyForEachNode->OnDataDeleted(INDEX_EQUAL_WITH_START_INDEX_DELETED);
    auto newIdsSize = LAZY_FOR_EACH_NODE_IDS.size() - 1;
    EXPECT_EQ(lazyForEachNode->ids_.size(), newIdsSize);

    /**
     * @tc.steps: step6. Delete index which is equal with end index.
     * @tc.expected: LazyForEachNode ids_ will be deleted the item.
     */
    lazyForEachNode->OnDataDeleted(NEW_END_ID_END);
    newIdsSize--;
    EXPECT_EQ(lazyForEachNode->ids_.size(), newIdsSize);

    /**
     * @tc.steps: step7. Delete index which is in the middle with start and end.
     * @tc.expected: LazyForEachNode ids_ will be deleted the item.
     */
    lazyForEachNode->OnDataDeleted(INDEX_MIDDLE);
    newIdsSize--;
    EXPECT_EQ(lazyForEachNode->ids_.size(), newIdsSize);
}

/**
 * @tc.name: ForEachSyntaxChangeDataFunctionTest007
 * @tc.desc: Create LazyForEach, update its Items and invoke OnDataChanged function.
 * @tc.type: FUNC
 */
HWTEST_F(LazyForEachSyntaxTestNg, ForEachSyntaxChangeDataFunctionTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Text and push it to view stack processor.
     * @tc.expected: Make Text as LazyForEach parent.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ViewStackProcessor::GetInstance()->Push(frameNode);

    /**
     * @tc.steps: step2. Invoke lazyForEach Create function.
     * @tc.expected: Create LazyForEachNode and can be pop from ViewStackProcessor.
     */
    LazyForEachModelNG lazyForEach;
    const RefPtr<LazyForEachActuator> mockLazyForEachActuator =
        AceType::MakeRefPtr<OHOS::Ace::Framework::MockLazyForEachBuilder>();
    lazyForEach.Create(mockLazyForEachActuator);
    auto lazyForEachNode = AceType::DynamicCast<LazyForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(lazyForEachNode != nullptr && lazyForEachNode->GetTag() == V2::JS_LAZY_FOR_EACH_ETS_TAG);

    UpdateItems(lazyForEachNode, mockLazyForEachActuator);

    /**
     * @tc.steps: step3. Change index which is less than start index.
     * @tc.expected: changeIndex is out of range, ignored.
     */
    lazyForEachNode->OnDataChanged(INDEX_LESS_THAN_START_INDEX);
    EXPECT_EQ(lazyForEachNode->ids_.size(), LAZY_FOR_EACH_NODE_IDS.size());

    /**
     * @tc.steps: step4. Change index which is greater than end index.
     * @tc.expected: changeIndex is out of range, ignored.
     */
    lazyForEachNode->OnDataChanged(INDEX_GREATER_THAN_END_INDEX);
    EXPECT_EQ(lazyForEachNode->ids_.size(), LAZY_FOR_EACH_NODE_IDS.size());

    /**
     * @tc.steps: step5. Change index which is in the middle with start and end.
     * @tc.expected: changeIndex is not out of range, change the index data.
     */
    lazyForEachNode->OnDataChanged(INDEX_MIDDLE);
    EXPECT_EQ(lazyForEachNode->ids_.size(), LAZY_FOR_EACH_NODE_IDS.size());
}

/**
 * @tc.name: ForEachSyntaxMoveDataFunctionTest008
 * @tc.desc: Create LazyForEach, update its Items and invoke OnDataMoved function.
 * @tc.type: FUNC
 */
HWTEST_F(LazyForEachSyntaxTestNg, ForEachSyntaxMoveDataFunctionTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Text and push it to view stack processor.
     * @tc.expected: Make Text as LazyForEach parent.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ViewStackProcessor::GetInstance()->Push(frameNode);

    /**
     * @tc.steps: step2. Invoke lazyForEach Create function.
     * @tc.expected: Create LazyForEachNode and can be pop from ViewStackProcessor.
     */
    LazyForEachModelNG lazyForEach;
    const RefPtr<LazyForEachActuator> mockLazyForEachActuator =
        AceType::MakeRefPtr<OHOS::Ace::Framework::MockLazyForEachBuilder>();
    lazyForEach.Create(mockLazyForEachActuator);
    auto lazyForEachNode = AceType::DynamicCast<LazyForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(lazyForEachNode != nullptr && lazyForEachNode->GetTag() == V2::JS_LAZY_FOR_EACH_ETS_TAG);

    UpdateItems(lazyForEachNode, mockLazyForEachActuator);

    /**
     * @tc.steps: step3. From index is less than start index, and to index is greater than end index.
     * @tc.expected: Both out of range, ignored.
     */
    lazyForEachNode->OnDataMoved(INDEX_LESS_THAN_START_INDEX, INDEX_GREATER_THAN_END_INDEX);
    EXPECT_EQ(lazyForEachNode->ids_.size(), LAZY_FOR_EACH_NODE_IDS.size());

    /**
     * @tc.steps: step4. From index is in middle of range, and to index is greater than end index..
     */
    lazyForEachNode->OnDataMoved(INDEX_MIDDLE, INDEX_GREATER_THAN_END_INDEX);
    EXPECT_EQ(lazyForEachNode->ids_.size(), LAZY_FOR_EACH_NODE_IDS.size());

    /**
     * @tc.steps: step5. From index is in middle of range, and to index is in middle of range.
     */
    lazyForEachNode->OnDataMoved(INDEX_MIDDLE_2, INDEX_MIDDLE);
    EXPECT_EQ(lazyForEachNode->ids_.size(), LAZY_FOR_EACH_NODE_IDS.size());
}

/**
 * @tc.name: ForEachSyntaxWrapperBuilderTest009
 * @tc.desc: Create LazyForEach, update its Items and update LazyLayoutWrapperBuilder layout range.
 * @tc.type: FUNC
 */
HWTEST_F(LazyForEachSyntaxTestNg, ForEachSyntaxWrapperBuilderTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Text and push it to view stack processor.
     * @tc.expected: Make Text as LazyForEach parent.
     */
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ViewStackProcessor::GetInstance()->Push(frameNode);

    /**
     * @tc.steps: step2. Invoke lazyForEach Create function.
     * @tc.expected: Create LazyForEachNode and can be pop from ViewStackProcessor.
     */
    LazyForEachModelNG lazyForEach;
    const RefPtr<LazyForEachActuator> mockLazyForEachActuator =
        AceType::MakeRefPtr<OHOS::Ace::Framework::MockLazyForEachBuilder>();
    lazyForEach.Create(mockLazyForEachActuator);
    auto lazyForEachNode = AceType::DynamicCast<LazyForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(lazyForEachNode != nullptr && lazyForEachNode->GetTag() == V2::JS_LAZY_FOR_EACH_ETS_TAG);

    UpdateItems(lazyForEachNode, mockLazyForEachActuator);

    /**
     * @tc.steps: step3. Create Parent LayoutWrapper.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_FALSE(geometryNode == nullptr);
    auto parentLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapper>(frameNode, geometryNode, frameNode->GetLayoutProperty());
    /**
     * @tc.steps: step4. Invoke AdjustLayoutWrapperTree, update lazyLayoutWrapperBuilder index range and its
     * currentChildCount_.
     * @tc.expected: lazyLayoutWrapperBuilder preNodeIds_ is equal with lazyForEachNode ids_.
     */
    lazyForEachNode->AdjustLayoutWrapperTree(parentLayoutWrapper, false, false);
    auto lazyLayoutWrapperBuilder =
        AceType::DynamicCast<LazyLayoutWrapperBuilder>(parentLayoutWrapper->layoutWrapperBuilder_);
    EXPECT_EQ(parentLayoutWrapper->GetTotalChildCount(), LAZY_FOR_EACH_NODE_IDS.size());
    EXPECT_EQ(lazyLayoutWrapperBuilder->preNodeIds_, LAZY_FOR_EACH_NODE_IDS);
}
} // namespace OHOS::Ace::NG
