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

#ifndef ECMASCRIPT_COMPILER_ASSEMBLER_MODULE_X64_H
#define ECMASCRIPT_COMPILER_ASSEMBLER_MODULE_X64_H

#include "ecmascript/compiler/assembler/x64/assembler_x64.h"
#include "ecmascript/compiler/assembler/x64/extended_assembler_x64.h"
#include "ecmascript/frames.h"

namespace panda::ecmascript::x64 {
class CommonCall {
public:
    static constexpr int FRAME_SLOT_SIZE = 8;
    static constexpr int DOUBLE_SLOT_SIZE = 16;
    static constexpr int TRIPLE_SLOT_SIZE = 24;
    static constexpr int QUADRUPLE_SLOT_SIZE = 32;
    static constexpr int QUINTUPLE_SLOT_SIZE = 40;
    static constexpr int SEXTUPLE_SLOT_SIZE = 48;
    static void CopyArgumentWithArgV(ExtendedAssembler *assembler, Register argc, Register argV);
    static void PushAsmInterpBridgeFrame(ExtendedAssembler *assembler);
    static void PopAsmInterpBridgeFrame(ExtendedAssembler *assembler);
    static void PushUndefinedWithArgc(ExtendedAssembler *assembler, Register argc);
    static void GetArgvAtStack(ExtendedAssembler *assembler);
    static void PushArgsWithArgvAndCheckStack(ExtendedAssembler *assembler, Register glue, Register argc, Register argv,
        Register op1, Register op2, Label *stackOverflow);
    static void StackOverflowCheck(ExtendedAssembler *assembler, Register glue, Register numArgs, Register op1,
        Register op2, Label *stackOverflow);
};

class OptimizedCall : public CommonCall {
public:
    static void CallRuntime(ExtendedAssembler *assembler);

    static void JSFunctionEntry(ExtendedAssembler *assembler);

    static void OptimizedCallOptimized(ExtendedAssembler *assembler);

    static void CallBuiltinTrampoline(ExtendedAssembler *assembler);

    static void JSProxyCallInternalWithArgV(ExtendedAssembler *assembler);

    static void JSCall(ExtendedAssembler *assembler);

    static void ConstructorJSCall(ExtendedAssembler *assembler);

    static void CallRuntimeWithArgv(ExtendedAssembler *assembler);

    static void JSCallWithArgV(ExtendedAssembler *assembler);

    static void ConstructorJSCallWithArgV(ExtendedAssembler *assembler);

    static void DeoptHandlerAsm(ExtendedAssembler *assembler);

    static void JSCallNew(ExtendedAssembler *assembler);

    static void JSCallNewWithArgV(ExtendedAssembler *assembler);

    static void GenJSCall(ExtendedAssembler *assembler, bool isNew);

    static void GenJSCallWithArgV(ExtendedAssembler *assembler, bool isNew);
private:
    static void DeoptEnterAsmInterp(ExtendedAssembler *assembler);
    static void JSCallCheck(ExtendedAssembler *assembler, Register jsFuncReg,
                            Label *lNonCallable, Label *lNotJSFunction, Label *lJSFunctionCall);
    static void ThrowNonCallableInternal(ExtendedAssembler *assembler, Register glueReg);
    static void CallOptimziedMethodInternal(ExtendedAssembler *assembler, Register glueReg, Register jsFuncReg,
                                            Register methodCallField, Register argc, Register codeAddrReg,
                                            Register expectedNumArgsReg);
    static void JSBoundFunctionCallInternal(ExtendedAssembler *assembler, Register jsFuncReg, Label *jsCall);
    static void JSProxyCallInternal(ExtendedAssembler *assembler, Register jsFuncReg);
    static void OptimizedCallAsmInterpreter(ExtendedAssembler *assembler);
    static void PushArgsWithArgV(ExtendedAssembler *assembler, Register jsfunc,
                                 Register actualNumArgs, Register argV, Label *pushCallThis);
    static void PushMandatoryJSArgs(ExtendedAssembler *assembler, Register jsfunc,
                                    Register thisObj, Register newTarget);
    static void PopJSFunctionArgs(ExtendedAssembler *assembler, Register expectedNumArgs);
    static void PushJSFunctionEntryFrame (ExtendedAssembler *assembler, Register prevFp);
    static void PopJSFunctionEntryFrame(ExtendedAssembler *assembler, Register glue);
    static void PushOptimizedUnfoldArgVFrame(ExtendedAssembler *assembler, Register callSiteSp);
    static void PopOptimizedUnfoldArgVFrame(ExtendedAssembler *assembler);
};

class AsmInterpreterCall : public CommonCall {
public:
    static void GeneratorReEnterAsmInterp(ExtendedAssembler *assembler);

    static void GeneratorReEnterAsmInterpDispatch(ExtendedAssembler *assembler);

    static void AsmInterpEntryDispatch(ExtendedAssembler *assembler);

    static void AsmInterpreterEntry(ExtendedAssembler *assembler);

    static void PushCallThisRangeAndDispatch(ExtendedAssembler *assembler);

    static void PushCallRangeAndDispatch(ExtendedAssembler *assembler);

    static void PushCallArgs3AndDispatch(ExtendedAssembler *assembler);

    static void PushCallArgs2AndDispatch(ExtendedAssembler *assembler);

    static void PushCallArg1AndDispatch(ExtendedAssembler *assembler);

    static void PushCallArg0AndDispatch(ExtendedAssembler *assembler);

    static void PushCallThisArg0AndDispatch(ExtendedAssembler *assembler);

    static void PushCallThisArg1AndDispatch(ExtendedAssembler *assembler);

    static void PushCallThisArgs2AndDispatch(ExtendedAssembler *assembler);

    static void PushCallThisArgs3AndDispatch(ExtendedAssembler *assembler);

    static void PushCallNewAndDispatch(ExtendedAssembler *assembler);

    static void PushCallNewAndDispatchNative(ExtendedAssembler *assembler);

    static void PushCallRangeAndDispatchNative(ExtendedAssembler *assembler);

    static void PushCallArgsAndDispatchNative(ExtendedAssembler *assembler);

    static void ResumeRspAndDispatch(ExtendedAssembler *assembler);

    static void ResumeRspAndReturn([[maybe_unused]] ExtendedAssembler *assembler);

    static void CallGetter(ExtendedAssembler *assembler);

    static void CallSetter(ExtendedAssembler *assembler);

    static void CallContainersArgs3(ExtendedAssembler *assembler);

    static void ResumeCaughtFrameAndDispatch(ExtendedAssembler *assembler);

    static void ResumeUncaughtFrameAndReturn(ExtendedAssembler *assembler);

private:
    static void PushFrameState(ExtendedAssembler *assembler, Register prevSpRegister, Register fpRegister,
        Register callTargetRegister, Register thisRegister, Register methodRegister, Register pcRegister,
        Register operatorRegister);
    static void PushGeneratorFrameState(ExtendedAssembler *assembler, Register prevSpRegister,
        Register fpRegister, Register callTargetRegister, Register thisRegister, Register methodRegister,
        Register contextRegister, Register pcRegister, Register operatorRegister);
    static void PushAsmInterpEntryFrame(ExtendedAssembler *assembler);
    static void PopAsmInterpEntryFrame(ExtendedAssembler *assembler);
    static void GetDeclaredNumArgsFromCallField(ExtendedAssembler *assembler, Register callFieldRegister,
        Register declaredNumArgsRegister);
    static void GetNumVregsFromCallField(ExtendedAssembler *assembler, Register callFieldRegister,
        Register numVregsRegister);
    static void PushUndefinedWithArgcAndCheckStack(ExtendedAssembler *assembler, Register glue, Register argc,
        Register op1, Register op2, Label *stackOverflow);
    static void ThrowStackOverflowExceptionAndReturn(ExtendedAssembler *assembler, Register glue, Register fp,
        Register op);
    static void HasPendingException(ExtendedAssembler *assembler, Register threadRegister);
    static void PushCallThis(ExtendedAssembler *assembler, JSCallMode mode, Label *stackOverflow);
    static Register GetThisRegsiter(ExtendedAssembler *assembler, JSCallMode mode, Register defaultRegister);
    static Register GetNewTargetRegsiter(ExtendedAssembler *assembler, JSCallMode mode, Register defaultRegister);
    static void PushVregs(ExtendedAssembler *assembler, Label *stackOverflow);
    static void DispatchCall(ExtendedAssembler *assembler, Register pcRegister, Register newSpRegister,
                             Register methodRegister, Register accRegister = rInvalid);
    static void CallNativeEntry(ExtendedAssembler *assemblSer);
    static void CallNativeWithArgv(ExtendedAssembler *assembler, bool callNew);
    static void CallNativeInternal(ExtendedAssembler *assembler, Register nativeCode);
    static void PushBuiltinFrame(ExtendedAssembler *assembler, Register glue, FrameType type);
    static void JSCallCommonEntry(ExtendedAssembler *assembler, JSCallMode mode);
    static void JSCallCommonFastPath(ExtendedAssembler *assembler, JSCallMode mode, Label *stackOverflow);
    static void JSCallCommonSlowPath(ExtendedAssembler *assembler, JSCallMode mode,
        Label *fastPathEntry, Label *pushCallThis, Label *stackOverflow);
    friend class OptimizedCall;
};
}  // namespace panda::ecmascript::x64
#endif  // ECMASCRIPT_COMPILER_ASSEMBLER_MODULE_X64_H
