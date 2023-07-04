/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ECMASCRIPT_RUNTIME_CALL_ID_H
#define ECMASCRIPT_RUNTIME_CALL_ID_H

#include "ecmascript/base/config.h"
#include "ecmascript/dfx/vmstat/runtime_stat.h"
#include "ecmascript/stubs/runtime_stubs.h"

namespace panda::ecmascript {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTERPRETER_CALLER_LIST(V)  \
    V(RunInternal)                  \
    V(Ldnan)                        \
    V(Ldinfinity)                   \
    V(Ldundefined)                  \
    V(Ldboolean)                    \
    V(Ldnumber)                     \
    V(Ldstring)                     \
    V(Ldnull)                       \
    V(Ldsymbol)                     \
    V(Ldfunction)                   \
    V(Ldglobal)                     \
    V(Ldtrue)                       \
    V(Ldfalse)                      \
    V(Tonumber)                     \
    V(Toboolean)                    \
    V(Add2)                      \
    V(Sub2)                      \
    V(Mul2)                      \
    V(Div2)                      \
    V(Mod2)                      \
    V(Eq)                        \
    V(NotEq)                     \
    V(Less)                      \
    V(LessEq)                    \
    V(Greater)                   \
    V(GreaterEq)                 \
    V(StrictNotEq)               \
    V(StrictEq)                  \
    V(Shl2)                      \
    V(Shr2)                      \
    V(Ashr2)                     \
    V(And2)                      \
    V(Or2)                       \
    V(Xor2)                      \
    V(Neg)                       \
    V(Not)                       \
    V(Inc)                       \
    V(Dec)                       \
    V(Exp)                       \
    V(Throw)                     \
    V(LdObjByIndex)              \
    V(StObjByIndex)              \
    V(LdObjByName)               \
    V(StObjByName)               \
    V(LdObjByValue)              \
    V(StObjByValue)              \
    V(StOwnByName)               \
    V(StOwnById)                 \
    V(StOwnByValue)              \
    V(Trygetobjprop)                \
    V(Delobjprop)                   \
    V(Defineglobalvar)              \
    V(Definelocalvar)               \
    V(Definefuncexpr)               \
    V(DefineFunc)                \
    V(NewobjRange)               \
    V(Refeq)                     \
    V(Typeof)                    \
    V(Ldnewobjrange)             \
    V(IsIn)                      \
    V(Instanceof)                \
    V(NewObjApply)               \
    V(CallArg0)                  \
    V(CallArg1)                  \
    V(CallArg2)                  \
    V(CallArg3)                  \
    V(CallThisRange)             \
    V(CallRange)                 \
    V(CallSpread)                \
    V(Newlexenv)                 \
    V(NewlexenvwithName)         \
    V(Stlexvar)                  \
    V(Ldlexvar)                  \
    V(Ldlexenv)                  \
    V(GetPropIterator)              \
    V(CreateIterResultObj)          \
    V(SuspendGenerator)             \
    V(ResumeGenerator)              \
    V(GetResumeMode)                \
    V(CreateGeneratorObj)           \
    V(DefineGetterSetterByValue)    \
    V(AsyncFunctionEnter)           \
    V(AsyncFunctionAwaitUncaught)   \
    V(AsyncFunctionResolveOrReject) \
    V(ThrowUndefined)               \
    V(ThrowConstAssignment)         \
    V(ThrowUndefinedIfHole)         \
    V(Copyrestargs)                 \
    V(Trystobjprop)                 \
    V(GetTemplateObject)            \
    V(GetIterator)                  \
    V(GetAsyncIterator)             \
    V(ThrowIfNotObject)             \
    V(ThrowThrowNotExists)          \
    V(CreateObjectWithExcludedKeys) \
    V(ThrowPatternNonCoercible)     \
    V(IterNext)                     \
    V(CloseIterator)                \
    V(StArraySpread)                \
    V(GetCallSpreadArgs)            \
    V(TryLoadICByName)              \
    V(LoadICByName)                 \
    V(GetPropertyByName)            \
    V(TryLoadICByValue)             \
    V(LoadICByValue)                \
    V(TryStoreICByName)             \
    V(StoreICByName)                \
    V(TryStoreICByValue)            \
    V(StoreICByValue)               \
    V(NotifyInlineCache)            \
    V(LoadGlobalICByName)           \
    V(StoreGlobalICByName)          \
    V(StoreICWithHandler)           \
    V(StorePrototype)               \
    V(StoreWithTransition)          \
    V(StoreTransWithProto)          \
    V(StoreWithAOT)                 \
    V(StoreField)                   \
    V(StoreGlobal)                  \
    V(LoadPrototype)                \
    V(LoadICWithHandler)            \
    V(StoreElement)                 \
    V(CallGetter)                   \
    V(CallSetter)                   \
    V(AddPropertyByName)            \
    V(AddPropertyByIndex)           \
    V(GetPropertyByIndex)           \
    V(GetPropertyByValue)           \
    V(SetPropertyByIndex)           \
    V(SetPropertyByValue)           \
    V(FastTypeOf)                   \
    V(FastSetPropertyByIndex)       \
    V(FastSetPropertyByValue)       \
    V(FastGetPropertyByName)        \
    V(FastGetPropertyByValue)       \
    V(FastGetPropertyByIndex)       \
    V(NewLexicalEnv)                \
    V(ExecuteNative)                \
    V(Execute)                      \
    V(AsmExecute)                   \
    V(ExecuteAot)                   \
    V(ToJSTaggedValueWithInt32)     \
    V(ToJSTaggedValueWithUint32)    \
    V(ThrowIfSuperNotCorrectCall)   \
    V(CreateEmptyArray)             \
    V(CreateEmptyObject)            \
    V(CreateObjectWithBuffer)       \
    V(CreateObjectHavingMethod)     \
    V(SetObjectWithProto)           \
    V(getmodulenamespace)           \
    V(StModuleVar)                  \
    V(LdModuleVar)                  \
    V(CreateRegExpWithLiteral)      \
    V(CreateArrayWithBuffer)        \
    V(GetNextPropName)              \
    V(CopyDataProperties)           \
    V(GetUnmapedArgs)               \
    V(TryStGlobalByName)            \
    V(LdGlobalVar)                  \
    V(StGlobalVar)                  \
    V(TryUpdateGlobalRecord)        \
    V(LdGlobalRecord)               \
    V(StGlobalRecord)               \
    V(ThrowReferenceError)          \
    V(ThrowTypeError)               \
    V(ThrowSyntaxError)             \
    V(NewClassFunc)                 \
    V(DefineClass)                  \
    V(SuperCall)                    \
    V(SuperCallSpread)              \
    V(DefineMethod)                 \
    V(LdSuperByValue)               \
    V(StSuperByValue)               \
    V(ThrowDeleteSuperProperty)     \
    V(ModWithTSType)                \
    V(MulWithTSType)                \
    V(SubWithTSType)                \
    V(DivWithTSType)                \
    V(AddWithTSType)                \
    V(GetBitOPDate)                 \
    V(ShlWithTSType)                \
    V(ShrWithTSType)                \
    V(AshrWithTSType)               \
    V(AndWithTSType)                \
    V(OrWithTSType)                 \
    V(XorWithTSType)                \
    V(EqualWithIC)                  \
    V(NotEqualWithIC)               \
    V(Compare)                      \
    V(LessWithIC)                   \
    V(LessEqWithIC)                 \
    V(GreaterWithIC)                \
    V(SetPropertyByName)            \
    V(GreaterEqWithIC)              \
    V(LdBigInt)                     \
    V(Tonumeric)                    \
    V(CreateAsyncGeneratorObj)      \
    V(AsyncGeneratorResolve)        \
    V(GetSuperConstructor)          \
    V(DynamicImport)                \
    V(LdPatchVar)                   \
    V(StPatchVar)                   \
    V(AsyncGeneratorReject)         \
    V(NotifyConcurrentResult)       \
    V(SetGeneratorState)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BUILTINS_API_LIST(V)                   \
    V(Array, Constructor)                     \
    V(Array, From)                            \
    V(Array, Of)                              \
    V(Array, IsArray)                         \
    V(Array, Entries)                         \
    V(Array, Species)                         \
    V(Array, Concat)                          \
    V(Array, CopyWithin)                      \
    V(Array, Every)                           \
    V(Array, Fill)                            \
    V(Array, Filter)                          \
    V(Array, Find)                            \
    V(Array, FindIndex)                       \
    V(Array, ForEach)                         \
    V(Array, IndexOf)                         \
    V(Array, Join)                            \
    V(Array, Keys)                            \
    V(Array, LastIndexOf)                     \
    V(Array, Map)                             \
    V(Array, Pop)                             \
    V(Array, Push)                            \
    V(Array, Reduce)                          \
    V(Array, ReduceRight)                     \
    V(Array, Reverse)                         \
    V(Array, Shift)                           \
    V(Array, Slice)                           \
    V(Array, Some)                            \
    V(Array, Sort)                            \
    V(Array, Splice)                          \
    V(Array, ToLocaleString)                  \
    V(Array, ToString)                        \
    V(Array, Unshift)                         \
    V(Array, Values)                          \
    V(Array, Includes)                        \
    V(Array, Flat)                            \
    V(Array, FlatMap)                         \
    V(ArrayBuffer, Constructor)               \
    V(ArrayBuffer, Slice)                     \
    V(ArrayBuffer, GetValueFromBuffer)        \
    V(ArrayBuffer, SetValueInBuffer)          \
    V(ArrayBuffer, CloneArrayBuffer)          \
    V(ArrayBuffer, AllocateArrayBuffer)       \
    V(SharedArrayBuffer, Constructor)         \
    V(SharedArrayBuffer, Slice)               \
    V(SharedArrayBuffer, AllocateSharedArrayBuffer) \
    V(SharedArrayBuffer, IsSharedArrayBuffer) \
    V(SharedArrayBuffer, Species)             \
    V(SharedArrayBuffer, GetByteLength)       \
    V(AsyncFunction, Constructor)             \
    V(Boolean, Constructor)                   \
    V(Boolean, ThisBooleanValue)              \
    V(DataView, Constructor)                  \
    V(DataView, GetBuffer)                    \
    V(DataView, GetByteLength)                \
    V(DataView, GetOffset)                    \
    V(DataView, GetViewValue)                 \
    V(DataView, SetViewValue)                 \
    V(Date, Constructor)                      \
    V(Date, Now)                              \
    V(Date, UTC)                              \
    V(Date, Parse)                            \
    V(Date, GetDateField)                     \
    V(Date, GetTime)                          \
    V(Date, SetTime)                          \
    V(Date, ToJSON)                           \
    V(Date, ValueOf)                          \
    V(Date, ToPrimitive)                      \
    V(Function, Constructor)                  \
    V(Function, PrototypeApply)               \
    V(Function, PrototypeBind)                \
    V(Function, PrototypeCall)                \
    V(Function, PrototypeToString)            \
    V(Function, PrototypeHasInstance)         \
    V(Generator, Constructor)                 \
    V(Generator, PrototypeNext)               \
    V(Generator, PrototypeReturn)             \
    V(Generator, PrototypeThrow)              \
    V(Global, IsFinite)                       \
    V(Global, IsNaN)                          \
    V(Global, PrintEntryPoint)                \
    V(Global, NewobjRange)                 \
    V(Global, CallJsBoundFunction)            \
    V(Global, CallJsProxy)                    \
    V(Global, DecodeURI)                      \
    V(Global, EncodeURI)                      \
    V(Global, DecodeURIComponent)             \
    V(Global, EncodeURIComponent)             \
    V(Iterator, Constructor)                  \
    V(Iterator, Next)                         \
    V(Iterator, Throw)                        \
    V(Iterator, Return)                       \
    V(Iterator, GetObj)                       \
    V(AsyncIterator, Constructor)             \
    V(AsyncIterator, Next)                    \
    V(AsyncIterator, Throw)                   \
    V(AsyncIterator, Return)                  \
    V(AsyncIterator, GetObj)                  \
    V(Json, Parse)                            \
    V(Json, Stringify)                        \
    V(Map, Constructor)                       \
    V(Map, Species)                           \
    V(Map, Clear)                             \
    V(Map, Delete)                            \
    V(Map, Entries)                           \
    V(Map, Get)                               \
    V(Map, Has)                               \
    V(Map, Keys)                              \
    V(Map, Set)                               \
    V(Map, GetSize)                           \
    V(Map, Values)                            \
    V(Map, ForEach)                           \
    V(Math, Abs)                              \
    V(Math, Acos)                             \
    V(Math, Acosh)                            \
    V(Math, Asin)                             \
    V(Math, Asinh)                            \
    V(Math, Atan)                             \
    V(Math, Atanh)                            \
    V(Math, Atan2)                            \
    V(Math, Cbrt)                             \
    V(Math, Ceil)                             \
    V(Math, Clz32)                            \
    V(Math, Cos)                              \
    V(Math, Cosh)                             \
    V(Math, Exp)                              \
    V(Math, Expm1)                            \
    V(Math, Floor)                            \
    V(Math, Fround)                           \
    V(Math, Hypot)                            \
    V(Math, Imul)                             \
    V(Math, Log)                              \
    V(Math, Log1p)                            \
    V(Math, Log10)                            \
    V(Math, Log2)                             \
    V(Math, Max)                              \
    V(Math, Min)                              \
    V(Math, Pow)                              \
    V(Math, Random)                           \
    V(Math, Round)                            \
    V(Math, Sign)                             \
    V(Math, Sin)                              \
    V(Math, Sinh)                             \
    V(Math, Sqrt)                             \
    V(Math, Tan)                              \
    V(Math, Tanh)                             \
    V(Math, Trunc)                            \
    V(Atomics, Add)                           \
    V(Atomics, And)                           \
    V(Atomics, Or)                            \
    V(Atomics, Xor)                           \
    V(Atomics, Sub)                           \
    V(Atomics, Exchange)                      \
    V(Atomics, CompareExchange)               \
    V(Atomics, Store)                         \
    V(Atomics, Load)                          \
    V(Atomics, IsLockFree)                    \
    V(Atomics, Wait)                          \
    V(Atomics, Notify)                        \
    V(Number, Constructor)                    \
    V(Number, IsFinite)                       \
    V(Number, IsInteger)                      \
    V(Number, IsNaN)                          \
    V(Number, IsSafeInteger)                  \
    V(Number, ParseFloat)                     \
    V(Number, ParseInt)                       \
    V(Number, ToExponential)                  \
    V(Number, ToFixed)                        \
    V(Number, ToLocaleString)                 \
    V(Number, ToPrecision)                    \
    V(Number, ToString)                       \
    V(Number, ValueOf)                        \
    V(Number, ThisNumberValue)                \
    V(BigInt, Constructor)                    \
    V(BigInt, AsUintN)                        \
    V(BigInt, AsIntN)                         \
    V(BigInt, ToLocaleString)                 \
    V(BigInt, ToString)                       \
    V(BigInt, ValueOf)                        \
    V(BigInt, ThisBigIntValue)                \
    V(AsyncGenerator, Constructor)            \
    V(AsyncGenerator, PrototypeNext)          \
    V(AsyncGenerator, PrototypeReturn)        \
    V(AsyncGenerator, PrototypeThrow)         \
    V(Object, Constructor)                    \
    V(Object, Assign)                         \
    V(Object, Create)                         \
    V(Object, CreateDataPropertyOnObjectFunctions) \
    V(Object, DefineProperties)               \
    V(Object, DefineProperty)                 \
    V(Object, Freeze)                         \
    V(Object, FromEntries)                    \
    V(Object, GetOwnPropertyDescriptor)       \
    V(Object, GetOwnPropertyKeys)             \
    V(Object, GetOwnPropertyNames)            \
    V(Object, GetOwnPropertySymbols)          \
    V(Object, GetPrototypeOf)                 \
    V(Object, Is)                             \
    V(Object, Keys)                           \
    V(Object, Values)                         \
    V(Object, PreventExtensions)              \
    V(Object, Seal)                           \
    V(Object, SetPrototypeOf)                 \
    V(Object, HasOwnProperty)                 \
    V(Object, IsPrototypeOf)                  \
    V(Object, ToLocaleString)                 \
    V(Object, GetBuiltinTag)                  \
    V(Object, ToString)                       \
    V(Object, ValueOf)                        \
    V(Object, ProtoGetter)                    \
    V(Object, ProtoSetter)                    \
    V(PromiseHandler, Resolve)                \
    V(PromiseHandler, Reject)                 \
    V(PromiseHandler, Executor)               \
    V(PromiseHandler, ResolveElementFunction) \
    V(PromiseHandler, AllSettledResolveElementFunction) \
    V(PromiseHandler, AllSettledRejectElementFunction) \
    V(PromiseHandler, AnyRejectElementFunction) \
    V(PromiseJob, Reaction)                   \
    V(PromiseJob, ResolveThenableJob)         \
    V(PromiseJob, DynamicImportJob)           \
    V(Promise, Constructor)                   \
    V(Promise, All)                           \
    V(Promise, Race)                          \
    V(Promise, Reject)                        \
    V(Promise, Resolve)                       \
    V(Promise, GetSpecies)                    \
    V(Promise, Catch)                         \
    V(Promise, Then)                          \
    V(Promise, PerformPromiseThen)            \
    V(Promise, Finally)                       \
    V(Promise, Any)                           \
    V(Promise, PerformPromiseAny)             \
    V(Promise, AllSettled)                    \
    V(Promise, PerformPromiseAllSettled)      \
    V(Proxy, Constructor)                     \
    V(Proxy, Revocable)                       \
    V(Proxy, InvalidateProxyFunction)         \
    V(Reflect, Apply)                         \
    V(Reflect, Constructor)                   \
    V(Reflect, DefineProperty)                \
    V(Reflect, DeleteProperty)                \
    V(Reflect, Get)                           \
    V(Reflect, GetOwnPropertyDescriptor)      \
    V(Reflect, GetPrototypeOf)                \
    V(Reflect, Has)                           \
    V(Reflect, OwnKeys)                       \
    V(Reflect, PreventExtensions)             \
    V(Reflect, Set)                           \
    V(Reflect, SetPrototypeOf)                \
    V(RegExp, Constructor)                    \
    V(RegExp, Exec)                           \
    V(RegExp, Test)                           \
    V(RegExp, ToString)                       \
    V(RegExp, GetFlags)                       \
    V(RegExp, GetSpecies)                     \
    V(RegExp, Match)                          \
    V(RegExp, MatchAll)                       \
    V(RegExp, Replace)                        \
    V(RegExp, Search)                         \
    V(RegExp, Split)                          \
    V(RegExp, Create)                         \
    V(Set, Constructor)                       \
    V(Set, Species)                           \
    V(Set, Add)                               \
    V(Set, Clear)                             \
    V(Set, Delete)                            \
    V(Set, Entries)                           \
    V(Set, Has)                               \
    V(Set, GetSize)                           \
    V(Set, Values)                            \
    V(Set, ForEach)                           \
    V(StringIterator, Next)                   \
    V(String, Constructor)                    \
    V(String, FromCharCode)                   \
    V(String, FromCodePoint)                  \
    V(String, Raw)                            \
    V(String, GetSubstitution)                \
    V(String, CharAt)                         \
    V(String, CharCodeAt)                     \
    V(String, CodePointAt)                    \
    V(String, Concat)                         \
    V(String, EndsWith)                       \
    V(String, Includes)                       \
    V(String, IndexOf)                        \
    V(String, LastIndexOf)                    \
    V(String, LocaleCompare)                  \
    V(String, Match)                          \
    V(String, MatchAll)                       \
    V(String, Normalize)                      \
    V(String, PadStart)                       \
    V(String, PadEnd)                         \
    V(String, Repeat)                         \
    V(String, Replace)                        \
    V(String, ReplaceAll)                     \
    V(String, Search)                         \
    V(String, Slice)                          \
    V(String, Split)                          \
    V(String, StartsWith)                     \
    V(String, Substring)                      \
    V(String, ToLocaleLowerCase)              \
    V(String, ToLocaleUpperCase)              \
    V(String, ToLowerCase)                    \
    V(String, ToString)                       \
    V(String, ToUpperCase)                    \
    V(String, Trim)                           \
    V(String, TrimStart)                      \
    V(String, TrimEnd)                        \
    V(String, TrimLeft)                       \
    V(String, TrimRight)                      \
    V(String, GetStringIterator)              \
    V(String, SubStr)                         \
    V(Symbol, Constructor)                    \
    V(Symbol, ToString)                       \
    V(Symbol, ValueOf)                        \
    V(Symbol, For)                            \
    V(Symbol, KeyFor)                         \
    V(Symbol, DescriptionGetter)              \
    V(Symbol, ThisSymbolValue)                \
    V(Symbol, ToPrimitive)                    \
    V(Symbol, SymbolDescriptiveString)        \
    V(TypedArray, BaseConstructor)            \
    V(TypedArray, From)                       \
    V(TypedArray, Of)                         \
    V(TypedArray, Species)                    \
    V(TypedArray, GetBuffer)                  \
    V(TypedArray, GetByteLength)              \
    V(TypedArray, GetByteOffset)              \
    V(TypedArray, CopyWithin)                 \
    V(TypedArray, Entries)                    \
    V(TypedArray, Every)                      \
    V(TypedArray, Filter)                     \
    V(TypedArray, ForEach)                    \
    V(TypedArray, Join)                       \
    V(TypedArray, Keys)                       \
    V(TypedArray, GetLength)                  \
    V(TypedArray, Map)                        \
    V(TypedArray, Set)                        \
    V(TypedArray, Slice)                      \
    V(TypedArray, Sort)                       \
    V(TypedArray, Subarray)                   \
    V(TypedArray, Values)                     \
    V(TypedArray, ToStringTag)                \
    V(WeakMap, Constructor)                   \
    V(WeakMap, Delete)                        \
    V(WeakMap, Get)                           \
    V(WeakMap, Has)                           \
    V(WeakMap, Set)                           \
    V(WeakSet, Constructor)                   \
    V(WeakSet, Delete)                        \
    V(WeakSet, Add)                           \
    V(WeakSet, Has)                           \
    V(WeakRef, Constructor)                   \
    V(WeakRef, Deref)                         \
    V(FinalizationRegistry, Constructor)      \
    V(FinalizationRegistry, Register)         \
    V(FinalizationRegistry, Unregister)       \
    V(ArrayList, Constructor)                 \
    V(ArrayList, Add)                         \
    V(ArrayList, Insert)                      \
    V(ArrayList, Clear)                       \
    V(ArrayList, Clone)                       \
    V(ArrayList, Has)                         \
    V(ArrayList, GetCapacity)                 \
    V(ArrayList, IncreaseCapacityTo)          \
    V(ArrayList, TrimToCurrentLength)         \
    V(ArrayList, GetIndexOf)                  \
    V(ArrayList, IsEmpty)                     \
    V(ArrayList, GetLastIndexOf)              \
    V(ArrayList, RemoveByIndex)               \
    V(ArrayList, Remove)                      \
    V(ArrayList, RemoveByRange)               \
    V(ArrayList, ReplaceAllElements)          \
    V(ArrayList, Sort)                        \
    V(ArrayList, SubArrayList)                \
    V(ArrayList, ConvertToArray)              \
    V(ArrayList, ForEach)                     \
    V(ArrayList, GetIteratorObj)              \
    V(ArrayList, Get)                         \
    V(ArrayList, Set)                         \
    V(ArrayList, GetSize)                     \
    V(LightWeightMap, Constructor)            \
    V(LightWeightMap, HasAll)                 \
    V(LightWeightMap, HasKey)                 \
    V(LightWeightMap, HasValue)               \
    V(LightWeightMap, IncreaseCapacityTo)     \
    V(LightWeightMap, Entries)                \
    V(LightWeightMap, Get)                    \
    V(LightWeightMap, GetIndexOfKey)          \
    V(LightWeightMap, GetIndexOfValue)        \
    V(LightWeightMap, IsEmpty)                \
    V(LightWeightMap, GetKeyAt)               \
    V(LightWeightMap, Keys)                   \
    V(LightWeightMap, SetAll)                 \
    V(LightWeightMap, Set)                    \
    V(LightWeightMap, Remove)                 \
    V(LightWeightMap, RemoveAt)               \
    V(LightWeightMap, Clear)                  \
    V(LightWeightMap, SetValueAt)             \
    V(LightWeightMap, ForEach)                \
    V(LightWeightMap, ToString)               \
    V(LightWeightMap, GetValueAt)             \
    V(LightWeightMap, Length)                 \
    V(LightWeightSet, Constructor)            \
    V(LightWeightSet, Add)                    \
    V(LightWeightSet, AddAll)                 \
    V(LightWeightSet, IsEmpty)                \
    V(LightWeightSet, GetValueAt)             \
    V(LightWeightSet, HasAll)                 \
    V(LightWeightSet, Has)                    \
    V(LightWeightSet, HasHash)                \
    V(LightWeightSet, Equal)                  \
    V(LightWeightSet, IncreaseCapacityTo)     \
    V(LightWeightSet, GetIteratorObj)         \
    V(LightWeightSet, Values)                 \
    V(LightWeightSet, Entries)                \
    V(LightWeightSet, ForEach)                \
    V(LightWeightSet, GetIndexOf)             \
    V(LightWeightSet, Remove)                 \
    V(LightWeightSet, RemoveAt)               \
    V(LightWeightSet, Clear)                  \
    V(LightWeightSet, ToString)               \
    V(LightWeightSet, ToArray)                \
    V(LightWeightSet, GetSize)                \
    V(PlainArray, Constructor)                \
    V(PlainArray, Add)                        \
    V(PlainArray, Clear)                      \
    V(PlainArray, Clone)                      \
    V(PlainArray, Has)                        \
    V(PlainArray, Get)                        \
    V(PlainArray, GetIteratorObj)             \
    V(PlainArray, ForEach)                    \
    V(PlainArray, ToString)                   \
    V(PlainArray, GetIndexOfKey)              \
    V(PlainArray, GetIndexOfValue)            \
    V(PlainArray, IsEmpty)                    \
    V(PlainArray, GetKeyAt)                   \
    V(PlainArray, Remove)                     \
    V(PlainArray, RemoveAt)                   \
    V(PlainArray, RemoveRangeFrom)            \
    V(PlainArray, SetValueAt)                 \
    V(PlainArray, GetValueAt)                 \
    V(PlainArray, GetSize)                    \
    V(HashMap, Constructor)                   \
    V(HashMap, HasKey)                        \
    V(HashMap, HasValue)                      \
    V(HashMap, Replace)                       \
    V(HashMap, Keys)                          \
    V(HashMap, Values)                        \
    V(HashMap, Entries)                       \
    V(HashMap, ForEach)                       \
    V(HashMap, Set)                           \
    V(HashMap, SetAll)                        \
    V(HashMap, Remove)                        \
    V(HashMap, Get)                           \
    V(HashMap, Clear)                         \
    V(HashMap, GetLength)                     \
    V(HashMap, IsEmpty)                       \
    V(HashSet, Constructor)                   \
    V(HashSet, IsEmpty)                       \
    V(HashSet, Has)                           \
    V(HashSet, Add)                           \
    V(HashSet, Remove)                        \
    V(HashSet, Clear)                         \
    V(HashSet, GetLength)                     \
    V(HashSet, Values)                        \
    V(HashSet, Entries)                       \
    V(HashSet, ForEach)                       \
    V(TreeMap, Constructor)                   \
    V(TreeMap, HasKey)                        \
    V(TreeMap, HasValue)                      \
    V(TreeMap, GetFirstKey)                   \
    V(TreeMap, GetLastKey)                    \
    V(TreeMap, Set)                           \
    V(TreeMap, Get)                           \
    V(TreeMap, SetAll)                        \
    V(TreeMap, Remove)                        \
    V(TreeMap, Clear)                         \
    V(TreeMap, GetLowerKey)                   \
    V(TreeMap, GetHigherKey)                  \
    V(TreeMap, Replace)                       \
    V(TreeMap, IsEmpty)                       \
    V(TreeMap, GetLength)                     \
    V(TreeMap, Keys)                          \
    V(TreeMap, Values)                        \
    V(TreeMap, Entries)                       \
    V(TreeMap, ForEach)                       \
    V(TreeSet, Constructor)                   \
    V(TreeSet, Add)                           \
    V(TreeSet, Remove)                        \
    V(TreeSet, Clear)                         \
    V(TreeSet, Has)                           \
    V(TreeSet, GetFirstValue)                 \
    V(TreeSet, GetLastValue)                  \
    V(TreeSet, GetLowerValue)                 \
    V(TreeSet, GetHigherValue)                \
    V(TreeSet, PopFirst)                      \
    V(TreeSet, PopLast)                       \
    V(TreeSet, IsEmpty)                       \
    V(TreeSet, GetLength)                     \
    V(TreeSet, Values)                        \
    V(TreeSet, ForEach)                       \
    V(TreeSet, Entries)                       \
    V(Deque, Constructor)                     \
    V(Deque, InsertFront)                     \
    V(Deque, InsertEnd)                       \
    V(Deque, GetFirst)                        \
    V(Deque, GetLast)                         \
    V(Deque, GetFront)                        \
    V(Deque, GetTail)                         \
    V(Deque, Has)                             \
    V(Deque, PopFirst)                        \
    V(Deque, PopLast)                         \
    V(Deque, ForEach)                         \
    V(Deque, GetIteratorObj)                  \
    V(Deque, GetSize)                         \
    V(Stack, Constructor)                     \
    V(Stack, Iterator)                        \
    V(Stack, IsEmpty)                         \
    V(Stack, Push)                            \
    V(Stack, Peek)                            \
    V(Stack, Pop)                             \
    V(Stack, Locate)                          \
    V(Stack, ForEach)                         \
    V(Stack, GetLength)                       \
    V(Vector, Constructor)                    \
    V(Vector, Add)                            \
    V(Vector, Insert)                         \
    V(Vector, SetLength)                      \
    V(Vector, GetCapacity)                    \
    V(Vector, IncreaseCapacityTo)             \
    V(Vector, Get)                            \
    V(Vector, GetIndexOf)                     \
    V(Vector, GetIndexFrom)                   \
    V(Vector, IsEmpty)                        \
    V(Vector, GetLastElement)                 \
    V(Vector, GetLastIndexOf)                 \
    V(Vector, GetLastIndexFrom)               \
    V(Vector, Remove)                         \
    V(Vector, RemoveByIndex)                  \
    V(Vector, RemoveByRange)                  \
    V(Vector, Set)                            \
    V(Vector, SubVector)                      \
    V(Vector, ToString)                       \
    V(Vector, GetSize)                        \
    V(Vector, ForEach)                        \
    V(Vector, ReplaceAllElements)             \
    V(Vector, Has)                            \
    V(Vector, Sort)                           \
    V(Vector, Clear)                          \
    V(Vector, Clone)                          \
    V(Vector, CopyToArray)                    \
    V(Vector, ConvertToArray)                 \
    V(Vector, GetFirstElement)                \
    V(Vector, TrimToCurrentLength)            \
    V(Vector, GetIteratorObj)                 \
    V(Queue, Constructor)                     \
    V(Queue, Add)                             \
    V(Queue, GetFirst)                        \
    V(Queue, Pop)                             \
    V(Queue, ForEach)                         \
    V(Queue, GetIteratorObj)                  \
    V(Queue, GetSize)                         \
    V(List, Constructor)                      \
    V(List, Add)                              \
    V(List, GetFirst)                         \
    V(List, GetLast)                          \
    V(List, Insert)                           \
    V(List, Clear)                            \
    V(List, RemoveByIndex)                    \
    V(List, Remove)                           \
    V(List, Has)                              \
    V(List, IsEmpty)                          \
    V(List, Get)                              \
    V(List, GetIndexOf)                       \
    V(List, GetLastIndexOf)                   \
    V(List, Set)                              \
    V(List, ForEach)                          \
    V(List, ReplaceAllElements)               \
    V(List, GetIteratorObj)                   \
    V(List, Equal)                            \
    V(List, Sort)                             \
    V(List, ConvertToArray)                   \
    V(List, GetSubList)                       \
    V(List, Length)                           \
    V(LinkedList, Constructor)                \
    V(LinkedList, Length)                     \
    V(LinkedList, Add)                        \
    V(LinkedList, GetFirst)                   \
    V(LinkedList, GetLast)                    \
    V(LinkedList, Insert)                     \
    V(LinkedList, AddFirst)                   \
    V(LinkedList, Clear)                      \
    V(LinkedList, Clone)                      \
    V(LinkedList, Has)                        \
    V(LinkedList, Get)                        \
    V(LinkedList, GetIndexOf)                 \
    V(LinkedList, GetLastIndexOf)             \
    V(LinkedList, RemoveByIndex)              \
    V(LinkedList, Remove)                     \
    V(LinkedList, RemoveFirst)                \
    V(LinkedList, RemoveLast)                 \
    V(LinkedList, RemoveFirstFound)           \
    V(LinkedList, RemoveLastFound)            \
    V(LinkedList, Set)                        \
    V(LinkedList, ConvertToArray)             \
    V(LinkedList, ForEach)                    \
    V(LinkedList, GetIteratorObj)

#define ABSTRACT_OPERATION_LIST(V) \
    V(JSTaggedValue, ToString)     \

#define MEM_ALLOCATE_AND_GC_LIST(V)  \
    V(FullGC_RunPhases)              \
    V(PartialGC_RunPhases)               \
    V(STWYoungGC_RunPhases)          \
    V(ConcurrentMarking)             \
    V(ConcurrentMarkingInitialize)   \
    V(WaitConcurrentMarkingFinished) \
    V(ReMarking)                     \
    V(ConcurrentSweepingInitialize)  \
    V(ConcurrentSweepingWait)        \
    V(ParallelEvacuationInitialize)  \
    V(ParallelEvacuation)            \
    V(ParallelUpdateReference)       \
    V(WaitUpdateFinished)            \
    V(UpdateRoot)                    \
    V(UpdateWeakReference)           \
    V(ParallelEvacuator)             \
    V(ParallelEvacuatorInitialize)   \
    V(ParallelEvacuatorFinalize)     \
    V(HugeSpaceExpand)               \
    V(NonMovableSpaceExpand)         \
    V(HeapPrepare)                   \

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTERPRETER_CALLER_ID(name) INTERPRETER_ID_##name,
#define RUNTIME_CALLER_ID(name) RUNTIME_ID_##name,
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BUILTINS_API_ID(class, name) BUILTINS_ID_##class##_##name,
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ABSTRACT_OPERATION_ID(class, name) ABSTRACT_ID_##class##_##name,

#define MEM_ALLOCATE_AND_GC_ID(name) MEM_ID##name,

enum EcmaRuntimeCallerId {
    INTERPRETER_CALLER_LIST(INTERPRETER_CALLER_ID) BUILTINS_API_LIST(BUILTINS_API_ID)
    ABSTRACT_OPERATION_LIST(ABSTRACT_OPERATION_ID)
    MEM_ALLOCATE_AND_GC_LIST(MEM_ALLOCATE_AND_GC_ID)
#define DEF_RUNTIME_ID(name) RUNTIME_ID_##name,
    RUNTIME_STUB_WITH_GC_LIST(DEF_RUNTIME_ID)
#undef DEF_RUNTIME_ID
    RUNTIME_CALLER_NUMBER,
};

#if ECMASCRIPT_ENABLE_INTERPRETER_RUNTIME_STAT
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTERPRETER_TRACE(thread, name)                                                        \
    [[maybe_unused]] JSThread *_js_thread_ = thread;                                           \
    [[maybe_unused]] EcmaRuntimeStat *_run_stat_ = _js_thread_->GetEcmaVM()->GetRuntimeStat(); \
    RuntimeTimerScope interpret_##name##_scope_(INTERPRETER_CALLER_ID(name) _run_stat_)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RUNTIME_TRACE(thread, name)                                                            \
    [[maybe_unused]] JSThread *_js_thread_ = thread;                                           \
    [[maybe_unused]] EcmaRuntimeStat *_run_stat_ = _js_thread_->GetEcmaVM()->GetRuntimeStat(); \
    RuntimeTimerScope interpret_##name##_scope_(RUNTIME_CALLER_ID(name) _run_stat_);           \
    [[maybe_unused]] RuntimeStateScope _runtime_state_##name##_scope_(_js_thread_)
#else
#define INTERPRETER_TRACE(thread, name) static_cast<void>(0) // NOLINT(cppcoreguidelines-macro-usage)
#if defined(ECMASCRIPT_SUPPORT_CPUPROFILER)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RUNTIME_TRACE(thread, name)                                                            \
    [[maybe_unused]] JSThread *_js_thread_ = thread;                                           \
    [[maybe_unused]] RuntimeStateScope _runtime_state_##name##_scope_(_js_thread_)
#else
#define RUNTIME_TRACE(thread, name) static_cast<void>(0) // NOLINT(cppcoreguidelines-macro-usage)
#endif // defined(ECMASCRIPT_SUPPORT_CPUPROFILER)
#endif // ECMASCRIPT_ENABLE_INTERPRETER_RUNTIME_STAT

#if ECMASCRIPT_ENABLE_BUILTINS_RUNTIME_STAT
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BUILTINS_API_TRACE(thread, class, name)                                                \
    [[maybe_unused]] JSThread *_js_thread_ = thread;                                           \
    [[maybe_unused]] EcmaRuntimeStat *_run_stat_ = _js_thread_->GetEcmaVM()->GetRuntimeStat(); \
    RuntimeTimerScope builtins_##class##name##_scope_(BUILTINS_API_ID(class, name) _run_stat_)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ABSTRACT_OPERATION_TRACE(thread, class, name)                                          \
    [[maybe_unused]] JSThread *_js_thread_ = thread;                                           \
    [[maybe_unused]] EcmaRuntimeStat *_run_stat_ = _js_thread_->GetEcmaVM()->GetRuntimeStat(); \
    RuntimeTimerScope abstract_##class##name##_scope_(ABSTRACT_OPERATION_ID(class, name) _run_stat_)
#else
#define BUILTINS_API_TRACE(thread, class, name) static_cast<void>(0) // NOLINT(cppcoreguidelines-macro-usage)
#define ABSTRACT_OPERATION_TRACE(class, name) static_cast<void>(0) // NOLINT(cppcoreguidelines-macro-usage)
#endif // ECMASCRIPT_ENABLE_BUILTINS_RUNTIME_STAT

#if ECMASCRIPT_ENABLE_ALLOCATE_AND_GC_RUNTIME_STAT
#define MEM_ALLOCATE_AND_GC_TRACE(vm, name)             \
    CHECK_JS_THREAD(vm);                                \
    EcmaRuntimeStat *_run_stat_ = vm->GetRuntimeStat(); \
    RuntimeTimerScope mem_##name##_scope_(MEM_ALLOCATE_AND_GC_ID(name) _run_stat_)
#else
#define MEM_ALLOCATE_AND_GC_TRACE(vm, name) static_cast<void>(0) // NOLINT(cppcoreguidelines-macro-usage)
#endif // ECMASCRIPT_ENABLE_ALLOCATE_AND_GC_RUNTIME_STAT
}  // namespace panda::ecmascript
#endif
