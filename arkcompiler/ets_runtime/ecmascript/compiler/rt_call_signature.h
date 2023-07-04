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

#ifndef ECMASCRIPT_COMPILER_RT_CALL_SIGNATURE_H
#define ECMASCRIPT_COMPILER_RT_CALL_SIGNATURE_H

#include "ecmascript/compiler/call_signature.h"
#include "ecmascript/stubs/runtime_stubs.h"

namespace panda::ecmascript::kungfu {
class RuntimeStubCSigns {
public:
    enum ID {
#define DEF_RUNTIME_STUB_ID(name) ID_##name,
        RUNTIME_STUB_LIST(DEF_RUNTIME_STUB_ID)
#undef DEF_RUNTIME_STUB_ID
        NUM_OF_STUBS
    };

    enum NoGCStubID {
#define DEF_RUNTIME_STUB_ID(name) NOGCSTUB_ID_##name,
        RUNTIME_ASM_STUB_LIST(DEF_RUNTIME_STUB_ID)
        RUNTIME_STUB_WITHOUT_GC_LIST(DEF_RUNTIME_STUB_ID)
#undef DEF_RUNTIME_STUB_ID
        NUM_OF_RTSTUBS_WITHOUT_GC
    };

    static void Initialize();

    static void GetASMCSigns(std::vector<const CallSignature*>& callSigns);

    static const CallSignature *Get(size_t index)
    {
        ASSERT(index < NUM_OF_RTSTUBS_WITHOUT_GC);
        return &callSigns_[index];
    }

    static std::string GetRTName(int index)
    {
        ASSERT(index < NUM_OF_STUBS);
        switch (index) {
#define DEF_STUB_NAME(name) case ID_##name: { return std::string("RTStub_") + #name; }
RUNTIME_STUB_LIST(DEF_STUB_NAME)
#undef DEF_STUB_NAME
            default:
                return "unknown";
        }
        return "unknown";
    }

private:
    static CallSignature callSigns_[NUM_OF_RTSTUBS_WITHOUT_GC];
};
#define RTSTUB_ID(name) kungfu::RuntimeStubCSigns::ID_##name
} // namespace panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_RT_CALL_SIGNATURE_H