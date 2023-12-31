/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_TEST_METHOD_H
#define PANDA_TOOLING_INSPECTOR_TEST_TEST_METHOD_H

#include <functional>

namespace panda {
class Method;
}  // namespace panda

namespace panda::tooling::inspector::test {
class Client;
class InstructionPointer;
class TestDebugger;

class TestMethod {
public:
    TestMethod(TestDebugger &debugger, Client &client) : debugger_(debugger), client_(client) {}

    Method *Get() const
    {
        return method_;
    }

    void Set(Method *method)
    {
        method_ = method;
    }

    void Call(const std::function<void(InstructionPointer &)> &body = [](auto & /* ip */) {});

private:
    TestDebugger &debugger_;
    Client &client_;
    Method *method_ {nullptr};
};
}  // namespace panda::tooling::inspector::test

#endif  // PANDA_TOOLING_INSPECTOR_TEST_TEST_METHOD_H
