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

#ifndef PANDA_BYTECODE_OPTIMIZER_COMMON_H
#define PANDA_BYTECODE_OPTIMIZER_COMMON_H

#include "compiler/optimizer/ir/constants.h"
#include "compiler/optimizer/ir/inst.h"

namespace panda::compiler {
class BinaryImmOperation;
}  // namespace panda::compiler

namespace panda::bytecodeopt {
static constexpr compiler::Register MIN_REGISTER_NUMBER = 0;
static constexpr compiler::Register MAX_NUM_SHORT_CALL_ARGS = 2;
static constexpr compiler::Register MAX_NUM_NON_RANGE_ARGS = 4;
static constexpr compiler::Register MAX_NUM_INPUTS = MAX_NUM_NON_RANGE_ARGS;
static constexpr panda::compiler::Register NUM_COMPACTLY_ENCODED_REGS = 16;

// Get the position where accumulator read happens.
uint8_t AccReadIndex(const compiler::Inst *inst);

bool CanConvertToIncI(const compiler::BinaryImmOperation *binop);
}  // namespace panda::bytecodeopt

#endif  // PANDA_BYTECODE_OPTIMIZER_COMMON_H
