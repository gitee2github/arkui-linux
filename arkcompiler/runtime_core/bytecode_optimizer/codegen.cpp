/*
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
#include "codegen.h"
#include "common.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "generate_ecma.inl"

namespace panda::bytecodeopt {

using panda_file::LiteralTag;

void DoLda(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::ACC_REG_ID) {
        result.emplace_back(pandasm::Create_LDA(reg));
    }
}

void DoSta(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::ACC_REG_ID) {
        result.emplace_back(pandasm::Create_STA(reg));
    }
}

void BytecodeGen::AppendCatchBlock(uint32_t type_id, const compiler::BasicBlock *try_begin,
                                   const compiler::BasicBlock *try_end, const compiler::BasicBlock *catch_begin,
                                   const compiler::BasicBlock *catch_end)
{
    auto cb = pandasm::Function::CatchBlock();
    if (type_id != 0) {
        cb.exception_record = ir_interface_->GetTypeIdByOffset(type_id);
    }
    cb.try_begin_label = BytecodeGen::LabelName(try_begin->GetId());
    cb.try_end_label = "end_" + BytecodeGen::LabelName(try_end->GetId());
    cb.catch_begin_label = BytecodeGen::LabelName(catch_begin->GetId());
    cb.catch_end_label =
        catch_end == nullptr ? cb.catch_begin_label : "end_" + BytecodeGen::LabelName(catch_end->GetId());
    catch_blocks_.emplace_back(cb);
}

void BytecodeGen::VisitTryBegin(const compiler::BasicBlock *bb)
{
    ASSERT(bb->IsTryBegin());
    auto try_inst = GetTryBeginInst(bb);
    auto try_end = try_inst->GetTryEndBlock();
    ASSERT(try_end != nullptr && try_end->IsTryEnd());

    bb->EnumerateCatchHandlers([&, bb, try_end](BasicBlock *catch_handler, size_t type_id) {
        AppendCatchBlock(type_id, bb, try_end, catch_handler);
        return true;
    });
}

bool BytecodeGen::RunImpl()
{
    Reserve(function_->ins.size());
    TypeInfoComponents elements;
    AddTypeInfoIndexForArguments(&elements);
    bool need_handle_ins_type = GetGraph()->GetRuntime()->HasInsTypeinfo();
    int32_t insn_order = 0;
    for (auto *bb : GetGraph()->GetBlocksLinearOrder()) {
        EmitLabel(BytecodeGen::LabelName(bb->GetId()));
        if (bb->IsTryEnd() || bb->IsCatchEnd()) {
            auto label = "end_" + BytecodeGen::LabelName(bb->GetId());
            EmitLabel(label);
        }
        for (const auto &inst : bb->AllInsts()) {
            auto start = GetResult().size();
            VisitInstruction(inst);
            if (!GetStatus()) {
                return false;
            }
            auto end = GetResult().size();
            ASSERT(end >= start);
            for (auto i = start; i < end; ++i) {
                AddLineNumber(inst, i);
                AddColumnNumber(inst, i);
            }
            if (need_handle_ins_type && end > start) {
                // fill ins types. Need to exclude invalid ins as they do not emit
                insn_order += std::count_if(GetResult().begin() + start, GetResult().end(),
                                            [](const auto &ins) { return ins.opcode != pandasm::Opcode::INVALID; });
                AddTypeInfoIndexForIns(insn_order - 1, inst->GetId(), &elements);
            }
        }
        if (bb->NeedsJump()) {
            EmitJump(bb);
            insn_order++;
        }
    }
    if (!GetStatus()) {
        return false;
    }
    // Visit try-blocks in order they were declared
    for (auto *bb : GetGraph()->GetTryBeginBlocks()) {
        VisitTryBegin(bb);
    }
    function_->ins = std::move(GetResult());
    function_->catch_blocks = catch_blocks_;
    if (need_handle_ins_type) {
        UpdateTypeInfoIndexAnnotation(&elements);
    }
    return true;
}

void BytecodeGen::AddTypeInfoIndexForArguments(TypeInfoComponents *elements) const
{
    std::unordered_map<int32_t, TypeInfoIndex> args_types_map;
    if (GetGraph()->GetRuntime()->FillArgTypePairs(&args_types_map)) {
        for (const auto &[arg, type] : args_types_map) {
            ASSERT(arg < 0);
            AddOrderAndTypeInfoIndex(arg, type, elements);
        }
    }
}

void BytecodeGen::AddOrderAndTypeInfoIndex(int32_t order, TypeInfoIndex type, TypeInfoComponents *elements) const
{
    pandasm::LiteralArray::Literal order_tag;
    order_tag.tag_ = LiteralTag::TAGVALUE;
    order_tag.value_ = static_cast<uint8_t>(LiteralTag::INTEGER);
    elements->emplace_back(order_tag);

    pandasm::LiteralArray::Literal order_lit;
    order_lit.tag_ = LiteralTag::INTEGER;
    order_lit.value_ = bit_cast<uint32_t>(order);
    elements->emplace_back(order_lit);

    pandasm::LiteralArray::Literal type_tag;
    type_tag.tag_ = LiteralTag::TAGVALUE;
    pandasm::LiteralArray::Literal type_lit;
    if (std::holds_alternative<std::string>(type)) {
        type_tag.value_ = static_cast<uint8_t>(LiteralTag::LITERALARRAY);
        type_lit.tag_ = LiteralTag::LITERALARRAY;
        type_lit.value_ = std::get<std::string>(type);
    } else {
        type_tag.value_ = static_cast<uint8_t>(LiteralTag::BUILTINTYPEINDEX);
        type_lit.tag_ = LiteralTag::BUILTINTYPEINDEX;
        type_lit.value_ = std::get<BuiltinIndexType>(type);
    }

    elements->emplace_back(type_tag);
    elements->emplace_back(type_lit);
}

void BytecodeGen::AddTypeInfoIndexForIns(int32_t order, size_t id, TypeInfoComponents *elements) const
{
    auto type = GetGraph()->GetRuntime()->GetTypeInfoIndexByInstId(id);
    if (type != NO_EXPLICIT_TYPE) {
        AddOrderAndTypeInfoIndex(order, type, elements);
    }
}

void BytecodeGen::UpdateTypeInfoIndexAnnotation(const TypeInfoComponents *elements)
{
#ifndef NDEBUG
    LOG(DEBUG, BYTECODE_OPTIMIZER) << "Typeinfo after optimization for function : " << function_->name;
    const size_t PAIR_GAP = 4;  // 4: tag, order, tag, value, ...
    ASSERT(elements->size() % PAIR_GAP == 0);
    for (size_t i = 1; i < elements->size(); i += PAIR_GAP) {
        auto order = bit_cast<int32_t>(std::get<uint32_t>((*elements)[i].value_));
        const auto &element = (*elements)[i + 2];  // 2: gap between order and value
        if (element.tag_ == LiteralTag::LITERALARRAY) {
            auto type = std::get<std::string>(element.value_);
            LOG(DEBUG, BYTECODE_OPTIMIZER) << "[" << order << ", " << type << "], ";
        } else {
            ASSERT(element.tag_ == LiteralTag::BUILTINTYPEINDEX);
            auto type = std::get<BuiltinIndexType>(element.value_);
            LOG(DEBUG, BYTECODE_OPTIMIZER) << "[" << order << ", " << type << "], ";
        }
    }
#endif

    const auto &key = *(GetGraph()->GetRuntime()->GetTypeLiteralArrayKey());
    auto &litarr_table = GetProgram()->literalarray_table;
    ASSERT(litarr_table.find(key) != litarr_table.end());
    pandasm::LiteralArray array(*elements);
    litarr_table[key] = array;
}

void BytecodeGen::EmitJump(const BasicBlock *bb)
{
    BasicBlock *suc_bb = nullptr;
    ASSERT(bb != nullptr);

    if (bb->GetLastInst() == nullptr) {
        ASSERT(bb->IsEmpty());
        suc_bb = bb->GetSuccsBlocks()[0];
        result_.push_back(pandasm::Create_JMP(BytecodeGen::LabelName(suc_bb->GetId())));
        return;
    }

    ASSERT(bb->GetLastInst() != nullptr);
    switch (bb->GetLastInst()->GetOpcode()) {
        case Opcode::If:
        case Opcode::IfImm:
            ASSERT(bb->GetSuccsBlocks().size() == compiler::MAX_SUCCS_NUM);
            suc_bb = bb->GetFalseSuccessor();
            break;
        default:
            suc_bb = bb->GetSuccsBlocks()[0];
            break;
    }
    result_.push_back(pandasm::Create_JMP(BytecodeGen::LabelName(suc_bb->GetId())));
}

void BytecodeGen::AddLineNumber(const Inst *inst, const size_t idx)
{
    if (ir_interface_ != nullptr && idx < result_.size()) {
        auto ln = ir_interface_->GetLineNumberByPc(inst->GetPc());
        result_[idx].ins_debug.SetLineNumber(ln);
    }
}

void BytecodeGen::AddColumnNumber(const Inst *inst, const uint32_t idx)
{
    if (ir_interface_ != nullptr && idx < result_.size()) {
        auto cn = ir_interface_->GetLineNumberByPc(inst->GetPc());
        result_[idx].ins_debug.SetColumnNumber(cn);
    }
}

void BytecodeGen::EncodeSpillFillData(const compiler::SpillFillData &sf)
{
    if (sf.SrcType() != compiler::LocationType::REGISTER || sf.DstType() != compiler::LocationType::REGISTER) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "EncodeSpillFillData with unknown move type, src_type: "
                                       << static_cast<int>(sf.SrcType())
                                       << " dst_type: " << static_cast<int>(sf.DstType());
        success_ = false;
        UNREACHABLE();
        return;
    }
    ASSERT(sf.GetType() != compiler::DataType::NO_TYPE);
    ASSERT(sf.SrcValue() != compiler::INVALID_REG && sf.DstValue() != compiler::INVALID_REG);

    if (sf.SrcValue() == sf.DstValue()) {
        return;
    }

    pandasm::Ins move;
    if (GetGraph()->IsDynamicMethod()) {
        result_.emplace_back(pandasm::Create_MOV(sf.DstValue(), sf.SrcValue()));
        return;
    }
    UNREACHABLE();
}

void BytecodeGen::VisitSpillFill(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    for (auto sf : inst->CastToSpillFill()->GetSpillFills()) {
        enc->EncodeSpillFillData(sf);
    }
}

template <typename UnaryPred>
bool HasUserPredicate(Inst *inst, UnaryPred p)
{
    bool found = false;
    for (auto const &u : inst->GetUsers()) {
        if (p(u.GetInst())) {
            found = true;
            break;
        }
    }
    return found;
}

void BytecodeGen::VisitConstant(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    auto type = inst->GetType();

    /* Do not emit unused code for Const -> CastValueToAnyType chains */
    if (enc->GetGraph()->IsDynamicMethod()) {
        if (!HasUserPredicate(inst,
                              [](Inst const *i) { return i->GetOpcode() != compiler::Opcode::CastValueToAnyType; })) {
            return;
        }
    }

    pandasm::Ins movi;
    movi.regs.emplace_back(inst->GetDstReg());
    switch (type) {
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
            if (enc->GetGraph()->IsDynamicMethod()) {
                enc->result_.emplace_back(pandasm::Create_LDAI(inst->CastToConstant()->GetInt64Value()));
                DoSta(inst->GetDstReg(), enc->result_);
            } else {
                UNREACHABLE();
            }
            break;
        case compiler::DataType::FLOAT64:
            if (enc->GetGraph()->IsDynamicMethod()) {
                enc->result_.emplace_back(pandasm::Create_FLDAI(inst->CastToConstant()->GetDoubleValue()));
                DoSta(inst->GetDstReg(), enc->result_);
            } else {
                UNREACHABLE();
            }
            break;
        case compiler::DataType::BOOL:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT32:
        case compiler::DataType::UINT32:
            if (enc->GetGraph()->IsDynamicMethod()) {
                enc->result_.emplace_back(pandasm::Create_LDAI(inst->CastToConstant()->GetInt32Value()));
                DoSta(inst->GetDstReg(), enc->result_);
            } else {
                UNREACHABLE();
            }
            break;
        case compiler::DataType::FLOAT32:
            if (enc->GetGraph()->IsDynamicMethod()) {
                enc->result_.emplace_back(pandasm::Create_FLDAI(inst->CastToConstant()->GetFloatValue()));
                DoSta(inst->GetDstReg(), enc->result_);
            } else {
                UNREACHABLE();
            }
            break;
        default:
            UNREACHABLE();
            LOG(ERROR, BYTECODE_OPTIMIZER) << "VisitConstant with unknown type" << type;
            enc->success_ = false;
    }
}

void BytecodeGen::EncodeSta(compiler::Register reg, compiler::DataType::Type type)
{
    pandasm::Opcode opc;
    switch (type) {
        case compiler::DataType::ANY:
            opc = pandasm::Opcode::STA;
            break;
        default:
            UNREACHABLE();
            LOG(ERROR, BYTECODE_OPTIMIZER) << "EncodeSta with unknown type" << type;
            success_ = false;
    }
    pandasm::Ins sta;
    sta.opcode = opc;
    sta.regs.emplace_back(reg);

    result_.emplace_back(sta);
}

// NOLINTNEXTLINE(readability-function-size)
void BytecodeGen::VisitIf(GraphVisitor *v, Inst *inst_base)
{
    auto enc = static_cast<BytecodeGen *>(v);
    auto inst = inst_base->CastToIf();
    switch (inst->GetInputType(0)) {
        case compiler::DataType::ANY: {
            if (enc->GetGraph()->IsDynamicMethod()) {
#if defined(ENABLE_BYTECODE_OPT) && defined(PANDA_WITH_ECMASCRIPT) && defined(ARK_INTRINSIC_SET)
                IfEcma(v, inst);
                break;
#endif
            }
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
            break;
        }
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
    }
}

#if defined(ENABLE_BYTECODE_OPT) && defined(PANDA_WITH_ECMASCRIPT)
static std::optional<coretypes::TaggedValue> IsEcmaConstTemplate(Inst const *inst)
{
    if (inst->GetOpcode() != compiler::Opcode::CastValueToAnyType) {
        return {};
    }
    auto cvat_inst = inst->CastToCastValueToAnyType();
    if (!cvat_inst->GetInput(0).GetInst()->IsConst()) {
        return {};
    }
    auto const_inst = cvat_inst->GetInput(0).GetInst()->CastToConstant();

    switch (cvat_inst->GetAnyType()) {
        case compiler::AnyBaseType::ECMASCRIPT_UNDEFINED_TYPE:
            return coretypes::TaggedValue(coretypes::TaggedValue::VALUE_UNDEFINED);
        case compiler::AnyBaseType::ECMASCRIPT_INT_TYPE:
            return coretypes::TaggedValue(static_cast<int32_t>(const_inst->GetIntValue()));
        case compiler::AnyBaseType::ECMASCRIPT_DOUBLE_TYPE:
            return coretypes::TaggedValue(const_inst->GetDoubleValue());
        case compiler::AnyBaseType::ECMASCRIPT_BOOLEAN_TYPE:
            return coretypes::TaggedValue(static_cast<bool>(const_inst->GetInt64Value() != 0));
        case compiler::AnyBaseType::ECMASCRIPT_NULL_TYPE:
            return coretypes::TaggedValue(coretypes::TaggedValue::VALUE_NULL);
        default:
            return {};
    }
}

#if defined(ARK_INTRINSIC_SET)
void BytecodeGen::IfEcma(GraphVisitor *v, compiler::IfInst *inst)
{
    auto enc = static_cast<BytecodeGen *>(v);

    compiler::Register reg = compiler::INVALID_REG_ID;
    coretypes::TaggedValue cmp_val;

    auto test_lhs = IsEcmaConstTemplate(inst->GetInput(0).GetInst());
    auto test_rhs = IsEcmaConstTemplate(inst->GetInput(1).GetInst());

    if (test_lhs.has_value() && test_lhs->IsBoolean()) {
        cmp_val = test_lhs.value();
        reg = inst->GetSrcReg(1);
    } else if (test_rhs.has_value() && test_rhs->IsBoolean()) {
        cmp_val = test_rhs.value();
        reg = inst->GetSrcReg(0);
    } else {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
        enc->success_ = false;
        return;
    }

    DoLda(reg, enc->result_);
    switch (inst->GetCc()) {
        case compiler::CC_EQ: {
            if (cmp_val.IsTrue()) {
                enc->result_.emplace_back(
                    pandasm::Create_ECMA_JTRUE(LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId())));
            } else {
                enc->result_.emplace_back(
                    pandasm::Create_ECMA_JFALSE(LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId())));
            }
            break;
        }
        case compiler::CC_NE: {
            if (cmp_val.IsTrue()) {
                enc->result_.emplace_back(pandasm::Create_ECMA_ISTRUE());
            } else {
                enc->result_.emplace_back(pandasm::Create_ECMA_ISFALSE());
            }
            enc->result_.emplace_back(
                pandasm::Create_ECMA_JFALSE(LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId())));
            break;
        }
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
            return;
    }
}
#endif
#endif

void BytecodeGen::VisitIfImm(GraphVisitor *v, Inst *inst_base)
{
    auto inst = inst_base->CastToIfImm();
    auto imm = inst->GetImm();
    if (imm == 0) {
        IfImmZero(v, inst_base);
        return;
    }
    IfImmNonZero(v, inst_base);
}

void BytecodeGen::IfImmZero(GraphVisitor *v, Inst *inst_base)
{
    auto enc = static_cast<BytecodeGen *>(v);
    auto inst = inst_base->CastToIfImm();
    ASSERT(enc->GetGraph()->IsDynamicMethod());
    DoLda(inst->GetSrcReg(0), enc->result_);
    auto label = LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());
    switch (inst->GetCc()) {
        case compiler::CC_EQ:
            enc->result_.emplace_back(pandasm::Create_JEQZ(label));
            return;
        case compiler::CC_NE:
            enc->result_.emplace_back(pandasm::Create_JNEZ(label));
            return;
        default:
            UNREACHABLE();
    }
}

// NOLINTNEXTLINE(readability-function-size)
void BytecodeGen::IfImmNonZero(GraphVisitor *v, Inst *inst_base)
{
    UNREACHABLE();
}

// NOLINTNEXTLINE(readability-function-size)
void BytecodeGen::IfImm64(GraphVisitor *v, Inst *inst_base)
{
    UNREACHABLE();
}

// NOLINTNEXTLINE(readability-function-size)
void BytecodeGen::VisitCast(GraphVisitor *v, Inst *inst_base)
{
    UNREACHABLE();
}

void BytecodeGen::VisitLoadString(GraphVisitor *v, Inst *inst_base)
{
    pandasm::Ins ins;
    auto enc = static_cast<BytecodeGen *>(v);
    auto inst = inst_base->CastToLoadString();

    /* Do not emit unused code for Str -> CastValueToAnyType chains */
    if (enc->GetGraph()->IsDynamicMethod()) {
        if (!HasUserPredicate(inst,
                              [](Inst const *i) { return i->GetOpcode() != compiler::Opcode::CastValueToAnyType; })) {
            return;
        }
    }

    enc->result_.emplace_back(pandasm::Create_LDA_STR(enc->ir_interface_->GetStringIdByOffset(inst->GetTypeId())));
    if (inst->GetDstReg() != compiler::ACC_REG_ID) {
        if (enc->GetGraph()->IsDynamicMethod()) {
            enc->result_.emplace_back(pandasm::Create_STA(inst->GetDstReg()));
        } else {
            UNREACHABLE();
        }
    }
}

void BytecodeGen::VisitReturn(GraphVisitor *v, Inst *inst_base)
{
    pandasm::Ins ins;
    auto enc = static_cast<BytecodeGen *>(v);
    auto inst = inst_base->CastToReturn();
    switch (inst->GetType()) {
        case compiler::DataType::ANY: {
#if defined(ENABLE_BYTECODE_OPT) && defined(PANDA_WITH_ECMASCRIPT)
            auto test_arg = IsEcmaConstTemplate(inst->GetInput(0).GetInst());
            if (test_arg.has_value() && test_arg->IsUndefined()) {
                enc->result_.emplace_back(pandasm::Create_RETURNUNDEFINED());
                break;
            }
#endif
            DoLda(inst->GetSrcReg(0), enc->result_);
            enc->result_.emplace_back(pandasm::Create_RETURN());
            break;
        }
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
    }
}

void BytecodeGen::VisitCastValueToAnyType([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst_base)
{
    auto enc = static_cast<BytecodeGen *>(v);

#if defined(ENABLE_BYTECODE_OPT) && defined(PANDA_WITH_ECMASCRIPT)
    auto cvat = inst_base->CastToCastValueToAnyType();
    switch (cvat->GetAnyType()) {
        case compiler::AnyBaseType::ECMASCRIPT_NULL_TYPE:
            enc->result_.emplace_back(pandasm::Create_LDNULL());
            break;
        case compiler::AnyBaseType::ECMASCRIPT_UNDEFINED_TYPE:
            if (!HasUserPredicate(cvat,
                                  [](Inst const *inst) { return inst->GetOpcode() != compiler::Opcode::Return; })) {
                return;
            }
            enc->result_.emplace_back(pandasm::Create_LDUNDEFINED());
            break;
        case compiler::AnyBaseType::ECMASCRIPT_INT_TYPE: {
            ASSERT(cvat->GetInput(0).GetInst()->IsConst());
            auto input = cvat->GetInput(0).GetInst()->CastToConstant();
            enc->result_.emplace_back(pandasm::Create_LDAI(input->GetIntValue()));
            break;
        }
        case compiler::AnyBaseType::ECMASCRIPT_DOUBLE_TYPE: {
            ASSERT(cvat->GetInput(0).GetInst()->IsConst());
            auto input = cvat->GetInput(0).GetInst()->CastToConstant();
            enc->result_.emplace_back(pandasm::Create_FLDAI(input->GetDoubleValue()));
            break;
        }
        case compiler::AnyBaseType::ECMASCRIPT_BOOLEAN_TYPE: {
            ASSERT(cvat->GetInput(0).GetInst()->IsBoolConst());
            auto input = cvat->GetInput(0).GetInst()->CastToConstant();
            if (!HasUserPredicate(cvat, [](Inst const *inst) { return inst->GetOpcode() != compiler::Opcode::If; })) {
                return;
            }
            uint64_t val = input->GetInt64Value();
            if (val != 0) {
                enc->result_.emplace_back(pandasm::Create_LDTRUE());
            } else {
                enc->result_.emplace_back(pandasm::Create_LDFALSE());
            }
            break;
        }
        case compiler::AnyBaseType::ECMASCRIPT_STRING_TYPE: {
            auto input = cvat->GetInput(0).GetInst()->CastToLoadString();
            enc->result_.emplace_back(
                pandasm::Create_LDA_STR(enc->ir_interface_->GetStringIdByOffset(input->GetTypeId())));
            break;
        }
        default:
            return;
    }
    DoSta(cvat->GetDstReg(), enc->result_);
#else
    LOG(ERROR, BYTECODE_OPTIMIZER) << "Codegen for " << compiler::GetOpcodeString(inst_base->GetOpcode()) << " failed";
    enc->success_ = false;
#endif
}

// NOLINTNEXTLINE(readability-function-size)
void BytecodeGen::VisitIntrinsic(GraphVisitor *visitor, Inst *inst_base)
{
    ASSERT(inst_base->IsIntrinsic());
    auto inst = inst_base->CastToIntrinsic();
    auto enc = static_cast<BytecodeGen *>(visitor);

    if (!enc->GetGraph()->IsDynamicMethod()) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
        enc->success_ = false;
    } else {
#ifdef ENABLE_BYTECODE_OPT
        VisitEcma(visitor, inst_base);
#endif
    }
}

void BytecodeGen::VisitCatchPhi(GraphVisitor *v, Inst *inst)
{
    if (inst->CastToCatchPhi()->IsAcc()) {
        for (auto &user : inst->GetUsers()) {
            if (!user.GetInst()->IsSaveState()) {
                UNREACHABLE();
            }
        }
    }
}

#include "generated/codegen_intrinsics.cpp"
#include "generated/insn_selection.cpp"
}  // namespace panda::bytecodeopt
