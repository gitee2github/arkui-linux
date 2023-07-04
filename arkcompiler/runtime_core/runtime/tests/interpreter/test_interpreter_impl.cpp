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

#include "runtime/tests/interpreter/test_interpreter_impl.h"

#include "runtime/interpreter/interpreter-inl.h"
#include "runtime/tests/interpreter/test_runtime_interface.h"
#include "interpreter-inl_gen.h"

namespace panda::interpreter::test {

void ExecuteImpl(ManagedThread *thread, const uint8_t *pc, Frame *frame)
{
    panda::interpreter::ExecuteImpl<RuntimeInterface, false, false>(thread, pc, frame);
}

}  // namespace panda::interpreter::test
