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

#include "ecmascript/builtins/builtins_symbol.h"

#include "ecmascript/ecma_runtime_call_info.h"
#include "ecmascript/ecma_string.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_hclass.h"
#include "ecmascript/js_object-inl.h"
#include "ecmascript/js_primitive_ref.h"
#include "ecmascript/js_tagged_value-inl.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/symbol_table.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::builtins;

namespace panda::test {
using Symbol = ecmascript::builtins::BuiltinsSymbol;
using BuiltinsBase = panda::ecmascript::base::BuiltinsBase;

class BuiltinsSymbolTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
        TestHelper::CreateEcmaVMWithScope(instance, thread, scope);
    }

    void TearDown() override
    {
        TestHelper::DestroyEcmaVMWithScope(instance, scope);
    }

    EcmaVM *instance {nullptr};
    EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};
};

// new Symbol.toString()
HWTEST_F_L0(BuiltinsSymbolTest, SymbolNoParameterToString)
{
    auto ecmaVM = thread->GetEcmaVM();

    JSHandle<JSSymbol> symbol = ecmaVM->GetFactory()->NewJSSymbol();

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(symbol.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = Symbol::ToString(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    ASSERT_TRUE(result.IsString());

    auto symbolValue = ecmaVM->GetFactory()->NewFromASCII("Symbol()");
    ASSERT_EQ(EcmaStringAccessor::Compare(*symbolValue, *resultHandle), 0);

    // Undefined  not Object
    ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    result = Symbol::ToString(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(thread->HasPendingException());
    EXPECT_EQ(result, JSTaggedValue::Exception());
    thread->ClearException();

    // No Symbol data
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(array.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    result = Symbol::ToString(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(thread->HasPendingException());
    EXPECT_EQ(result, JSTaggedValue::Exception());
    thread->ClearException();
}

// new Symbol("aaa").toString()
HWTEST_F_L0(BuiltinsSymbolTest, SymbolWithParameterToString)
{
    auto ecmaVM = thread->GetEcmaVM();

    JSHandle<JSSymbol> symbol = ecmaVM->GetFactory()->NewPublicSymbolWithChar("aaa");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(symbol.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = Symbol::ToString(ecmaRuntimeCallInfo);
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    ASSERT_TRUE(result.IsString());

    auto symbolValue = ecmascript::base::BuiltinsBase::GetTaggedString(thread, "Symbol(aaa)");
    ASSERT_EQ(EcmaStringAccessor::Compare(reinterpret_cast<EcmaString *>(symbolValue.GetRawData()), *resultHandle), 0);
}

// new Symbol().valueOf()
HWTEST_F_L0(BuiltinsSymbolTest, SymbolNoParameterValueOf)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();

    JSHandle<JSSymbol> symbol = ecmaVM->GetFactory()->NewJSSymbol();

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(symbol.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsSymbol::ValueOf(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(result.IsSymbol());
    ASSERT_EQ(result.GetRawData() == (JSTaggedValue(*symbol)).GetRawData(), true);

    JSHandle<JSFunction> symbolObject(env->GetSymbolFunction());
    JSHandle<JSTaggedValue> symbolValue(symbol);
    JSHandle<JSPrimitiveRef> symbolRef = ecmaVM->GetFactory()->NewJSPrimitiveRef(symbolObject, symbolValue);

    auto otherEcmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    otherEcmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    otherEcmaRuntimeCallInfo->SetThis(symbolRef.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, otherEcmaRuntimeCallInfo);
    JSTaggedValue otherResult = BuiltinsSymbol::ValueOf(otherEcmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(otherResult.IsSymbol());
    ASSERT_EQ(otherResult.GetRawData() == (JSTaggedValue(*symbol)).GetRawData(), true);

    // Undefined not Object
    ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    result = Symbol::ValueOf(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(thread->HasPendingException());
    EXPECT_EQ(result, JSTaggedValue::Exception());
    thread->ClearException();

    // No Symbol data
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(array.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    result = Symbol::ValueOf(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(thread->HasPendingException());
    EXPECT_EQ(result, JSTaggedValue::Exception());
    thread->ClearException();
}

// new Symbol("bbb").valueOf()
HWTEST_F_L0(BuiltinsSymbolTest, SymbolWithParameterValueOf)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();

    JSHandle<JSSymbol> symbol = ecmaVM->GetFactory()->NewPublicSymbolWithChar("bbb");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(symbol.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsSymbol::ValueOf(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(result.IsSymbol());
    ASSERT_EQ(result.GetRawData() == (JSTaggedValue(*symbol)).GetRawData(), true);

    JSHandle<JSFunction> symbolObject(env->GetSymbolFunction());
    JSHandle<JSTaggedValue> symbolValue(symbol);
    JSHandle<JSPrimitiveRef> symbolRef = ecmaVM->GetFactory()->NewJSPrimitiveRef(symbolObject, symbolValue);

    auto otherEcmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    otherEcmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    otherEcmaRuntimeCallInfo->SetThis(symbolRef.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, otherEcmaRuntimeCallInfo);
    JSTaggedValue otherResult = BuiltinsSymbol::ValueOf(otherEcmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(otherResult.IsSymbol());
    ASSERT_EQ(otherResult.GetRawData() == (JSTaggedValue(*symbol)).GetRawData(), true);
}

// new Symbol().for
HWTEST_F_L0(BuiltinsSymbolTest, SymbolWithParameterFor)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();

    JSHandle<SymbolTable> tableHandle(env->GetRegisterSymbols());

    JSHandle<EcmaString> string = ecmaVM->GetFactory()->NewFromASCII("ccc");
    ASSERT_EQ(EcmaStringAccessor(string).GetLength(), 3U);
    JSHandle<JSTaggedValue> string_handle(string);
    ASSERT_EQ(tableHandle->ContainsKey(string_handle.GetTaggedValue()), false);

    JSHandle<JSSymbol> symbol = ecmaVM->GetFactory()->NewSymbolWithTableWithChar("ccc");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, string.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsSymbol::For(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(tableHandle->ContainsKey(string_handle.GetTaggedValue()), true);

    JSTaggedValue target(*symbol);
    ASSERT_EQ(result.GetRawData() == target.GetRawData(), true);
}

// Symbol.keyFor (sym)
HWTEST_F_L0(BuiltinsSymbolTest, SymbolKeyFor)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();

    JSHandle<JSSymbol> symbol = ecmaVM->GetFactory()->NewPublicSymbolWithChar("bbb");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, symbol.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsSymbol::KeyFor(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetRawData(), JSTaggedValue::VALUE_UNDEFINED);

    JSHandle<EcmaString> string = ecmaVM->GetFactory()->NewFromASCII("ccc");
    ASSERT_EQ(EcmaStringAccessor(string).GetLength(), 3U);

    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetCallArg(0, string.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    BuiltinsSymbol::For(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<JSSymbol> otherSymbol = ecmaVM->GetFactory()->NewPublicSymbolWithChar("ccc");
    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetCallArg(0, otherSymbol.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue otherResult = BuiltinsSymbol::KeyFor(ecmaRuntimeCallInfo2);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_TRUE(otherResult.IsString());
    JSHandle<SymbolTable> tableHandle(env->GetRegisterSymbols());
    JSTaggedValue stringValue(*string);
    ASSERT_EQ(tableHandle->ContainsKey(stringValue), true);

    // not symbol
    ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetCallArg(0, JSTaggedValue::Undefined());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    result = Symbol::KeyFor(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(thread->HasPendingException());
    EXPECT_EQ(result, JSTaggedValue::Exception());
    thread->ClearException();
}

// Symbol.ToPrimitive()
HWTEST_F_L0(BuiltinsSymbolTest, SymbolToPrimitive)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();

    JSHandle<JSSymbol> symbol = ecmaVM->GetFactory()->NewJSSymbol();

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(symbol.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsSymbol::ToPrimitive(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(result.IsSymbol());
    ASSERT_EQ(result.GetRawData() == (JSTaggedValue(*symbol)).GetRawData(), true);

    JSHandle<JSFunction> symbolObject(env->GetSymbolFunction());
    JSHandle<JSTaggedValue> symbolValue(symbol);
    JSHandle<JSPrimitiveRef> symbolRef = ecmaVM->GetFactory()->NewJSPrimitiveRef(symbolObject, symbolValue);

    auto otherEcmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    otherEcmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    otherEcmaRuntimeCallInfo->SetThis(symbolRef.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, otherEcmaRuntimeCallInfo);
    JSTaggedValue otherResult = BuiltinsSymbol::ToPrimitive(otherEcmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(otherResult.IsSymbol());
    ASSERT_EQ(otherResult.GetRawData() == (JSTaggedValue(*symbol)).GetRawData(), true);
}

// constructor
HWTEST_F_L0(BuiltinsSymbolTest, SymbolConstructor)
{
    auto ecmaVM = thread->GetEcmaVM();

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue::Undefined());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsSymbol::SymbolConstructor(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(result.IsSymbol());
    JSSymbol *sym = reinterpret_cast<JSSymbol *>(result.GetRawData());
    ASSERT_EQ(sym->GetDescription().IsUndefined(), true);

    JSHandle<EcmaString> string = ecmaVM->GetFactory()->NewFromASCII("ddd");

    auto otherEcmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    otherEcmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    otherEcmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    otherEcmaRuntimeCallInfo->SetCallArg(0, string.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, otherEcmaRuntimeCallInfo);
    JSTaggedValue result1 = BuiltinsSymbol::SymbolConstructor(otherEcmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    JSHandle<EcmaString> resultString = JSTaggedValue::ToString(
        thread, JSHandle<JSTaggedValue>(thread, reinterpret_cast<JSSymbol *>(result1.GetRawData())->GetDescription()));
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultString, *string), 0);
}

HWTEST_F_L0(BuiltinsSymbolTest, SymbolGetter)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();

    JSHandle<JSSymbol> symbol = ecmaVM->GetFactory()->NewPublicSymbolWithChar("");
    JSHandle<EcmaString> string = ecmaVM->GetFactory()->NewFromASCII("");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(symbol.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsSymbol::DescriptionGetter(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_TRUE(result.IsString());
    EcmaString *resString = reinterpret_cast<EcmaString *>(result.GetRawData());
    ASSERT_EQ(EcmaStringAccessor(resString).GetLength(), 0U);
    ASSERT_EQ(EcmaStringAccessor::StringsAreEqual(resString, *string), true);

    // value is not symbol
    JSHandle<JSFunction> symbolObject(env->GetSymbolFunction());
    JSHandle<JSTaggedValue> symbolValue(symbol);
    JSHandle<JSPrimitiveRef> symbolRef = ecmaVM->GetFactory()->NewJSPrimitiveRef(symbolObject, symbolValue);
    ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(symbolRef.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    result = BuiltinsSymbol::DescriptionGetter(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_TRUE(result.IsString());
    resString = reinterpret_cast<EcmaString *>(result.GetRawData());
    ASSERT_EQ(EcmaStringAccessor(resString).GetLength(), 0U);
    ASSERT_EQ(EcmaStringAccessor::StringsAreEqual(resString, *string), true);

    // Undefined
    ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    result = BuiltinsSymbol::DescriptionGetter(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(thread->HasPendingException());
    EXPECT_EQ(result, JSTaggedValue::Exception());
    thread->ClearException();
}
}  // namespace panda::test
