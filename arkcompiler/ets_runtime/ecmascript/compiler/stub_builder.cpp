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

#include "ecmascript/compiler/stub_builder-inl.h"

#include "ecmascript/compiler/assembler_module.h"
#include "ecmascript/compiler/llvm_ir_builder.h"
#include "ecmascript/compiler/rt_call_signature.h"
#include "ecmascript/js_api/js_api_arraylist.h"
#include "ecmascript/js_api/js_api_vector.h"
#include "ecmascript/js_object.h"
#include "ecmascript/mem/remembered_set.h"
#include "ecmascript/message_string.h"
#include "ecmascript/tagged_dictionary.h"
#include "ecmascript/tagged_hash_table.h"

namespace panda::ecmascript::kungfu {
void StubBuilder::Jump(Label *label)
{
    ASSERT(label);
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto jump = env_->GetBuilder()->Goto(currentControl);
    currentLabel->SetControl(jump);
    label->AppendPredecessor(currentLabel);
    label->MergeControl(currentLabel->GetControl());
    env_->SetCurrentLabel(nullptr);
}

void StubBuilder::Branch(GateRef condition, Label *trueLabel, Label *falseLabel)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    GateRef ifBranch = env_->GetBuilder()->Branch(currentControl, condition);
    currentLabel->SetControl(ifBranch);
    GateRef ifTrue = env_->GetBuilder()->IfTrue(ifBranch);
    trueLabel->AppendPredecessor(env_->GetCurrentLabel());
    trueLabel->MergeControl(ifTrue);
    GateRef ifFalse = env_->GetBuilder()->IfFalse(ifBranch);
    falseLabel->AppendPredecessor(env_->GetCurrentLabel());
    falseLabel->MergeControl(ifFalse);
    env_->SetCurrentLabel(nullptr);
}

void StubBuilder::Switch(GateRef index, Label *defaultLabel, int64_t *keysValue, Label *keysLabel, int numberOfKeys)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    GateRef switchBranch = env_->GetBuilder()->SwitchBranch(currentControl, index, numberOfKeys);
    currentLabel->SetControl(switchBranch);
    for (int i = 0; i < numberOfKeys; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        GateRef switchCase = env_->GetBuilder()->SwitchCase(switchBranch, keysValue[i]);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        keysLabel[i].AppendPredecessor(currentLabel);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        keysLabel[i].MergeControl(switchCase);
    }

    GateRef defaultCase = env_->GetBuilder()->DefaultCase(switchBranch);
    defaultLabel->AppendPredecessor(currentLabel);
    defaultLabel->MergeControl(defaultCase);
    env_->SetCurrentLabel(nullptr);
}

void StubBuilder::LoopBegin(Label *loopHead)
{
    ASSERT(loopHead);
    auto loopControl = env_->GetBuilder()->LoopBegin(loopHead->GetControl());
    loopHead->SetControl(loopControl);
    loopHead->SetPreControl(loopControl);
    loopHead->Bind();
    env_->SetCurrentLabel(loopHead);
}

void StubBuilder::LoopEnd(Label *loopHead)
{
    ASSERT(loopHead);
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto loopend = env_->GetBuilder()->LoopEnd(currentControl);
    currentLabel->SetControl(loopend);
    loopHead->AppendPredecessor(currentLabel);
    loopHead->MergeControl(loopend);
    loopHead->Seal();
    loopHead->MergeAllControl();
    loopHead->MergeAllDepend();
    env_->SetCurrentLabel(nullptr);
}

// FindElementWithCache in ecmascript/layout_info-inl.h
GateRef StubBuilder::FindElementWithCache(GateRef glue, GateRef layoutInfo, GateRef hclass,
    GateRef key, GateRef propsNum)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->SubCfgEntry(&subEntry);
    DEFVARIABLE(result, VariableType::INT32(), Int32(-1));
    DEFVARIABLE(i, VariableType::INT32(), Int32(0));
    Label exit(env);
    Label notExceedUpper(env);
    Label exceedUpper(env);
    Label afterExceedCon(env);
    // 9 : Builtins Object properties number is nine
    Branch(Int32LessThanOrEqual(propsNum, Int32(9)), &notExceedUpper, &exceedUpper);
    {
        Bind(&notExceedUpper);
            Label loopHead(env);
            Label loopEnd(env);
            Label afterLoop(env);
            Jump(&loopHead);
            LoopBegin(&loopHead);
            {
                Label propsNumIsZero(env);
                Label propsNumNotZero(env);
                Branch(Int32Equal(propsNum, Int32(0)), &propsNumIsZero, &propsNumNotZero);
                Bind(&propsNumIsZero);
                Jump(&afterLoop);
                Bind(&propsNumNotZero);
                GateRef elementAddr = GetPropertiesAddrFromLayoutInfo(layoutInfo);
                GateRef keyInProperty = Load(VariableType::JS_ANY(),
                                             elementAddr,
                                             PtrMul(ZExtInt32ToPtr(*i),
                                                    IntPtr(sizeof(panda::ecmascript::Properties))));
                Label equal(env);
                Label notEqual(env);
                Label afterEqualCon(env);
                Branch(Equal(keyInProperty, key), &equal, &notEqual);
                Bind(&equal);
                result = *i;
                Jump(&exit);
                Bind(&notEqual);
                Jump(&afterEqualCon);
                Bind(&afterEqualCon);
                i = Int32Add(*i, Int32(1));
                Branch(Int32UnsignedLessThan(*i, propsNum), &loopEnd, &afterLoop);
                Bind(&loopEnd);
                LoopEnd(&loopHead);
            }
            Bind(&afterLoop);
            result = Int32(-1);
            Jump(&exit);
        Bind(&exceedUpper);
        Jump(&afterExceedCon);
    }
    Bind(&afterExceedCon);
    result = CallNGCRuntime(glue, RTSTUB_ID(FindElementWithCache), { glue, hclass, key, propsNum });
    Jump(&exit);
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::FindElementFromNumberDictionary(GateRef glue, GateRef elements, GateRef index)
{
    auto env = GetEnvironment();
    Label subentry(env);
    env->SubCfgEntry(&subentry);
    DEFVARIABLE(result, VariableType::INT32(), Int32(-1));
    Label exit(env);
    GateRef capcityoffset =
        PtrMul(IntPtr(JSTaggedValue::TaggedTypeSize()),
               IntPtr(TaggedHashTable<NumberDictionary>::SIZE_INDEX));
    GateRef dataoffset = IntPtr(TaggedArray::DATA_OFFSET);
    GateRef capacity = GetInt32OfTInt(Load(VariableType::INT64(), elements,
                                           PtrAdd(dataoffset, capcityoffset)));
    DEFVARIABLE(count, VariableType::INT32(), Int32(1));
    GateRef len = Int32(sizeof(int) / sizeof(uint8_t));
    GateRef hash = CallRuntime(glue, RTSTUB_ID(GetHash32),
        { IntToTaggedInt(index), IntToTaggedInt(len) });
    DEFVARIABLE(entry, VariableType::INT32(),
        Int32And(TruncInt64ToInt32(ChangeTaggedPointerToInt64(hash)), Int32Sub(capacity, Int32(1))));
    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Jump(&loopHead);
    LoopBegin(&loopHead);
    GateRef element = GetKeyFromDictionary<NumberDictionary>(elements, *entry);
    Label isHole(env);
    Label notHole(env);
    Branch(TaggedIsHole(element), &isHole, &notHole);
    Bind(&isHole);
    Jump(&loopEnd);
    Bind(&notHole);
    Label isUndefined(env);
    Label notUndefined(env);
    Branch(TaggedIsUndefined(element), &isUndefined, &notUndefined);
    Bind(&isUndefined);
    result = Int32(-1);
    Jump(&exit);
    Bind(&notUndefined);
    Label isMatch(env);
    Label notMatch(env);
    Branch(Int32Equal(index, GetInt32OfTInt(element)), &isMatch, &notMatch);
    Bind(&isMatch);
    result = *entry;
    Jump(&exit);
    Bind(&notMatch);
    Jump(&loopEnd);
    Bind(&loopEnd);
    entry = GetNextPositionForHash(*entry, *count, capacity);
    count = Int32Add(*count, Int32(1));
    LoopEnd(&loopHead);
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

// int TaggedHashTable<Derived>::FindEntry(const JSTaggedValue &key) in tagged_hash_table.h
GateRef StubBuilder::FindEntryFromNameDictionary(GateRef glue, GateRef elements, GateRef key)
{
    auto env = GetEnvironment();
    Label funcEntry(env);
    env->SubCfgEntry(&funcEntry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::INT32(), Int32(-1));
    GateRef capcityoffset =
        PtrMul(IntPtr(JSTaggedValue::TaggedTypeSize()),
               IntPtr(TaggedHashTable<NumberDictionary>::SIZE_INDEX));
    GateRef dataoffset = IntPtr(TaggedArray::DATA_OFFSET);
    GateRef capacity = GetInt32OfTInt(Load(VariableType::INT64(), elements,
                                           PtrAdd(dataoffset, capcityoffset)));
    DEFVARIABLE(count, VariableType::INT32(), Int32(1));
    DEFVARIABLE(hash, VariableType::INT32(), Int32(0));
    // NameDictionary::hash
    Label isSymbol(env);
    Label notSymbol(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Label beforeDefineHash(env);
    Branch(IsSymbol(key), &isSymbol, &notSymbol);
    Bind(&isSymbol);
    {
        hash = GetInt32OfTInt(Load(VariableType::INT64(), key,
            IntPtr(JSSymbol::HASHFIELD_OFFSET)));
        Jump(&beforeDefineHash);
    }
    Bind(&notSymbol);
    {
        Label isString(env);
        Label notString(env);
        Branch(IsString(key), &isString, &notString);
        Bind(&isString);
        {
            hash = GetHashcodeFromString(glue, key);
            Jump(&beforeDefineHash);
        }
        Bind(&notString);
        {
            Jump(&beforeDefineHash);
        }
    }
    Bind(&beforeDefineHash);
    // GetFirstPosition(hash, size)
    DEFVARIABLE(entry, VariableType::INT32(), Int32And(*hash, Int32Sub(capacity, Int32(1))));
    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        GateRef element = GetKeyFromDictionary<NameDictionary>(elements, *entry);
        Label isHole(env);
        Label notHole(env);
        Branch(TaggedIsHole(element), &isHole, &notHole);
        {
            Bind(&isHole);
            {
                Jump(&loopEnd);
            }
            Bind(&notHole);
            {
                Label isUndefined(env);
                Label notUndefined(env);
                Branch(TaggedIsUndefined(element), &isUndefined, &notUndefined);
                {
                    Bind(&isUndefined);
                    {
                        result = Int32(-1);
                        Jump(&exit);
                    }
                    Bind(&notUndefined);
                    {
                        Label isMatch(env);
                        Label notMatch(env);
                        Branch(Equal(key, element), &isMatch, &notMatch);
                        {
                            Bind(&isMatch);
                            {
                                result = *entry;
                                Jump(&exit);
                            }
                            Bind(&notMatch);
                            {
                                Jump(&loopEnd);
                            }
                        }
                    }
                }
            }
        }
        Bind(&loopEnd);
        {
            entry = GetNextPositionForHash(*entry, *count, capacity);
            count = Int32Add(*count, Int32(1));
            LoopEnd(&loopHead);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::IsMatchInTransitionDictionary(GateRef element, GateRef key, GateRef metaData, GateRef attr)
{
    return BoolAnd(Equal(element, key), Int32Equal(metaData, attr));
}

// metaData is int32 type
GateRef StubBuilder::FindEntryFromTransitionDictionary(GateRef glue, GateRef elements, GateRef key, GateRef metaData)
{
    auto env = GetEnvironment();
    Label funcEntry(env);
    env->SubCfgEntry(&funcEntry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::INT32(), Int32(-1));
    GateRef capcityoffset =
        PtrMul(IntPtr(JSTaggedValue::TaggedTypeSize()),
               IntPtr(TaggedHashTable<NumberDictionary>::SIZE_INDEX));
    GateRef dataoffset = IntPtr(TaggedArray::DATA_OFFSET);
    GateRef capacity = GetInt32OfTInt(Load(VariableType::INT64(), elements,
                                           PtrAdd(dataoffset, capcityoffset)));
    DEFVARIABLE(count, VariableType::INT32(), Int32(1));
    DEFVARIABLE(hash, VariableType::INT32(), Int32(0));
    // TransitionDictionary::hash
    Label isSymbol(env);
    Label notSymbol(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Label beforeDefineHash(env);
    Branch(IsSymbol(key), &isSymbol, &notSymbol);
    Bind(&isSymbol);
    {
        hash = GetInt32OfTInt(Load(VariableType::INT64(), key,
            IntPtr(panda::ecmascript::JSSymbol::HASHFIELD_OFFSET)));
        Jump(&beforeDefineHash);
    }
    Bind(&notSymbol);
    {
        Label isString(env);
        Label notString(env);
        Branch(IsString(key), &isString, &notString);
        Bind(&isString);
        {
            hash = GetHashcodeFromString(glue, key);
            Jump(&beforeDefineHash);
        }
        Bind(&notString);
        {
            Jump(&beforeDefineHash);
        }
    }
    Bind(&beforeDefineHash);
    hash = Int32Add(*hash, metaData);
    // GetFirstPosition(hash, size)
    DEFVARIABLE(entry, VariableType::INT32(), Int32And(*hash, Int32Sub(capacity, Int32(1))));
    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        GateRef element = GetKeyFromDictionary<TransitionsDictionary>(elements, *entry);
        Label isHole(env);
        Label notHole(env);
        Branch(TaggedIsHole(element), &isHole, &notHole);
        {
            Bind(&isHole);
            {
                Jump(&loopEnd);
            }
            Bind(&notHole);
            {
                Label isUndefined(env);
                Label notUndefined(env);
                Branch(TaggedIsUndefined(element), &isUndefined, &notUndefined);
                {
                    Bind(&isUndefined);
                    {
                        result = Int32(-1);
                        Jump(&exit);
                    }
                    Bind(&notUndefined);
                    {
                        Label isMatch(env);
                        Label notMatch(env);
                        Branch(
                            IsMatchInTransitionDictionary(element, key, metaData,
                                GetAttributesFromDictionary<TransitionsDictionary>(elements, *entry)),
                            &isMatch, &notMatch);
                        {
                            Bind(&isMatch);
                            {
                                result = *entry;
                                Jump(&exit);
                            }
                            Bind(&notMatch);
                            {
                                Jump(&loopEnd);
                            }
                        }
                    }
                }
            }
        }
        Bind(&loopEnd);
        {
            entry = GetNextPositionForHash(*entry, *count, capacity);
            count = Int32Add(*count, Int32(1));
            LoopEnd(&loopHead);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::JSObjectGetProperty(GateRef obj, GateRef hclass, GateRef attr)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Undefined());
    Label inlinedProp(env);
    Label notInlinedProp(env);
    GateRef attrOffset = GetOffsetFieldInPropAttr(attr);
    Branch(IsInlinedProperty(attr), &inlinedProp, &notInlinedProp);
    {
        Bind(&inlinedProp);
        {
            result = GetPropertyInlinedProps(obj, hclass, attrOffset);
            Jump(&exit);
        }
        Bind(&notInlinedProp);
        {
            // compute outOfLineProp offset, get it and return
            GateRef array =
                Load(VariableType::INT64(), obj, IntPtr(JSObject::PROPERTIES_OFFSET));
            result = GetValueFromTaggedArray(array, Int32Sub(attrOffset,
                GetInlinedPropertiesFromHClass(hclass)));
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

void StubBuilder::JSObjectSetProperty(GateRef glue, GateRef obj, GateRef hclass, GateRef attr, GateRef value)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->SubCfgEntry(&subEntry);
    Label exit(env);
    Label inlinedProp(env);
    Label notInlinedProp(env);
    GateRef attrOffset = GetOffsetFieldInPropAttr(attr);
    Branch(IsInlinedProperty(attr), &inlinedProp, &notInlinedProp);
    {
        Bind(&inlinedProp);
        {
            SetPropertyInlinedProps(glue, obj, hclass, value, attrOffset);
            Jump(&exit);
        }
        Bind(&notInlinedProp);
        {
            // compute outOfLineProp offset, get it and return
            GateRef array = Load(VariableType::JS_POINTER(), obj,
                                 IntPtr(JSObject::PROPERTIES_OFFSET));
            SetValueToTaggedArray(VariableType::JS_ANY(), glue, array, Int32Sub(attrOffset,
                GetInlinedPropertiesFromHClass(hclass)), value);
            Jump(&exit);
        }
    }
    Bind(&exit);
    env->SubCfgExit();
    return;
}

GateRef StubBuilder::ComputePropertyCapacityInJSObj(GateRef oldLength)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->SubCfgEntry(&subEntry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::INT32(), Int32(0));
    GateRef newL = Int32Add(oldLength, Int32(JSObject::PROPERTIES_GROW_SIZE));
    Label reachMax(env);
    Label notReachMax(env);
    Branch(Int32GreaterThan(newL, Int32(JSHClass::MAX_CAPACITY_OF_OUT_OBJECTS)),
        &reachMax, &notReachMax);
    {
        Bind(&reachMax);
        result = Int32(JSHClass::MAX_CAPACITY_OF_OUT_OBJECTS);
        Jump(&exit);
        Bind(&notReachMax);
        result = newL;
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::CallGetterHelper(GateRef glue, GateRef receiver, GateRef holder, GateRef accessor)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->SubCfgEntry(&subEntry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Exception());

    Label isInternal(env);
    Label notInternal(env);
    Branch(IsAccessorInternal(accessor), &isInternal, &notInternal);
    Bind(&isInternal);
    {
        Label arrayLength(env);
        Label tryContinue(env);
        auto lengthAccessor = GetGlobalConstantValue(
            VariableType::JS_POINTER(), glue, ConstantIndex::ARRAY_LENGTH_ACCESSOR);
        Branch(Equal(accessor, lengthAccessor), &arrayLength, &tryContinue);
        Bind(&arrayLength);
        {
            result = Load(VariableType::JS_ANY(), holder,
                          IntPtr(JSArray::LENGTH_OFFSET));
            Jump(&exit);
        }
        Bind(&tryContinue);
        result = CallRuntime(glue, RTSTUB_ID(CallInternalGetter), { accessor, holder });
        Jump(&exit);
    }
    Bind(&notInternal);
    {
        auto getter = Load(VariableType::JS_ANY(), accessor,
                           IntPtr(AccessorData::GETTER_OFFSET));
        Label objIsUndefined(env);
        Label objNotUndefined(env);
        Branch(TaggedIsUndefined(getter), &objIsUndefined, &objNotUndefined);
        // if getter is undefined, return undefiend
        Bind(&objIsUndefined);
        {
            result = Undefined();
            Jump(&exit);
        }
        Bind(&objNotUndefined);
        {
            auto retValue = JSCallDispatch(glue, getter, Int32(0), 0, Circuit::NullGate(),
                                           JSCallMode::CALL_GETTER, { receiver });
            Label noPendingException(env);
            Branch(HasPendingException(glue), &exit, &noPendingException);
            Bind(&noPendingException);
            {
                result = retValue;
                Jump(&exit);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::CallSetterHelper(GateRef glue, GateRef receiver, GateRef accessor, GateRef value)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->SubCfgEntry(&subEntry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Exception());

    Label isInternal(env);
    Label notInternal(env);
    Branch(IsAccessorInternal(accessor), &isInternal, &notInternal);
    Bind(&isInternal);
    {
        result = CallRuntime(glue, RTSTUB_ID(CallInternalSetter), { receiver, accessor, value });
        Jump(&exit);
    }
    Bind(&notInternal);
    {
        auto setter = Load(VariableType::JS_ANY(), accessor,
                           IntPtr(AccessorData::SETTER_OFFSET));
        Label objIsUndefined(env);
        Label objNotUndefined(env);
        Branch(TaggedIsUndefined(setter), &objIsUndefined, &objNotUndefined);
        Bind(&objIsUndefined);
        {
            CallRuntime(glue, RTSTUB_ID(ThrowSetterIsUndefinedException), {});
            result = Exception();
            Jump(&exit);
        }
        Bind(&objNotUndefined);
        {
            auto retValue = JSCallDispatch(glue, setter, Int32(1), 0, Circuit::NullGate(),
                                           JSCallMode::CALL_SETTER, { receiver, value });
            Label noPendingException(env);
            Branch(HasPendingException(glue), &exit, &noPendingException);
            Bind(&noPendingException);
            {
                result = retValue;
                Jump(&exit);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::ShouldCallSetter(GateRef receiver, GateRef holder, GateRef accessor, GateRef attr)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->SubCfgEntry(&subEntry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::BOOL(), True());
    Label isInternal(env);
    Label notInternal(env);
    Branch(IsAccessorInternal(accessor), &isInternal, &notInternal);
    Bind(&isInternal);
    {
        Label receiverEqualsHolder(env);
        Label receiverNotEqualsHolder(env);
        Branch(Equal(receiver, holder), &receiverEqualsHolder, &receiverNotEqualsHolder);
        Bind(&receiverEqualsHolder);
        {
            result = IsWritable(attr);
            Jump(&exit);
        }
        Bind(&receiverNotEqualsHolder);
        {
            result = False();
            Jump(&exit);
        }
    }
    Bind(&notInternal);
    {
        result = True();
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

void StubBuilder::JSHClassAddProperty(GateRef glue, GateRef receiver, GateRef key, GateRef attr)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->SubCfgEntry(&subEntry);
    Label exit(env);
    GateRef hclass = LoadHClass(receiver);
    GateRef metaData = GetPropertyMetaDataFromAttr(attr);
    GateRef newClass = FindTransitions(glue, receiver, hclass, key, metaData);
    Label findHClass(env);
    Label notFindHClass(env);
    Branch(Equal(newClass, Undefined()), &notFindHClass, &findHClass);
    Bind(&findHClass);
    {
        Jump(&exit);
    }
    Bind(&notFindHClass);
    {
        GateRef type = GetObjectType(hclass);
        GateRef size = Int32Mul(GetInlinedPropsStartFromHClass(hclass),
                                Int32(JSTaggedValue::TaggedTypeSize()));
        GateRef inlineProps = GetInlinedPropertiesFromHClass(hclass);
        GateRef newJshclass = CallRuntime(glue, RTSTUB_ID(NewEcmaHClass),
            { IntToTaggedInt(size), IntToTaggedInt(type),
              IntToTaggedInt(inlineProps) });
        CopyAllHClass(glue, newJshclass, hclass);
        CallRuntime(glue, RTSTUB_ID(UpdateLayOutAndAddTransition),
                    { hclass, newJshclass, key, IntToTaggedInt(attr) });
#if ECMASCRIPT_ENABLE_IC
        NotifyHClassChanged(glue, hclass, newJshclass);
#endif
        StoreHClass(glue, receiver, newJshclass);
        Jump(&exit);
    }
    Bind(&exit);
    env->SubCfgExit();
    return;
}

// if condition:objHandle->IsJSArray() &&
//      keyHandle.GetTaggedValue() == thread->GlobalConstants()->GetConstructorString()
GateRef StubBuilder::SetHasConstructorCondition(GateRef glue, GateRef receiver, GateRef key)
{
    GateRef gConstOffset = PtrAdd(glue,
                                  IntPtr(JSThread::GlueData::GetGlobalConstOffset(env_->Is32Bit())));
    GateRef gCtorStr = Load(VariableType::JS_ANY(),
        gConstOffset,
        Int64Mul(Int64(sizeof(JSTaggedValue)),
            Int64(static_cast<uint64_t>(ConstantIndex::CONSTRUCTOR_STRING_INDEX))));
    GateRef isCtorStr = Equal(key, gCtorStr);
    return BoolAnd(IsJsArray(receiver), isCtorStr);
}

// Note: set return exit node
GateRef StubBuilder::AddPropertyByName(GateRef glue, GateRef receiver, GateRef key, GateRef value,
                                GateRef propertyAttributes)
{
    auto env = GetEnvironment();
    Label subentry(env);
    env->SubCfgEntry(&subentry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Undefined());
    Label setHasCtor(env);
    Label notSetHasCtor(env);
    Label afterCtorCon(env);
    GateRef hclass = LoadHClass(receiver);
    Branch(SetHasConstructorCondition(glue, receiver, key), &setHasCtor, &notSetHasCtor);
    {
        Bind(&setHasCtor);
        SetHasConstructorToHClass(glue, hclass, Int32(1));
        Jump(&afterCtorCon);
        Bind(&notSetHasCtor);
        Jump(&afterCtorCon);
    }
    Bind(&afterCtorCon);
    // 0x111 : default attribute for property: writable, enumerable, configurable
    DEFVARIABLE(attr, VariableType::INT32(), propertyAttributes);
    GateRef numberOfProps = GetNumberOfPropsFromHClass(hclass);
    GateRef inlinedProperties = GetInlinedPropertiesFromHClass(hclass);
    Label hasUnusedInProps(env);
    Label noUnusedInProps(env);
    Label afterInPropsCon(env);
    Branch(Int32UnsignedLessThan(numberOfProps, inlinedProperties), &hasUnusedInProps, &noUnusedInProps);
    {
        Bind(&noUnusedInProps);
        Jump(&afterInPropsCon);
        Bind(&hasUnusedInProps);
        {
            SetPropertyInlinedProps(glue, receiver, hclass, value, numberOfProps);
            attr = SetOffsetFieldInPropAttr(*attr, numberOfProps);
            attr = SetIsInlinePropsFieldInPropAttr(*attr, Int32(1)); // 1: set inInlineProps true
            JSHClassAddProperty(glue, receiver, key, *attr);
            result = Undefined();
            Jump(&exit);
        }
    }
    Bind(&afterInPropsCon);
    DEFVARIABLE(array, VariableType::JS_POINTER(), GetPropertiesArray(receiver));
    DEFVARIABLE(length, VariableType::INT32(), GetLengthOfTaggedArray(*array));
    Label lenIsZero(env);
    Label lenNotZero(env);
    Label afterLenCon(env);
    Branch(Int32Equal(*length, Int32(0)), &lenIsZero, &lenNotZero);
    {
        Bind(&lenIsZero);
        {
            length = Int32(JSObject::MIN_PROPERTIES_LENGTH);
            array = CallRuntime(glue, RTSTUB_ID(NewTaggedArray), { IntToTaggedInt(*length) });
            SetPropertiesArray(VariableType::JS_POINTER(), glue, receiver, *array);
            Jump(&afterLenCon);
        }
        Bind(&lenNotZero);
        Jump(&afterLenCon);
    }
    Bind(&afterLenCon);
    Label isDictMode(env);
    Label notDictMode(env);
    Branch(IsDictionaryMode(*array), &isDictMode, &notDictMode);
    {
        Bind(&isDictMode);
        {
            GateRef res = CallRuntime(glue, RTSTUB_ID(NameDictPutIfAbsent),
                                      {receiver, *array, key, value, IntToTaggedInt(*attr), TaggedFalse()});
            SetPropertiesArray(VariableType::JS_POINTER(), glue, receiver, res);
            Jump(&exit);
        }
        Bind(&notDictMode);
        {
            attr = SetIsInlinePropsFieldInPropAttr(*attr, Int32(0));
            GateRef outProps = Int32Sub(numberOfProps, inlinedProperties);
            Label isArrayFull(env);
            Label arrayNotFull(env);
            Label afterArrLenCon(env);
            Branch(Int32Equal(*length, outProps), &isArrayFull, &arrayNotFull);
            {
                Bind(&isArrayFull);
                {
                    Label ChangeToDict(env);
                    Label notChangeToDict(env);
                    Label afterDictChangeCon(env);
                    Branch(Int32Equal(*length, Int32(JSHClass::MAX_CAPACITY_OF_OUT_OBJECTS)),
                        &ChangeToDict, &notChangeToDict);
                    {
                        Bind(&ChangeToDict);
                        {
                            attr = SetDictionaryOrderFieldInPropAttr(*attr,
                                Int32(PropertyAttributes::MAX_CAPACITY_OF_PROPERTIES));
                            GateRef res = CallRuntime(glue, RTSTUB_ID(NameDictPutIfAbsent),
                                { receiver, *array, key, value, IntToTaggedInt(*attr), TaggedTrue() });
                            SetPropertiesArray(VariableType::JS_POINTER(), glue, receiver, res);
                            result = Undefined();
                            Jump(&exit);
                        }
                        Bind(&notChangeToDict);
                        Jump(&afterDictChangeCon);
                    }
                    Bind(&afterDictChangeCon);
                    GateRef capacity = ComputePropertyCapacityInJSObj(*length);
                    array = CallRuntime(glue, RTSTUB_ID(CopyArray),
                        { *array, IntToTaggedInt(*length), IntToTaggedInt(capacity) });
                    SetPropertiesArray(VariableType::JS_POINTER(), glue, receiver, *array);
                    Jump(&afterArrLenCon);
                }
                Bind(&arrayNotFull);
                Jump(&afterArrLenCon);
            }
            Bind(&afterArrLenCon);
            {
                attr = SetOffsetFieldInPropAttr(*attr, numberOfProps);
                JSHClassAddProperty(glue, receiver, key, *attr);
                SetValueToTaggedArray(VariableType::JS_ANY(), glue, *array, outProps, value);
                Jump(&exit);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

void StubBuilder::ThrowTypeAndReturn(GateRef glue, int messageId, GateRef val)
{
    GateRef msgIntId = Int32(messageId);
    CallRuntime(glue, RTSTUB_ID(ThrowTypeError), { IntToTaggedInt(msgIntId) });
    Return(val);
}

GateRef StubBuilder::TaggedToRepresentation(GateRef value)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    DEFVARIABLE(resultRep, VariableType::INT64(),
                Int64(static_cast<int32_t>(Representation::OBJECT)));
    Label isInt(env);
    Label notInt(env);

    Branch(TaggedIsInt(value), &isInt, &notInt);
    Bind(&isInt);
    {
        resultRep = Int64(static_cast<int32_t>(Representation::INT));
        Jump(&exit);
    }
    Bind(&notInt);
    {
        Label isDouble(env);
        Label notDouble(env);
        Branch(TaggedIsDouble(value), &isDouble, &notDouble);
        Bind(&isDouble);
        {
            resultRep = Int64(static_cast<int32_t>(Representation::DOUBLE));
            Jump(&exit);
        }
        Bind(&notDouble);
        {
            resultRep = Int64(static_cast<int32_t>(Representation::OBJECT));
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *resultRep;
    env->SubCfgExit();
    return ret;
}

void StubBuilder::Store(VariableType type, GateRef glue, GateRef base, GateRef offset, GateRef value)
{
    if (!env_->IsAsmInterp()) {
        env_->GetBuilder()->Store(type, glue, base, offset, value);
    } else {
        auto depend = env_->GetCurrentLabel()->GetDepend();
        GateRef ptr = PtrAdd(base, offset);
        GateRef result = env_->GetCircuit()->NewGate(
            env_->GetCircuit()->Store(), MachineType::NOVALUE,
            { depend, value, ptr }, type.GetGateType());
        env_->GetCurrentLabel()->SetDepend(result);
        if (type == VariableType::JS_POINTER() || type == VariableType::JS_ANY()) {
            auto env = GetEnvironment();
            Label entry(env);
            env->SubCfgEntry(&entry);
            Label exit(env);
            Label isHeapObject(env);

            Branch(TaggedIsHeapObject(value), &isHeapObject, &exit);
            Bind(&isHeapObject);
            {
                CallNGCRuntime(glue, RTSTUB_ID(StoreBarrier), { glue, base, offset, value });
                Jump(&exit);
            }
            Bind(&exit);
            env->SubCfgExit();
        }
    }
}

void StubBuilder::SetValueWithBarrier(GateRef glue, GateRef obj, GateRef offset, GateRef value)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label isHeapObject(env);
    Label isVailedIndex(env);
    Label notValidIndex(env);

    Branch(TaggedIsHeapObject(value), &isHeapObject, &exit);
    Bind(&isHeapObject);
    {
        // ObjectAddressToRange function may cause obj is not an object. GC may not mark this obj.
        GateRef objectRegion = ObjectAddressToRange(obj);
        GateRef valueRegion = ObjectAddressToRange(value);
        GateRef slotAddr = PtrAdd(TaggedCastToIntPtr(obj), offset);
        GateRef objectNotInYoung = BoolNot(InYoungGeneration(objectRegion));
        GateRef valueRegionInYoung = InYoungGeneration(valueRegion);
        Branch(BoolAnd(objectNotInYoung, valueRegionInYoung), &isVailedIndex, &notValidIndex);
        Bind(&isVailedIndex);
        {
            GateRef loadOffset = IntPtr(Region::PackedData::GetOldToNewSetOffset(env_->Is32Bit()));
            auto oldToNewSet = Load(VariableType::NATIVE_POINTER(), objectRegion, loadOffset);
            Label isNullPtr(env);
            Label notNullPtr(env);
            Branch(IntPtrEuqal(oldToNewSet, IntPtr(0)), &isNullPtr, &notNullPtr);
            Bind(&notNullPtr);
            {
                // (slotAddr - this) >> TAGGED_TYPE_SIZE_LOG
                GateRef bitOffsetPtr = IntPtrLSR(PtrSub(slotAddr, objectRegion), IntPtr(TAGGED_TYPE_SIZE_LOG));
                GateRef bitOffset = TruncPtrToInt32(bitOffsetPtr);
                GateRef bitPerWordLog2 = Int32(GCBitset::BIT_PER_WORD_LOG2);
                GateRef bytePerWord = Int32(GCBitset::BYTE_PER_WORD);
                // bitOffset >> BIT_PER_WORD_LOG2
                GateRef index = Int32LSR(bitOffset, bitPerWordLog2);
                GateRef byteIndex = Int32Mul(index, bytePerWord);
                // bitset_[index] |= mask;
                GateRef bitsetData = PtrAdd(oldToNewSet, IntPtr(RememberedSet::GCBITSET_DATA_OFFSET));
                GateRef oldsetValue = Load(VariableType::INT32(), bitsetData, byteIndex);
                GateRef newmapValue = Int32Or(oldsetValue, GetBitMask(bitOffset));

                Store(VariableType::INT32(), glue, bitsetData, byteIndex, newmapValue);
                Jump(&notValidIndex);
            }
            Bind(&isNullPtr);
            {
                CallNGCRuntime(glue, RTSTUB_ID(InsertOldToNewRSet), { glue, obj, offset });
                Jump(&notValidIndex);
            }
        }
        Bind(&notValidIndex);
        {
            Label marking(env);
            bool isArch32 = GetEnvironment()->Is32Bit();
            GateRef stateBitFieldAddr = Int64Add(glue,
                                                 Int64(JSThread::GlueData::GetStateBitFieldOffset(isArch32)));
            GateRef stateBitField = Load(VariableType::INT64(), stateBitFieldAddr, Int64(0));
            // mask: 1 << JSThread::CONCURRENT_MARKING_BITFIELD_NUM - 1
            GateRef markingBitMask = Int64Sub(
                Int64LSL(Int64(1), Int64(JSThread::CONCURRENT_MARKING_BITFIELD_NUM)), Int64(1));
            GateRef state = Int64And(stateBitField, markingBitMask);
            Branch(Int64Equal(state, Int64(static_cast<int64_t>(MarkStatus::READY_TO_MARK))), &exit, &marking);

            Bind(&marking);
            CallNGCRuntime(
                glue,
                RTSTUB_ID(MarkingBarrier), { glue, obj, offset, value });
            Jump(&exit);
        }
    }
    Bind(&exit);
    env->SubCfgExit();
}

GateRef StubBuilder::TaggedIsBigInt(GateRef obj)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label isHeapObject(env);
    DEFVARIABLE(result, VariableType::BOOL(), False());
    Branch(TaggedIsHeapObject(obj), &isHeapObject, &exit);
    Bind(&isHeapObject);
    {
        result = Int32Equal(GetObjectType(LoadHClass(obj)),
                            Int32(static_cast<int32_t>(JSType::BIGINT)));
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::TaggedIsPropertyBox(GateRef obj)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label isHeapObject(env);
    DEFVARIABLE(result, VariableType::BOOL(), False());
    Branch(TaggedIsHeapObject(obj), &isHeapObject, &exit);
    Bind(&isHeapObject);
    {
        GateRef type = GetObjectType(LoadHClass(obj));
        result = Int32Equal(type, Int32(static_cast<int32_t>(JSType::PROPERTY_BOX)));
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::TaggedIsAccessor(GateRef x)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label isHeapObject(env);
    DEFVARIABLE(result, VariableType::BOOL(), False());
    Branch(TaggedIsHeapObject(x), &isHeapObject, &exit);
    Bind(&isHeapObject);
    {
        GateRef type = GetObjectType(LoadHClass(x));
        result = BoolOr(Int32Equal(type, Int32(static_cast<int32_t>(JSType::ACCESSOR_DATA))),
                        Int32Equal(type, Int32(static_cast<int32_t>(JSType::INTERNAL_ACCESSOR))));
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::IsUtf16String(GateRef string)
{
    // compressedStringsEnabled fixed to true constant
    GateRef len = Load(VariableType::INT32(), string, IntPtr(EcmaString::MIX_LENGTH_OFFSET));
    return Int32Equal(
        Int32And(len, Int32(EcmaString::STRING_COMPRESSED_BIT)),
        Int32(EcmaString::STRING_UNCOMPRESSED));
}

GateRef StubBuilder::IsUtf8String(GateRef string)
{
    // compressedStringsEnabled fixed to true constant
    GateRef len = Load(VariableType::INT32(), string, IntPtr(EcmaString::MIX_LENGTH_OFFSET));
    return Int32Equal(
        Int32And(len, Int32(EcmaString::STRING_COMPRESSED_BIT)),
        Int32(EcmaString::STRING_COMPRESSED));
}

GateRef StubBuilder::IsInternalString(GateRef string)
{
    // compressedStringsEnabled fixed to true constant
    GateRef len = Load(VariableType::INT32(), string, IntPtr(EcmaString::MIX_LENGTH_OFFSET));
    return Int32NotEqual(
        Int32And(len, Int32(EcmaString::STRING_INTERN_BIT)),
        Int32(0));
}

GateRef StubBuilder::IsDigit(GateRef ch)
{
    return BoolAnd(Int32LessThanOrEqual(ch, Int32('9')),
        Int32GreaterThanOrEqual(ch, Int32('0')));
}

GateRef StubBuilder::StringToElementIndex(GateRef string)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::INT32(), Int32(-1));
    Label greatThanZero(env);
    Label inRange(env);
    auto len = GetLengthFromString(string);
    Branch(Int32Equal(len, Int32(0)), &exit, &greatThanZero);
    Bind(&greatThanZero);
    Branch(Int32GreaterThan(len, Int32(MAX_ELEMENT_INDEX_LEN)), &exit, &inRange);
    Bind(&inRange);
    {
        GateRef dataUtf16 = PtrAdd(string, IntPtr(EcmaString::DATA_OFFSET));
        DEFVARIABLE(c, VariableType::INT32(), Int32(0));
        Label isUtf16(env);
        Label isUtf8(env);
        Label getChar1(env);
        GateRef isUtf16String = IsUtf16String(string);
        Branch(isUtf16String, &isUtf16, &isUtf8);
        Bind(&isUtf16);
        {
            c = ZExtInt16ToInt32(Load(VariableType::INT16(), dataUtf16));
            Jump(&getChar1);
        }
        Bind(&isUtf8);
        {
            c = ZExtInt8ToInt32(Load(VariableType::INT8(), dataUtf16));
            Jump(&getChar1);
        }
        Bind(&getChar1);
        {
            Label isDigitZero(env);
            Label notDigitZero(env);
            Branch(Int32Equal(*c, Int32('0')), &isDigitZero, &notDigitZero);
            Bind(&isDigitZero);
            {
                Label lengthIsOne(env);
                Branch(Int32Equal(len, Int32(1)), &lengthIsOne, &exit);
                Bind(&lengthIsOne);
                {
                    result = Int32(0);
                    Jump(&exit);
                }
            }
            Bind(&notDigitZero);
            {
                Label isDigit(env);
                DEFVARIABLE(i, VariableType::INT32(), Int32(1));
                DEFVARIABLE(n, VariableType::INT32(), Int32Sub(*c, Int32('0')));
                Branch(IsDigit(*c), &isDigit, &exit);
                Label loopHead(env);
                Label loopEnd(env);
                Label afterLoop(env);
                Bind(&isDigit);
                Branch(Int32UnsignedLessThan(*i, len), &loopHead, &afterLoop);
                LoopBegin(&loopHead);
                {
                    Label isUtf16A(env);
                    Label notUtf16(env);
                    Label getChar2(env);
                    Branch(isUtf16String, &isUtf16A, &notUtf16);
                    Bind(&isUtf16A);
                    {
                        // 2 : 2 means utf16 char width is two bytes
                        auto charOffset = PtrMul(ZExtInt32ToPtr(*i),  IntPtr(2));
                        c = ZExtInt16ToInt32(Load(VariableType::INT16(), dataUtf16, charOffset));
                        Jump(&getChar2);
                    }
                    Bind(&notUtf16);
                    {
                        c = ZExtInt8ToInt32(Load(VariableType::INT8(), dataUtf16, ZExtInt32ToPtr(*i)));
                        Jump(&getChar2);
                    }
                    Bind(&getChar2);
                    {
                        Label isDigit2(env);
                        Label notDigit2(env);
                        Branch(IsDigit(*c), &isDigit2, &notDigit2);
                        Bind(&isDigit2);
                        {
                            // 10 means the base of digit is 10.
                            n = Int32Add(Int32Mul(*n, Int32(10)),
                                         Int32Sub(*c, Int32('0')));
                            i = Int32Add(*i, Int32(1));
                            Branch(Int32UnsignedLessThan(*i, len), &loopEnd, &afterLoop);
                        }
                        Bind(&notDigit2);
                        Jump(&exit);
                    }
                }
                Bind(&loopEnd);
                LoopEnd(&loopHead);
                Bind(&afterLoop);
                {
                    Label lessThanMaxIndex(env);
                    Branch(Int32UnsignedLessThan(*n, Int32(JSObject::MAX_ELEMENT_INDEX)),
                           &lessThanMaxIndex, &exit);
                    Bind(&lessThanMaxIndex);
                    {
                        result = *n;
                        Jump(&exit);
                    }
                }
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::TryToElementsIndex(GateRef key)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label isKeyInt(env);
    Label notKeyInt(env);

    DEFVARIABLE(resultKey, VariableType::INT32(), Int32(-1));
    Branch(TaggedIsInt(key), &isKeyInt, &notKeyInt);
    Bind(&isKeyInt);
    {
        resultKey = GetInt32OfTInt(key);
        Jump(&exit);
    }
    Bind(&notKeyInt);
    {
        Label isString(env);
        Label notString(env);
        Branch(TaggedIsString(key), &isString, &notString);
        Bind(&isString);
        {
            resultKey = StringToElementIndex(key);
            Jump(&exit);
        }
        Bind(&notString);
        {
            Label isDouble(env);
            Branch(TaggedIsDouble(key), &isDouble, &exit);
            Bind(&isDouble);
            {
                GateRef number = GetDoubleOfTDouble(key);
                GateRef integer = ChangeFloat64ToInt32(number);
                Label isEqual(env);
                Branch(DoubleEqual(number, ChangeInt32ToFloat64(integer)), &isEqual, &exit);
                Bind(&isEqual);
                {
                    resultKey = integer;
                    Jump(&exit);
                }
            }
        }
    }
    Bind(&exit);
    auto ret = *resultKey;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::LdGlobalRecord(GateRef glue, GateRef key)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);

    DEFVARIABLE(result, VariableType::JS_ANY(), Undefined());
    GateRef glueGlobalEnvOffset = IntPtr(JSThread::GlueData::GetGlueGlobalEnvOffset(env->Is32Bit()));
    GateRef glueGlobalEnv = Load(VariableType::NATIVE_POINTER(), glue, glueGlobalEnvOffset);
    GateRef globalRecord = GetGlobalEnvValue(VariableType::JS_ANY(), glueGlobalEnv, GlobalEnv::GLOBAL_RECORD);
    GateRef recordEntry = FindEntryFromNameDictionary(glue, globalRecord, key);
    Label foundInGlobalRecord(env);
    Branch(Int32NotEqual(recordEntry, Int32(-1)), &foundInGlobalRecord, &exit);
    Bind(&foundInGlobalRecord);
    {
        result = GetBoxFromGlobalDictionary(globalRecord, recordEntry);
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::LoadFromField(GateRef receiver, GateRef handlerInfo)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label handlerInfoIsInlinedProps(env);
    Label handlerInfoNotInlinedProps(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Undefined());
    GateRef index = HandlerBaseGetOffset(handlerInfo);
    Branch(HandlerBaseIsInlinedProperty(handlerInfo), &handlerInfoIsInlinedProps, &handlerInfoNotInlinedProps);
    Bind(&handlerInfoIsInlinedProps);
    {
        result = Load(VariableType::JS_ANY(), receiver, PtrMul(ZExtInt32ToPtr(index),
            IntPtr(JSTaggedValue::TaggedTypeSize())));
        Jump(&exit);
    }
    Bind(&handlerInfoNotInlinedProps);
    {
        result = GetValueFromTaggedArray(GetPropertiesArray(receiver), index);
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::LoadGlobal(GateRef cell)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label cellIsInvalid(env);
    Label cellNotInvalid(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Branch(IsInvalidPropertyBox(cell), &cellIsInvalid, &cellNotInvalid);
    Bind(&cellIsInvalid);
    {
        Jump(&exit);
    }
    Bind(&cellNotInvalid);
    {
        result = GetValueFromPropertyBox(cell);
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::CheckPolyHClass(GateRef cachedValue, GateRef hclass)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label iLessLength(env);
    Label hasHclass(env);
    Label cachedValueNotWeak(env);
    DEFVARIABLE(i, VariableType::INT32(), Int32(0));
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Branch(TaggedIsWeak(cachedValue), &exit, &cachedValueNotWeak);
    Bind(&cachedValueNotWeak);
    {
        GateRef length = GetLengthOfTaggedArray(cachedValue);
        Jump(&loopHead);
        LoopBegin(&loopHead);
        {
            Branch(Int32UnsignedLessThan(*i, length), &iLessLength, &exit);
            Bind(&iLessLength);
            {
                GateRef element = GetValueFromTaggedArray(cachedValue, *i);
                Branch(Equal(LoadObjectFromWeakRef(element), hclass), &hasHclass, &loopEnd);
                Bind(&hasHclass);
                result = GetValueFromTaggedArray(cachedValue,
                                                 Int32Add(*i, Int32(1)));
                Jump(&exit);
            }
            Bind(&loopEnd);
            i = Int32Add(*i, Int32(2));  // 2 means one ic, two slot
            LoopEnd(&loopHead);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::LoadICWithHandler(GateRef glue, GateRef receiver, GateRef argHolder, GateRef argHandler)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label handlerIsInt(env);
    Label handlerNotInt(env);
    Label handlerInfoIsField(env);
    Label handlerInfoNotField(env);
    Label handlerInfoIsNonExist(env);
    Label handlerInfoNotNonExist(env);
    Label handlerIsPrototypeHandler(env);
    Label handlerNotPrototypeHandler(env);
    Label cellHasChanged(env);
    Label loopHead(env);
    Label loopEnd(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Undefined());
    DEFVARIABLE(holder, VariableType::JS_ANY(), argHolder);
    DEFVARIABLE(handler, VariableType::JS_ANY(), argHandler);

    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        Branch(TaggedIsInt(*handler), &handlerIsInt, &handlerNotInt);
        Bind(&handlerIsInt);
        {
            GateRef handlerInfo = GetInt32OfTInt(*handler);
            Branch(IsField(handlerInfo), &handlerInfoIsField, &handlerInfoNotField);
            Bind(&handlerInfoIsField);
            {
                result = LoadFromField(*holder, handlerInfo);
                Jump(&exit);
            }
            Bind(&handlerInfoNotField);
            {
                Branch(IsNonExist(handlerInfo), &handlerInfoIsNonExist, &handlerInfoNotNonExist);
                Bind(&handlerInfoIsNonExist);
                Jump(&exit);
                Bind(&handlerInfoNotNonExist);
                GateRef accessor = LoadFromField(*holder, handlerInfo);
                result = CallGetterHelper(glue, receiver, *holder, accessor);
                Jump(&exit);
            }
        }
        Bind(&handlerNotInt);
        Branch(TaggedIsPrototypeHandler(*handler), &handlerIsPrototypeHandler, &handlerNotPrototypeHandler);
        Bind(&handlerIsPrototypeHandler);
        {
            GateRef cellValue = GetProtoCell(*handler);
            Branch(GetHasChanged(cellValue), &cellHasChanged, &loopEnd);
            Bind(&cellHasChanged);
            {
                result = Hole();
                Jump(&exit);
            }
            Bind(&loopEnd);
            holder = GetPrototypeHandlerHolder(*handler);
            handler = GetPrototypeHandlerHandlerInfo(*handler);
            LoopEnd(&loopHead);
        }
    }
    Bind(&handlerNotPrototypeHandler);
    result = LoadGlobal(*handler);
    Jump(&exit);

    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::LoadElement(GateRef receiver, GateRef key)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label indexLessZero(env);
    Label indexNotLessZero(env);
    Label lengthLessIndex(env);
    Label lengthNotLessIndex(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    GateRef index = TryToElementsIndex(key);
    Branch(Int32LessThan(index, Int32(0)), &indexLessZero, &indexNotLessZero);
    Bind(&indexLessZero);
    {
        Jump(&exit);
    }
    Bind(&indexNotLessZero);
    {
        GateRef elements = GetElementsArray(receiver);
        Branch(Int32LessThanOrEqual(GetLengthOfTaggedArray(elements), index), &lengthLessIndex, &lengthNotLessIndex);
        Bind(&lengthLessIndex);
        Jump(&exit);
        Bind(&lengthNotLessIndex);
        result = GetValueFromTaggedArray(elements, index);
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::ICStoreElement(GateRef glue, GateRef receiver, GateRef key, GateRef value, GateRef handler)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label indexLessZero(env);
    Label indexNotLessZero(env);
    Label handerInfoIsJSArray(env);
    Label handerInfoNotJSArray(env);
    Label isJsCOWArray(env);
    Label isNotJsCOWArray(env);
    Label setElementsLength(env);
    Label indexGreaterLength(env);
    Label indexGreaterCapacity(env);
    Label callRuntime(env);
    Label storeElement(env);
    Label handlerIsInt(env);
    Label handlerNotInt(env);
    Label cellHasChanged(env);
    Label cellHasNotChanged(env);
    Label loopHead(env);
    Label loopEnd(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    DEFVARIABLE(varHandler, VariableType::JS_ANY(), handler);
    GateRef index = TryToElementsIndex(key);
    Branch(Int32LessThan(index, Int32(0)), &indexLessZero, &indexNotLessZero);
    Bind(&indexLessZero);
    {
        Jump(&exit);
    }
    Bind(&indexNotLessZero);
    {
        Jump(&loopHead);
        LoopBegin(&loopHead);
        Branch(TaggedIsInt(*varHandler), &handlerIsInt, &handlerNotInt);
        Bind(&handlerIsInt);
        {
            GateRef handlerInfo = GetInt32OfTInt(*varHandler);
            Branch(HandlerBaseIsJSArray(handlerInfo), &handerInfoIsJSArray, &handerInfoNotJSArray);
            Bind(&handerInfoIsJSArray);
            {
                Branch(IsJsCOWArray(receiver), &isJsCOWArray, &isNotJsCOWArray);
                Bind(&isJsCOWArray);
                {
                    CallRuntime(glue, RTSTUB_ID(CheckAndCopyArray), {receiver});
                    Jump(&setElementsLength);
                }
                Bind(&isNotJsCOWArray);
                {
                    Jump(&setElementsLength);
                }
                Bind(&setElementsLength);
                {
                    GateRef oldLength = GetArrayLength(receiver);
                    Branch(Int32GreaterThanOrEqual(index, oldLength), &indexGreaterLength, &handerInfoNotJSArray);
                    Bind(&indexGreaterLength);
                    Store(VariableType::INT64(), glue, receiver,
                        IntPtr(panda::ecmascript::JSArray::LENGTH_OFFSET),
                        IntToTaggedInt(Int32Add(index, Int32(1))));
                }
                Jump(&handerInfoNotJSArray);
            }
            Bind(&handerInfoNotJSArray);
            {
                GateRef elements = GetElementsArray(receiver);
                GateRef capacity = GetLengthOfTaggedArray(elements);
                Branch(Int32GreaterThanOrEqual(index, capacity), &callRuntime, &storeElement);
                Bind(&callRuntime);
                {
                    result = CallRuntime(glue,
                        RTSTUB_ID(TaggedArraySetValue),
                        { receiver, value, elements, IntToTaggedInt(index),
                          IntToTaggedInt(capacity) });
                    Jump(&exit);
                }
                Bind(&storeElement);
                {
                    SetValueToTaggedArray(VariableType::JS_ANY(), glue, elements, index, value);
                    result = Undefined();
                    Jump(&exit);
                }
            }
        }
        Bind(&handlerNotInt);
        {
            GateRef cellValue = GetProtoCell(*varHandler);
            Branch(GetHasChanged(cellValue), &cellHasChanged, &loopEnd);
            Bind(&cellHasChanged);
            {
                Jump(&exit);
            }
            Bind(&loopEnd);
            {
                varHandler = GetPrototypeHandlerHandlerInfo(*varHandler);
                LoopEnd(&loopHead);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::GetArrayLength(GateRef object)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label lengthIsInt(env);
    Label lengthNotInt(env);
    DEFVARIABLE(result, VariableType::INT32(), Int32(0));
    GateRef lengthOffset = IntPtr(panda::ecmascript::JSArray::LENGTH_OFFSET);
    GateRef length = Load(VariableType::INT64(), object, lengthOffset);
    Branch(TaggedIsInt(length), &lengthIsInt, &lengthNotInt);
    Bind(&lengthIsInt);
    {
        result = GetInt32OfTInt(length);
        Jump(&exit);
    }
    Bind(&lengthNotInt);
    {
        result = ChangeFloat64ToInt32(GetDoubleOfTDouble(length));
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::StoreICWithHandler(GateRef glue, GateRef receiver, GateRef argHolder,
                                        GateRef value, GateRef argHandler)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label handlerIsInt(env);
    Label handlerNotInt(env);
    Label handlerInfoIsField(env);
    Label handlerInfoNotField(env);
    Label handlerIsTransitionHandler(env);
    Label handlerNotTransitionHandler(env);
    Label handlerIsTransWithProtoHandler(env);
    Label handlerNotTransWithProtoHandler(env);
    Label handlerIsPrototypeHandler(env);
    Label handlerNotPrototypeHandler(env);
    Label handlerIsPropertyBox(env);
    Label handlerNotPropertyBox(env);
    Label handlerIsStoreTSHandler(env);
    Label handlerNotStoreTSHandler(env);
    Label aotHandlerInfoIsField(env);
    Label aotHandlerInfoNotField(env);
    Label cellHasChanged(env);
    Label cellNotChanged(env);
    Label aotCellNotChanged(env);
    Label loopHead(env);
    Label loopEnd(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Undefined());
    DEFVARIABLE(holder, VariableType::JS_ANY(), argHolder);
    DEFVARIABLE(handler, VariableType::JS_ANY(), argHandler);
    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        Branch(TaggedIsInt(*handler), &handlerIsInt, &handlerNotInt);
        Bind(&handlerIsInt);
        {
            GateRef handlerInfo = GetInt32OfTInt(*handler);
            Branch(IsField(handlerInfo), &handlerInfoIsField, &handlerInfoNotField);
            Bind(&handlerInfoIsField);
            {
                StoreField(glue, receiver, value, handlerInfo);
                Jump(&exit);
            }
            Bind(&handlerInfoNotField);
            {
                GateRef accessor = LoadFromField(*holder, handlerInfo);
                result = CallSetterHelper(glue, receiver, accessor, value);
                Jump(&exit);
            }
        }
        Bind(&handlerNotInt);
        {
            Branch(TaggedIsTransitionHandler(*handler), &handlerIsTransitionHandler, &handlerNotTransitionHandler);
            Bind(&handlerIsTransitionHandler);
            {
                StoreWithTransition(glue, receiver, value, *handler);
                Jump(&exit);
            }
            Bind(&handlerNotTransitionHandler);
            {
                Branch(TaggedIsTransWithProtoHandler(*handler), &handlerIsTransWithProtoHandler,
                    &handlerNotTransWithProtoHandler);
                Bind(&handlerIsTransWithProtoHandler);
                {
                    GateRef cellValue = GetProtoCell(*handler);
                    Branch(GetHasChanged(cellValue), &cellHasChanged, &cellNotChanged);
                    Bind(&cellNotChanged);
                    {
                        StoreWithTransition(glue, receiver, value, *handler, true);
                        Jump(&exit);
                    }
                }
                Bind(&handlerNotTransWithProtoHandler);
                {
                    Branch(TaggedIsPrototypeHandler(*handler), &handlerIsPrototypeHandler, &handlerNotPrototypeHandler);
                    Bind(&handlerNotPrototypeHandler);
                    {
                        Branch(TaggedIsPropertyBox(*handler), &handlerIsPropertyBox, &handlerNotPropertyBox);
                        Bind(&handlerIsPropertyBox);
                        StoreGlobal(glue, value, *handler);
                        Jump(&exit);
                    }
                }
            }
        }
        Bind(&handlerIsPrototypeHandler);
        {
            GateRef cellValue = GetProtoCell(*handler);
            Branch(GetHasChanged(cellValue), &cellHasChanged, &loopEnd);
            Bind(&loopEnd);
            {
                holder = GetPrototypeHandlerHolder(*handler);
                handler = GetPrototypeHandlerHandlerInfo(*handler);
                LoopEnd(&loopHead);
            }
        }
        Bind(&handlerNotPropertyBox);
        {
            Branch(TaggedIsStoreTSHandler(*handler), &handlerIsStoreTSHandler, &handlerNotStoreTSHandler);
            Bind(&handlerIsStoreTSHandler);
            {
                GateRef cellValue = GetProtoCell(*handler);
                Branch(GetHasChanged(cellValue), &cellHasChanged, &aotCellNotChanged);
                Bind(&aotCellNotChanged);
                {
                    holder = GetStoreTSHandlerHolder(*handler);
                    handler = GetStoreTSHandlerHandlerInfo(*handler);
                    GateRef handlerInfo = GetInt32OfTInt(*handler);
                    Branch(IsField(handlerInfo), &aotHandlerInfoIsField, &aotHandlerInfoNotField);
                    Bind(&aotHandlerInfoIsField);
                    {
                        StoreField(glue, receiver, value, handlerInfo);
                        Jump(&exit);
                    }
                    Bind(&aotHandlerInfoNotField);
                    {
                        GateRef accessor = LoadFromField(*holder, handlerInfo);
                        result = CallSetterHelper(glue, receiver, accessor, value);
                        Jump(&exit);
                    }
                }
            }
            Bind(&handlerNotStoreTSHandler);
            Jump(&exit);
        }
        Bind(&cellHasChanged);
        {
            result = Hole();
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

void StubBuilder::StoreField(GateRef glue, GateRef receiver, GateRef value, GateRef handler)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label handlerIsInlinedProperty(env);
    Label handlerNotInlinedProperty(env);
    GateRef index = HandlerBaseGetOffset(handler);
    Branch(HandlerBaseIsInlinedProperty(handler), &handlerIsInlinedProperty, &handlerNotInlinedProperty);
    Bind(&handlerIsInlinedProperty);
    {
        Store(VariableType::JS_ANY(),
              glue,
              receiver,
              PtrMul(ZExtInt32ToPtr(index), IntPtr(JSTaggedValue::TaggedTypeSize())),
              value);
        Jump(&exit);
    }
    Bind(&handlerNotInlinedProperty);
    {
        GateRef array = GetPropertiesArray(receiver);
        SetValueToTaggedArray(VariableType::JS_ANY(), glue, array, index, value);
        Jump(&exit);
    }
    Bind(&exit);
    env->SubCfgExit();
}

void StubBuilder::StoreWithTransition(GateRef glue, GateRef receiver, GateRef value, GateRef handler,
                                      bool withPrototype)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);

    Label handlerInfoIsInlinedProps(env);
    Label handlerInfoNotInlinedProps(env);
    Label indexMoreCapacity(env);
    Label indexLessCapacity(env);
    GateRef newHClass;
    GateRef handlerInfo;
    if (withPrototype) {
        newHClass = GetTransWithProtoHClass(handler);
        handlerInfo = GetInt32OfTInt(GetTransWithProtoHandlerInfo(handler));
    } else {
        newHClass = GetTransitionHClass(handler);
        handlerInfo = GetInt32OfTInt(GetTransitionHandlerInfo(handler));
    }

    StoreHClass(glue, receiver, newHClass);
    Branch(HandlerBaseIsInlinedProperty(handlerInfo), &handlerInfoIsInlinedProps, &handlerInfoNotInlinedProps);
    Bind(&handlerInfoNotInlinedProps);
    {
        GateRef array = GetPropertiesArray(receiver);
        GateRef capacity = GetLengthOfTaggedArray(array);
        GateRef index = HandlerBaseGetOffset(handlerInfo);
        Branch(Int32GreaterThanOrEqual(index, capacity), &indexMoreCapacity, &indexLessCapacity);
        Bind(&indexMoreCapacity);
        {
            CallRuntime(glue,
                        RTSTUB_ID(PropertiesSetValue),
                        { receiver, value, array, IntToTaggedInt(capacity),
                          IntToTaggedInt(index) });
            Jump(&exit);
        }
        Bind(&indexLessCapacity);
        {
            Store(VariableType::JS_ANY(),
                  glue,
                  PtrAdd(array, IntPtr(TaggedArray::DATA_OFFSET)),
                  PtrMul(ZExtInt32ToPtr(index), IntPtr(JSTaggedValue::TaggedTypeSize())),
                  value);
            Jump(&exit);
        }
    }
    Bind(&handlerInfoIsInlinedProps);
    {
        StoreField(glue, receiver, value, handlerInfo);
        Jump(&exit);
    }
    Bind(&exit);
    env->SubCfgExit();
}

GateRef StubBuilder::StoreGlobal(GateRef glue, GateRef value, GateRef cell)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label cellIsInvalid(env);
    Label cellNotInvalid(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Branch(IsInvalidPropertyBox(cell), &cellIsInvalid, &cellNotInvalid);
    Bind(&cellIsInvalid);
    {
        Jump(&exit);
    }
    Bind(&cellNotInvalid);
    {
        Store(VariableType::JS_ANY(), glue, cell, IntPtr(PropertyBox::VALUE_OFFSET), value);
        result = Undefined();
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

template<typename DictionaryT>
GateRef StubBuilder::GetAttributesFromDictionary(GateRef elements, GateRef entry)
{
    GateRef arrayIndex =
    Int32Add(Int32(DictionaryT::TABLE_HEADER_SIZE),
             Int32Mul(entry, Int32(DictionaryT::ENTRY_SIZE)));
    GateRef attributesIndex =
        Int32Add(arrayIndex, Int32(DictionaryT::ENTRY_DETAILS_INDEX));
    auto attrValue = GetValueFromTaggedArray(elements, attributesIndex);
    return GetInt32OfTInt(attrValue);
}

template<typename DictionaryT>
GateRef StubBuilder::GetValueFromDictionary(GateRef elements, GateRef entry)
{
    GateRef arrayIndex =
        Int32Add(Int32(DictionaryT::TABLE_HEADER_SIZE),
                 Int32Mul(entry, Int32(DictionaryT::ENTRY_SIZE)));
    GateRef valueIndex =
        Int32Add(arrayIndex, Int32(DictionaryT::ENTRY_VALUE_INDEX));
    return GetValueFromTaggedArray(elements, valueIndex);
}

template<typename DictionaryT>
GateRef StubBuilder::GetKeyFromDictionary(GateRef elements, GateRef entry)
{
    auto env = GetEnvironment();
    Label subentry(env);
    env->SubCfgEntry(&subentry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Undefined());
    Label ltZero(env);
    Label notLtZero(env);
    Label gtLength(env);
    Label notGtLength(env);
    GateRef dictionaryLength =
        Load(VariableType::INT32(), elements, IntPtr(TaggedArray::LENGTH_OFFSET));
    GateRef arrayIndex =
        Int32Add(Int32(DictionaryT::TABLE_HEADER_SIZE),
                 Int32Mul(entry, Int32(DictionaryT::ENTRY_SIZE)));
    Branch(Int32LessThan(arrayIndex, Int32(0)), &ltZero, &notLtZero);
    Bind(&ltZero);
    Jump(&exit);
    Bind(&notLtZero);
    Branch(Int32GreaterThan(arrayIndex, dictionaryLength), &gtLength, &notGtLength);
    Bind(&gtLength);
    Jump(&exit);
    Bind(&notGtLength);
    result = GetValueFromTaggedArray(elements, arrayIndex);
    Jump(&exit);
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

inline void StubBuilder::UpdateValueAndAttributes(GateRef glue, GateRef elements, GateRef index,
                                                  GateRef value, GateRef attr)
{
    GateRef arrayIndex =
        Int32Add(Int32(NameDictionary::TABLE_HEADER_SIZE),
                 Int32Mul(index, Int32(NameDictionary::ENTRY_SIZE)));
    GateRef valueIndex =
        Int32Add(arrayIndex, Int32(NameDictionary::ENTRY_VALUE_INDEX));
    GateRef attributesIndex =
        Int32Add(arrayIndex, Int32(NameDictionary::ENTRY_DETAILS_INDEX));
    SetValueToTaggedArray(VariableType::JS_ANY(), glue, elements, valueIndex, value);
    GateRef attroffset =
        PtrMul(ZExtInt32ToPtr(attributesIndex), IntPtr(JSTaggedValue::TaggedTypeSize()));
    GateRef dataOffset = PtrAdd(attroffset, IntPtr(TaggedArray::DATA_OFFSET));
    Store(VariableType::INT64(), glue, elements, dataOffset, IntToTaggedInt(attr));
}

inline void StubBuilder::UpdateValueInDict(GateRef glue, GateRef elements, GateRef index, GateRef value)
{
    GateRef arrayIndex = Int32Add(Int32(NameDictionary::TABLE_HEADER_SIZE),
        Int32Mul(index, Int32(NameDictionary::ENTRY_SIZE)));
    GateRef valueIndex = Int32Add(arrayIndex, Int32(NameDictionary::ENTRY_VALUE_INDEX));
    SetValueToTaggedArray(VariableType::JS_ANY(), glue, elements, valueIndex, value);
}

GateRef StubBuilder::GetPropertyByIndex(GateRef glue, GateRef receiver, GateRef index)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    DEFVARIABLE(holder, VariableType::JS_ANY(), receiver);
    Label exit(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label loopExit(env);
    Label afterLoop(env);
    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        GateRef hclass = LoadHClass(*holder);
        GateRef jsType = GetObjectType(hclass);
        Label isSpecialIndexed(env);
        Label notSpecialIndexed(env);
        Branch(IsSpecialIndexedObj(jsType), &isSpecialIndexed, &notSpecialIndexed);
        Bind(&isSpecialIndexed);
        {
            // TypeArray
            Label isFastTypeArray(env);
            Label notFastTypeArray(env);
            Branch(IsFastTypeArray(jsType), &isFastTypeArray, &notFastTypeArray);
            Bind(&isFastTypeArray);
            {
                result = CallRuntime(glue, RTSTUB_ID(GetTypeArrayPropertyByIndex),
                    { *holder, IntToTaggedInt(index), IntToTaggedInt(jsType)});
                Jump(&exit);
            }
            Bind(&notFastTypeArray);

            Label isSpecialContainer(env);
            Label notSpecialContainer(env);
            // Add SpecialContainer
            Branch(IsSpecialContainer(jsType), &isSpecialContainer, &notSpecialContainer);
            Bind(&isSpecialContainer);
            {
                result = GetContainerProperty(glue, *holder, index, jsType);
                Jump(&exit);
            }
            Bind(&notSpecialContainer);
            {
                result = Hole();
                Jump(&exit);
            }
        }
        Bind(&notSpecialIndexed);
        {
            GateRef elements = GetElementsArray(*holder);
            Label isDictionaryElement(env);
            Label notDictionaryElement(env);
            Branch(IsDictionaryElement(hclass), &isDictionaryElement, &notDictionaryElement);
            Bind(&notDictionaryElement);
            {
                Label lessThanLength(env);
                Label notLessThanLength(env);
                Branch(Int32UnsignedLessThan(index, GetLengthOfTaggedArray(elements)),
                       &lessThanLength, &notLessThanLength);
                Bind(&lessThanLength);
                {
                    Label notHole(env);
                    Label isHole(env);
                    GateRef value = GetValueFromTaggedArray(elements, index);
                    Branch(TaggedIsNotHole(value), &notHole, &isHole);
                    Bind(&notHole);
                    {
                        result = value;
                        Jump(&exit);
                    }
                    Bind(&isHole);
                    {
                        Jump(&loopExit);
                    }
                }
                Bind(&notLessThanLength);
                {
                    result = Hole();
                    Jump(&exit);
                }
            }
            Bind(&isDictionaryElement);
            {
                GateRef entryA = FindElementFromNumberDictionary(glue, elements, index);
                Label notNegtiveOne(env);
                Label negtiveOne(env);
                Branch(Int32NotEqual(entryA, Int32(-1)), &notNegtiveOne, &negtiveOne);
                Bind(&notNegtiveOne);
                {
                    GateRef attr = GetAttributesFromDictionary<NumberDictionary>(elements, entryA);
                    GateRef value = GetValueFromDictionary<NumberDictionary>(elements, entryA);
                    Label isAccessor(env);
                    Label notAccessor(env);
                    Branch(IsAccessor(attr), &isAccessor, &notAccessor);
                    Bind(&isAccessor);
                    {
                        result = CallGetterHelper(glue, receiver, *holder, value);
                        Jump(&exit);
                    }
                    Bind(&notAccessor);
                    {
                        result = value;
                        Jump(&exit);
                    }
                }
                Bind(&negtiveOne);
                Jump(&loopExit);
            }
            Bind(&loopExit);
            {
                holder = GetPrototypeFromHClass(LoadHClass(*holder));
                Branch(TaggedIsHeapObject(*holder), &loopEnd, &afterLoop);
            }
        }
        Bind(&loopEnd);
        LoopEnd(&loopHead);
        Bind(&afterLoop);
        {
            result = Undefined();
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::GetPropertyByValue(GateRef glue, GateRef receiver, GateRef keyValue)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(key, VariableType::JS_ANY(), keyValue);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Label isNumberOrStringSymbol(env);
    Label notNumber(env);
    Label isStringOrSymbol(env);
    Label notStringOrSymbol(env);
    Label exit(env);

    Branch(TaggedIsNumber(*key), &isNumberOrStringSymbol, &notNumber);
    Bind(&notNumber);
    {
        Branch(TaggedIsStringOrSymbol(*key), &isNumberOrStringSymbol, &notStringOrSymbol);
        Bind(&notStringOrSymbol);
        {
            result = Hole();
            Jump(&exit);
        }
    }
    Bind(&isNumberOrStringSymbol);
    {
        GateRef index = TryToElementsIndex(*key);
        Label validIndex(env);
        Label notValidIndex(env);
        Branch(Int32GreaterThanOrEqual(index, Int32(0)), &validIndex, &notValidIndex);
        Bind(&validIndex);
        {
            result = GetPropertyByIndex(glue, receiver, index);
            Jump(&exit);
        }
        Bind(&notValidIndex);
        {
            Label notNumber1(env);
            Label getByName(env);
            Branch(TaggedIsNumber(*key), &exit, &notNumber1);
            Bind(&notNumber1);
            {
                Label isString(env);
                Label notString(env);
                Label isInternalString(env);
                Label notIntenalString(env);
                Branch(TaggedIsString(*key), &isString, &notString);
                Bind(&isString);
                {
                    Branch(IsInternalString(*key), &isInternalString, &notIntenalString);
                    Bind(&isInternalString);
                    Jump(&getByName);
                    Bind(&notIntenalString);
                    {
                        key = CallRuntime(glue, RTSTUB_ID(NewInternalString), { *key });
                        Jump(&getByName);
                    }
                }
                Bind(&notString);
                {
                    Jump(&getByName);
                }
            }
            Bind(&getByName);
            {
                result = GetPropertyByName(glue, receiver, *key);
                Jump(&exit);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::GetPropertyByName(GateRef glue, GateRef receiver, GateRef key)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    DEFVARIABLE(holder, VariableType::JS_ANY(), receiver);
    Label exit(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label loopExit(env);
    Label afterLoop(env);
    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        GateRef hclass = LoadHClass(*holder);
        GateRef jsType = GetObjectType(hclass);
        Label isSIndexObj(env);
        Label notSIndexObj(env);
        Branch(IsSpecialIndexedObj(jsType), &isSIndexObj, &notSIndexObj);
        Bind(&isSIndexObj);
        {
            // TypeArray
            Label isFastTypeArray(env);
            Label notFastTypeArray(env);
            Branch(IsFastTypeArray(jsType), &isFastTypeArray, &notFastTypeArray);
            Bind(&isFastTypeArray);
            {
                result = GetTypeArrayPropertyByName(glue, receiver, *holder, key, jsType);
                Label isNull(env);
                Label notNull(env);
                Branch(TaggedIsNull(*result), &isNull, &notNull);
                Bind(&isNull);
                {
                    result = Hole();
                    Jump(&exit);
                }
                Bind(&notNull);
                Branch(TaggedIsHole(*result), &notSIndexObj, &exit);
            }
            Bind(&notFastTypeArray);
            {
                result = Hole();
                Jump(&exit);
            }
        }
        Bind(&notSIndexObj);
        {
            Label isDicMode(env);
            Label notDicMode(env);
            Branch(IsDictionaryModeByHClass(hclass), &isDicMode, &notDicMode);
            Bind(&notDicMode);
            {
                GateRef layOutInfo = GetLayoutFromHClass(hclass);
                GateRef propsNum = GetNumberOfPropsFromHClass(hclass);
                // int entry = layoutInfo->FindElementWithCache(thread, hclass, key, propsNumber)
                GateRef entryA = FindElementWithCache(glue, layOutInfo, hclass, key, propsNum);
                Label hasEntry(env);
                Label noEntry(env);
                // if branch condition : entry != -1
                Branch(Int32NotEqual(entryA, Int32(-1)), &hasEntry, &noEntry);
                Bind(&hasEntry);
                {
                    // PropertyAttributes attr(layoutInfo->GetAttr(entry))
                    GateRef propAttr = GetPropAttrFromLayoutInfo(layOutInfo, entryA);
                    GateRef attr = GetInt32OfTInt(propAttr);
                    GateRef value = JSObjectGetProperty(*holder, hclass, attr);
                    Label isPropertyBox(env);
                    Label notPropertyBox(env);
                    Branch(TaggedIsPropertyBox(value), &isPropertyBox, &notPropertyBox);
                    Bind(&isPropertyBox);
                    {
                        result = GetValueFromPropertyBox(value);
                        Jump(&exit);
                    }
                    Bind(&notPropertyBox);
                    Label isAccessor(env);
                    Label notAccessor(env);
                    Branch(IsAccessor(attr), &isAccessor, &notAccessor);
                    Bind(&isAccessor);
                    {
                        result = CallGetterHelper(glue, receiver, *holder, value);
                        Jump(&exit);
                    }
                    Bind(&notAccessor);
                    {
                        Label notHole(env);
                        Branch(TaggedIsHole(value), &noEntry, &notHole);
                        Bind(&notHole);
                        {
                            result = value;
                            Jump(&exit);
                        }
                    }
                }
                Bind(&noEntry);
                {
                    Jump(&loopExit);
                }
            }
            Bind(&isDicMode);
            {
                GateRef array = GetPropertiesArray(*holder);
                // int entry = dict->FindEntry(key)
                GateRef entryB = FindEntryFromNameDictionary(glue, array, key);
                Label notNegtiveOne(env);
                Label negtiveOne(env);
                // if branch condition : entry != -1
                Branch(Int32NotEqual(entryB, Int32(-1)), &notNegtiveOne, &negtiveOne);
                Bind(&notNegtiveOne);
                {
                    // auto value = dict->GetValue(entry)
                    GateRef attr = GetAttributesFromDictionary<NameDictionary>(array, entryB);
                    // auto attr = dict->GetAttributes(entry)
                    GateRef value = GetValueFromDictionary<NameDictionary>(array, entryB);
                    Label isAccessor1(env);
                    Label notAccessor1(env);
                    Branch(IsAccessor(attr), &isAccessor1, &notAccessor1);
                    Bind(&isAccessor1);
                    {
                        result = CallGetterHelper(glue, receiver, *holder, value);
                        Jump(&exit);
                    }
                    Bind(&notAccessor1);
                    {
                        result = value;
                        Jump(&exit);
                    }
                }
                Bind(&negtiveOne);
                Jump(&loopExit);
            }
            Bind(&loopExit);
            {
                holder = GetPrototypeFromHClass(LoadHClass(*holder));
                Branch(TaggedIsHeapObject(*holder), &loopEnd, &afterLoop);
            }
        }
        Bind(&loopEnd);
        LoopEnd(&loopHead);
        Bind(&afterLoop);
        {
            result = Undefined();
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

void StubBuilder::CopyAllHClass(GateRef glue, GateRef dstHClass, GateRef srcHClass)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    auto proto = GetPrototypeFromHClass(srcHClass);
    SetPrototypeToHClass(VariableType::JS_POINTER(), glue, dstHClass, proto);
    SetBitFieldToHClass(glue, dstHClass, GetBitFieldFromHClass(srcHClass));
    SetNumberOfPropsToHClass(glue, dstHClass, GetNumberOfPropsFromHClass(srcHClass));
    SetTransitionsToHClass(VariableType::INT64(), glue, dstHClass, Undefined());
    SetProtoChangeDetailsToHClass(VariableType::INT64(), glue, dstHClass, Null());
    SetEnumCacheToHClass(VariableType::INT64(), glue, dstHClass, Null());
    SetLayoutToHClass(VariableType::JS_POINTER(), glue, dstHClass, GetLayoutFromHClass(srcHClass));
    env->SubCfgExit();
    return;
}

GateRef StubBuilder::FindTransitions(GateRef glue, GateRef receiver, GateRef hclass, GateRef key, GateRef metaData)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    GateRef transitionOffset = IntPtr(JSHClass::TRANSTIONS_OFFSET);
    GateRef transition = Load(VariableType::JS_POINTER(), hclass, transitionOffset);
    DEFVARIABLE(result, VariableType::JS_ANY(), transition);

    Label notUndefined(env);
    Branch(Equal(transition, Undefined()), &exit, &notUndefined);
    Bind(&notUndefined);
    {
        Label isWeak(env);
        Label notWeak(env);
        Branch(TaggedIsWeak(transition), &isWeak, &notWeak);
        Bind(&isWeak);
        {
            GateRef transitionHClass = LoadObjectFromWeakRef(transition);
            GateRef propNums = GetNumberOfPropsFromHClass(transitionHClass);
            GateRef last = Int32Sub(propNums, Int32(1));
            GateRef layoutInfo = GetLayoutFromHClass(transitionHClass);
            GateRef cachedKey = GetKeyFromLayoutInfo(layoutInfo, last);
            GateRef cachedAttr = GetInt32OfTInt(GetPropAttrFromLayoutInfo(layoutInfo, last));
            GateRef cachedMetaData = GetPropertyMetaDataFromAttr(cachedAttr);
            Label keyMatch(env);
            Label isMatch(env);
            Label notMatch(env);
            Branch(Equal(cachedKey, key), &keyMatch, &notMatch);
            Bind(&keyMatch);
            {
                Branch(Int32Equal(metaData, cachedMetaData), &isMatch, &notMatch);
                Bind(&isMatch);
                {
#if ECMASCRIPT_ENABLE_IC
                    NotifyHClassChanged(glue, hclass, transitionHClass);
#endif
                    StoreHClass(glue, receiver, transitionHClass);
                    Jump(&exit);
                }
            }
            Bind(&notMatch);
            {
                result = Undefined();
                Jump(&exit);
            }
        }
        Bind(&notWeak);
        {
            // need to find from dictionary
            GateRef entryA = FindEntryFromTransitionDictionary(glue, transition, key, metaData);
            Label isFound(env);
            Label notFound(env);
            Branch(Int32NotEqual(entryA, Int32(-1)), &isFound, &notFound);
            Bind(&isFound);
            auto value = GetValueFromDictionary<TransitionsDictionary>(transition, entryA);
            Label valueUndefined(env);
            Label valueNotUndefined(env);
            Branch(Int64NotEqual(value, Undefined()), &valueNotUndefined,
                &valueUndefined);
            Bind(&valueNotUndefined);
            {
                GateRef newHClass = LoadObjectFromWeakRef(value);
                result = newHClass;
#if ECMASCRIPT_ENABLE_IC
                NotifyHClassChanged(glue, hclass, newHClass);
#endif
                StoreHClass(glue, receiver, newHClass);
                Jump(&exit);
                Bind(&notFound);
                result = Undefined();
                Jump(&exit);
            }
            Bind(&valueUndefined);
            {
                result = Undefined();
                Jump(&exit);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::SetPropertyByIndex(GateRef glue, GateRef receiver, GateRef index, GateRef value, bool useOwn)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(returnValue, VariableType::JS_ANY(), Hole());
    DEFVARIABLE(holder, VariableType::JS_ANY(), receiver);
    Label exit(env);
    Label ifEnd(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label loopExit(env);
    Label afterLoop(env);
    Label isJsCOWArray(env);
    Label isNotJsCOWArray(env);
    Label setElementsArray(env);
    if (!useOwn) {
        Jump(&loopHead);
        LoopBegin(&loopHead);
    }
    GateRef hclass = LoadHClass(*holder);
    GateRef jsType = GetObjectType(hclass);
    Label isSpecialIndex(env);
    Label notSpecialIndex(env);
    Branch(IsSpecialIndexedObj(jsType), &isSpecialIndex, &notSpecialIndex);
    Bind(&isSpecialIndex);
    {
        // TypeArray
        Label isFastTypeArray(env);
        Label notFastTypeArray(env);
        Branch(IsFastTypeArray(jsType), &isFastTypeArray, &notFastTypeArray);
        Bind(&isFastTypeArray);
        {
            returnValue = CallRuntime(glue, RTSTUB_ID(SetTypeArrayPropertyByIndex),
                { receiver, IntToTaggedInt(index), value, IntToTaggedInt(jsType)});
            Jump(&exit);
        }
        Bind(&notFastTypeArray);
        returnValue = Hole();
        Jump(&exit);
    }
    Bind(&notSpecialIndex);
    {
        GateRef elements = GetElementsArray(*holder);
        Label isDictionaryElement(env);
        Label notDictionaryElement(env);
        Branch(IsDictionaryElement(hclass), &isDictionaryElement, &notDictionaryElement);
        Bind(&notDictionaryElement);
        {
            Label isReceiver(env);
            if (useOwn) {
                Branch(Equal(*holder, receiver), &isReceiver, &ifEnd);
            } else {
                Branch(Equal(*holder, receiver), &isReceiver, &afterLoop);
            }
            Bind(&isReceiver);
            {
                GateRef length = GetLengthOfTaggedArray(elements);
                Label inRange(env);
                if (useOwn) {
                    Branch(Int64LessThan(index, length), &inRange, &ifEnd);
                } else {
                    Branch(Int64LessThan(index, length), &inRange, &loopExit);
                }
                Bind(&inRange);
                {
                    GateRef value1 = GetValueFromTaggedArray(elements, index);
                    Label notHole(env);
                    if (useOwn) {
                        Branch(Int64NotEqual(value1, Hole()), &notHole, &ifEnd);
                    } else {
                        Branch(Int64NotEqual(value1, Hole()), &notHole, &loopExit);
                    }
                    Bind(&notHole);
                    {
                        Branch(IsJsCOWArray(*holder), &isJsCOWArray, &isNotJsCOWArray);
                        Bind(&isJsCOWArray);
                        {
                            CallRuntime(glue, RTSTUB_ID(CheckAndCopyArray), {*holder});
                            GateRef newElements = GetElementsArray(*holder);
                            SetValueToTaggedArray(VariableType::JS_ANY(), glue, newElements, index, value);
                            returnValue = Undefined();
                            Jump(&exit);
                        }
                        Bind(&isNotJsCOWArray);
                        {
                            Jump(&setElementsArray);
                        }
                        Bind(&setElementsArray);
                        {
                            SetValueToTaggedArray(VariableType::JS_ANY(), glue, elements, index, value);
                            returnValue = Undefined();
                            Jump(&exit);
                        }
                    }
                }
            }
        }
        Bind(&isDictionaryElement);
        {
            returnValue = Hole();
            Jump(&exit);
        }
    }
    if (useOwn) {
        Bind(&ifEnd);
    } else {
        Bind(&loopExit);
        {
            holder = GetPrototypeFromHClass(LoadHClass(*holder));
            Branch(TaggedIsHeapObject(*holder), &loopEnd, &afterLoop);
        }
        Bind(&loopEnd);
        LoopEnd(&loopHead);
        Bind(&afterLoop);
    }
    Label isExtensible(env);
    Label notExtensible(env);
    Branch(IsExtensible(receiver), &isExtensible, &notExtensible);
    Bind(&isExtensible);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(AddElementInternal),
            { receiver, IntToTaggedInt(index), value,
            IntToTaggedInt(Int32(PropertyAttributes::GetDefaultAttributes())) });
        Label success(env);
        Label failed(env);
        Branch(TaggedIsTrue(result), &success, &failed);
        Bind(&success);
        {
            returnValue = Undefined();
            Jump(&exit);
        }
        Bind(&failed);
        {
            returnValue = Exception();
            Jump(&exit);
        }
    }
    Bind(&notExtensible);
    {
        GateRef taggedId = Int32(GET_MESSAGE_STRING_ID(SetPropertyWhenNotExtensible));
        CallRuntime(glue, RTSTUB_ID(ThrowTypeError), { IntToTaggedInt(taggedId) });
        returnValue = Exception();
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *returnValue;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::SetPropertyByName(GateRef glue, GateRef receiver, GateRef key, GateRef value, bool useOwn)
{
    auto env = GetEnvironment();
    Label entryPass(env);
    env->SubCfgEntry(&entryPass);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    DEFVARIABLE(holder, VariableType::JS_POINTER(), receiver);
    DEFVARIABLE(receiverHoleEntry, VariableType::INT32(), Int32(-1));
    Label exit(env);
    Label ifEnd(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label loopExit(env);
    Label afterLoop(env);
    if (!useOwn) {
        // a do-while loop
        Jump(&loopHead);
        LoopBegin(&loopHead);
    }
    // auto *hclass = holder.GetTaggedObject()->GetClass()
    // JSType jsType = hclass->GetObjectType()
    GateRef hclass = LoadHClass(*holder);
    GateRef jsType = GetObjectType(hclass);
    Label isSIndexObj(env);
    Label notSIndexObj(env);
    // if branch condition : IsSpecialIndexedObj(jsType)
    Branch(IsSpecialIndexedObj(jsType), &isSIndexObj, &notSIndexObj);
    Bind(&isSIndexObj);
    {
        Label isFastTypeArray(env);
        Label notFastTypeArray(env);
        Branch(IsFastTypeArray(jsType), &isFastTypeArray, &notFastTypeArray);
        Bind(&isFastTypeArray);
        {
            result = SetTypeArrayPropertyByName(glue, receiver, *holder, key, value, jsType);
            Label isNull(env);
            Label notNull(env);
            Branch(TaggedIsNull(*result), &isNull, &notNull);
            Bind(&isNull);
            {
                result = Hole();
                Jump(&exit);
            }
            Bind(&notNull);
            Branch(TaggedIsHole(*result), &notSIndexObj, &exit);
        }
        Bind(&notFastTypeArray);

        Label isSpecialContainer(env);
        Label notSpecialContainer(env);
        // Add SpecialContainer
        Branch(IsSpecialContainer(jsType), &isSpecialContainer, &notSpecialContainer);
        Bind(&isSpecialContainer);
        {
            GateRef taggedId = Int32(GET_MESSAGE_STRING_ID(CanNotSetPropertyOnContainer));
            CallRuntime(glue, RTSTUB_ID(ThrowTypeError), { IntToTaggedInt(taggedId) });
            result = Exception();
            Jump(&exit);
        }
        Bind(&notSpecialContainer);
        {
            result = Hole();
            Jump(&exit);
        }
    }
    Bind(&notSIndexObj);
    {
        Label isDicMode(env);
        Label notDicMode(env);
        // if branch condition : LIKELY(!hclass->IsDictionaryMode())
        Branch(IsDictionaryModeByHClass(hclass), &isDicMode, &notDicMode);
        Bind(&notDicMode);
        {
            // LayoutInfo *layoutInfo = LayoutInfo::Cast(hclass->GetAttributes().GetTaggedObject())
            GateRef layOutInfo = GetLayoutFromHClass(hclass);
            // int propsNumber = hclass->NumberOfPropsFromHClass()
            GateRef propsNum = GetNumberOfPropsFromHClass(hclass);
            // int entry = layoutInfo->FindElementWithCache(thread, hclass, key, propsNumber)
            GateRef entry = FindElementWithCache(glue, layOutInfo, hclass, key, propsNum);
            Label hasEntry(env);
            // if branch condition : entry != -1
            if (useOwn) {
                Branch(Int32NotEqual(entry, Int32(-1)), &hasEntry, &ifEnd);
            } else {
                Branch(Int32NotEqual(entry, Int32(-1)), &hasEntry, &loopExit);
            }
            Bind(&hasEntry);
            {
                // PropertyAttributes attr(layoutInfo->GetAttr(entry))
                GateRef propAttr = GetPropAttrFromLayoutInfo(layOutInfo, entry);
                GateRef attr = GetInt32OfTInt(propAttr);
                Label isAccessor(env);
                Label notAccessor(env);
                Branch(IsAccessor(attr), &isAccessor, &notAccessor);
                Bind(&isAccessor);
                {
                    // auto accessor = JSObject::Cast(holder)->GetProperty(hclass, attr)
                    GateRef accessor = JSObjectGetProperty(*holder, hclass, attr);
                    Label shouldCall(env);
                    // ShouldCallSetter(receiver, *holder, accessor, attr)
                    Branch(ShouldCallSetter(receiver, *holder, accessor, attr), &shouldCall, &notAccessor);
                    Bind(&shouldCall);
                    {
                        result = CallSetterHelper(glue, receiver, accessor, value);
                        Jump(&exit);
                    }
                }
                Bind(&notAccessor);
                {
                    Label writable(env);
                    Label notWritable(env);
                    Branch(IsWritable(attr), &writable, &notWritable);
                    Bind(&notWritable);
                    {
                        GateRef taggedId = Int32(GET_MESSAGE_STRING_ID(SetReadOnlyProperty));
                        CallRuntime(glue, RTSTUB_ID(ThrowTypeError), { IntToTaggedInt(taggedId) });
                        result = Exception();
                        Jump(&exit);
                    }
                    Bind(&writable);
                    {
                        Label isTS(env);
                        Label notTS(env);
                        Branch(IsTSHClass(hclass), &isTS, &notTS);
                        Bind(&isTS);
                        {
                            GateRef attrVal = JSObjectGetProperty(*holder, hclass, attr);
                            Label attrValIsHole(env);
                            Branch(TaggedIsHole(attrVal), &attrValIsHole, &notTS);
                            Bind(&attrValIsHole);
                            {
                                Label storeReceiverHoleEntry(env);
                                Label noNeedStore(env);
                                GateRef checkReceiverHoleEntry = Int32Equal(*receiverHoleEntry, Int32(-1));
                                GateRef checkHolderEqualsRecv = Equal(*holder, receiver);
                                Branch(BoolAnd(checkReceiverHoleEntry, checkHolderEqualsRecv),
                                    &storeReceiverHoleEntry, &noNeedStore);
                                Bind(&storeReceiverHoleEntry);
                                {
                                    receiverHoleEntry = entry;
                                    Jump(&noNeedStore);
                                }
                                Bind(&noNeedStore);
                                if (useOwn) {
                                    Jump(&ifEnd);
                                } else {
                                    Jump(&loopExit);
                                }
                            }
                        }
                        Bind(&notTS);
                        Label holdEqualsRecv(env);
                        if (useOwn) {
                            Branch(Equal(*holder, receiver), &holdEqualsRecv, &ifEnd);
                        } else {
                            Branch(Equal(*holder, receiver), &holdEqualsRecv, &afterLoop);
                        }
                        Bind(&holdEqualsRecv);
                        {
                            // JSObject::Cast(holder)->SetProperty(thread, hclass, attr, value)
                            // return JSTaggedValue::Undefined()
                            JSObjectSetProperty(glue, *holder, hclass, attr, value);
                            result = Undefined();
                            Jump(&exit);
                        }
                    }
                }
            }
        }
        Bind(&isDicMode);
        {
            GateRef array = GetPropertiesArray(*holder);
            // int entry = dict->FindEntry(key)
            GateRef entry1 = FindEntryFromNameDictionary(glue, array, key);
            Label notNegtiveOne(env);
            // if branch condition : entry != -1
            if (useOwn) {
                Branch(Int32NotEqual(entry1, Int32(-1)), &notNegtiveOne, &ifEnd);
            } else {
                Branch(Int32NotEqual(entry1, Int32(-1)), &notNegtiveOne, &loopExit);
            }
            Bind(&notNegtiveOne);
            {
                // auto attr = dict->GetAttributes(entry)
                GateRef attr1 = GetAttributesFromDictionary<NameDictionary>(array, entry1);
                Label isAccessor1(env);
                Label notAccessor1(env);
                // if branch condition : UNLIKELY(attr.IsAccessor())
                Branch(IsAccessor(attr1), &isAccessor1, &notAccessor1);
                Bind(&isAccessor1);
                {
                    // auto accessor = dict->GetValue(entry)
                    GateRef accessor1 = GetValueFromDictionary<NameDictionary>(array, entry1);
                    Label shouldCall1(env);
                    Branch(ShouldCallSetter(receiver, *holder, accessor1, attr1), &shouldCall1, &notAccessor1);
                    Bind(&shouldCall1);
                    {
                        result = CallSetterHelper(glue, receiver, accessor1, value);
                        Jump(&exit);
                    }
                }
                Bind(&notAccessor1);
                {
                    Label writable1(env);
                    Label notWritable1(env);
                    Branch(IsWritable(attr1), &writable1, &notWritable1);
                    Bind(&notWritable1);
                    {
                        GateRef taggedId = Int32(GET_MESSAGE_STRING_ID(SetReadOnlyProperty));
                        CallRuntime(glue, RTSTUB_ID(ThrowTypeError), { IntToTaggedInt(taggedId) });
                        result = Exception();
                        Jump(&exit);
                    }
                    Bind(&writable1);
                    {
                        Label holdEqualsRecv1(env);
                        if (useOwn) {
                            Branch(Equal(*holder, receiver), &holdEqualsRecv1, &ifEnd);
                        } else {
                            Branch(Equal(*holder, receiver), &holdEqualsRecv1, &afterLoop);
                        }
                        Bind(&holdEqualsRecv1);
                        {
                            // dict->UpdateValue(thread, entry, value)
                            // return JSTaggedValue::Undefined()
                            UpdateValueInDict(glue, array, entry1, value);
                            result = Undefined();
                            Jump(&exit);
                        }
                    }
                }
            }
        }
    }
    if (useOwn) {
        Bind(&ifEnd);
    } else {
        Bind(&loopExit);
        {
            // holder = hclass->GetPrototype()
            holder = GetPrototypeFromHClass(LoadHClass(*holder));
            // loop condition for a do-while loop
            Branch(TaggedIsHeapObject(*holder), &loopEnd, &afterLoop);
        }
        Bind(&loopEnd);
        LoopEnd(&loopHead);
        Bind(&afterLoop);
    }

    Label holeEntryNotNegtiveOne(env);
    Label holeEntryIfEnd(env);
    Branch(Int32NotEqual(*receiverHoleEntry, Int32(-1)), &holeEntryNotNegtiveOne, &holeEntryIfEnd);
    Bind(&holeEntryNotNegtiveOne);
    {
        GateRef receiverHClass = LoadHClass(receiver);
        GateRef receiverLayoutInfo = GetLayoutFromHClass(receiverHClass);
        GateRef holePropAttr = GetPropAttrFromLayoutInfo(receiverLayoutInfo, *receiverHoleEntry);
        GateRef holeAttr = GetInt32OfTInt(holePropAttr);
        JSObjectSetProperty(glue, receiver, receiverHClass, holeAttr, value);
        result = Undefined();
        Jump(&exit);
    }
    Bind(&holeEntryIfEnd);

    Label extensible(env);
    Label inextensible(env);
    Branch(IsExtensible(receiver), &extensible, &inextensible);
    Bind(&inextensible);
    {
        GateRef taggedId = Int32(GET_MESSAGE_STRING_ID(SetPropertyWhenNotExtensible));
        CallRuntime(glue, RTSTUB_ID(ThrowTypeError), { IntToTaggedInt(taggedId) });
        result = Exception();
        Jump(&exit);
    }
    Bind(&extensible);
    {
        result = AddPropertyByName(glue, receiver, key, value,
            Int32(PropertyAttributes::GetDefaultAttributes()));
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::SetPropertyByValue(GateRef glue, GateRef receiver, GateRef key, GateRef value, bool useOwn)
{
    auto env = GetEnvironment();
    Label subEntry1(env);
    env->SubCfgEntry(&subEntry1);
    DEFVARIABLE(varKey, VariableType::JS_ANY(), key);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Label isNumberOrStringSymbol(env);
    Label notNumber(env);
    Label isStringOrSymbol(env);
    Label notStringOrSymbol(env);
    Label exit(env);
    Branch(TaggedIsNumber(*varKey), &isNumberOrStringSymbol, &notNumber);
    Bind(&notNumber);
    {
        Branch(TaggedIsStringOrSymbol(*varKey), &isNumberOrStringSymbol, &notStringOrSymbol);
        Bind(&notStringOrSymbol);
        {
            result = Hole();
            Jump(&exit);
        }
    }
    Bind(&isNumberOrStringSymbol);
    {
        GateRef index = TryToElementsIndex(*varKey);
        Label validIndex(env);
        Label notValidIndex(env);
        Branch(Int32GreaterThanOrEqual(index, Int32(0)), &validIndex, &notValidIndex);
        Bind(&validIndex);
        {
            result = SetPropertyByIndex(glue, receiver, index, value, useOwn);
            Jump(&exit);
        }
        Bind(&notValidIndex);
        {
            Label isNumber1(env);
            Label notNumber1(env);
            Label setByName(env);
            Branch(TaggedIsNumber(*varKey), &isNumber1, &notNumber1);
            Bind(&isNumber1);
            {
                result = Hole();
                Jump(&exit);
            }
            Bind(&notNumber1);
            {
                Label isString(env);
                Label notIntenalString(env);
                Branch(TaggedIsString(*varKey), &isString, &setByName);
                Bind(&isString);
                {
                    Branch(IsInternalString(*varKey), &setByName, &notIntenalString);
                    Bind(&notIntenalString);
                    {
                        varKey = CallRuntime(glue, RTSTUB_ID(NewInternalString), { *varKey });
                        Jump(&setByName);
                    }
                }
            }
            Bind(&setByName);
            {
                result = SetPropertyByName(glue, receiver, *varKey, value, useOwn);
                Jump(&exit);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

void StubBuilder::NotifyHClassChanged(GateRef glue, GateRef oldHClass, GateRef newHClass)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label isProtoType(env);
    Branch(IsProtoTypeHClass(oldHClass), &isProtoType, &exit);
    Bind(&isProtoType);
    {
        Label notEqualHClass(env);
        Branch(Equal(oldHClass, newHClass), &exit, &notEqualHClass);
        Bind(&notEqualHClass);
        {
            SetIsProtoTypeToHClass(glue, newHClass, True());
            CallRuntime(glue, RTSTUB_ID(NoticeThroughChainAndRefreshUser), { oldHClass, newHClass });
            Jump(&exit);
        }
    }
    Bind(&exit);
    env->SubCfgExit();
    return;
}

GateRef StubBuilder::GetContainerProperty(GateRef glue, GateRef receiver, GateRef index, GateRef jsType)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());

    Label isDefaultLabel(env);
    Label noDefaultLabel(env);
    Branch(IsSpecialContainer(jsType), &noDefaultLabel, &isDefaultLabel);
    Bind(&noDefaultLabel);
    {
        result = JSAPIContainerGet(glue, receiver, index);
        Jump(&exit);
    }
    Bind(&isDefaultLabel);
    {
        Jump(&exit);
    }
    Bind(&exit);

    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::FastTypeOf(GateRef glue, GateRef obj)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);

    GateRef gConstAddr = PtrAdd(glue,
        IntPtr(JSThread::GlueData::GetGlobalConstOffset(env->Is32Bit())));
    GateRef undefinedIndex = GetGlobalConstantString(ConstantIndex::UNDEFINED_STRING_INDEX);
    GateRef gConstUndefinedStr = Load(VariableType::JS_POINTER(), gConstAddr, undefinedIndex);
    DEFVARIABLE(result, VariableType::JS_POINTER(), gConstUndefinedStr);
    Label objIsTrue(env);
    Label objNotTrue(env);
    Label defaultLabel(env);
    GateRef gConstBooleanStr = Load(VariableType::JS_POINTER(), gConstAddr,
        GetGlobalConstantString(ConstantIndex::BOOLEAN_STRING_INDEX));
    Branch(TaggedIsTrue(obj), &objIsTrue, &objNotTrue);
    Bind(&objIsTrue);
    {
        result = gConstBooleanStr;
        Jump(&exit);
    }
    Bind(&objNotTrue);
    {
        Label objIsFalse(env);
        Label objNotFalse(env);
        Branch(TaggedIsFalse(obj), &objIsFalse, &objNotFalse);
        Bind(&objIsFalse);
        {
            result = gConstBooleanStr;
            Jump(&exit);
        }
        Bind(&objNotFalse);
        {
            Label objIsNull(env);
            Label objNotNull(env);
            Branch(TaggedIsNull(obj), &objIsNull, &objNotNull);
            Bind(&objIsNull);
            {
                result = Load(VariableType::JS_POINTER(), gConstAddr,
                    GetGlobalConstantString(ConstantIndex::OBJECT_STRING_INDEX));
                Jump(&exit);
            }
            Bind(&objNotNull);
            {
                Label objIsUndefined(env);
                Label objNotUndefined(env);
                Branch(TaggedIsUndefined(obj), &objIsUndefined, &objNotUndefined);
                Bind(&objIsUndefined);
                {
                    result = Load(VariableType::JS_POINTER(), gConstAddr,
                        GetGlobalConstantString(ConstantIndex::UNDEFINED_STRING_INDEX));
                    Jump(&exit);
                }
                Bind(&objNotUndefined);
                Jump(&defaultLabel);
            }
        }
    }
    Bind(&defaultLabel);
    {
        Label objIsHeapObject(env);
        Label objNotHeapObject(env);
        Branch(TaggedIsHeapObject(obj), &objIsHeapObject, &objNotHeapObject);
        Bind(&objIsHeapObject);
        {
            Label objIsString(env);
            Label objNotString(env);
            Branch(IsString(obj), &objIsString, &objNotString);
            Bind(&objIsString);
            {
                result = Load(VariableType::JS_POINTER(), gConstAddr,
                    GetGlobalConstantString(ConstantIndex::STRING_STRING_INDEX));
                Jump(&exit);
            }
            Bind(&objNotString);
            {
                Label objIsSymbol(env);
                Label objNotSymbol(env);
                Branch(IsSymbol(obj), &objIsSymbol, &objNotSymbol);
                Bind(&objIsSymbol);
                {
                    result = Load(VariableType::JS_POINTER(), gConstAddr,
                        GetGlobalConstantString(ConstantIndex::SYMBOL_STRING_INDEX));
                    Jump(&exit);
                }
                Bind(&objNotSymbol);
                {
                    Label objIsCallable(env);
                    Label objNotCallable(env);
                    Branch(IsCallable(obj), &objIsCallable, &objNotCallable);
                    Bind(&objIsCallable);
                    {
                        result = Load(VariableType::JS_POINTER(), gConstAddr,
                            GetGlobalConstantString(ConstantIndex::FUNCTION_STRING_INDEX));
                        Jump(&exit);
                    }
                    Bind(&objNotCallable);
                    {
                        Label objIsBigInt(env);
                        Label objNotBigInt(env);
                        Branch(TaggedObjectIsBigInt(obj), &objIsBigInt, &objNotBigInt);
                        Bind(&objIsBigInt);
                        {
                            result = Load(VariableType::JS_POINTER(), gConstAddr,
                                GetGlobalConstantString(ConstantIndex::BIGINT_STRING_INDEX));
                            Jump(&exit);
                        }
                        Bind(&objNotBigInt);
                        {
                            result = Load(VariableType::JS_POINTER(), gConstAddr,
                                GetGlobalConstantString(ConstantIndex::OBJECT_STRING_INDEX));
                            Jump(&exit);
                        }
                    }
                }
            }
        }
        Bind(&objNotHeapObject);
        {
            Label objIsNum(env);
            Label objNotNum(env);
            Branch(TaggedIsNumber(obj), &objIsNum, &objNotNum);
            Bind(&objIsNum);
            {
                result = Load(VariableType::JS_POINTER(), gConstAddr,
                    GetGlobalConstantString(ConstantIndex::NUMBER_STRING_INDEX));
                Jump(&exit);
            }
            Bind(&objNotNum);
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::InstanceOf(GateRef glue, GateRef object, GateRef target)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Label exit(env);

    // 1.If Type(target) is not Object, throw a TypeError exception.
    Label targetIsHeapObject(env);
    Label targetIsEcmaObject(env);
    Label targetNotEcmaObject(env);
    Branch(TaggedIsHeapObject(target), &targetIsHeapObject, &targetNotEcmaObject);
    Bind(&targetIsHeapObject);
    Branch(TaggedObjectIsEcmaObject(target), &targetIsEcmaObject, &targetNotEcmaObject);
    Bind(&targetNotEcmaObject);
    {
        GateRef taggedId = Int32(GET_MESSAGE_STRING_ID(TargetTypeNotObject));
        CallRuntime(glue, RTSTUB_ID(ThrowTypeError), { IntToTaggedInt(taggedId) });
        result = Exception();
        Jump(&exit);
    }
    Bind(&targetIsEcmaObject);
    {
        // 2.Let instOfHandler be GetMethod(target, @@hasInstance).
        GateRef glueGlobalEnvOffset = IntPtr(JSThread::GlueData::GetGlueGlobalEnvOffset(env->Is32Bit()));
        GateRef glueGlobalEnv = Load(VariableType::NATIVE_POINTER(), glue, glueGlobalEnvOffset);
        GateRef hasInstanceSymbol = GetGlobalEnvValue(VariableType::JS_ANY(), glueGlobalEnv,
                                                      GlobalEnv::HASINSTANCE_SYMBOL_INDEX);
        GateRef instof = GetMethod(glue, target, hasInstanceSymbol);

        // 3.ReturnIfAbrupt(instOfHandler).
        Label isPendingException1(env);
        Label noPendingException1(env);
        Branch(HasPendingException(glue), &isPendingException1, &noPendingException1);
        Bind(&isPendingException1);
        {
            result = Exception();
            Jump(&exit);
        }
        Bind(&noPendingException1);

        // 4.If instOfHandler is not undefined, then
        Label instOfNotUndefined(env);
        Label instOfIsUndefined(env);
        Branch(TaggedIsUndefined(instof), &instOfIsUndefined, &instOfNotUndefined);
        Bind(&instOfNotUndefined);
        {
            GateRef retValue = JSCallDispatch(glue, instof, Int32(1), 0, Circuit::NullGate(),
                                              JSCallMode::CALL_SETTER, { target, object });
            result =  FastToBoolean(retValue);
            Jump(&exit);
        }
        Bind(&instOfIsUndefined);
        {
            // 5.If IsCallable(target) is false, throw a TypeError exception.
            Label targetIsCallable1(env);
            Label targetNotCallable1(env);
            Branch(IsCallable(target), &targetIsCallable1, &targetNotCallable1);
            Bind(&targetNotCallable1);
            {
                GateRef taggedId = Int32(GET_MESSAGE_STRING_ID(InstanceOfErrorTargetNotCallable));
                CallRuntime(glue, RTSTUB_ID(ThrowTypeError), { IntToTaggedInt(taggedId) });
                result = Exception();
                Jump(&exit);
            }
            Bind(&targetIsCallable1);
            {
                // 6.Return ? OrdinaryHasInstance(target, object).
                result = OrdinaryHasInstance(glue, target, object);
                Jump(&exit);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::GetMethod(GateRef glue, GateRef obj, GateRef key)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Label exit(env);

    GateRef value = FastGetPropertyByName(glue, obj, key);

    Label isPendingException2(env);
    Label noPendingException2(env);
    Branch(HasPendingException(glue), &isPendingException2, &noPendingException2);
    Bind(&isPendingException2);
    {
        result = Exception();
        Jump(&exit);
    }
    Bind(&noPendingException2);
    Label valueIsUndefinedOrNull(env);
    Label valueNotUndefinedOrNull(env);
    Branch(TaggedIsUndefinedOrNull(value), &valueIsUndefinedOrNull, &valueNotUndefinedOrNull);
    Bind(&valueIsUndefinedOrNull);
    {
        result = Undefined();
        Jump(&exit);
    }
    Bind(&valueNotUndefinedOrNull);
    {
        Label valueIsCallable(env);
        Label valueNotCallable(env);
        Branch(IsCallable(value), &valueIsCallable, &valueNotCallable);
        Bind(&valueNotCallable);
        {
            GateRef taggedId = Int32(GET_MESSAGE_STRING_ID(NonCallable));
            CallRuntime(glue, RTSTUB_ID(ThrowTypeError), { IntToTaggedInt(taggedId) });
            result = Exception();
            Jump(&exit);
        }
        Bind(&valueIsCallable);
        {
            result = value;
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::FastGetPropertyByName(GateRef glue, GateRef obj, GateRef key)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Label exit(env);
    Label checkResult(env);
    Label fastpath(env);
    Label slowpath(env);

    Branch(TaggedIsHeapObject(obj), &fastpath, &slowpath);
    Bind(&fastpath);
    {
        result = GetPropertyByName(glue, obj, key);
        Branch(TaggedIsHole(*result), &slowpath, &exit);
    }
    Bind(&slowpath);
    {
        result = CallRuntime(glue, RTSTUB_ID(LoadICByName),
                             { Undefined(), obj, key, Int64ToTaggedPtr(Int32(0)) });
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::OrdinaryHasInstance(GateRef glue, GateRef target, GateRef obj)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Label exit(env);
    DEFVARIABLE(object, VariableType::JS_ANY(), obj);

    // 1. If IsCallable(C) is false, return false.
    Label targetIsCallable2(env);
    Label targetNotCallable2(env);
    Branch(IsCallable(target), &targetIsCallable2, &targetNotCallable2);
    Bind(&targetNotCallable2);
    {
        result = TaggedFalse();
        Jump(&exit);
    }
    Bind(&targetIsCallable2);
    {
        // 2. If C has a [[BoundTargetFunction]] internal slot, then
        //    a. Let BC be the value of C's [[BoundTargetFunction]] internal slot.
        //    b. Return InstanceofOperator(O,BC)  (see 12.9.4).
        Label targetIsBoundFunction(env);
        Label targetNotBoundFunction(env);
        Branch(IsBoundFunction(target), &targetIsBoundFunction, &targetNotBoundFunction);
        Bind(&targetIsBoundFunction);
        {
            GateRef boundTarget = Load(VariableType::JS_ANY(), target, IntPtr(JSBoundFunction::BOUND_TARGET_OFFSET));
            result = CallRuntime(glue, RTSTUB_ID(InstanceOf), { obj, boundTarget });
            Jump(&exit);
        }
        Bind(&targetNotBoundFunction);
        {
            // 3. If Type(O) is not Object, return false
            Label objIsHeapObject(env);
            Label objIsEcmaObject(env);
            Label objNotEcmaObject(env);
            Branch(TaggedIsHeapObject(obj), &objIsHeapObject, &objNotEcmaObject);
            Bind(&objIsHeapObject);
            Branch(TaggedObjectIsEcmaObject(obj), &objIsEcmaObject, &objNotEcmaObject);
            Bind(&objNotEcmaObject);
            {
                result = TaggedFalse();
                Jump(&exit);
            }
            Bind(&objIsEcmaObject);
            {
                // 4. Let P be Get(C, "prototype").
                auto prototypeString = GetGlobalConstantValue(
                    VariableType::JS_POINTER(), glue, ConstantIndex::PROTOTYPE_STRING_INDEX);

                GateRef constructorPrototype = FastGetPropertyByName(glue, target, prototypeString);

                // 5. ReturnIfAbrupt(P).
                // no throw exception, so needn't return
                Label isPendingException3(env);
                Label noPendingException3(env);
                Branch(HasPendingException(glue), &isPendingException3, &noPendingException3);
                Bind(&isPendingException3);
                {
                    result = Exception();
                    Jump(&exit);
                }
                Bind(&noPendingException3);

                // 6. If Type(P) is not Object, throw a TypeError exception.
                Label constructorPrototypeIsHeapObject(env);
                Label constructorPrototypeIsEcmaObject(env);
                Label constructorPrototypeNotEcmaObject(env);
                Branch(TaggedIsHeapObject(constructorPrototype), &constructorPrototypeIsHeapObject,
                    &constructorPrototypeNotEcmaObject);
                Bind(&constructorPrototypeIsHeapObject);
                Branch(TaggedObjectIsEcmaObject(constructorPrototype), &constructorPrototypeIsEcmaObject,
                    &constructorPrototypeNotEcmaObject);
                Bind(&constructorPrototypeNotEcmaObject);
                {
                    GateRef taggedId = Int32(GET_MESSAGE_STRING_ID(InstanceOfErrorTargetNotCallable));
                    CallRuntime(glue, RTSTUB_ID(ThrowTypeError), { IntToTaggedInt(taggedId) });
                    result = Exception();
                    Jump(&exit);
                }
                Bind(&constructorPrototypeIsEcmaObject);
                {
                    // 7. Repeat
                    //    a.Let O be O.[[GetPrototypeOf]]().
                    //    b.ReturnIfAbrupt(O).
                    //    c.If O is null, return false.
                    //    d.If SameValue(P, O) is true, return true.
                    Label loopHead(env);
                    Label loopEnd(env);
                    Label afterLoop(env);
                    Label strictEqual1(env);
                    Label notStrictEqual1(env);
                    Label shouldReturn(env);
                    Label shouldContinue(env);

                    Branch(TaggedIsNull(*object), &afterLoop, &loopHead);
                    LoopBegin(&loopHead);
                    {
                        GateRef isEqual = SameValue(glue, *object, constructorPrototype);

                        Branch(isEqual, &strictEqual1, &notStrictEqual1);
                        Bind(&strictEqual1);
                        {
                            result = TaggedTrue();
                            Jump(&exit);
                        }
                        Bind(&notStrictEqual1);
                        {
                            object = GetPrototype(glue, *object);

                            Branch(HasPendingException(glue), &shouldReturn, &shouldContinue);
                            Bind(&shouldReturn);
                            {
                                result = Exception();
                                Jump(&exit);
                            }
                        }
                        Bind(&shouldContinue);
                        Branch(TaggedIsNull(*object), &afterLoop, &loopEnd);
                    }
                    Bind(&loopEnd);
                    LoopEnd(&loopHead);
                    Bind(&afterLoop);
                    {
                        result = TaggedFalse();
                        Jump(&exit);
                    }
                }
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::GetPrototype(GateRef glue, GateRef object)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Label exit(env);
    Label objectIsHeapObject(env);
    Label objectIsEcmaObject(env);
    Label objectNotEcmaObject(env);

    Branch(TaggedIsHeapObject(object), &objectIsHeapObject, &objectNotEcmaObject);
    Bind(&objectIsHeapObject);
    Branch(TaggedObjectIsEcmaObject(object), &objectIsEcmaObject, &objectNotEcmaObject);
    Bind(&objectNotEcmaObject);
    {
        GateRef taggedId = Int32(GET_MESSAGE_STRING_ID(CanNotGetNotEcmaObject));
        CallRuntime(glue, RTSTUB_ID(ThrowTypeError), { IntToTaggedInt(taggedId) });
        result = Exception();
        Jump(&exit);
    }
    Bind(&objectIsEcmaObject);
    {
        Label objectIsJsProxy(env);
        Label objectNotIsJsProxy(env);
        Branch(IsJsProxy(object), &objectIsJsProxy, &objectNotIsJsProxy);
        Bind(&objectIsJsProxy);
        {
            result = CallRuntime(glue, RTSTUB_ID(CallGetPrototype), { object });
            Jump(&exit);
        }
        Bind(&objectNotIsJsProxy);
        {
            result = GetPrototypeFromHClass(LoadHClass(object));
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::SameValue(GateRef glue, GateRef left, GateRef right)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::BOOL(), False());
    Label exit(env);
    DEFVARIABLE(doubleLeft, VariableType::FLOAT64(), Double(0.0));
    DEFVARIABLE(doubleRight, VariableType::FLOAT64(), Double(0.0));
    Label strictEqual2(env);
    Label stringEqualCheck(env);
    Label stringCompare(env);
    Label bigIntEqualCheck(env);
    Label numberEqualCheck1(env);

    Branch(Equal(left, right), &strictEqual2, &numberEqualCheck1);
    Bind(&strictEqual2);
    {
        result = True();
        Jump(&exit);
    }
    Bind(&numberEqualCheck1);
    {
        Label leftIsNumber(env);
        Label leftIsNotNumber(env);
        Branch(TaggedIsNumber(left), &leftIsNumber, &leftIsNotNumber);
        Bind(&leftIsNumber);
        {
            Label rightIsNumber(env);
            Branch(TaggedIsNumber(right), &rightIsNumber, &exit);
            Bind(&rightIsNumber);
            {
                Label numberEqualCheck2(env);
                Label leftIsInt(env);
                Label leftNotInt(env);
                Label getRight(env);
                Branch(TaggedIsInt(left), &leftIsInt, &leftNotInt);
                Bind(&leftIsInt);
                {
                    doubleLeft = ChangeInt32ToFloat64(GetInt32OfTInt(left));
                    Jump(&getRight);
                }
                Bind(&leftNotInt);
                {
                    doubleLeft = GetDoubleOfTDouble(left);
                    Jump(&getRight);
                }
                Bind(&getRight);
                {
                    Label rightIsInt(env);
                    Label rightNotInt(env);
                    Branch(TaggedIsInt(right), &rightIsInt, &rightNotInt);
                    Bind(&rightIsInt);
                    {
                        doubleRight = ChangeInt32ToFloat64(GetInt32OfTInt(right));
                        Jump(&numberEqualCheck2);
                    }
                    Bind(&rightNotInt);
                    {
                        doubleRight = GetDoubleOfTDouble(right);
                        Jump(&numberEqualCheck2);
                    }
                }
                Bind(&numberEqualCheck2);
                {
                    Label boolAndCheck(env);
                    Label signbitCheck(env);
                    Branch(DoubleEqual(*doubleLeft, *doubleRight), &signbitCheck, &boolAndCheck);
                    Bind(&signbitCheck);
                    {
                        GateRef leftEncoding = CastDoubleToInt64(*doubleLeft);
                        GateRef RightEncoding = CastDoubleToInt64(*doubleRight);
                        Label leftIsMinusZero(env);
                        Label leftNotMinusZero(env);
                        Branch(Int64Equal(leftEncoding, Int64(base::MINUS_ZERO_BITS)),
                            &leftIsMinusZero, &leftNotMinusZero);
                        Bind(&leftIsMinusZero);
                        {
                            Label rightIsMinusZero(env);
                            Branch(Int64Equal(RightEncoding, Int64(base::MINUS_ZERO_BITS)), &rightIsMinusZero, &exit);
                            Bind(&rightIsMinusZero);
                            {
                                result = True();
                                Jump(&exit);
                            }
                        }
                        Bind(&leftNotMinusZero);
                        {
                            Label rightNotMinusZero(env);
                            Branch(Int64Equal(RightEncoding, Int64(base::MINUS_ZERO_BITS)), &exit, &rightNotMinusZero);
                            Bind(&rightNotMinusZero);
                            {
                                result = True();
                                Jump(&exit);
                            }
                        }
                    }
                    Bind(&boolAndCheck);
                    {
                        result = BoolAnd(DoubleIsNAN(*doubleLeft), DoubleIsNAN(*doubleRight));
                        Jump(&exit);
                    }
                }
            }
        }
        Bind(&leftIsNotNumber);
        Branch(TaggedIsNumber(right), &exit, &stringEqualCheck);
        Bind(&stringEqualCheck);
        Branch(BothAreString(left, right), &stringCompare, &bigIntEqualCheck);
        Bind(&stringCompare);
        {
            result = FastStringEqual(glue, left, right);
            Jump(&exit);
        }
        Bind(&bigIntEqualCheck);
        {
            Label leftIsBigInt(env);
            Label leftIsNotBigInt(env);
            Branch(TaggedIsBigInt(left), &leftIsBigInt, &exit);
            Bind(&leftIsBigInt);
            {
                Label rightIsBigInt(env);
                Branch(TaggedIsBigInt(right), &rightIsBigInt, &exit);
                Bind(&rightIsBigInt);
                result = CallNGCRuntime(glue, RTSTUB_ID(BigIntEquals), { left, right });
                Jump(&exit);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::FastStringEqual(GateRef glue, GateRef left, GateRef right)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::BOOL(), False());
    Label exit(env);
    Label lengthCompare(env);
    Label hashcodeCompare(env);
    Label contentsCompare(env);

    Branch(Int32Equal(ZExtInt1ToInt32(IsUtf16String(left)), ZExtInt1ToInt32(IsUtf16String(right))),
        &lengthCompare, &exit);

    Bind(&lengthCompare);
    Branch(Int32Equal(GetLengthFromString(left), GetLengthFromString(right)), &hashcodeCompare,
        &exit);

    Bind(&hashcodeCompare);
    Branch(Int32Equal(GetHashcodeFromString(glue, left), GetHashcodeFromString(glue, right)), &contentsCompare,
        &exit);

    Bind(&contentsCompare);
    {
        result = CallNGCRuntime(glue, RTSTUB_ID(StringsAreEquals), { left, right });
        Jump(&exit);
    }

    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::FastStrictEqual(GateRef glue, GateRef left, GateRef right)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::BOOL(), False());
    Label strictEqual(env);
    Label leftIsNumber(env);
    Label leftIsNotNumber(env);
    Label sameVariableCheck(env);
    Label stringEqualCheck(env);
    Label stringCompare(env);
    Label bigIntEqualCheck(env);
    Label exit(env);
    Branch(TaggedIsNumber(left), &leftIsNumber, &leftIsNotNumber);
    Bind(&leftIsNumber);
    {
        Label rightIsNumber(env);
        Branch(TaggedIsNumber(right), &rightIsNumber, &exit);
        Bind(&rightIsNumber);
        {
            DEFVARIABLE(doubleLeft, VariableType::FLOAT64(), Double(0.0));
            DEFVARIABLE(doubleRight, VariableType::FLOAT64(), Double(0.0));
            Label leftIsInt(env);
            Label leftNotInt(env);
            Label getRight(env);
            Label numberEqualCheck(env);
            Branch(TaggedIsInt(left), &leftIsInt, &leftNotInt);
            Bind(&leftIsInt);
            {
                doubleLeft = ChangeInt32ToFloat64(GetInt32OfTInt(left));
                Jump(&getRight);
            }
            Bind(&leftNotInt);
            {
                doubleLeft = GetDoubleOfTDouble(left);
                Jump(&getRight);
            }
            Bind(&getRight);
            {
                Label rightIsInt(env);
                Label rightNotInt(env);
                Branch(TaggedIsInt(right), &rightIsInt, &rightNotInt);
                Bind(&rightIsInt);
                {
                    doubleRight = ChangeInt32ToFloat64(GetInt32OfTInt(right));
                    Jump(&numberEqualCheck);
                }
                Bind(&rightNotInt);
                {
                    doubleRight = GetDoubleOfTDouble(right);
                    Jump(&numberEqualCheck);
                }
            }
            Bind(&numberEqualCheck);
            {
                Label doubleEqualCheck(env);
                Branch(BoolOr(DoubleIsNAN(*doubleLeft), DoubleIsNAN(*doubleRight)), &exit, &doubleEqualCheck);
                Bind(&doubleEqualCheck);
                {
                    result = DoubleEqual(*doubleLeft, *doubleRight);
                    Jump(&exit);
                }
            }
        }
    }
    Bind(&leftIsNotNumber);
    Branch(TaggedIsNumber(right), &exit, &sameVariableCheck);
    Bind(&sameVariableCheck);
    Branch(Equal(left, right), &strictEqual, &stringEqualCheck);
    Bind(&stringEqualCheck);
    Branch(BothAreString(left, right), &stringCompare, &bigIntEqualCheck);
    Bind(&stringCompare);
    {
        result = FastStringEqual(glue, left, right);
        Jump(&exit);
    }
    Bind(&bigIntEqualCheck);
    {
        Label leftIsBigInt(env);
        Label leftIsNotBigInt(env);
        Branch(TaggedIsBigInt(left), &leftIsBigInt, &exit);
        Bind(&leftIsBigInt);
        {
            Label rightIsBigInt(env);
            Branch(TaggedIsBigInt(right), &rightIsBigInt, &exit);
            Bind(&rightIsBigInt);
            result = CallNGCRuntime(glue, RTSTUB_ID(BigIntEquals), { left, right });
            Jump(&exit);
        }
    }
    Bind(&strictEqual);
    {
        result = True();
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::FastEqual(GateRef left, GateRef right)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Label leftEqualRight(env);
    Label leftNotEqualRight(env);
    Label exit(env);
    Branch(Equal(left, right), &leftEqualRight, &leftNotEqualRight);
    Bind(&leftEqualRight);
    {
        Label leftIsDouble(env);
        Label leftNotDoubleOrLeftNotNan(env);
        Branch(TaggedIsDouble(left), &leftIsDouble, &leftNotDoubleOrLeftNotNan);
        Bind(&leftIsDouble);
        {
            GateRef doubleLeft = GetDoubleOfTDouble(left);
            Label leftIsNan(env);
            Branch(DoubleIsNAN(doubleLeft), &leftIsNan, &leftNotDoubleOrLeftNotNan);
            Bind(&leftIsNan);
            {
                result = TaggedFalse();
                Jump(&exit);
            }
        }
        Bind(&leftNotDoubleOrLeftNotNan);
        {
            result = TaggedTrue();
            Jump(&exit);
        }
    }
    Bind(&leftNotEqualRight);
    {
        Label leftIsNumber(env);
        Label leftNotNumberOrLeftNotIntOrRightNotInt(env);
        Branch(TaggedIsNumber(left), &leftIsNumber, &leftNotNumberOrLeftNotIntOrRightNotInt);
        Bind(&leftIsNumber);
        {
            Label leftIsInt(env);
            Branch(TaggedIsInt(left), &leftIsInt, &leftNotNumberOrLeftNotIntOrRightNotInt);
            Bind(&leftIsInt);
            {
                Label rightIsInt(env);
                Branch(TaggedIsInt(right), &rightIsInt, &leftNotNumberOrLeftNotIntOrRightNotInt);
                Bind(&rightIsInt);
                {
                    result = TaggedFalse();
                    Jump(&exit);
                }
            }
        }
        Bind(&leftNotNumberOrLeftNotIntOrRightNotInt);
        {
            Label rightIsUndefinedOrNull(env);
            Label leftOrRightNotUndefinedOrNull(env);
            Branch(TaggedIsUndefinedOrNull(right), &rightIsUndefinedOrNull, &leftOrRightNotUndefinedOrNull);
            Bind(&rightIsUndefinedOrNull);
            {
                Label leftIsHeapObject(env);
                Label leftNotHeapObject(env);
                Branch(TaggedIsHeapObject(left), &leftIsHeapObject, &leftNotHeapObject);
                Bind(&leftIsHeapObject);
                {
                    result = TaggedFalse();
                    Jump(&exit);
                }
                Bind(&leftNotHeapObject);
                {
                    Label leftIsUndefinedOrNull(env);
                    Branch(TaggedIsUndefinedOrNull(left), &leftIsUndefinedOrNull, &leftOrRightNotUndefinedOrNull);
                    Bind(&leftIsUndefinedOrNull);
                    {
                        result = TaggedTrue();
                        Jump(&exit);
                    }
                }
            }
            Bind(&leftOrRightNotUndefinedOrNull);
            {
                Label leftIsBool(env);
                Label leftNotBoolOrRightNotSpecial(env);
                Branch(TaggedIsBoolean(left), &leftIsBool, &leftNotBoolOrRightNotSpecial);
                Bind(&leftIsBool);
                {
                    Label rightIsSpecial(env);
                    Branch(TaggedIsSpecial(right), &rightIsSpecial, &leftNotBoolOrRightNotSpecial);
                    Bind(&rightIsSpecial);
                    {
                        result = TaggedFalse();
                        Jump(&exit);
                    }
                }
                Bind(&leftNotBoolOrRightNotSpecial);
                {
                    Jump(&exit);
                }
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::FastToBoolean(GateRef value)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Label exit(env);

    Label isSpecial(env);
    Label notSpecial(env);
    Label isNumber(env);
    Label isInt(env);
    Label isDouble(env);
    Label notNumber(env);
    Label notNan(env);
    Label isString(env);
    Label notString(env);
    Label isBigint(env);
    Label lengthIsOne(env);
    Label returnTrue(env);
    Label returnFalse(env);

    Branch(TaggedIsSpecial(value), &isSpecial, &notSpecial);
    Bind(&isSpecial);
    {
        Branch(TaggedIsTrue(value), &returnTrue, &returnFalse);
    }
    Bind(&notSpecial);
    {
        Branch(TaggedIsNumber(value), &isNumber, &notNumber);
        Bind(&notNumber);
        {
            Branch(IsString(value), &isString, &notString);
            Bind(&isString);
            {
                auto len = GetLengthFromString(value);
                Branch(Int32Equal(len, Int32(0)), &returnFalse, &returnTrue);
            }
            Bind(&notString);
            Branch(TaggedObjectIsBigInt(value), &isBigint, &returnTrue);
            Bind(&isBigint);
            {
                auto len = Load(VariableType::INT32(), value, IntPtr(BigInt::LENGTH_OFFSET));
                Branch(Int32Equal(len, Int32(1)), &lengthIsOne, &returnTrue);
                Bind(&lengthIsOne);
                {
                    auto data = PtrAdd(value, IntPtr(BigInt::DATA_OFFSET));
                    auto data0 = Load(VariableType::INT32(), data, Int32(0));
                    Branch(Int32Equal(data0, Int32(0)), &returnFalse, &returnTrue);
                }
            }
        }
        Bind(&isNumber);
        {
            Branch(TaggedIsInt(value), &isInt, &isDouble);
            Bind(&isInt);
            {
                auto intValue = GetInt32OfTInt(value);
                Branch(Int32Equal(intValue, Int32(0)), &returnFalse, &returnTrue);
            }
            Bind(&isDouble);
            {
                auto doubleValue = GetDoubleOfTDouble(value);
                Branch(DoubleIsNAN(doubleValue), &returnFalse, &notNan);
                Bind(&notNan);
                Branch(DoubleEqual(doubleValue, Double(0.0)), &returnFalse, &returnTrue);
            }
        }
    }
    Bind(&returnTrue);
    {
        result = TaggedTrue();
        Jump(&exit);
    }
    Bind(&returnFalse);
    {
        result = TaggedFalse();
        Jump(&exit);
    }

    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::FastDiv(GateRef left, GateRef right)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    DEFVARIABLE(doubleLeft, VariableType::FLOAT64(), Double(0));
    DEFVARIABLE(doubleRight, VariableType::FLOAT64(), Double(0));
    Label leftIsNumber(env);
    Label leftNotNumberOrRightNotNumber(env);
    Label leftIsNumberAndRightIsNumber(env);
    Label leftIsDoubleAndRightIsDouble(env);
    Label exit(env);
    Branch(TaggedIsNumber(left), &leftIsNumber, &leftNotNumberOrRightNotNumber);
    Bind(&leftIsNumber);
    {
        Label rightIsNumber(env);
        Branch(TaggedIsNumber(right), &rightIsNumber, &leftNotNumberOrRightNotNumber);
        Bind(&rightIsNumber);
        {
            Label leftIsInt(env);
            Label leftNotInt(env);
            Branch(TaggedIsInt(left), &leftIsInt, &leftNotInt);
            Bind(&leftIsInt);
            {
                doubleLeft = ChangeInt32ToFloat64(GetInt32OfTInt(left));
                Jump(&leftIsNumberAndRightIsNumber);
            }
            Bind(&leftNotInt);
            {
                doubleLeft = GetDoubleOfTDouble(left);
                Jump(&leftIsNumberAndRightIsNumber);
            }
        }
    }
    Bind(&leftNotNumberOrRightNotNumber);
    {
        Jump(&exit);
    }
    Bind(&leftIsNumberAndRightIsNumber);
    {
        Label rightIsInt(env);
        Label rightNotInt(env);
        Branch(TaggedIsInt(right), &rightIsInt, &rightNotInt);
        Bind(&rightIsInt);
        {
            doubleRight = ChangeInt32ToFloat64(GetInt32OfTInt(right));
            Jump(&leftIsDoubleAndRightIsDouble);
        }
        Bind(&rightNotInt);
        {
            doubleRight = GetDoubleOfTDouble(right);
            Jump(&leftIsDoubleAndRightIsDouble);
        }
    }
    Bind(&leftIsDoubleAndRightIsDouble);
    {
        Label rightIsZero(env);
        Label rightNotZero(env);
        Branch(DoubleEqual(*doubleRight, Double(0.0)), &rightIsZero, &rightNotZero);
        Bind(&rightIsZero);
        {
            Label leftIsZero(env);
            Label leftNotZero(env);
            Label leftIsZeroOrNan(env);
            Label leftNotZeroAndNotNan(env);
            Branch(DoubleEqual(*doubleLeft, Double(0.0)), &leftIsZero, &leftNotZero);
            Bind(&leftIsZero);
            {
                Jump(&leftIsZeroOrNan);
            }
            Bind(&leftNotZero);
            {
                Label leftIsNan(env);
                Branch(DoubleIsNAN(*doubleLeft), &leftIsNan, &leftNotZeroAndNotNan);
                Bind(&leftIsNan);
                {
                    Jump(&leftIsZeroOrNan);
                }
            }
            Bind(&leftIsZeroOrNan);
            {
                result = DoubleToTaggedDoublePtr(Double(base::NAN_VALUE));
                Jump(&exit);
            }
            Bind(&leftNotZeroAndNotNan);
            {
                GateRef intLeftTmp = CastDoubleToInt64(*doubleLeft);
                GateRef intRightTmp = CastDoubleToInt64(*doubleRight);
                GateRef flagBit = Int64And(Int64Xor(intLeftTmp, intRightTmp), Int64(base::DOUBLE_SIGN_MASK));
                GateRef tmpResult = Int64Xor(flagBit, CastDoubleToInt64(Double(base::POSITIVE_INFINITY)));
                result = DoubleToTaggedDoublePtr(CastInt64ToFloat64(tmpResult));
                Jump(&exit);
            }
        }
        Bind(&rightNotZero);
        {
            result = DoubleToTaggedDoublePtr(DoubleDiv(*doubleLeft, *doubleRight));
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::FastBinaryOp(GateRef left, GateRef right,
                                  const BinaryOperation& intOp,
                                  const BinaryOperation& floatOp)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    DEFVARIABLE(doubleLeft, VariableType::FLOAT64(), Double(0));
    DEFVARIABLE(doubleRight, VariableType::FLOAT64(), Double(0));

    Label exit(env);
    Label doFloatOp(env);
    Label doIntOp(env);
    Label leftIsNumber(env);
    Label rightIsNumber(env);
    Label leftIsIntRightIsDouble(env);
    Label rightIsInt(env);
    Label rightIsDouble(env);

    Branch(TaggedIsNumber(left), &leftIsNumber, &exit);
    Bind(&leftIsNumber);
    {
        Branch(TaggedIsNumber(right), &rightIsNumber, &exit);
        Bind(&rightIsNumber);
        {
            Label leftIsInt(env);
            Label leftIsDouble(env);
            Branch(TaggedIsInt(left), &leftIsInt, &leftIsDouble);
            Bind(&leftIsInt);
            {
                Branch(TaggedIsInt(right), &doIntOp, &leftIsIntRightIsDouble);
                Bind(&leftIsIntRightIsDouble);
                {
                    doubleLeft = ChangeInt32ToFloat64(GetInt32OfTInt(left));
                    doubleRight = GetDoubleOfTDouble(right);
                    Jump(&doFloatOp);
                }
            }
            Bind(&leftIsDouble);
            {
                Branch(TaggedIsInt(right), &rightIsInt, &rightIsDouble);
                Bind(&rightIsInt);
                {
                    doubleLeft = GetDoubleOfTDouble(left);
                    doubleRight = ChangeInt32ToFloat64(GetInt32OfTInt(right));
                    Jump(&doFloatOp);
                }
                Bind(&rightIsDouble);
                {
                    doubleLeft = GetDoubleOfTDouble(left);
                    doubleRight = GetDoubleOfTDouble(right);
                    Jump(&doFloatOp);
                }
            }
        }
    }
    Bind(&doIntOp);
    {
        result = intOp(env, left, right);
        Jump(&exit);
    }
    Bind(&doFloatOp);
    {
        result = floatOp(env, *doubleLeft, *doubleRight);
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

template<OpCode Op>
GateRef StubBuilder::FastAddSubAndMul(GateRef left, GateRef right)
{
    auto intOperation = [=](Environment *env, GateRef left, GateRef right) {
        Label entry(env);
        env->SubCfgEntry(&entry);
        DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
        Label exit(env);
        Label overflow(env);
        Label notOverflow(env);
        auto res = BinaryOp<Op, MachineType::I64>(GetInt64OfTInt(left), GetInt64OfTInt(right));
        auto condition1 = Int64GreaterThan(res, Int64(INT32_MAX));
        auto condition2 = Int64LessThan(res, Int64(INT32_MIN));
        Branch(BoolOr(condition1, condition2), &overflow, &notOverflow);
        Bind(&overflow);
        {
            auto doubleLeft = ChangeInt32ToFloat64(GetInt32OfTInt(left));
            auto doubleRight = ChangeInt32ToFloat64(GetInt32OfTInt(right));
            auto ret = BinaryOp<Op, MachineType::F64>(doubleLeft, doubleRight);
            result = DoubleToTaggedDoublePtr(ret);
            Jump(&exit);
        }
        Bind(&notOverflow);
        {
            result = IntToTaggedPtr(TruncInt64ToInt32(res));
            Jump(&exit);
        }
        Bind(&exit);
        auto ret = *result;
        env->SubCfgExit();
        return ret;
    };
    auto floatOperation = [=]([[maybe_unused]] Environment *env, GateRef left, GateRef right) {
        auto res = BinaryOp<Op, MachineType::F64>(left, right);
        return DoubleToTaggedDoublePtr(res);
    };
    return FastBinaryOp(left, right, intOperation, floatOperation);
}

GateRef StubBuilder::FastAdd(GateRef left, GateRef right)
{
    return FastAddSubAndMul<OpCode::ADD>(left, right);
}

GateRef StubBuilder::FastSub(GateRef left, GateRef right)
{
    return FastAddSubAndMul<OpCode::SUB>(left, right);
}

GateRef StubBuilder::FastMul(GateRef left, GateRef right)
{
    return FastAddSubAndMul<OpCode::MUL>(left, right);
}

GateRef StubBuilder::FastMod(GateRef glue, GateRef left, GateRef right)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    DEFVARIABLE(intLeft, VariableType::INT32(), Int32(0));
    DEFVARIABLE(intRight, VariableType::INT32(), Int32(0));
    DEFVARIABLE(doubleLeft, VariableType::FLOAT64(), Double(0));
    DEFVARIABLE(doubleRight, VariableType::FLOAT64(), Double(0));
    Label leftIsInt(env);
    Label leftNotIntOrRightNotInt(env);
    Label exit(env);
    Branch(TaggedIsInt(left), &leftIsInt, &leftNotIntOrRightNotInt);
    Bind(&leftIsInt);
    {
        Label rightIsInt(env);
        Branch(TaggedIsInt(right), &rightIsInt, &leftNotIntOrRightNotInt);
        Bind(&rightIsInt);
        {
            intLeft = GetInt32OfTInt(left);
            intRight = GetInt32OfTInt(right);
            Label leftGreaterZero(env);
            Branch(Int32GreaterThan(*intLeft, Int32(0)), &leftGreaterZero, &leftNotIntOrRightNotInt);
            Bind(&leftGreaterZero);
            {
                Label rightGreaterZero(env);
                Branch(Int32GreaterThan(*intRight, Int32(0)), &rightGreaterZero, &leftNotIntOrRightNotInt);
                Bind(&rightGreaterZero);
                {
                    result = IntToTaggedPtr(Int32Mod(*intLeft, *intRight));
                    Jump(&exit);
                }
            }
        }
    }
    Bind(&leftNotIntOrRightNotInt);
    {
        Label leftIsNumber(env);
        Label leftNotNumberOrRightNotNumber(env);
        Label leftIsNumberAndRightIsNumber(env);
        Label leftIsDoubleAndRightIsDouble(env);
        Branch(TaggedIsNumber(left), &leftIsNumber, &leftNotNumberOrRightNotNumber);
        Bind(&leftIsNumber);
        {
            Label rightIsNumber(env);
            Branch(TaggedIsNumber(right), &rightIsNumber, &leftNotNumberOrRightNotNumber);
            Bind(&rightIsNumber);
            {
                Label leftIsInt1(env);
                Label leftNotInt1(env);
                Branch(TaggedIsInt(left), &leftIsInt1, &leftNotInt1);
                Bind(&leftIsInt1);
                {
                    doubleLeft = ChangeInt32ToFloat64(GetInt32OfTInt(left));
                    Jump(&leftIsNumberAndRightIsNumber);
                }
                Bind(&leftNotInt1);
                {
                    doubleLeft = GetDoubleOfTDouble(left);
                    Jump(&leftIsNumberAndRightIsNumber);
                }
            }
        }
        Bind(&leftNotNumberOrRightNotNumber);
        {
            Jump(&exit);
        }
        Bind(&leftIsNumberAndRightIsNumber);
        {
            Label rightIsInt1(env);
            Label rightNotInt1(env);
            Branch(TaggedIsInt(right), &rightIsInt1, &rightNotInt1);
            Bind(&rightIsInt1);
            {
                doubleRight = ChangeInt32ToFloat64(GetInt32OfTInt(right));
                Jump(&leftIsDoubleAndRightIsDouble);
            }
            Bind(&rightNotInt1);
            {
                doubleRight = GetDoubleOfTDouble(right);
                Jump(&leftIsDoubleAndRightIsDouble);
            }
        }
        Bind(&leftIsDoubleAndRightIsDouble);
        {
            Label rightNotZero(env);
            Label rightIsZeroOrNanOrLeftIsNanOrInf(env);
            Label rightNotZeroAndNanAndLeftNotNanAndInf(env);
            Branch(DoubleEqual(*doubleRight, Double(0.0)), &rightIsZeroOrNanOrLeftIsNanOrInf, &rightNotZero);
            Bind(&rightNotZero);
            {
                Label rightNotNan(env);
                Branch(DoubleIsNAN(*doubleRight), &rightIsZeroOrNanOrLeftIsNanOrInf, &rightNotNan);
                Bind(&rightNotNan);
                {
                    Label leftNotNan(env);
                    Branch(DoubleIsNAN(*doubleLeft), &rightIsZeroOrNanOrLeftIsNanOrInf, &leftNotNan);
                    Bind(&leftNotNan);
                    {
                        Branch(DoubleIsINF(*doubleLeft), &rightIsZeroOrNanOrLeftIsNanOrInf,
                            &rightNotZeroAndNanAndLeftNotNanAndInf);
                    }
                }
            }
            Bind(&rightIsZeroOrNanOrLeftIsNanOrInf);
            {
                result = DoubleToTaggedDoublePtr(Double(base::NAN_VALUE));
                Jump(&exit);
            }
            Bind(&rightNotZeroAndNanAndLeftNotNanAndInf);
            {
                Label leftNotZero(env);
                Label leftIsZeroOrRightIsInf(env);
                Branch(DoubleEqual(*doubleLeft, Double(0.0)), &leftIsZeroOrRightIsInf, &leftNotZero);
                Bind(&leftNotZero);
                {
                    Label rightNotInf(env);
                    Branch(DoubleIsINF(*doubleRight), &leftIsZeroOrRightIsInf, &rightNotInf);
                    Bind(&rightNotInf);
                    {
                        result = CallNGCRuntime(glue, RTSTUB_ID(FloatMod), { *doubleLeft, *doubleRight });
                        Jump(&exit);
                    }
                }
                Bind(&leftIsZeroOrRightIsInf);
                {
                    result = DoubleToTaggedDoublePtr(*doubleLeft);
                    Jump(&exit);
                }
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::GetGlobalOwnProperty(GateRef glue, GateRef receiver, GateRef key)
{
    auto env = GetEnvironment();
    Label entryLabel(env);
    env->SubCfgEntry(&entryLabel);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    GateRef properties = GetPropertiesFromJSObject(receiver);
    GateRef entry = FindEntryFromNameDictionary(glue, properties, key);
    Label notNegtiveOne(env);
    Label exit(env);
    Branch(Int32NotEqual(entry, Int32(-1)), &notNegtiveOne, &exit);
    Bind(&notNegtiveOne);
    {
        result = GetValueFromGlobalDictionary(properties, entry);
        Label callGetter(env);
        Branch(TaggedIsAccessor(*result), &callGetter, &exit);
        Bind(&callGetter);
        {
            result = CallGetterHelper(glue, receiver, receiver, *result);
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::GetStringFromConstPool(GateRef glue, GateRef constpool, GateRef index)
{
    GateRef module = Circuit::NullGate();
    return env_->GetBuilder()->GetObjectFromConstPool(glue, constpool, module, index, ConstPoolType::STRING);
}

GateRef StubBuilder::GetMethodFromConstPool(GateRef glue, GateRef constpool, GateRef index)
{
    GateRef module = Circuit::NullGate();
    return env_->GetBuilder()->GetObjectFromConstPool(glue, constpool, module, index, ConstPoolType::METHOD);
}

GateRef StubBuilder::GetArrayLiteralFromConstPool(GateRef glue, GateRef constpool, GateRef index, GateRef module)
{
    return env_->GetBuilder()->GetObjectFromConstPool(glue, constpool, module, index, ConstPoolType::ARRAY_LITERAL);
}

GateRef StubBuilder::GetObjectLiteralFromConstPool(GateRef glue, GateRef constpool, GateRef index, GateRef module)
{
    return env_->GetBuilder()->GetObjectFromConstPool(glue, constpool, module, index, ConstPoolType::OBJECT_LITERAL);
}

GateRef StubBuilder::JSAPIContainerGet(GateRef glue, GateRef receiver, GateRef index)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());

    GateRef lengthOffset = IntPtr(panda::ecmascript::JSAPIArrayList::LENGTH_OFFSET);
    GateRef length = GetInt32OfTInt(Load(VariableType::INT64(), receiver, lengthOffset));
    Label isVailedIndex(env);
    Label notValidIndex(env);
    Branch(BoolAnd(Int32GreaterThanOrEqual(index, Int32(0)),
        Int32UnsignedLessThan(index, length)), &isVailedIndex, &notValidIndex);
    Bind(&isVailedIndex);
    {
        GateRef elements = GetElementsArray(receiver);
        result = GetValueFromTaggedArray(elements, index);
        Jump(&exit);
    }
    Bind(&notValidIndex);
    {
        GateRef taggedId = Int32(GET_MESSAGE_STRING_ID(GetPropertyOutOfBounds));
        CallRuntime(glue, RTSTUB_ID(ThrowTypeError), { IntToTaggedInt(taggedId) });
        result = Exception();
        Jump(&exit);
    }

    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::DoubleToInt(GateRef glue, GateRef x)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label overflow(env);

    GateRef xInt = ChangeFloat64ToInt32(x);
    DEFVARIABLE(result, VariableType::INT32(), xInt);

    if (env->IsAmd64()) {
        // 0x80000000: amd64 overflow return value
        Branch(Int32Equal(xInt, Int32(0x80000000)), &overflow, &exit);
    } else {
        GateRef xInt64 = CastDoubleToInt64(x);
        // exp = (u64 & DOUBLE_EXPONENT_MASK) >> DOUBLE_SIGNIFICAND_SIZE - DOUBLE_EXPONENT_BIAS
        GateRef exp = Int64And(xInt64, Int64(base::DOUBLE_EXPONENT_MASK));
        exp = TruncInt64ToInt32(Int64LSR(exp, Int64(base::DOUBLE_SIGNIFICAND_SIZE)));
        exp = Int32Sub(exp, Int32(base::DOUBLE_EXPONENT_BIAS));
        GateRef bits = Int32(base::INT32_BITS - 1);
        // exp < 32 - 1
        Branch(Int32LessThan(exp, bits), &exit, &overflow);
    }
    Bind(&overflow);
    {
        result = CallNGCRuntime(glue, RTSTUB_ID(DoubleToInt), { x });
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

void StubBuilder::ReturnExceptionIfAbruptCompletion(GateRef glue)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    Label hasPendingException(env);
    GateRef exceptionOffset = IntPtr(JSThread::GlueData::GetExceptionOffset(env->IsArch32Bit()));
    GateRef exception = Load(VariableType::JS_ANY(), glue, exceptionOffset);
    Branch(TaggedIsNotHole(exception), &hasPendingException, &exit);
    Bind(&hasPendingException);
    Return(Exception());
    Bind(&exit);
    env->SubCfgExit();
    return;
}

GateRef StubBuilder::GetHashcodeFromString(GateRef glue, GateRef value)
{
    auto env = GetEnvironment();
    Label subentry(env);
    env->SubCfgEntry(&subentry);
    Label noRawHashcode(env);
    Label exit(env);
    DEFVARIABLE(hashcode, VariableType::INT32(), Int32(0));
    hashcode = Load(VariableType::INT32(), value, IntPtr(EcmaString::HASHCODE_OFFSET));
    Branch(Int32Equal(*hashcode, Int32(0)), &noRawHashcode, &exit);
    Bind(&noRawHashcode);
    {
        hashcode = GetInt32OfTInt(CallRuntime(glue, RTSTUB_ID(ComputeHashcode), { value }));
        Store(VariableType::INT32(), glue, value, IntPtr(EcmaString::HASHCODE_OFFSET), *hashcode);
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *hashcode;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::ConstructorCheck(GateRef glue, GateRef ctor, GateRef outPut, GateRef thisObj)
{
    auto env = GetEnvironment();
    Label entryPass(env);
    Label exit(env);
    env->SubCfgEntry(&entryPass);
    DEFVARIABLE(result, VariableType::JS_ANY(), Exception());
    Label isHeapObject(env);
    Label isEcmaObj(env);
    Label notEcmaObj(env);
    Branch(TaggedIsHeapObject(outPut), &isHeapObject, &notEcmaObj);
    Bind(&isHeapObject);
    Branch(TaggedObjectIsEcmaObject(outPut), &isEcmaObj, &notEcmaObj);
    Bind(&isEcmaObj);
    {
        result = outPut;
        Jump(&exit);
    }
    Bind(&notEcmaObj);
    {
        Label ctorIsBase(env);
        Label ctorNotBase(env);
        Branch(IsBase(ctor), &ctorIsBase, &ctorNotBase);
        Bind(&ctorIsBase);
        {
            result = thisObj;
            Jump(&exit);
        }
        Bind(&ctorNotBase);
        {
            Label throwExeption(env);
            Label returnObj(env);
            Branch(TaggedIsUndefined(outPut), &returnObj, &throwExeption);
            Bind(&returnObj);
            result = thisObj;
            Jump(&exit);
            Bind(&throwExeption);
            {
                CallRuntime(glue, RTSTUB_ID(ThrowNonConstructorException), {});
                Jump(&exit);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::JSCallDispatch(GateRef glue, GateRef func, GateRef actualNumArgs, GateRef jumpSize,
                                    GateRef hotnessCounter, JSCallMode mode, std::initializer_list<GateRef> args)
{
    auto env = GetEnvironment();
    Label entryPass(env);
    Label exit(env);
    env->SubCfgEntry(&entryPass);
    DEFVARIABLE(result, VariableType::JS_ANY(), Exception());
    // 1. call initialize
    Label funcIsHeapObject(env);
    Label funcIsCallable(env);
    Label funcNotCallable(env);
    // save pc
    SavePcIfNeeded(glue);
    GateRef bitfield = 0;
    if (!AssemblerModule::IsCallNew(mode)) {
        Branch(TaggedIsHeapObject(func), &funcIsHeapObject, &funcNotCallable);
        Bind(&funcIsHeapObject);
        GateRef hclass = LoadHClass(func);
        bitfield = Load(VariableType::INT32(), hclass, IntPtr(JSHClass::BIT_FIELD_OFFSET));
        Branch(IsCallableFromBitField(bitfield), &funcIsCallable, &funcNotCallable);
        Bind(&funcNotCallable);
        {
            CallRuntime(glue, RTSTUB_ID(ThrowNotCallableException), {});
            Jump(&exit);
        }
        Bind(&funcIsCallable);
    }
    GateRef method = GetMethodFromJSFunction(func);
    GateRef callField = GetCallFieldFromMethod(method);
    GateRef isNativeMask = Int64(static_cast<uint64_t>(1) << MethodLiteral::IsNativeBit::START_BIT);

    // 2. call dispatch
    Label methodIsNative(env);
    Label methodNotNative(env);
    Branch(Int64NotEqual(Int64And(callField, isNativeMask), Int64(0)), &methodIsNative, &methodNotNative);
    auto data = std::begin(args);
    Label notFastBuiltinsArg0(env);
    Label notFastBuiltinsArg1(env);
    Label notFastBuiltinsArg2(env);
    Label notFastBuiltinsArg3(env);
    Label notFastBuiltins(env);
    // 3. call native
    Bind(&methodIsNative);
    {
        GateRef nativeCode = Load(VariableType::NATIVE_POINTER(), method,
            IntPtr(Method::NATIVE_POINTER_OR_BYTECODE_ARRAY_OFFSET));
        GateRef newTarget = Undefined();
        GateRef thisValue = Undefined();
        GateRef numArgs = Int32Add(actualNumArgs, Int32(NUM_MANDATORY_JSFUNC_ARGS));
        switch (mode) {
            case JSCallMode::CALL_THIS_ARG0: {
                thisValue = data[0];
                CallFastPath(glue, nativeCode, func, thisValue, actualNumArgs, callField,
                    method, &notFastBuiltinsArg0, &exit, &result, args, mode);
                Bind(&notFastBuiltinsArg0);
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgsAndDispatchNative),
                    { nativeCode, glue, numArgs, func, newTarget, thisValue });
                break;
            }
            case JSCallMode::CALL_ARG0:
            case JSCallMode::DEPRECATED_CALL_ARG0:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgsAndDispatchNative),
                    { nativeCode, glue, numArgs, func, newTarget, thisValue });
                break;
            case JSCallMode::CALL_THIS_ARG1: {
                thisValue = data[1];
                CallFastPath(glue, nativeCode, func, thisValue, actualNumArgs, callField,
                    method, &notFastBuiltinsArg1, &exit, &result, args, mode);
                Bind(&notFastBuiltinsArg1);
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgsAndDispatchNative),
                    { nativeCode, glue, numArgs, func, newTarget, thisValue, data[0]});
                break;
            }
            case JSCallMode::CALL_ARG1:
            case JSCallMode::DEPRECATED_CALL_ARG1:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgsAndDispatchNative),
                    { nativeCode, glue, numArgs, func, newTarget, thisValue, data[0]});
                break;
            case JSCallMode::CALL_THIS_ARG2: {
                thisValue = data[2];
                CallFastPath(glue, nativeCode, func, thisValue, actualNumArgs, callField,
                    method, &notFastBuiltinsArg2, &exit, &result, args, mode);
                Bind(&notFastBuiltinsArg2);
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgsAndDispatchNative),
                    { nativeCode, glue, numArgs, func, newTarget, thisValue, data[0], data[1] });
                break;
            }
            case JSCallMode::CALL_ARG2:
            case JSCallMode::DEPRECATED_CALL_ARG2:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgsAndDispatchNative),
                    { nativeCode, glue, numArgs, func, newTarget, thisValue, data[0], data[1] });
                break;
            case JSCallMode::CALL_THIS_ARG3: {
                thisValue = data[3];
                CallFastPath(glue, nativeCode, func, thisValue, actualNumArgs, callField,
                    method, &notFastBuiltinsArg3, &exit, &result, args, mode);
                Bind(&notFastBuiltinsArg3);
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgsAndDispatchNative),
                    { nativeCode, glue, numArgs, func,
                        newTarget, thisValue, data[0], data[1], data[2] }); // 2: args2
                break;
            }
            case JSCallMode::CALL_ARG3:
            case JSCallMode::DEPRECATED_CALL_ARG3:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgsAndDispatchNative),
                    { nativeCode, glue, numArgs, func,
                      newTarget, thisValue, data[0], data[1], data[2] }); // 2: args2
                break;
            case JSCallMode::CALL_THIS_WITH_ARGV:
            case JSCallMode::DEPRECATED_CALL_THIS_WITH_ARGV: {
                thisValue = data[2]; // 2: this input
                [[fallthrough]];
            }
            case JSCallMode::CALL_WITH_ARGV:
            case JSCallMode::DEPRECATED_CALL_WITH_ARGV:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallRangeAndDispatchNative),
                    { glue, nativeCode, func, thisValue, data[0], data[1] });
                break;
            case JSCallMode::DEPRECATED_CALL_CONSTRUCTOR_WITH_ARGV:
            case JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV: {
                CallFastPath(glue, nativeCode, func, thisValue, actualNumArgs, callField,
                    method, &notFastBuiltins, &exit, &result, args, mode);
                Bind(&notFastBuiltins);
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallNewAndDispatchNative),
                    { glue, nativeCode, func, data[2], data[0], data[1] });
                break;
            }
            case JSCallMode::CALL_GETTER:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgsAndDispatchNative),
                    { nativeCode, glue, numArgs, func, newTarget, data[0] });
                break;
            case JSCallMode::CALL_SETTER:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgsAndDispatchNative),
                    { nativeCode, glue, numArgs, func, newTarget, data[0], data[1] });
                break;
            case JSCallMode::CALL_THIS_ARG3_WITH_RETURN:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgsAndDispatchNative),
                    { nativeCode, glue, numArgs, func, newTarget, data[0], data[1], data[2], data[3] });
                break;
            default:
                UNREACHABLE();
        }
        Jump(&exit);
    }
    // 4. call nonNative
    Bind(&methodNotNative);
    Label funcIsClassConstructor(env);
    Label funcNotClassConstructor(env);
    PGOProfiler(glue, func);
    if (!AssemblerModule::IsCallNew(mode)) {
        Branch(IsClassConstructorFromBitField(bitfield), &funcIsClassConstructor, &funcNotClassConstructor);
        Bind(&funcIsClassConstructor);
        {
            CallRuntime(glue, RTSTUB_ID(ThrowCallConstructorException), {});
            Jump(&exit);
        }
        Bind(&funcNotClassConstructor);
    }
    GateRef sp = 0;
    if (env->IsAsmInterp()) {
        sp = Argument(static_cast<size_t>(InterpreterHandlerInputs::SP));
    }
    Label methodisAot(env);
    Label methodNotAot(env);
    {
        GateRef isAotMask = Int64(static_cast<uint64_t>(1) << MethodLiteral::IsAotCodeBit::START_BIT);
        Branch(Int64Equal(Int64And(callField, isAotMask), Int64(0)),  &methodNotAot, &methodisAot);
        Bind(&methodisAot);
        {
            GateRef newTarget = Undefined();
            GateRef thisValue = Undefined();
            GateRef lexEnv = env_->GetBuilder()->GetFunctionLexicalEnv(func);
            GateRef realNumArgs = Int64Add(ZExtInt32ToInt64(actualNumArgs), Int64(NUM_MANDATORY_JSFUNC_ARGS));
            switch (mode) {
                case JSCallMode::CALL_THIS_ARG0:
                    thisValue = data[0];
                    [[fallthrough]];
                case JSCallMode::CALL_ARG0:
                case JSCallMode::DEPRECATED_CALL_ARG0:
                    result = CallNGCRuntime(glue, RTSTUB_ID(JSCall),
                        { glue, lexEnv, realNumArgs, func, newTarget, thisValue});
                    Jump(&exit);
                    break;
                case JSCallMode::CALL_THIS_ARG1:
                    thisValue = data[1];
                    [[fallthrough]];
                case JSCallMode::CALL_ARG1:
                case JSCallMode::DEPRECATED_CALL_ARG1:
                    result = CallNGCRuntime(glue, RTSTUB_ID(JSCall),
                        { glue, lexEnv, realNumArgs, func, newTarget, thisValue,  data[0] });
                    Jump(&exit);
                    break;
                case JSCallMode::CALL_THIS_ARG2:
                    thisValue = data[2];
                    [[fallthrough]];
                case JSCallMode::CALL_ARG2:
                case JSCallMode::DEPRECATED_CALL_ARG2:
                    result = CallNGCRuntime(glue, RTSTUB_ID(JSCall),
                        { glue, lexEnv, realNumArgs, func, newTarget, thisValue,  data[0], data[1] });
                    Jump(&exit);
                    break;
                case JSCallMode::CALL_THIS_ARG3:
                    thisValue = data[3];
                    [[fallthrough]];
                case JSCallMode::CALL_ARG3:
                case JSCallMode::DEPRECATED_CALL_ARG3:
                    result = CallNGCRuntime(glue, RTSTUB_ID(JSCall),
                        { glue, lexEnv, realNumArgs, func, newTarget, thisValue,
                          data[0], data[1], data[2] }); // 2: args2
                    Jump(&exit);
                    break;
                case JSCallMode::CALL_THIS_WITH_ARGV:
                case JSCallMode::DEPRECATED_CALL_THIS_WITH_ARGV:
                    thisValue = data[2]; // 2: this input
                    [[fallthrough]];
                case JSCallMode::CALL_WITH_ARGV:
                case JSCallMode::DEPRECATED_CALL_WITH_ARGV:
                    result = CallNGCRuntime(glue, RTSTUB_ID(JSCallWithArgV),
                        { glue, ZExtInt32ToInt64(actualNumArgs), func, newTarget, thisValue, data[1] });
                    Jump(&exit);
                    break;
                case JSCallMode::DEPRECATED_CALL_CONSTRUCTOR_WITH_ARGV:
                case JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV:
                    result = CallNGCRuntime(glue, RTSTUB_ID(ConstructorJSCallWithArgV),
                        { glue, ZExtInt32ToInt64(actualNumArgs), func, func, data[2], data[1]});
                    result = ConstructorCheck(glue, func, *result, data[2]);  // 2: the second index
                    Jump(&exit);
                    break;
                case JSCallMode::CALL_GETTER:
                    result = CallNGCRuntime(glue, RTSTUB_ID(JSCall),
                        { glue, lexEnv, realNumArgs, func, newTarget, data[0]});
                    Jump(&exit);
                    break;
                case JSCallMode::CALL_SETTER:
                    result = CallNGCRuntime(glue, RTSTUB_ID(JSCall),
                        { glue, lexEnv, realNumArgs, func, newTarget, data[0], data[1]});
                    Jump(&exit);
                    break;
                case JSCallMode::CALL_THIS_ARG3_WITH_RETURN:
                    result = CallNGCRuntime(glue, RTSTUB_ID(JSCall),
                        { glue, lexEnv, realNumArgs, func, newTarget, data[0], data[1], data[2], data[3] });
                    Jump(&exit);
                    break;
                default:
                    UNREACHABLE();
            }
        }
        Bind(&methodNotAot);
        if (jumpSize != 0) {
            SaveJumpSizeIfNeeded(glue, jumpSize);
        }
        SaveHotnessCounterIfNeeded(glue, sp, hotnessCounter, mode);
        switch (mode) {
            case JSCallMode::CALL_THIS_ARG0:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallThisArg0AndDispatch),
                    { glue, sp, func, method, callField, data[0] });
                Return();
                break;
            case JSCallMode::CALL_ARG0:
            case JSCallMode::DEPRECATED_CALL_ARG0:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArg0AndDispatch),
                    { glue, sp, func, method, callField });
                Return();
                break;
            case JSCallMode::CALL_THIS_ARG1:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallThisArg1AndDispatch),
                    { glue, sp, func, method, callField, data[0], data[1] });
                Return();
                break;
            case JSCallMode::CALL_ARG1:
            case JSCallMode::DEPRECATED_CALL_ARG1:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArg1AndDispatch),
                    { glue, sp, func, method, callField, data[0] });
                Return();
                break;
            case JSCallMode::CALL_THIS_ARG2:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallThisArgs2AndDispatch),
                    { glue, sp, func, method, callField, data[0], data[1], data[2] });
                Return();
                break;
            case JSCallMode::CALL_ARG2:
            case JSCallMode::DEPRECATED_CALL_ARG2:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgs2AndDispatch),
                    { glue, sp, func, method, callField, data[0], data[1] });
                Return();
                break;
            case JSCallMode::CALL_THIS_ARG3:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallThisArgs3AndDispatch),
                    { glue, sp, func, method, callField, data[0], data[1], data[2], data[3] });
                Return();
                break;
            case JSCallMode::CALL_ARG3:
            case JSCallMode::DEPRECATED_CALL_ARG3:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallArgs3AndDispatch),
                    { glue, sp, func, method, callField, data[0], data[1], data[2] }); // 2: args2
                Return();
                break;
            case JSCallMode::CALL_WITH_ARGV:
            case JSCallMode::DEPRECATED_CALL_WITH_ARGV:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallRangeAndDispatch),
                    { glue, sp, func, method, callField, data[0], data[1] });
                Return();
                break;
            case JSCallMode::CALL_THIS_WITH_ARGV:
            case JSCallMode::DEPRECATED_CALL_THIS_WITH_ARGV:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallThisRangeAndDispatch),
                    { glue, sp, func, method, callField, data[0], data[1], data[2] });
                Return();
                break;
            case JSCallMode::DEPRECATED_CALL_CONSTRUCTOR_WITH_ARGV:
            case JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV:
                result = CallNGCRuntime(glue, RTSTUB_ID(PushCallNewAndDispatch),
                    { glue, sp, func, method, callField, data[0], data[1], data[2] });
                Return();
                break;
            case JSCallMode::CALL_GETTER:
                result = CallNGCRuntime(glue, RTSTUB_ID(CallGetter),
                    { glue, func, method, callField, data[0] });
                Jump(&exit);
                break;
            case JSCallMode::CALL_SETTER:
                result = CallNGCRuntime(glue, RTSTUB_ID(CallSetter),
                    { glue, func, method, callField, data[1], data[0] });
                Jump(&exit);
                break;
            case JSCallMode::CALL_THIS_ARG3_WITH_RETURN:
                result = CallNGCRuntime(glue, RTSTUB_ID(CallContainersArgs3),
                    { glue, func, method, callField, data[1], data[2], data[3], data[0] });
                Jump(&exit);
                break;
            default:
                UNREACHABLE();
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

void StubBuilder::CallFastPath(GateRef glue, GateRef nativeCode, GateRef func, GateRef thisValue,
    GateRef actualNumArgs, GateRef callField, GateRef method, Label* notFastBuiltins, Label* exit, Variable* result,
    std::initializer_list<GateRef> args, JSCallMode mode)
{
    auto env = GetEnvironment();
    Label isFastBuiltins(env);
    auto data = std::begin(args);
    GateRef numArgs = ZExtInt32ToPtr(actualNumArgs);
    GateRef isFastBuiltinsMask = Int64(static_cast<uint64_t>(1) << MethodLiteral::IsFastBuiltinBit::START_BIT);
    Branch(Int64NotEqual(Int64And(callField, isFastBuiltinsMask), Int64(0)),
        &isFastBuiltins, notFastBuiltins);
    Bind(&isFastBuiltins);
    {
        GateRef builtinId = GetBuiltinId(method);
        GateRef ret;
        switch (mode) {
            case JSCallMode::CALL_THIS_ARG0:
                ret = DispatchBuiltins(glue, builtinId, { glue, nativeCode, func, Undefined(), thisValue, numArgs });
                break;
            case JSCallMode::CALL_THIS_ARG1:
                ret = DispatchBuiltins(glue, builtinId, { glue, nativeCode, func, Undefined(),
                                                          thisValue, numArgs, data[0] });
                break;
            case JSCallMode::CALL_THIS_ARG2:
                ret = DispatchBuiltins(glue, builtinId, { glue, nativeCode, func, Undefined(), thisValue,
                                                          numArgs, data[0], data[1] });
                break;
            case JSCallMode::CALL_THIS_ARG3:
                ret = DispatchBuiltins(glue, builtinId, { glue, nativeCode, func, Undefined(), thisValue,
                                                          numArgs, data[0], data[1], data[2] });
                break;
            case JSCallMode::DEPRECATED_CALL_CONSTRUCTOR_WITH_ARGV:
            case JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV:
                ret = DispatchBuiltinsWithArgv(glue, builtinId, { glue, nativeCode, func, func, thisValue,
                                                                  numArgs, data[1] });
                break;
            default:
                UNREACHABLE();
        }
        result->WriteVariable(ret);
        Jump(exit);
    }
    Bind(notFastBuiltins);
}

GateRef StubBuilder::TryStringOrSymbelToElementIndex(GateRef key)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::INT32(), Int32(-1));

    Label keyNotSymbol(env);
    Branch(IsSymbol(key), &exit, &keyNotSymbol);
    Bind(&keyNotSymbol);

    Label greatThanZero(env);
    Label inRange(env);
    auto len = GetLengthFromString(key);
    Branch(Int32Equal(len, Int32(0)), &exit, &greatThanZero);
    Bind(&greatThanZero);
    Branch(Int32GreaterThan(len, Int32(MAX_ELEMENT_INDEX_LEN)), &exit, &inRange);
    Bind(&inRange);
    {
        Label isUtf8(env);
        Branch(IsUtf16String(key), &exit, &isUtf8);
        Bind(&isUtf8);

        GateRef data = PtrAdd(key, IntPtr(EcmaString::DATA_OFFSET));
        DEFVARIABLE(c, VariableType::INT32(), Int32(0));
        c = ZExtInt8ToInt32(Load(VariableType::INT8(), data));
        Label isDigitZero(env);
        Label notDigitZero(env);
        Branch(Int32Equal(*c, Int32('0')), &isDigitZero, &notDigitZero);
        Bind(&isDigitZero);
        {
            Label lengthIsOne(env);
            Branch(Int32Equal(len, Int32(1)), &lengthIsOne, &exit);
            Bind(&lengthIsOne);
            {
                result = Int32(0);
                Jump(&exit);
            }
        }
        Bind(&notDigitZero);
        {
            Label isDigit(env);
            Label notIsDigit(env);
            DEFVARIABLE(i, VariableType::INT32(), Int32(1));
            DEFVARIABLE(n, VariableType::INT32(), Int32Sub(*c, Int32('0')));

            Branch(IsDigit(*c), &isDigit, &notIsDigit);
            Label loopHead(env);
            Label loopEnd(env);
            Label afterLoop(env);
            Bind(&isDigit);
            Branch(Int32UnsignedLessThan(*i, len), &loopHead, &afterLoop);
            LoopBegin(&loopHead);
            {
                c = ZExtInt8ToInt32(Load(VariableType::INT8(), data, ZExtInt32ToPtr(*i)));
                Label isDigit2(env);
                Label notDigit2(env);
                Branch(IsDigit(*c), &isDigit2, &notDigit2);
                Bind(&isDigit2);
                {
                    // 10 means the base of digit is 10.
                    n = Int32Add(Int32Mul(*n, Int32(10)),
                                 Int32Sub(*c, Int32('0')));
                    i = Int32Add(*i, Int32(1));
                    Branch(Int32UnsignedLessThan(*i, len), &loopEnd, &afterLoop);
                }
                Bind(&notDigit2);
                {
                    Label hasPoint(env);
                    Branch(Int32Equal(*c, Int32('.')), &hasPoint, &exit);
                    Bind(&hasPoint);
                    {
                        result = Int32(-2); // -2:return -2 means should goto slow path
                        Jump(&exit);
                    }
                }
            }
            Bind(&loopEnd);
            LoopEnd(&loopHead);
            Bind(&afterLoop);
            {
                Label lessThanMaxIndex(env);
                Branch(Int32UnsignedLessThan(*n, Int32(JSObject::MAX_ELEMENT_INDEX)),
                       &lessThanMaxIndex, &exit);
                Bind(&lessThanMaxIndex);
                {
                    result = *n;
                    Jump(&exit);
                }
            }
            Bind(&notIsDigit);
            {
                Label isNegative(env);
                Branch(Int32Equal(*c, Int32('-')), &isNegative, &exit);
                Bind(&isNegative);
                {
                    result = Int32(-2); // -2:return -2 means should goto slow path
                    Jump(&exit);
                }
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::GetTypeArrayPropertyByName(GateRef glue, GateRef receiver, GateRef holder,
                                                GateRef key, GateRef jsType)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());

    Label notOnProtoChain(env);
    Branch(Int64NotEqual(receiver, holder), &exit, &notOnProtoChain);
    Bind(&notOnProtoChain);

    auto negativeZero = GetGlobalConstantValue(
        VariableType::JS_POINTER(), glue, ConstantIndex::NEGATIVE_ZERO_STRING_INDEX);
    Label isNegativeZero(env);
    Label notNegativeZero(env);
    Branch(Equal(negativeZero, key), &isNegativeZero, &notNegativeZero);
    Bind(&isNegativeZero);
    {
        result = Undefined();
        Jump(&exit);
    }
    Bind(&notNegativeZero);
    {
        GateRef index = TryStringOrSymbelToElementIndex(key);
        Label validIndex(env);
        Label notValidIndex(env);
        Branch(Int32GreaterThanOrEqual(index, Int32(0)), &validIndex, &notValidIndex);
        Bind(&validIndex);
        {
            result = CallRuntime(glue, RTSTUB_ID(GetTypeArrayPropertyByIndex),
                                 { holder, IntToTaggedInt(index), IntToTaggedInt(jsType) });
            Jump(&exit);
        }
        Bind(&notValidIndex);
        {
            Label returnNull(env);
            Branch(Int32Equal(index, Int32(-2)), &returnNull, &exit); // -2:equal -2 means should goto slow path
            Bind(&returnNull);
            {
                result = Null();
                Jump(&exit);
            }
        }
    }

    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

GateRef StubBuilder::SetTypeArrayPropertyByName(GateRef glue, GateRef receiver, GateRef holder, GateRef key,
                                                GateRef value, GateRef jsType)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->SubCfgEntry(&entry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Label notOnProtoChain(env);
    Branch(Int64NotEqual(receiver, holder), &exit, &notOnProtoChain);
    Bind(&notOnProtoChain);

    auto negativeZero = GetGlobalConstantValue(
        VariableType::JS_POINTER(), glue, ConstantIndex::NEGATIVE_ZERO_STRING_INDEX);
    Label isNegativeZero(env);
    Label notNegativeZero(env);
    Branch(Equal(negativeZero, key), &isNegativeZero, &notNegativeZero);
    Bind(&isNegativeZero);
    {
        Label isObj(env);
        Label notObj(env);
        Branch(IsEcmaObject(value), &isObj, &notObj);
        Bind(&isObj);
        {
            result = Null();
            Jump(&exit);
        }
        Bind(&notObj);
        result = Undefined();
        Jump(&exit);
    }
    Bind(&notNegativeZero);
    {
        GateRef index = TryStringOrSymbelToElementIndex(key);
        Label validIndex(env);
        Label notValidIndex(env);
        Branch(Int32GreaterThanOrEqual(index, Int32(0)), &validIndex, &notValidIndex);
        Bind(&validIndex);
        {
            result = CallRuntime(glue, RTSTUB_ID(SetTypeArrayPropertyByIndex),
                { receiver, IntToTaggedInt(index), value, IntToTaggedInt(jsType) });
            Jump(&exit);
        }
        Bind(&notValidIndex);
        {
            Label returnNull(env);
            Branch(Int32Equal(index, Int32(-2)), &returnNull, &exit); // -2:equal -2 means should goto slow path
            Bind(&returnNull);
            {
                result = Null();
                Jump(&exit);
            }
        }
    }

    Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

void StubBuilder::PGOProfiler(GateRef glue, GateRef func)
{
    auto env = GetEnvironment();
    if (env->IsAsmInterp()) {
        Label subEntry(env);
        env->SubCfgEntry(&subEntry);

        // Decode bitfield
        GateRef stateBitFieldAddr = Int64Add(glue,
                                             Int64(JSThread::GlueData::GetStateBitFieldOffset(env->Is32Bit())));
        GateRef stateBitField = Load(VariableType::INT64(), stateBitFieldAddr, Int64(0));
        // equal stateBitField >> JSThread::CONCURRENT_MARKING_BITFIELD_NUM
        GateRef profilerBitOffset = Int64LSR(stateBitField, Int64(JSThread::CONCURRENT_MARKING_BITFIELD_NUM));
        // mask: 1 << JSThread::PGO_PROFILER_BITFIELD_NUM - 1
        GateRef profilerBitMask = Int64Sub(Int64LSL(Int64(1), Int64(JSThread::PGO_PROFILER_BITFIELD_NUM)), Int64(1));
        GateRef state = Int64And(profilerBitOffset, profilerBitMask);
        Label exit(env);
        Label pgoProfiler(env);
        Branch(Int64Equal(state, Int64(static_cast<int64_t>(PGOProfilerStatus::PGO_PROFILER_ENABLE))),
            &pgoProfiler, &exit);
        Bind(&pgoProfiler);
        {
            CallNGCRuntime(glue, RTSTUB_ID(PGOProfiler), { glue, func });
            Jump(&exit);
        }
        Bind(&exit);
        env->SubCfgExit();
    }
}

void StubBuilder::Assert(int messageId, int line, GateRef glue, GateRef condition, Label *nextLabel)
{
    auto env = GetEnvironment();
    Label ok(env);
    Label notOk(env);
    Branch(condition, &ok, &notOk);
    Bind(&ok);
    {
        Jump(nextLabel);
    }
    Bind(&notOk);
    {
        FatalPrint(glue, { Int32(messageId), Int32(line) });
        Jump(nextLabel);
    }
}
}  // namespace panda::ecmascript::kungfu
