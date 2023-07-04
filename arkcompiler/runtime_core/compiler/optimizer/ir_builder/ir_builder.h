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

#ifndef PANDA_IR_BUILDER_H
#define PANDA_IR_BUILDER_H

#include "optimizer/ir/graph.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/pass.h"
#include "utils/logger.h"
#include "pbc_iterator.h"
#include "inst_builder.h"

namespace panda {
class Method;
}  // namespace panda

namespace panda::compiler {
/**
 * Build IR from panda bytecode
 */
class IrBuilder : public Optimization {
    struct Boundaries {
        uint32_t begin_pc;
        uint32_t end_pc;
    };

    struct CatchCodeBlock {
        uint32_t pc {};
        uint32_t type_id {};
    };

    struct TryCodeBlock {
        Boundaries boundaries {};                           // NOLINT(misc-non-private-member-variables-in-classes)
        BasicBlock *begin_bb {nullptr};                     // NOLINT(misc-non-private-member-variables-in-classes)
        BasicBlock *end_bb {nullptr};                       // NOLINT(misc-non-private-member-variables-in-classes)
        ArenaVector<CatchCodeBlock> *catches {nullptr};     // NOLINT(misc-non-private-member-variables-in-classes)
        ArenaVector<BasicBlock *> *basic_blocks {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
        uint32_t id {INVALID_ID};                           // NOLINT(misc-non-private-member-variables-in-classes)
        bool contains_throwable_inst {false};               // NOLINT(misc-non-private-member-variables-in-classes)

        void Init(Graph *graph, uint32_t try_id)
        {
            id = try_id;
            auto allocator = graph->GetLocalAllocator();
            catches = allocator->New<ArenaVector<CatchCodeBlock>>(allocator->Adapter());
            begin_bb = graph->CreateEmptyBlock(boundaries.begin_pc);
            begin_bb->SetTryBegin(true);
            end_bb = graph->CreateEmptyBlock(boundaries.end_pc);
            end_bb->SetTryEnd(true);
            // Order of try-blocks should be saved in the graph to restore it in the generated bytecode
            graph->AppendTryBeginBlock(begin_bb);
        }
    };

public:
    explicit IrBuilder(Graph *graph) : IrBuilder(graph, graph->GetMethod(), nullptr) {}

    IrBuilder(Graph *graph, RuntimeInterface::MethodPtr method, CallInst *caller_inst)
        : Optimization(graph),
          blocks_(graph->GetLocalAllocator()->Adapter()),
          catches_pc_(graph->GetLocalAllocator()->Adapter()),
          try_blocks_(graph->GetLocalAllocator()->Adapter()),
          opened_try_blocks_(graph->GetLocalAllocator()->Adapter()),
          catch_handlers_(graph->GetLocalAllocator()->Adapter()),
          inst_defs_(graph->GetLocalAllocator()->Adapter()),
          method_(method),
          is_inlined_graph_(caller_inst != nullptr),
          caller_inst_(caller_inst)
    {
    }

    NO_COPY_SEMANTIC(IrBuilder);
    NO_MOVE_SEMANTIC(IrBuilder);
    ~IrBuilder() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "IrBuilder";
    }

    auto GetMethod() const
    {
        return method_;
    }

private:
    void CreateBlock(size_t pc)
    {
        if (blocks_[pc] == nullptr) {
            blocks_[pc] = GetGraph()->CreateEmptyBlock();
            blocks_[pc]->SetGuestPc(pc);
        }
    }

    BasicBlock *GetBlockForPc(size_t pc)
    {
        return blocks_[pc];
    }

    BasicBlock *GetPrevBlockForPc(size_t pc)
    {
        do {
            ASSERT(pc > 0);
            pc--;
        } while (blocks_[pc] == nullptr || blocks_[pc]->GetGraph() == nullptr);
        return blocks_[pc];
    }

    bool CheckMethodLimitations(const BytecodeInstructions &instructions, size_t vregs_count);
    void BuildBasicBlocks(const BytecodeInstructions &instructions);
    bool BuildBasicBlock(BasicBlock *bb, InstBuilder *inst_builder, const uint8_t *instructions_buf);
    bool BuildInstructionsForBB(BasicBlock *bb, InstBuilder *inst_builder, const uint8_t *instructions_buf);
    void SplitConstant(ConstantInst *const_inst);
    void ConnectBasicBlocks(const BytecodeInstructions &instructions);
    void CreateTryCatchBoundariesBlocks();
    void ResolveTryCatchBlocks();
    void ConnectTryCatchBlocks();
    IrBuilder::TryCodeBlock *InsertTryBlockInfo(const Boundaries &try_boundaries);
    void TrackTryBoundaries(size_t pc, const BytecodeInstruction &inst);
    BasicBlock *GetBlockToJump(BytecodeInstruction *inst, size_t pc);
    BasicBlock *GetBlockForSaveStateDeoptimize(BasicBlock *block);
    void MarkTryCatchBlocks(Marker marker);
    template <class Callback>
    void EnumerateTryBlocksCoveredPc(uint32_t pc, const Callback &callback);
    void ConnectTryCodeBlock(const TryCodeBlock &try_block, const ArenaMap<uint32_t, BasicBlock *> &catch_blocks);
    void ProcessThrowableInstructions(InstBuilder *inst_builder, Inst *throwable_inst);
    void RestoreTryEnd(const TryCodeBlock &try_block);

private:
    ArenaVector<BasicBlock *> blocks_;
    ArenaSet<uint32_t> catches_pc_;
    ArenaMultiMap<uint32_t, TryCodeBlock> try_blocks_;
    ArenaList<TryCodeBlock *> opened_try_blocks_;
    ArenaUnorderedSet<BasicBlock *> catch_handlers_;
    InstVector inst_defs_;
    RuntimeInterface::MethodPtr method_ = nullptr;
    bool is_inlined_graph_ {false};
    CallInst *caller_inst_ {nullptr};
};

}  // namespace panda::compiler

#endif  // PANDA_IR_BUILDER_H
