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

#ifndef BLOCK_HOOK_SCOPE_H
#define BLOCK_HOOK_SCOPE_H

#include "libpandabase/macros.h"

namespace panda::ecmascript::base {
class BlockHookScope {
public:
    BlockHookScope();
    ~BlockHookScope();
    NO_COPY_SEMANTIC(BlockHookScope);
    NO_MOVE_SEMANTIC(BlockHookScope);

#ifdef BLOCK_HOOK
private:
    bool previousState_ {true};
#endif
};
}
#endif  // BLOCK_HOOK_SCOPE_H