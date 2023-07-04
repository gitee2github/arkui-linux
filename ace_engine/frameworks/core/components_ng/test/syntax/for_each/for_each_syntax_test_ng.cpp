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

#include <utility>

#include "gtest/gtest.h"

#include "base/memory/ace_type.h"

#define private public
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/syntax/for_each_model_ng.h"
#include "core/components_ng/syntax/for_each_node.h"
#include "core/pipeline_ng/test/mock/mock_pipeline_base.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::string NODE_TAG("node");
constexpr bool IS_ATOMIC_NODE = false;
const std::list<std::string> ID_ARRAY = { "0" };
const std::list<std::string> FOR_EACH_ARRAY = { "0", "1", "2", "3" };
const std::list<std::string> FOR_EACH_IDS = { "0", "1", "2", "3", "4", "5" };
} // namespace

class ForEachSyntaxTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;
};

void ForEachSyntaxTestNg::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "ForEachSyntaxTestNg SetUpTestCase";
}

void ForEachSyntaxTestNg::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "ForEachSyntaxTestNg TearDownTestCase";
}

void ForEachSyntaxTestNg::SetUp()
{
    MockPipelineBase::SetUp();
}

void ForEachSyntaxTestNg::TearDown()
{
    MockPipelineBase::TearDown();
}

/**
 * @tc.name: ForEachSyntaxCreateTest001
 * @tc.desc: Create ForEach.
 * @tc.type: FUNC
 */
HWTEST_F(ForEachSyntaxTestNg, ForEachSyntaxTest001, TestSize.Level1)
{
    ForEachModelNG forEach;
    forEach.Create();
    auto forEachNode = AceType::DynamicCast<ForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(forEachNode != nullptr && forEachNode->GetTag() == V2::JS_FOR_EACH_ETS_TAG);
    EXPECT_EQ(forEachNode->IsAtomicNode(), IS_ATOMIC_NODE);
}

/**
 * @tc.name: ForEachSyntaxTest002
 * @tc.desc: Create ForEach.
 * @tc.type: FUNC
 */
HWTEST_F(ForEachSyntaxTestNg, ForEachSyntaxTest002, TestSize.Level1)
{
    ForEachModelNG forEach;
    forEach.Create();
    forEach.Pop();
    auto node = ViewStackProcessor::GetInstance()->GetMainElementNode();
    // ViewStackProcessor will not pop when it's size equals 1.
    EXPECT_FALSE(ViewStackProcessor::GetInstance()->GetMainElementNode() == nullptr);
}

/**
 * @tc.name: ForEachSyntaxIdTest003
 * @tc.desc: Create ForEach and set its ids.
 * @tc.type: FUNC
 */
HWTEST_F(ForEachSyntaxTestNg, ForEachSyntaxIdTest003, TestSize.Level1)
{
    ForEachModelNG forEach;
    forEach.Create();
    std::list<std::string> ids = FOR_EACH_IDS;
    forEach.SetNewIds(std::move(ids));

    auto forEachNode = AceType::DynamicCast<ForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(forEachNode != nullptr && forEachNode->GetTag() == V2::JS_FOR_EACH_ETS_TAG);

    // tempIds_ is empty.
    auto tempIds = forEachNode->GetTempIds();
    EXPECT_TRUE(tempIds.empty());
    // CreateTempItems, swap ids_ and tempIds_.
    forEachNode->CreateTempItems();
    EXPECT_EQ(forEachNode->GetTempIds(), FOR_EACH_IDS);
}

/**
 * @tc.name: ForEachSyntaxUpdateTest004
 * @tc.desc: Create ForEach and update children.
 * @tc.type: FUNC
 */
HWTEST_F(ForEachSyntaxTestNg, ForEachSyntaxUpdateTest004, TestSize.Level1)
{
    ForEachModelNG forEach;
    forEach.Create();
    std::list<std::string> ids = FOR_EACH_IDS;
    forEach.SetNewIds(std::move(ids));

    auto forEachNode = AceType::DynamicCast<ForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(forEachNode != nullptr && forEachNode->GetTag() == V2::JS_FOR_EACH_ETS_TAG);

    /**
    // corresponding ets code:
    //     ForEach(this.arr,
    //         (item: number) => {
    //             Blank()
    //         })
    */
    for (auto iter = FOR_EACH_ARRAY.begin(); iter != FOR_EACH_ARRAY.end(); iter++) {
        auto childFrameNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
        forEachNode->AddChild(childFrameNode);
    }
    EXPECT_EQ(forEachNode->GetChildren().size(), FOR_EACH_ARRAY.size());

    // tempIds_ is empty.
    auto tempIds = forEachNode->GetTempIds();
    EXPECT_TRUE(tempIds.empty());
    // CreateTempItems, swap ids_ and tempIds_, children_ and tempChildren_.
    forEachNode->CreateTempItems();
    EXPECT_EQ(forEachNode->GetTempIds(), FOR_EACH_IDS);
}

/**
 * @tc.name: ForEachSyntaxUpdateTest005
 * @tc.desc: Create ForEach and update children.
 * @tc.type: FUNC
 */
HWTEST_F(ForEachSyntaxTestNg, ForEachSyntaxUpdateTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set branch id which is same as before.
     */
    ForEachModelNG forEach;
    forEach.Create();
    std::list<std::string> ids = FOR_EACH_ARRAY;
    forEach.SetNewIds(std::move(ids));
    auto forEachNode = AceType::DynamicCast<ForEachNode>(ViewStackProcessor::GetInstance()->Finish());
    EXPECT_TRUE(forEachNode != nullptr && forEachNode->GetTag() == V2::JS_FOR_EACH_ETS_TAG);

    /**
     * @tc.steps: step2. CompareAndUpdateChildren when newIds and oldIds are same.
     */
    forEachNode->CreateTempItems();
    std::list<std::string> ids2 = ID_ARRAY;
    forEachNode->SetIds(std::move(ids2));
    forEachNode->onMainTree_ = true;
    auto node = AceType::MakeRefPtr<FrameNode>(NODE_TAG, -1, AceType::MakeRefPtr<Pattern>());
    forEachNode->SetParent(node);
    forEachNode->children_ = { node };
    forEachNode->CompareAndUpdateChildren();
    auto tempIds = forEachNode->GetTempIds();
    EXPECT_TRUE(tempIds.empty());
}
} // namespace OHOS::Ace::NG
