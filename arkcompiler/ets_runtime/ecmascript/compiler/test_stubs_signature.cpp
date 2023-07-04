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

#include "ecmascript/compiler/call_signature.h"
namespace panda::ecmascript::kungfu {
#ifndef NDEBUG
DEF_CALL_SIGNATURE(FooAOT)
{
    // 7 : 7 input parameters
    CallSignature fooAot("FooAOT", 0, 8,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = fooAot;
    std::array<VariableType, 8> params = { // 8 : 8 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),     // lexenv
        VariableType::INT64(),
        VariableType::JS_ANY(),     // calltarget
        VariableType::JS_ANY(),     // newTarget
        VariableType::JS_ANY(),     // thisTarget
        VariableType::JS_ANY(),     // a
        VariableType::JS_ANY(),     // b
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::WebKitJSCallConv);
}

DEF_CALL_SIGNATURE(Foo1AOT)
{
    // 7 : 7 input parameters
    CallSignature foo1Aot("Foo1AOT", 0, 8,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = foo1Aot;
    std::array<VariableType, 8> params = { // 8 : 8 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),     // lexenv
        VariableType::INT64(),
        VariableType::JS_ANY(),     // calltarget
        VariableType::JS_ANY(),     // newTarget
        VariableType::JS_ANY(),     // thisTarget
        VariableType::JS_ANY(),     // a
        VariableType::JS_ANY(),     // b
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::WebKitJSCallConv);
}

DEF_CALL_SIGNATURE(Foo2AOT)
{
    // 7 : 7 input parameters
    CallSignature foo2Aot("Foo2AOT", 0, 8,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = foo2Aot;
    std::array<VariableType, 8> params = { // 8 : 8 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),     // lexenv
        VariableType::INT64(),
        VariableType::JS_ANY(),     // calltarget
        VariableType::JS_ANY(),     // newTarget
        VariableType::JS_ANY(),     // thisTarget
        VariableType::JS_ANY(),     // a
        VariableType::JS_ANY(),     // b
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::WebKitJSCallConv);
}

DEF_CALL_SIGNATURE(FooNativeAOT)
{
    // 7 : 7 input parameters
    CallSignature foo2Aot("FooNativeAOT", 0, 8,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = foo2Aot;
    std::array<VariableType, 8> params = { // 8 : 8 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),     // lexenv
        VariableType::INT64(),
        VariableType::JS_ANY(),     // calltarget
        VariableType::JS_ANY(),     // newTarget
        VariableType::JS_ANY(),     // thisTarget
        VariableType::JS_ANY(),     // a
        VariableType::JS_ANY(),     // b
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::WebKitJSCallConv);
}

DEF_CALL_SIGNATURE(FooBoundAOT)
{
    // 7 : 7 input parameters
    CallSignature foo2Aot("FooBoundAOT", 0, 8,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = foo2Aot;
    std::array<VariableType, 8> params = { // 8 : 8 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),     // lexenv
        VariableType::INT64(),
        VariableType::JS_ANY(),     // calltarget
        VariableType::JS_ANY(),     // newTarget
        VariableType::JS_ANY(),     // thisTarget
        VariableType::JS_ANY(),     // a
        VariableType::JS_ANY(),     // b
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::WebKitJSCallConv);
}

DEF_CALL_SIGNATURE(Bar1AOT)
{
    // 8 : 8 input parameters
    CallSignature barAot("Bar1AOT", 0, 9,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = barAot;
    std::array<VariableType, 9> params = { // 9 : 9 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),     // lexenv
        VariableType::INT64(),
        VariableType::JS_ANY(),     // calltarget
        VariableType::JS_ANY(),     // newTarget
        VariableType::JS_ANY(),     // thisTarget
        VariableType::JS_ANY(),     // a
        VariableType::JS_ANY(),     // b
        VariableType::JS_ANY(),     // c
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::WebKitJSCallConv);
}

DEF_CALL_SIGNATURE(BarAOT)
{
    // 7 : 7 input parameters
    CallSignature barAot("BarAOT", 0, 8,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = barAot;
    std::array<VariableType, 8> params = { // 8 : 8 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),     // lexEnv
        VariableType::INT64(),
        VariableType::JS_ANY(),     // calltarget
        VariableType::JS_ANY(),     // newTarget
        VariableType::JS_ANY(),     // thisTarget
        VariableType::JS_ANY(),     // a
        VariableType::JS_ANY(),     // b
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::WebKitJSCallConv);
}

DEF_CALL_SIGNATURE(FooProxyAOT)
{
    // 8 : 8 input parameters
    CallSignature fooProxyAot("FooProxyAOT", 0, 8,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = fooProxyAot;
    std::array<VariableType, 8> params = { // 8 : 8 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),     // lexEnv
        VariableType::INT64(),
        VariableType::JS_ANY(),     // calltarget
        VariableType::JS_ANY(),     // newTarget
        VariableType::JS_ANY(),     // thisTarget
        VariableType::JS_ANY(),     // a
        VariableType::JS_ANY(),     // b
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::WebKitJSCallConv);
}

DEF_CALL_SIGNATURE(FooProxy2AOT)
{
    // 8 : 8 input parameters
    CallSignature FooProxy2AOT("FooProxy2AOT", 0, 8,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = FooProxy2AOT;
    std::array<VariableType, 8> params = { // 8 : 8 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),     // lexEnv
        VariableType::INT64(),
        VariableType::JS_ANY(),     // calltarget
        VariableType::JS_ANY(),     // newTarget
        VariableType::JS_ANY(),     // thisTarget
        VariableType::JS_ANY(),     // a
        VariableType::JS_ANY(),     // b
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::WebKitJSCallConv);
}

DEF_CALL_SIGNATURE(Bar2AOT)
{
    // 7 : 7 input parameters
    CallSignature bar2Aot("Bar2AOT", 0, 6,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = bar2Aot;
    std::array<VariableType, 6> params = { // 6 : 6 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),     // lexenv
        VariableType::INT64(),
        VariableType::JS_ANY(),     // calltarget
        VariableType::JS_ANY(),     // newTarget
        VariableType::JS_ANY(),     // thisTarget
    };
    callSign->SetParameters(params.data());
    callSign->SetVariadicArgs(true);
    callSign->SetCallConv(CallSignature::CallConv::WebKitJSCallConv);
}

DEF_CALL_SIGNATURE(TestAbsoluteAddressRelocation)
{
    // 1 : 1 input parameters
    CallSignature TestAbsoluteAddressRelocation("TestAbsoluteAddressRelocation", 0, 1,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::INT64()); // undefined or hole
    *callSign = TestAbsoluteAddressRelocation;
    // 1 : 1 input parameters
    std::array<VariableType, 1> params = {
        VariableType::INT64(),
    };
    callSign->SetParameters(params.data());
}

#endif
}  // namespace panda::ecmascript::kungfu