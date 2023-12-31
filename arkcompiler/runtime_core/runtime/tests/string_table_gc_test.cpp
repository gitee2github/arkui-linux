/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "string_table_base_test.h"

namespace panda::mem::test {
class StringTableGCTest : public StringTableBaseTest {
public:
    StringTableGCTest() {}
};

TEST_F(StringTableGCTest, StringTableStwGCTest)
{
    SetupRuntime("stw");
    RunStringTableTests();
}

TEST_F(StringTableGCTest, StringTableGenGCTest)
{
    SetupRuntime("gen-gc");
    RunStringTableTests();
}

TEST_F(StringTableGCTest, StringTableG1GCTest)
{
    SetupRuntime("g1-gc");
    RunStringTableTests();
}
}  // namespace panda::mem::test