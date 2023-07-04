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

#ifndef ECMASCRIPT_COMPILER_GATE_META_DATA_CACHE_H
#define ECMASCRIPT_COMPILER_GATE_META_DATA_CACHE_H

#include <string>

#include "ecmascript/compiler/bytecodes.h"
#include "ecmascript/compiler/type.h"
#include "ecmascript/mem/chunk.h"
#include "ecmascript/mem/chunk_containers.h"

#include "libpandabase/macros.h"

namespace panda::ecmascript::kungfu {

#define CACHED_VALUE_LIST(V) \
    V(1)                     \
    V(2)                     \
    V(3)                     \
    V(4)                     \
    V(5)                     \

// cached ARG list
#define CACHED_ARG_LIST(V)   \
    V(0)                     \
    V(1)                     \
    V(2)                     \
    V(3)                     \
    V(4)                     \
    V(5)                     \
    V(6)                     \
    V(7)                     \
    V(8)                     \
    V(9)                     \
    V(10)                    \

struct GateMetaDataChache {
static constexpr size_t ONE_VALUE = 1;
static constexpr size_t TWO_VALUE = 2;
static constexpr size_t THREE_VALUE = 3;
static constexpr size_t FOUR_VALUE = 4;
static constexpr size_t FIVE_VALUE = 5;

#define DECLARE_CACHED_GATE_META(NAME, OP, R, S, D, V)      \
    GateMetaData cached##NAME##_ { OpCode::OP, R, S, D, V };
    IMMUTABLE_META_DATA_CACHE_LIST(DECLARE_CACHED_GATE_META)
#undef DECLARE_CACHED_GATE_META

#define DECLARE_CACHED_VALUE_META(VALUE)                                                        \
GateMetaData cachedMerge##VALUE##_ { OpCode::MERGE, GateFlags::CONTROL, VALUE, 0, 0 };          \
GateMetaData cachedDependSelector##VALUE##_ { OpCode::DEPEND_SELECTOR, GateFlags::FIXED, 1, VALUE, 0 };
CACHED_VALUE_LIST(DECLARE_CACHED_VALUE_META)
#undef DECLARE_CACHED_VALUE_META

#define DECLARE_CACHED_VALUE_META(VALUE)                               \
OneParameterMetaData cachedArg##VALUE##_ { OpCode::ARG, GateFlags::HAS_ROOT, 0, 0, 0, VALUE };
CACHED_ARG_LIST(DECLARE_CACHED_VALUE_META)
#undef DECLARE_CACHED_VALUE_META

#define DECLARE_CACHED_GATE_META(NAME, OP, R, S, D, V)                 \
    GateMetaData cached##NAME##1_{ OpCode::OP, R, S, D, ONE_VALUE };   \
    GateMetaData cached##NAME##2_{ OpCode::OP, R, S, D, TWO_VALUE };   \
    GateMetaData cached##NAME##3_{ OpCode::OP, R, S, D, THREE_VALUE }; \
    GateMetaData cached##NAME##4_{ OpCode::OP, R, S, D, FOUR_VALUE };  \
    GateMetaData cached##NAME##5_{ OpCode::OP, R, S, D, FIVE_VALUE };
GATE_META_DATA_LIST_WITH_VALUE_IN(DECLARE_CACHED_GATE_META)
#undef DECLARE_CACHED_GATE_META

#define DECLARE_CACHED_GATE_META(NAME, OP, R, S, D, V)                            \
    OneParameterMetaData cached##NAME##1_{ OpCode::OP, R, S, D, V, ONE_VALUE };   \
    OneParameterMetaData cached##NAME##2_{ OpCode::OP, R, S, D, V, TWO_VALUE };   \
    OneParameterMetaData cached##NAME##3_{ OpCode::OP, R, S, D, V, THREE_VALUE }; \
    OneParameterMetaData cached##NAME##4_{ OpCode::OP, R, S, D, V, FOUR_VALUE };  \
    OneParameterMetaData cached##NAME##5_{ OpCode::OP, R, S, D, V, FIVE_VALUE };
GATE_META_DATA_LIST_WITH_VALUE(DECLARE_CACHED_GATE_META)
#undef DECLARE_CACHED_GATE_META
};

class GateMetaBuilder {
public:
#define DECLARE_GATE_META(NAME, OP, R, S, D, V) \
    const GateMetaData* NAME();
    IMMUTABLE_META_DATA_CACHE_LIST(DECLARE_GATE_META)
#undef DECLARE_GATE_META

#define DECLARE_GATE_META(NAME, OP, R, S, D, V)                        \
    const GateMetaData* NAME(size_t value);
    GATE_META_DATA_LIST_WITH_SIZE(DECLARE_GATE_META)
#undef DECLARE_GATE_META

#define DECLARE_GATE_META(NAME, OP, R, S, D, V)                        \
    const GateMetaData* NAME(uint64_t value);
    GATE_META_DATA_LIST_WITH_ONE_PARAMETER(DECLARE_GATE_META)
#undef DECLARE_GATE_META

    explicit GateMetaBuilder(Chunk* chunk);
    const GateMetaData* JSBytecode(size_t valuesIn, EcmaOpcode opcode, uint32_t bcIndex, GateFlags flags)
    {
        return new (chunk_) JSBytecodeMetaData(valuesIn, opcode, bcIndex, flags);
    }

    const GateMetaData* TypedBinaryOp(uint64_t value, TypedBinOp binOp)
    {
        return new (chunk_) TypedBinaryMegaData(value, binOp);
    }

    const GateMetaData* Nop()
    {
        return &cachedNop_;
    }

    GateMetaData* NewGateMetaData(const GateMetaData* other)
    {
        auto meta = new (chunk_) GateMetaData(other->opcode_, other->GetFlags(),
            other->statesIn_, other->dependsIn_, other->valuesIn_);
        meta->SetKind(GateMetaData::Kind::MUTABLE_WITH_SIZE);
        return meta;
    }

private:
    const GateMetaDataChache cache_;
    const GateMetaData cachedNop_ { OpCode::NOP, GateFlags::NONE_FLAG, 0, 0, 0 };
    Chunk* chunk_;
};
} // namespace panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_GATE_META_DATA_CACHE_H
