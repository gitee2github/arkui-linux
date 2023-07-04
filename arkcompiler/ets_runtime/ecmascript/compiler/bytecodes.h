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

#ifndef ECMASCRIPT_COMPILER_BYTECODES_H
#define ECMASCRIPT_COMPILER_BYTECODES_H

#include <cstddef>
#include <array>

#include "libpandabase/macros.h"
#include "libpandabase/utils/bit_field.h"
#include "libpandafile/bytecode_instruction-inl.h"
#include "ecmascript/js_tagged_value.h"

namespace panda::ecmascript::kungfu {
using VRegIDType = uint32_t;
using ICSlotIdType = uint16_t;
using ImmValueType = uint64_t;
using EcmaOpcode = BytecodeInstruction::Opcode;

class BytecodeCircuitBuilder;
class Bytecodes;
class BytecodeInfo;
class BytecodeIterator;
enum BytecodeFlags : uint32_t {
    READ_ACC = 1 << 0, // 1: flag bit
    WRITE_ACC = 1 << 1, // 1: flag 1
    SUPPORT_DEOPT = 1 << 2, // 2: flag 2
    GENERAL_BC = 1 << 3,
    READ_THIS_OBJECT = 1 << 4,
    NO_SIDE_EFFECTS = 1 << 5,
    READ_ENV = 1 << 7,
    WRITE_ENV = 1 << 8,
};

enum BytecodeKind : uint32_t {
    GENERAL = 0,
    THROW_BC,
    RETURN_BC,
    JUMP_IMM,
    CONDITIONAL_JUMP,
    MOV,
    SET_CONSTANT,
    SUSPEND,
    RESUME,
    DISCARDED,
};

class BytecodeMetaData {
public:
    static constexpr uint32_t MAX_OPCODE_SIZE = 16;
    static constexpr uint32_t MAX_SIZE_BITS = 4;
    static constexpr uint32_t BYTECODE_FLAGS_SIZE = 9;
    static constexpr uint32_t BYTECODE_KIND_SIZE = 4;

    using OpcodeField = panda::BitField<EcmaOpcode, 0, MAX_OPCODE_SIZE>;
    using SizeField = OpcodeField::NextField<size_t, MAX_SIZE_BITS>;
    using KindField = SizeField::NextField<BytecodeKind, BYTECODE_KIND_SIZE>;
    using FlagsField = KindField::NextField<BytecodeFlags, BYTECODE_FLAGS_SIZE>;

    bool HasAccIn() const
    {
        return HasFlag(BytecodeFlags::READ_ACC);
    }

    bool IsNoSideEffects() const
    {
        return HasFlag(BytecodeFlags::NO_SIDE_EFFECTS);
    }

    bool HasThisIn() const
    {
        return HasFlag(BytecodeFlags::READ_THIS_OBJECT);
    }

    bool HasAccOut() const
    {
        return HasFlag(BytecodeFlags::WRITE_ACC);
    }

    bool HasEnvIn() const
    {
        return HasFlag(BytecodeFlags::READ_ENV);
    }

    bool HasEnvOut() const
    {
        return HasFlag(BytecodeFlags::WRITE_ENV);
    }

    bool IsMov() const
    {
        return GetKind() == BytecodeKind::MOV;
    }

    bool IsReturn() const
    {
        return GetKind() == BytecodeKind::RETURN_BC;
    }

    bool IsThrow() const
    {
        return GetKind() == BytecodeKind::THROW_BC;
    }

    bool IsJump() const
    {
        return IsJumpImm() || IsCondJump();
    }

    bool IsCondJump() const
    {
        return GetKind() == BytecodeKind::CONDITIONAL_JUMP;
    }

    bool IsJumpImm() const
    {
        return GetKind() == BytecodeKind::JUMP_IMM;
    }

    bool IsSuspend() const
    {
        return GetKind() == BytecodeKind::SUSPEND;
    }

    bool IsSetConstant() const
    {
        return GetKind() == BytecodeKind::SET_CONSTANT;
    }

    bool SupportDeopt() const
    {
        return HasFlag(BytecodeFlags::SUPPORT_DEOPT);
    }

    size_t GetSize() const
    {
        return SizeField::Get(value_);
    }

    bool IsGeneral() const
    {
        return HasFlag(BytecodeFlags::GENERAL_BC);
    }

    bool IsGeneratorRelative() const
    {
        return (GetKind() == BytecodeKind::RESUME) || (GetKind() == BytecodeKind::SUSPEND);
    }

    bool IsDiscarded() const
    {
        return GetKind() == BytecodeKind::DISCARDED;
    }

    inline EcmaOpcode GetOpcode() const
    {
        return OpcodeField::Get(value_);
    }

private:
    BytecodeMetaData() = default;
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(BytecodeMetaData);
    DEFAULT_COPY_SEMANTIC(BytecodeMetaData);
    BytecodeMetaData(uint64_t value) : value_(value) {}

    static BytecodeMetaData InitBytecodeMetaData(const uint8_t *pc);

    inline bool HasFlag(BytecodeFlags flag) const
    {
        return (GetFlags() & flag) == flag;
    }

    inline BytecodeFlags GetFlags() const
    {
        return FlagsField::Get(value_);
    }

    inline BytecodeKind GetKind() const
    {
        return KindField::Get(value_);
    }

    uint64_t value_ {0};
    friend class Bytecodes;
    friend class BytecodeInfo;
    friend class BytecodeCircuitBuilder;
};

class Bytecodes {
public:
    static constexpr uint32_t NUM_BYTECODES = 0xFF;
    static constexpr uint32_t OPCODE_MASK = 0xFF00;
    static constexpr uint32_t BYTE_SIZE = 8;
    static constexpr uint32_t CALLRUNTIME_PREFIX_OPCODE_INDEX = 251;
    static constexpr uint32_t DEPRECATED_PREFIX_OPCODE_INDEX = 252;
    static constexpr uint32_t WIDE_PREFIX_OPCODE_INDEX = 253;
    static constexpr uint32_t THROW_PREFIX_OPCODE_INDEX = 254;
    static constexpr uint32_t MIN_PREFIX_OPCODE_INDEX = CALLRUNTIME_PREFIX_OPCODE_INDEX;

    static constexpr uint32_t LAST_OPCODE =
        static_cast<uint32_t>(EcmaOpcode::GETASYNCITERATOR_IMM8);
    static constexpr uint32_t LAST_DEPRECATED_OPCODE =
        static_cast<uint32_t>(EcmaOpcode::DEPRECATED_DYNAMICIMPORT_PREF_V8);
    static constexpr uint32_t LAST_WIDE_OPCODE =
        static_cast<uint32_t>(EcmaOpcode::WIDE_STPATCHVAR_PREF_IMM16);
    static constexpr uint32_t LAST_THROW_OPCODE =
        static_cast<uint32_t>(EcmaOpcode::THROW_UNDEFINEDIFHOLEWITHNAME_PREF_ID16);
    static constexpr uint32_t LAST_CALLRUNTIME_OPCODE =
        static_cast<uint32_t>(EcmaOpcode::CALLRUNTIME_NOTIFYCONCURRENTRESULT_PREF_NONE);

    static_assert(CALLRUNTIME_PREFIX_OPCODE_INDEX ==
        static_cast<uint32_t>(EcmaOpcode::CALLRUNTIME_NOTIFYCONCURRENTRESULT_PREF_NONE));
    static_assert(DEPRECATED_PREFIX_OPCODE_INDEX ==
        static_cast<uint32_t>(EcmaOpcode::DEPRECATED_LDLEXENV_PREF_NONE));
    static_assert(WIDE_PREFIX_OPCODE_INDEX ==
        static_cast<uint32_t>(EcmaOpcode::WIDE_CREATEOBJECTWITHEXCLUDEDKEYS_PREF_IMM16_V8_V8));
    static_assert(THROW_PREFIX_OPCODE_INDEX ==
        static_cast<uint32_t>(EcmaOpcode::THROW_PREF_NONE));

    Bytecodes();
    Bytecodes(const Bytecodes&) = delete;
    void operator=(const Bytecodes&) = delete;

    static EcmaOpcode GetOpcode(const uint8_t *pc)
    {
        uint8_t primary = ReadByte(pc);
        if (primary >= MIN_PREFIX_OPCODE_INDEX) {
            uint8_t secondary = ReadByte1(pc);
            return static_cast<EcmaOpcode>((secondary << 8U) | primary); // 8: byte size
        }
        return static_cast<EcmaOpcode>(primary);
    }

    BytecodeMetaData GetBytecodeMetaData(const uint8_t *pc) const
    {
        uint8_t primary = ReadByte(pc);
        if (primary >= MIN_PREFIX_OPCODE_INDEX) {
            uint8_t secondary = ReadByte1(pc);
            if (primary == CALLRUNTIME_PREFIX_OPCODE_INDEX) {
                return callRuntimeBytecodes_[secondary];
            } else if (primary == DEPRECATED_PREFIX_OPCODE_INDEX) {
                return deprecatedBytecodes_[secondary];
            } else if (primary == WIDE_PREFIX_OPCODE_INDEX) {
                return wideBytecodes_[secondary];
            } else {
                ASSERT(primary == THROW_PREFIX_OPCODE_INDEX);
                return throwBytecodes_[secondary];
            }
        }
        return bytecodes_[primary];
    }

private:
    static uint8_t ReadByte(const uint8_t *pc)
    {
        return *pc;
    }
    static uint8_t ReadByte1(const uint8_t *pc)
    {
        return *(pc + 1); // 1: byte1
    }
    BytecodeMetaData InitBytecodeMetaData(const uint8_t *pc);

    std::array<BytecodeMetaData, NUM_BYTECODES> bytecodes_{};
    std::array<BytecodeMetaData, NUM_BYTECODES> callRuntimeBytecodes_{};
    std::array<BytecodeMetaData, NUM_BYTECODES> deprecatedBytecodes_{};
    std::array<BytecodeMetaData, NUM_BYTECODES> wideBytecodes_{};
    std::array<BytecodeMetaData, NUM_BYTECODES> throwBytecodes_{};
};

enum class ConstDataIDType : uint8_t {
    StringIDType,
    MethodIDType,
    ArrayLiteralIDType,
    ObjectLiteralIDType,
    ClassLiteralIDType,
};

class VirtualRegister {
public:
    explicit VirtualRegister(VRegIDType id) : id_(id)
    {
    }
    ~VirtualRegister() = default;

    void SetId(VRegIDType id)
    {
        id_ = id;
    }

    VRegIDType GetId() const
    {
        return id_;
    }

private:
    VRegIDType id_;
};

class Immediate {
public:
    explicit Immediate(ImmValueType value) : value_(value)
    {
    }
    ~Immediate() = default;

    void SetValue(ImmValueType value)
    {
        value_ = value;
    }

    ImmValueType ToJSTaggedValueInt() const
    {
        return value_ | JSTaggedValue::TAG_INT;
    }

    ImmValueType ToJSTaggedValueDouble() const
    {
        return JSTaggedValue(bit_cast<double>(value_)).GetRawData();
    }

    ImmValueType GetValue() const
    {
        return value_;
    }

private:
    ImmValueType value_;
};


class ICSlotId {
public:
    explicit ICSlotId(ICSlotIdType id) : id_(id)
    {
    }
    ~ICSlotId() = default;

    void SetId(ICSlotIdType id)
    {
        id_ = id;
    }

    ICSlotIdType GetId() const
    {
        return id_;
    }

private:
    ICSlotIdType id_;
};

class ConstDataId {
public:
    explicit ConstDataId(ConstDataIDType type, uint16_t id)
        :type_(type), id_(id)
    {
    }

    explicit ConstDataId(uint64_t bitfield)
    {
        type_ = ConstDataIDType(bitfield >> TYPE_SHIFT);
        id_ = bitfield & ((1 << TYPE_SHIFT) - 1);
    }

    ~ConstDataId() = default;

    void SetId(uint16_t id)
    {
        id_ = id;
    }

    uint16_t GetId() const
    {
        return id_;
    }

    void SetType(ConstDataIDType type)
    {
        type_ = type;
    }

    ConstDataIDType GetType() const
    {
        return type_;
    }

    bool IsStringId() const
    {
        return type_ == ConstDataIDType::StringIDType;
    }

    bool IsMethodId() const
    {
        return type_ == ConstDataIDType::MethodIDType;
    }

    bool IsClassLiteraId() const
    {
        return type_ == ConstDataIDType::ClassLiteralIDType;
    }

    bool IsObjectLiteralID() const
    {
        return type_ == ConstDataIDType::ObjectLiteralIDType;
    }

    bool IsArrayLiteralID() const
    {
        return type_ == ConstDataIDType::ArrayLiteralIDType;
    }

    uint64_t CaculateBitField() const
    {
        return (static_cast<uint8_t>(type_) << TYPE_SHIFT) | id_;
    }

private:
    static constexpr int TYPE_SHIFT = 16;
    ConstDataIDType type_;
    uint16_t id_;
};

class BytecodeInfo {
public:
    // set of id, immediate and read register
    std::vector<std::variant<ConstDataId, ICSlotId, Immediate, VirtualRegister>> inputs {};
    std::vector<VRegIDType> vregOut {}; // write register

    bool Deopt() const
    {
        return metaData_.SupportDeopt();
    }

    bool AccOut() const
    {
        return metaData_.HasAccOut();
    }

    bool AccIn() const
    {
        return metaData_.HasAccIn();
    }

    bool EnvIn() const
    {
        return metaData_.HasEnvIn();
    }

    bool EnvOut() const
    {
        return metaData_.HasEnvOut();
    }

    bool NoSideEffects() const
    {
        return metaData_.IsNoSideEffects();
    }

    bool ThisObjectIn() const
    {
        return metaData_.HasThisIn();
    }

    size_t GetSize() const
    {
        return metaData_.GetSize();
    }

    bool IsDef() const
    {
        return (!vregOut.empty()) || AccOut();
    }

    bool IsOut(VRegIDType reg, uint32_t index) const
    {
        bool isDefined = (!vregOut.empty() && (reg == vregOut.at(index)));
        return isDefined;
    }

    bool IsMov() const
    {
        return metaData_.IsMov();
    }

    bool IsJump() const
    {
        return metaData_.IsJump();
    }

    bool IsCondJump() const
    {
        return metaData_.IsCondJump();
    }

    bool IsReturn() const
    {
        return metaData_.IsReturn();
    }

    bool IsThrow() const
    {
        return metaData_.IsThrow();
    }

    bool IsSuspend() const
    {
        return metaData_.IsSuspend();
    }

    bool IsDiscarded() const
    {
        return metaData_.IsDiscarded();
    }

    bool IsSetConstant() const
    {
        return metaData_.IsSetConstant();
    }

    bool IsGeneral() const
    {
        return metaData_.IsGeneral();
    }

    bool IsGeneratorRelative() const
    {
        return metaData_.IsGeneratorRelative();
    }

    size_t ComputeValueInputCount() const
    {
        return (AccIn() ? 1 : 0) + (ThisObjectIn() ? 1 : 0) + inputs.size();
    }

    size_t ComputeOutCount() const
    {
        return (AccOut() ? 1 : 0) + vregOut.size();
    }

    bool IsBc(EcmaOpcode ecmaOpcode) const
    {
        return metaData_.GetOpcode() == ecmaOpcode;
    }

    inline EcmaOpcode GetOpcode() const
    {
        return metaData_.GetOpcode();
    }

    static void InitBytecodeInfo(BytecodeCircuitBuilder *builder,
        BytecodeInfo &info, const uint8_t* pc);

private:
    BytecodeMetaData metaData_ { 0 };
    friend class BytecodeCircuitBuilder;
};

class BytecodeIterator {
public:
    BytecodeIterator() = default;
    explicit BytecodeIterator(BytecodeCircuitBuilder *builder,
        uint32_t start, uint32_t end)
        : builder_(builder), start_(start), end_(end) {}
    void Reset(BytecodeCircuitBuilder *builder,
        uint32_t start, uint32_t end)
    {
        builder_ = builder;
        start_ = start;
        end_ = end;
    }

    BytecodeIterator& operator++()
    {
        if (InRange()) {
            index_++;
        }
        return *this;
    }
    BytecodeIterator& operator--()
    {
        if (InRange()) {
            index_--;
        }
        return *this;
    }

    void Goto(uint32_t i)
    {
        index_ = i;
    }

    void GotoStart()
    {
        index_ = start_;
        ASSERT(InRange());
    }

    void GotoEnd()
    {
        index_ = end_;
        ASSERT(InRange());
    }

    bool InRange() const
    {
        return (index_ <= end_) && (index_ >= start_);
    }

    bool Done() const
    {
        return !InRange();
    }

    uint32_t Index() const
    {
        return index_;
    }

    const BytecodeInfo &GetBytecodeInfo() const;
    const uint8_t *PeekNextPc(size_t i) const;
    const uint8_t *PeekPrevPc(size_t i) const;

private:
    static constexpr int32_t INVALID_INDEX = -1;

    BytecodeCircuitBuilder *builder_ {nullptr};
    uint32_t start_ {0};
    uint32_t end_ {0};
    uint32_t index_{ INVALID_INDEX };
};
}  // panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_BYTECODES_H