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

#ifndef ECMASCRIPT_COMPILER_BC_CALL_SIGNATURE_H
#define ECMASCRIPT_COMPILER_BC_CALL_SIGNATURE_H

#include "ecmascript/base/config.h"
#include "ecmascript/compiler/rt_call_signature.h"

namespace panda::ecmascript::kungfu {
#define IGNORE_BC_STUB(...)
#define ASM_UNUSED_BC_STUB_LIST(T)                      \
    T(HandleOverflowD8)                                 \
    T(HandleOverflowD9)                                 \
    T(HandleOverflowDA)                                 \
    T(HandleOverflowDB)                                 \
    T(HandleOverflowDC)                                 \
    T(HandleOverflowDD)                                 \
    T(HandleOverflowDE)                                 \
    T(HandleOverflowDF)                                 \
    T(HandleOverflowE0)                                 \
    T(HandleOverflowE1)                                 \
    T(HandleOverflowE2)                                 \
    T(HandleOverflowE3)                                 \
    T(HandleOverflowE4)                                 \
    T(HandleOverflowE5)                                 \
    T(HandleOverflowE6)                                 \
    T(HandleOverflowE7)                                 \
    T(HandleOverflowE8)                                 \
    T(HandleOverflowE9)                                 \
    T(HandleOverflowEA)                                 \
    T(HandleOverflowEB)                                 \
    T(HandleOverflowEC)                                 \
    T(HandleOverflowED)                                 \
    T(HandleOverflowEE)                                 \
    T(HandleOverflowEF)                                 \
    T(HandleOverflowF0)                                 \
    T(HandleOverflowF1)                                 \
    T(HandleOverflowF2)                                 \
    T(HandleOverflowF3)                                 \
    T(HandleOverflowF4)                                 \
    T(HandleOverflowF5)                                 \
    T(HandleOverflowF6)                                 \
    T(HandleOverflowF7)                                 \
    T(HandleOverflowF8)                                 \
    T(HandleOverflowF9)                                 \
    T(HandleOverflowFA)                                 \


// V: Not Enabled, T: Enabled, D: Always Disable SingleStepDebugging
#define ASM_INTERPRETER_BC_STUB_LIST(V, T, D)                             \
    T(HandleLdundefined)                                                  \
    T(HandleLdnull)                                                       \
    T(HandleLdtrue)                                                       \
    T(HandleLdfalse)                                                      \
    T(HandleCreateemptyobject)                                            \
    T(HandleCreateemptyarrayImm8)                                         \
    T(HandleCreatearraywithbufferImm8Id16)                                \
    T(HandleCreateobjectwithbufferImm8Id16)                               \
    T(HandleNewobjrangeImm8Imm8V8)                                        \
    T(HandleNewlexenvImm8)                                                \
    T(HandleAdd2Imm8V8)                                                   \
    T(HandleSub2Imm8V8)                                                   \
    T(HandleMul2Imm8V8)                                                   \
    T(HandleDiv2Imm8V8)                                                   \
    T(HandleMod2Imm8V8)                                                   \
    T(HandleEqImm8V8)                                                     \
    T(HandleNoteqImm8V8)                                                  \
    T(HandleLessImm8V8)                                                   \
    T(HandleLesseqImm8V8)                                                 \
    T(HandleGreaterImm8V8)                                                \
    T(HandleGreatereqImm8V8)                                              \
    T(HandleShl2Imm8V8)                                                   \
    T(HandleShr2Imm8V8)                                                   \
    T(HandleAshr2Imm8V8)                                                  \
    T(HandleAnd2Imm8V8)                                                   \
    T(HandleOr2Imm8V8)                                                    \
    T(HandleXor2Imm8V8)                                                   \
    T(HandleExpImm8V8)                                                    \
    T(HandleTypeofImm8)                                                   \
    T(HandleTonumberImm8)                                                 \
    T(HandleTonumericImm8)                                                \
    T(HandleNegImm8)                                                      \
    T(HandleNotImm8)                                                      \
    T(HandleIncImm8)                                                      \
    T(HandleDecImm8)                                                      \
    T(HandleIstrue)                                                       \
    T(HandleIsfalse)                                                      \
    T(HandleIsinImm8V8)                                                   \
    T(HandleInstanceofImm8V8)                                             \
    T(HandleStrictnoteqImm8V8)                                            \
    T(HandleStricteqImm8V8)                                               \
    D(HandleCallarg0Imm8)                                                 \
    D(HandleCallarg1Imm8V8)                                               \
    D(HandleCallargs2Imm8V8V8)                                            \
    D(HandleCallargs3Imm8V8V8V8)                                          \
    D(HandleCallthis0Imm8V8)                                              \
    D(HandleCallthis1Imm8V8V8)                                            \
    D(HandleCallthis2Imm8V8V8V8)                                          \
    D(HandleCallthis3Imm8V8V8V8V8)                                        \
    D(HandleCallthisrangeImm8Imm8V8)                                      \
    T(HandleSupercallthisrangeImm8Imm8V8)                                 \
    T(HandleDefinefuncImm8Id16Imm8)                                       \
    T(HandleDefinemethodImm8Id16Imm8)                                     \
    T(HandleDefineclasswithbufferImm8Id16Id16Imm16V8)                     \
    T(HandleGetnextpropnameV8)                                            \
    T(HandleLdobjbyvalueImm8V8)                                           \
    T(HandleStobjbyvalueImm8V8V8)                                         \
    T(HandleLdsuperbyvalueImm8V8)                                         \
    T(HandleLdobjbyindexImm8Imm16)                                        \
    T(HandleStobjbyindexImm8V8Imm16)                                      \
    T(HandleLdlexvarImm4Imm4)                                             \
    T(HandleStlexvarImm4Imm4)                                             \
    T(HandleLdaStrId16)                                                   \
    T(HandleTryldglobalbynameImm8Id16)                                    \
    T(HandleTrystglobalbynameImm8Id16)                                    \
    T(HandleLdglobalvarImm16Id16)                                         \
    T(HandleLdobjbynameImm8Id16)                                          \
    T(HandleStobjbynameImm8Id16V8)                                        \
    T(HandleMovV4V4)                                                      \
    T(HandleMovV8V8)                                                      \
    T(HandleLdsuperbynameImm8Id16)                                        \
    T(HandleStconsttoglobalrecordImm16Id16)                               \
    T(HandleSttoglobalrecordImm16Id16)                                    \
    T(HandleLdthisbynameImm8Id16)                                         \
    T(HandleStthisbynameImm8Id16)                                         \
    T(HandleLdthisbyvalueImm8)                                            \
    T(HandleStthisbyvalueImm8V8)                                          \
    T(HandleJmpImm8)                                                      \
    T(HandleJmpImm16)                                                     \
    T(HandleJeqzImm8)                                                     \
    T(HandleJeqzImm16)                                                    \
    T(HandleJnezImm8)                                                     \
    T(HandleJstricteqzImm8)                                               \
    T(HandleJnstricteqzImm8)                                              \
    T(HandleJeqnullImm8)                                                  \
    T(HandleJnenullImm8)                                                  \
    T(HandleJstricteqnullImm8)                                            \
    T(HandleJnstricteqnullImm8)                                           \
    T(HandleJequndefinedImm8)                                             \
    T(HandleJneundefinedImm8)                                             \
    T(HandleJstrictequndefinedImm8)                                       \
    T(HandleJnstrictequndefinedImm8)                                      \
    T(HandleJeqV8Imm8)                                                    \
    T(HandleJneV8Imm8)                                                    \
    T(HandleJstricteqV8Imm8)                                              \
    T(HandleJnstricteqV8Imm8)                                             \
    T(HandleLdaV8)                                                        \
    T(HandleStaV8)                                                        \
    T(HandleLdaiImm32)                                                    \
    T(HandleFldaiImm64)                                                   \
    T(HandleReturn)                                                       \
    T(HandleReturnundefined)                                              \
    T(HandleGetpropiterator)                                              \
    T(HandleGetiteratorImm8)                                              \
    T(HandleCloseiteratorImm8V8)                                          \
    T(HandlePoplexenv)                                                    \
    T(HandleLdnan)                                                        \
    T(HandleLdinfinity)                                                   \
    T(HandleGetunmappedargs)                                              \
    T(HandleLdglobal)                                                     \
    T(HandleLdnewtarget)                                                  \
    T(HandleLdthis)                                                       \
    T(HandleLdhole)                                                       \
    T(HandleCreateregexpwithliteralImm8Id16Imm8)                          \
    T(HandleCreateregexpwithliteralImm16Id16Imm8)                         \
    D(HandleCallrangeImm8Imm8V8)                                          \
    T(HandleDefinefuncImm16Id16Imm8)                                      \
    T(HandleDefineclasswithbufferImm16Id16Id16Imm16V8)                    \
    T(HandleGettemplateobjectImm8)                                        \
    T(HandleSetobjectwithprotoImm8V8)                                     \
    T(HandleStownbyvalueImm8V8V8)                                         \
    T(HandleStownbyindexImm8V8Imm16)                                      \
    T(HandleStownbynameImm8Id16V8)                                        \
    T(HandleGetmodulenamespaceImm8)                                       \
    T(HandleStmodulevarImm8)                                              \
    T(HandleLdlocalmodulevarImm8)                                         \
    T(HandleLdexternalmodulevarImm8)                                      \
    T(HandleStglobalvarImm16Id16)                                         \
    T(HandleCreateemptyarrayImm16)                                        \
    T(HandleCreatearraywithbufferImm16Id16)                               \
    T(HandleCreateobjectwithbufferImm16Id16)                              \
    T(HandleNewobjrangeImm16Imm8V8)                                       \
    T(HandleTypeofImm16)                                                  \
    T(HandleLdobjbyvalueImm16V8)                                          \
    T(HandleStobjbyvalueImm16V8V8)                                        \
    T(HandleLdsuperbyvalueImm16V8)                                        \
    T(HandleLdobjbyindexImm16Imm16)                                       \
    T(HandleStobjbyindexImm16V8Imm16)                                     \
    T(HandleLdlexvarImm8Imm8)                                             \
    T(HandleStlexvarImm8Imm8)                                             \
    T(HandleTryldglobalbynameImm16Id16)                                   \
    T(HandleTrystglobalbynameImm16Id16)                                   \
    T(HandleStownbynamewithnamesetImm8Id16V8)                             \
    T(HandleMovV16V16)                                                    \
    T(HandleLdobjbynameImm16Id16)                                         \
    T(HandleStobjbynameImm16Id16V8)                                       \
    T(HandleLdsuperbynameImm16Id16)                                       \
    T(HandleLdthisbynameImm16Id16)                                        \
    T(HandleStthisbynameImm16Id16)                                        \
    T(HandleLdthisbyvalueImm16)                                           \
    T(HandleStthisbyvalueImm16V8)                                         \
    T(HandleAsyncgeneratorrejectV8)                                       \
    T(HandleJmpImm32)                                                     \
    T(HandleStownbyvaluewithnamesetImm8V8V8)                              \
    T(HandleJeqzImm32)                                                    \
    T(HandleJnezImm16)                                                    \
    T(HandleJnezImm32)                                                    \
    T(HandleJstricteqzImm16)                                              \
    T(HandleJnstricteqzImm16)                                             \
    T(HandleJeqnullImm16)                                                 \
    T(HandleJnenullImm16)                                                 \
    T(HandleJstricteqnullImm16)                                           \
    T(HandleJnstricteqnullImm16)                                          \
    T(HandleJequndefinedImm16)                                            \
    T(HandleJneundefinedImm16)                                            \
    T(HandleJstrictequndefinedImm16)                                      \
    T(HandleJnstrictequndefinedImm16)                                     \
    T(HandleJeqV8Imm16)                                                   \
    T(HandleJneV8Imm16)                                                   \
    T(HandleJstricteqV8Imm16)                                             \
    T(HandleJnstricteqV8Imm16)                                            \
    T(HandleGetiteratorImm16)                                             \
    T(HandleCloseiteratorImm16V8)                                         \
    T(HandleLdsymbol)                                                     \
    T(HandleAsyncfunctionenter)                                           \
    T(HandleLdfunction)                                                   \
    T(HandleDebugger)                                                     \
    T(HandleCreategeneratorobjV8)                                         \
    T(HandleCreateiterresultobjV8V8)                                      \
    T(HandleCreateobjectwithexcludedkeysImm8V8V8)                         \
    T(HandleNewobjapplyImm8V8)                                            \
    T(HandleNewobjapplyImm16V8)                                           \
    T(HandleNewlexenvwithnameImm8Id16)                                    \
    T(HandleCreateasyncgeneratorobjV8)                                    \
    T(HandleAsyncgeneratorresolveV8V8V8)                                  \
    T(HandleSupercallspreadImm8V8)                                        \
    T(HandleApplyImm8V8V8)                                                \
    T(HandleSupercallarrowrangeImm8Imm8V8)                                \
    T(HandleDefinegettersetterbyvalueV8V8V8V8)                            \
    T(HandleDynamicimport)                                                \
    T(HandleDefinemethodImm16Id16Imm8)                                    \
    T(HandleResumegenerator)                                              \
    T(HandleGetresumemode)                                                \
    T(HandleGettemplateobjectImm16)                                       \
    T(HandleDelobjpropV8)                                                 \
    T(HandleSuspendgeneratorV8)                                           \
    T(HandleAsyncfunctionawaituncaughtV8)                                 \
    T(HandleCopydatapropertiesV8)                                         \
    T(HandleStarrayspreadV8V8)                                            \
    T(HandleSetobjectwithprotoImm16V8)                                    \
    T(HandleStownbyvalueImm16V8V8)                                        \
    T(HandleStsuperbyvalueImm8V8V8)                                       \
    T(HandleStsuperbyvalueImm16V8V8)                                      \
    T(HandleStownbyindexImm16V8Imm16)                                     \
    T(HandleStownbynameImm16Id16V8)                                       \
    T(HandleAsyncfunctionresolveV8)                                       \
    T(HandleAsyncfunctionrejectV8)                                        \
    T(HandleCopyrestargsImm8)                                             \
    T(HandleStsuperbynameImm8Id16V8)                                      \
    T(HandleStsuperbynameImm16Id16V8)                                     \
    T(HandleStownbyvaluewithnamesetImm16V8V8)                             \
    T(HandleLdbigintId16)                                                 \
    T(HandleStownbynamewithnamesetImm16Id16V8)                            \
    T(HandleNop)                                                          \
    T(HandleSetgeneratorstateImm8)                                        \
    T(HandleGetasynciteratorImm8)                                         \
    ASM_UNUSED_BC_STUB_LIST(T)                                            \
    T(HandleCallRuntime)                                                  \
    T(HandleDeprecated)                                                   \
    T(HandleWide)                                                         \
    T(HandleThrow)                                                        \
    D(ExceptionHandler)

// V: Not Enabled, T: Enabled, D: Always Disable SingleStepDebugging
#define ASM_INTERPRETER_DEPRECATED_STUB_LIST(V, T, D)                     \
    T(HandleDeprecatedLdlexenvPrefNone)                                   \
    T(HandleDeprecatedPoplexenvPrefNone)                                  \
    T(HandleDeprecatedGetiteratornextPrefV8V8)                            \
    T(HandleDeprecatedCreatearraywithbufferPrefImm16)                     \
    T(HandleDeprecatedCreateobjectwithbufferPrefImm16)                    \
    T(HandleDeprecatedTonumberPrefV8)                                     \
    T(HandleDeprecatedTonumericPrefV8)                                    \
    T(HandleDeprecatedNegPrefV8)                                          \
    T(HandleDeprecatedNotPrefV8)                                          \
    T(HandleDeprecatedIncPrefV8)                                          \
    T(HandleDeprecatedDecPrefV8)                                          \
    D(HandleDeprecatedCallarg0PrefV8)                                     \
    D(HandleDeprecatedCallarg1PrefV8V8)                                   \
    D(HandleDeprecatedCallargs2PrefV8V8V8)                                \
    D(HandleDeprecatedCallargs3PrefV8V8V8V8)                              \
    D(HandleDeprecatedCallrangePrefImm16V8)                               \
    D(HandleDeprecatedCallspreadPrefV8V8V8)                               \
    D(HandleDeprecatedCallthisrangePrefImm16V8)                           \
    T(HandleDeprecatedDefineclasswithbufferPrefId16Imm16Imm16V8V8)        \
    T(HandleDeprecatedResumegeneratorPrefV8)                              \
    T(HandleDeprecatedGetresumemodePrefV8)                                \
    T(HandleDeprecatedGettemplateobjectPrefV8)                            \
    T(HandleDeprecatedDelobjpropPrefV8V8)                                 \
    T(HandleDeprecatedSuspendgeneratorPrefV8V8)                           \
    T(HandleDeprecatedAsyncfunctionawaituncaughtPrefV8V8)                 \
    T(HandleDeprecatedCopydatapropertiesPrefV8V8)                         \
    T(HandleDeprecatedSetobjectwithprotoPrefV8V8)                         \
    T(HandleDeprecatedLdobjbyvaluePrefV8V8)                               \
    T(HandleDeprecatedLdsuperbyvaluePrefV8V8)                             \
    T(HandleDeprecatedLdobjbyindexPrefV8Imm32)                            \
    T(HandleDeprecatedAsyncfunctionresolvePrefV8V8V8)                     \
    T(HandleDeprecatedAsyncfunctionrejectPrefV8V8V8)                      \
    T(HandleDeprecatedStlexvarPrefImm4Imm4V8)                             \
    T(HandleDeprecatedStlexvarPrefImm8Imm8V8)                             \
    T(HandleDeprecatedStlexvarPrefImm16Imm16V8)                           \
    T(HandleDeprecatedGetmodulenamespacePrefId32)                         \
    T(HandleDeprecatedStmodulevarPrefId32)                                \
    T(HandleDeprecatedLdobjbynamePrefId32V8)                              \
    T(HandleDeprecatedLdsuperbynamePrefId32V8)                            \
    T(HandleDeprecatedLdmodulevarPrefId32Imm8)                            \
    T(HandleDeprecatedStconsttoglobalrecordPrefId32)                      \
    T(HandleDeprecatedStlettoglobalrecordPrefId32)                        \
    T(HandleDeprecatedStclasstoglobalrecordPrefId32)                      \
    T(HandleDeprecatedLdhomeobjectPrefNone)                               \
    T(HandleDeprecatedCreateobjecthavingmethodPrefImm16)                  \
    T(HandleDeprecatedDynamicimportPrefV8)                                \
    T(HandleDeprecatedAsyncgeneratorrejectPrefV8V8)

// V: Not Enabled, T: Enabled, D: Always Disable SingleStepDebugging
#define ASM_INTERPRETER_WIDE_STUB_LIST(V, T, D)                           \
    T(HandleWideCreateobjectwithexcludedkeysPrefImm16V8V8)                \
    T(HandleWideNewobjrangePrefImm16V8)                                   \
    T(HandleWideNewlexenvPrefImm16)                                       \
    T(HandleWideNewlexenvwithnamePrefImm16Id16)                           \
    D(HandleWideCallrangePrefImm16V8)                                     \
    D(HandleWideCallthisrangePrefImm16V8)                                 \
    T(HandleWideSupercallthisrangePrefImm16V8)                            \
    T(HandleWideSupercallarrowrangePrefImm16V8)                           \
    T(HandleWideLdobjbyindexPrefImm32)                                    \
    T(HandleWideStobjbyindexPrefV8Imm32)                                  \
    T(HandleWideStownbyindexPrefV8Imm32)                                  \
    T(HandleWideCopyrestargsPrefImm16)                                    \
    T(HandleWideLdlexvarPrefImm16Imm16)                                   \
    T(HandleWideStlexvarPrefImm16Imm16)                                   \
    T(HandleWideGetmodulenamespacePrefImm16)                              \
    T(HandleWideStmodulevarPrefImm16)                                     \
    T(HandleWideLdlocalmodulevarPrefImm16)                                \
    T(HandleWideLdexternalmodulevarPrefImm16)                             \
    T(HandleWideLdpatchvarPrefImm16)                                      \
    T(HandleWideStpatchvarPrefImm16)

// V: Not Enabled, T: Enabled, D: Always Disable SingleStepDebugging
#define ASM_INTERPRETER_THROW_STUB_LIST(V, T, D)                          \
    T(HandleThrowPrefNone)                                                \
    T(HandleThrowNotexistsPrefNone)                                       \
    T(HandleThrowPatternnoncoerciblePrefNone)                             \
    T(HandleThrowDeletesuperpropertyPrefNone)                             \
    T(HandleThrowConstassignmentPrefV8)                                   \
    T(HandleThrowIfnotobjectPrefV8)                                       \
    T(HandleThrowUndefinedifholePrefV8V8)                                 \
    T(HandleThrowIfsupernotcorrectcallPrefImm8)                           \
    T(HandleThrowIfsupernotcorrectcallPrefImm16)                          \
    T(HandleThrowUndefinedifholewithnamePrefId16)

// V: Not Enabled, T: Enabled, D: Always Disable SingleStepDebugging
#define ASM_INTERPRETER_CALLRUNTIME_STUB_LIST(V, T, D)                    \
    T(HandleCallRuntimeNotifyConcurrentResultPrefNone)                    \

#define ASM_INTERPRETER_BC_HELPER_STUB_LIST(V)          \
    V(SingleStepDebugging)                              \
    V(BCDebuggerEntry)                                  \
    V(BCDebuggerExceptionEntry)                         \
    V(NewObjectRangeThrowException)                     \
    V(ThrowStackOverflowException)

#define INTERPRETER_DISABLE_SINGLE_STEP_DEBUGGING_BC_STUB_LIST(V)           \
    ASM_INTERPRETER_BC_STUB_LIST(IGNORE_BC_STUB, IGNORE_BC_STUB, V)         \
    ASM_INTERPRETER_DEPRECATED_STUB_LIST(IGNORE_BC_STUB, IGNORE_BC_STUB, V) \
    ASM_INTERPRETER_WIDE_STUB_LIST(IGNORE_BC_STUB, IGNORE_BC_STUB, V)       \
    ASM_INTERPRETER_THROW_STUB_LIST(IGNORE_BC_STUB, IGNORE_BC_STUB, V)      \
    ASM_INTERPRETER_CALLRUNTIME_STUB_LIST(IGNORE_BC_STUB, IGNORE_BC_STUB, V)

#define INTERPRETER_BC_STUB_LIST(V)                            \
    ASM_INTERPRETER_BC_STUB_LIST(IGNORE_BC_STUB, V, V)         \
    ASM_INTERPRETER_DEPRECATED_STUB_LIST(IGNORE_BC_STUB, V, V) \
    ASM_INTERPRETER_WIDE_STUB_LIST(IGNORE_BC_STUB, V, V)       \
    ASM_INTERPRETER_THROW_STUB_LIST(IGNORE_BC_STUB, V, V)      \
    ASM_INTERPRETER_CALLRUNTIME_STUB_LIST(IGNORE_BC_STUB, V, V)

#define ASM_INTERPRETER_BC_STUB_ID_LIST(V) \
    ASM_INTERPRETER_BC_STUB_LIST(V, V, V)

#define ASM_INTERPRETER_SECOND_BC_STUB_ID_LIST(V) \
    ASM_INTERPRETER_WIDE_STUB_LIST(V, V, V)       \
    ASM_INTERPRETER_THROW_STUB_LIST(V, V, V)      \
    ASM_INTERPRETER_DEPRECATED_STUB_LIST(V, V, V) \
    ASM_INTERPRETER_CALLRUNTIME_STUB_LIST(V, V, V)

class BytecodeStubCSigns {
public:
    // is uint8 max
    static constexpr size_t LAST_VALID_OPCODE = 0xFF;
    // all valid stub, include normal and helper stub
    enum ValidID {
#define DEF_VALID_BC_STUB_ID(name) name,
        INTERPRETER_BC_STUB_LIST(DEF_VALID_BC_STUB_ID)
        ASM_INTERPRETER_BC_HELPER_STUB_LIST(DEF_VALID_BC_STUB_ID)
#undef DEF_VALID_BC_STUB_ID
        NUM_OF_VALID_STUBS
    };

#define DEF_BC_STUB_ID(name) PREF_ID_##name,
    enum WideID {
        ASM_INTERPRETER_WIDE_STUB_LIST(DEF_BC_STUB_ID, DEF_BC_STUB_ID, DEF_BC_STUB_ID)
        NUM_OF_WIDE_STUBS
    };
    enum ThrowID {
        ASM_INTERPRETER_THROW_STUB_LIST(DEF_BC_STUB_ID, DEF_BC_STUB_ID, DEF_BC_STUB_ID)
        NUM_OF_THROW_STUBS
    };
    enum DeprecatedID {
        ASM_INTERPRETER_DEPRECATED_STUB_LIST(DEF_BC_STUB_ID, DEF_BC_STUB_ID, DEF_BC_STUB_ID)
        NUM_OF_DEPRECATED_STUBS
    };
    enum CallRuntimeID {
        ASM_INTERPRETER_CALLRUNTIME_STUB_LIST(DEF_BC_STUB_ID, DEF_BC_STUB_ID, DEF_BC_STUB_ID)
        NUM_OF_CALLRUNTIME_STUBS
    };
#undef DEF_BC_STUB_ID

#define DEF_BC_STUB_ID(name) ID_##name,
    enum ID {
        ASM_INTERPRETER_BC_STUB_ID_LIST(DEF_BC_STUB_ID)
        NUM_OF_ALL_NORMAL_STUBS,
        lastOpcode = LAST_VALID_OPCODE, // last Opcode is max opcode size
        ASM_INTERPRETER_SECOND_BC_STUB_ID_LIST(DEF_BC_STUB_ID)
        ASM_INTERPRETER_BC_HELPER_STUB_LIST(DEF_BC_STUB_ID)
        NUM_OF_STUBS,
        ID_Wide_Start = lastOpcode + 1,
        ID_Throw_Start = ID_Wide_Start + NUM_OF_WIDE_STUBS,
        ID_Deprecated_Start = ID_Throw_Start + NUM_OF_THROW_STUBS,
        ID_CallRuntime_Start = ID_Deprecated_Start + NUM_OF_DEPRECATED_STUBS
    };
#undef DEF_BC_STUB_ID

    static void Initialize();

    static void GetCSigns(std::vector<const CallSignature*>& outCSigns);

    static const CallSignature* Get(size_t index)
    {
        ASSERT(index < NUM_OF_VALID_STUBS);
        return &callSigns_[index];
    }

    static const CallSignature* BCDebuggerHandler()
    {
        return &bcDebuggerHandlerCSign_;
    }

    static const CallSignature* BCHandler()
    {
        return &bcHandlerCSign_;
    }

private:
    static CallSignature callSigns_[NUM_OF_VALID_STUBS];
    static CallSignature bcHandlerCSign_;
    static CallSignature bcDebuggerHandlerCSign_;
};

enum class InterpreterHandlerInputs : size_t {
    GLUE = 0,
    SP,
    PC,
    CONSTPOOL,
    PROFILE_TYPE_INFO,
    ACC,
    HOTNESS_COUNTER,
    NUM_OF_INPUTS
};

enum class CallDispatchInputs : size_t {
    GLUE = 0,
    SP,
    CALL_TARGET,
    METHOD,
    CALL_FIELD,
    ARG0,
    ARG1,
    ARG2,
    ARG3,
    NUM_OF_INPUTS,

    ARGC = ARG0,
    ARGV = ARG1,
};

#define BYTECODE_STUB_END_ID BytecodeStubCSigns::ID_ExceptionHandler
#define BCSTUB_ID(name) kungfu::BytecodeStubCSigns::ID_##name
}  // namespace panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_BC_CALL_SIGNATURE_H
