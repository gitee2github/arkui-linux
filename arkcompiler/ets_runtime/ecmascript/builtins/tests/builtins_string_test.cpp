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

#include "ecmascript/builtins/builtins_string.h"

#include "ecmascript/base/builtins_base.h"
#include "ecmascript/builtins/builtins_regexp.h"
#include "ecmascript/ecma_runtime_call_info.h"
#include "ecmascript/ecma_string.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_primitive_ref.h"
#include "ecmascript/js_regexp.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::builtins;

namespace panda::test {
class BuiltinsStringTest : public testing::Test {
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
        JSRuntimeOptions options;
#if PANDA_TARGET_LINUX
        // for consistency requirement, use ohos_icu4j/data as icu-data-path
        options.SetIcuDataPath(ICU_PATH);
#endif
        options.SetEnableForceGC(true);
        instance = JSNApi::CreateEcmaVM(options);
        instance->SetEnableForceGC(true);
        ASSERT_TRUE(instance != nullptr) << "Cannot create EcmaVM";
        thread = instance->GetJSThread();
        scope = new EcmaHandleScope(thread);
    }

    void TearDown() override
    {
        TestHelper::DestroyEcmaVMWithScope(instance, scope);
    }

    EcmaVM *instance {nullptr};
    EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};
};

JSTaggedValue CreateBuiltinsStringRegExpObjByPatternAndFlags(JSThread *thread, const JSHandle<EcmaString> &pattern,
                                                             const JSHandle<EcmaString> &flags)
{
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> regexp(env->GetRegExpFunction());
    JSHandle<JSObject> globalObject(thread, env->GetGlobalObject());

    // 8 : test case
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, regexp.GetTaggedValue(), 8);
    ecmaRuntimeCallInfo->SetFunction(regexp.GetTaggedValue());
    ecmaRuntimeCallInfo->SetThis(globalObject.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, pattern.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, flags.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsRegExp::RegExpConstructor(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    return result;
}

HWTEST_F_L0(BuiltinsStringTest, StringConstructor1)
{
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();

    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSFunction> string(env->GetStringFunction());
    JSHandle<JSObject> globalObject(thread, env->GetGlobalObject());
    JSHandle<EcmaString> string2 = factory->NewFromASCII("ABC");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, string.GetTaggedValue(), 6);
    ecmaRuntimeCallInfo->SetFunction(string.GetTaggedValue());
    ecmaRuntimeCallInfo->SetThis(globalObject.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, string2.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::StringConstructor(ecmaRuntimeCallInfo);
    JSTaggedValue value(static_cast<JSTaggedType>(result.GetRawData()));
    ASSERT_TRUE(value.IsECMAObject());
    JSHandle<JSPrimitiveRef> ref(thread, JSPrimitiveRef::Cast(value.GetTaggedObject()));
    JSTaggedValue test = factory->NewFromASCII("ABC").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(EcmaString::Cast(ref->GetValue().GetTaggedObject()),
        reinterpret_cast<EcmaString *>(test.GetRawData())),
        0);
}

// String.fromCharCode(65, 66, 67)
HWTEST_F_L0(BuiltinsStringTest, fromCharCode1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    const double arg1 = 65;
    const double arg2 = 66;
    const double arg3 = 67;

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(arg1));
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(arg2));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(arg3));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::FromCharCode(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSTaggedValue value(static_cast<JSTaggedType>(result.GetRawData()));
    JSHandle<JSTaggedValue> valueHandle(thread, JSTaggedValue(value.GetTaggedObject()));
    JSTaggedValue test = factory->NewFromASCII("ABC").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(EcmaString::Cast(valueHandle->GetTaggedObject()),
        reinterpret_cast<EcmaString *>(test.GetRawData())),
        0);
}

// String.fromCodePoint(65, 66, 67)
HWTEST_F_L0(BuiltinsStringTest, fromCodePoint1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    const double arg1 = 65;
    const double arg2 = 66;
    const double arg3 = 67;

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(arg1));
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(arg2));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(arg3));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::FromCodePoint(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromASCII("ABC").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// "abcabcabc".charAt(5)
HWTEST_F_L0(BuiltinsStringTest, charAt1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisVal = factory->NewFromASCII("abcabcabc");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisVal.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<double>(5)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::CharAt(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromASCII("c").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// "一二三四".charAt(2)
HWTEST_F_L0(BuiltinsStringTest, charAt2)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisVal = factory->NewFromUtf8("一二三四");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisVal.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<double>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::CharAt(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromUtf8("三").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// "abcabcabc".charAt(-1)
HWTEST_F_L0(BuiltinsStringTest, charAt3)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisVal = factory->NewFromASCII("abcabcabc");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisVal.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<double>(-1)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::CharAt(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->GetEmptyString().GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// "ABC".charCodeAt(0)
HWTEST_F_L0(BuiltinsStringTest, charCodeAt1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisVal = factory->NewFromASCII("ABC");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisVal.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<double>(0)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::CharCodeAt(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue(65).GetRawData());
}

// "ABC".charCodeAt(-1)
HWTEST_F_L0(BuiltinsStringTest, charCodeAt2)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisVal = factory->NewFromASCII("ABC");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisVal.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<double>(-1)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::CharCodeAt(ecmaRuntimeCallInfo);

    JSTaggedValue test = BuiltinsString::GetTaggedDouble(base::NAN_VALUE);
    ASSERT_EQ(result.GetRawData(), test.GetRawData());
}

// "ABC".codePointAt(1)
HWTEST_F_L0(BuiltinsStringTest, codePointAt1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisVal = factory->NewFromASCII("ABC");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisVal.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<double>(1)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::CodePointAt(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue(66).GetRawData());
}

// 'a'.concat('b', 'c', 'd')
HWTEST_F_L0(BuiltinsStringTest, concat1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("a");
    JSHandle<EcmaString> val1 = factory->NewFromASCII("b");
    JSHandle<EcmaString> val2 = factory->NewFromASCII("c");
    JSHandle<EcmaString> val3 = factory->NewFromASCII("d");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val1.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, val2.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(2, val3.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Concat(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromASCII("abcd").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// "abcabcabc".indexof('b')
HWTEST_F_L0(BuiltinsStringTest, indexof1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abcabcabc");
    JSHandle<EcmaString> val = factory->NewFromASCII("b");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::IndexOf(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue(1).GetRawData());
}

// "abcabcabc".indexof('b', 2)
HWTEST_F_L0(BuiltinsStringTest, indexof2)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abcabcabc");
    JSHandle<EcmaString> val = factory->NewFromASCII("b");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<double>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::IndexOf(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue(4).GetRawData());
}

// "abcabcabc".indexof('d')
HWTEST_F_L0(BuiltinsStringTest, indexof3)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abcabcabc");
    JSHandle<EcmaString> val = factory->NewFromASCII("d");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::IndexOf(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue(-1).GetRawData());
}

// "abcabcabc".lastIndexOf('b')
HWTEST_F_L0(BuiltinsStringTest, lastIndexOf1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abcabcabc");
    JSHandle<EcmaString> val = factory->NewFromASCII("b");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::LastIndexOf(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue(7).GetRawData());
}
// "abcabcabc".lastIndexOf('b', 2)
HWTEST_F_L0(BuiltinsStringTest, lastIndexOf2)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abcabcabc");
    JSHandle<EcmaString> val = factory->NewFromASCII("b");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<double>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::LastIndexOf(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue(1).GetRawData());
}

// "abcabcabc".lastIndexOf('d')
HWTEST_F_L0(BuiltinsStringTest, lastIndexOf3)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abcabcabc");
    JSHandle<EcmaString> val = factory->NewFromASCII("d");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::LastIndexOf(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue(-1).GetRawData());
}

// "abcabcabc".includes('b')
HWTEST_F_L0(BuiltinsStringTest, Includes2)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abcabcabc");
    JSHandle<EcmaString> val = factory->NewFromASCII("b");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Includes(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue::True().GetRawData());
}

// "abccccccc".includes('b'，2)
HWTEST_F_L0(BuiltinsStringTest, Includes3)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abccccccc");
    JSHandle<EcmaString> val = factory->NewFromASCII("b");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<double>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Includes(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue::False().GetRawData());
}

// "一二三四".includes('二')
HWTEST_F_L0(BuiltinsStringTest, Includes4)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromUtf8("一二三四");
    JSHandle<EcmaString> val = factory->NewFromUtf8("二");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Includes(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue::True().GetRawData());
}

// "To be, or not to be, that is the question.".startsWith('To be')
HWTEST_F_L0(BuiltinsStringTest, startsWith1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("To be, or not to be, that is the question.");
    JSHandle<EcmaString> val = factory->NewFromASCII("To be");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::StartsWith(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue::True().GetRawData());
}

// "To be, or not to be, that is the question.".startsWith('not to be')
HWTEST_F_L0(BuiltinsStringTest, startsWith2)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("To be, or not to be, that is the question.");
    JSHandle<EcmaString> val = factory->NewFromASCII("not to be");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::StartsWith(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue::False().GetRawData());
}

// "To be, or not to be, that is the question.".startsWith('not to be', 10)
HWTEST_F_L0(BuiltinsStringTest, startsWith3)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("To be, or not to be, that is the question.");
    JSHandle<EcmaString> val = factory->NewFromASCII("not to be");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<double>(10)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::StartsWith(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue::True().GetRawData());
}

// "To be, or not to be, that is the question.".endsWith('question.')
HWTEST_F_L0(BuiltinsStringTest, endsWith1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("To be, or not to be, that is the question.");
    JSHandle<EcmaString> val = factory->NewFromASCII("question.");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::EndsWith(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue::True().GetRawData());
}

// "To be, or not to be, that is the question.".endsWith('to be')
HWTEST_F_L0(BuiltinsStringTest, endsWith2)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("To be, or not to be, that is the question.");
    JSHandle<EcmaString> val = factory->NewFromASCII("to be");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::EndsWith(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue::False().GetRawData());
}

// "To be, or not to be, that is the question.".endsWith('to be', 19)
HWTEST_F_L0(BuiltinsStringTest, endsWith3)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("To be, or not to be, that is the question.");
    JSHandle<EcmaString> val = factory->NewFromASCII("to be");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<double>(19)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::EndsWith(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue::True().GetRawData());
}

// "有ABC".toLocaleLowerCase()
HWTEST_F_L0(BuiltinsStringTest, toLocaleLowerCase2)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> this_str = factory->NewFromUtf8("有ABC");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(this_str.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::ToLocaleLowerCase(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> result_handle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromUtf8("有abc").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*result_handle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// "ABC".toLowerCase()
HWTEST_F_L0(BuiltinsStringTest, toLowerCase1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("ABC");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::ToLowerCase(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSHandle<EcmaString> test = factory->NewFromASCII("abc");
    ASSERT_TRUE(JSTaggedValue::SameValue(resultHandle.GetTaggedValue(), test.GetTaggedValue()));
}

// "abc".toUpperCase()
HWTEST_F_L0(BuiltinsStringTest, toUpperCase1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abc");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::ToUpperCase(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromASCII("ABC").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// "abc".localecompare('b')
HWTEST_F_L0(BuiltinsStringTest, localecompare1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abc");
    JSHandle<EcmaString> val = factory->NewFromASCII("b");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::LocaleCompare(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue(-1).GetRawData());
}

// "abc".localecompare('abc')
HWTEST_F_L0(BuiltinsStringTest, localecompare2)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abc");
    JSHandle<EcmaString> val = factory->NewFromASCII("abc");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::LocaleCompare(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue(0).GetRawData());
}

// "abc".localecompare('aa')
HWTEST_F_L0(BuiltinsStringTest, localecompare3)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abc");
    JSHandle<EcmaString> val = factory->NewFromASCII("aa");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::LocaleCompare(ecmaRuntimeCallInfo);

    ASSERT_EQ(result.GetRawData(), JSTaggedValue(1).GetRawData());
}

// "你好".localecompare('辅助')
HWTEST_F_L0(BuiltinsStringTest, localecompare4)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    std::string referenceStr = "你好";
    std::string compareStr = "辅助";
    JSHandle<EcmaString> thisStr = factory->NewFromStdString(referenceStr);
    JSHandle<EcmaString> val = factory->NewFromStdString(compareStr);
    JSHandle<EcmaString> locale = factory->NewFromASCII("zh-Hans");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, locale.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::LocaleCompare(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_GT(result.GetRawData(), JSTaggedValue(0).GetRawData());
}

// "abc".normalize('NFC')
HWTEST_F_L0(BuiltinsStringTest, normalize1)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> this_str = factory->NewFromASCII("abc");
    JSHandle<EcmaString> val = factory->NewFromASCII("NFC");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(this_str.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, val.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Normalize(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> result_handle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromASCII("abc").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*result_handle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// "abc".repeat(5)
HWTEST_F_L0(BuiltinsStringTest, repeat1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abc");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<double>(5)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Repeat(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromASCII("abcabcabcabcabc").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// 'The morning is upon us.'.slice(4, -2)
HWTEST_F_L0(BuiltinsStringTest, slice1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("The morning is upon us.");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<double>(4)));
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<double>(-2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Slice(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromASCII("morning is upon u").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// 'The morning is upon us.'.slice(12)
HWTEST_F_L0(BuiltinsStringTest, slice2)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("The morning is upon us.");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<double>(12)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Slice(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromASCII("is upon us.").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// 'Mozilla'.substring(3, -3)
HWTEST_F_L0(BuiltinsStringTest, substring1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("Mozilla");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<double>(3)));
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<double>(-3)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Substring(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromASCII("Moz").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// 'Mozilla'.substring(7, 4)
HWTEST_F_L0(BuiltinsStringTest, substring2)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("Mozilla");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<double>(7)));
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<double>(4)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Substring(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromASCII("lla").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

// "   Hello world!   ".trim()
HWTEST_F_L0(BuiltinsStringTest, trim1)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("   Hello world!   ");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Trim(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromASCII("Hello world!").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

HWTEST_F_L0(BuiltinsStringTest, trim2)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();

    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("   Hello world!   ");
    JSHandle<JSFunction> stringObject(env->GetStringFunction());
    JSHandle<JSTaggedValue> value(thread, JSTaggedValue(thisStr.GetTaggedValue().GetTaggedObject()));
    JSHandle<JSPrimitiveRef> str = factory->NewJSPrimitiveRef(stringObject, value);

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(str.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Trim(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = factory->NewFromASCII("Hello world!").GetTaggedValue();
    ASSERT_EQ(EcmaStringAccessor::Compare(*resultHandle, reinterpret_cast<EcmaString *>(test.GetRawData())), 0);
}

HWTEST_F_L0(BuiltinsStringTest, ToString)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();

    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abcabcabc");
    JSHandle<JSFunction> stringObject(env->GetStringFunction());
    JSHandle<JSTaggedValue> value(thread, JSTaggedValue(thisStr.GetTaggedValue().GetTaggedObject()));
    JSHandle<JSPrimitiveRef> str = factory->NewJSPrimitiveRef(stringObject, value);

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(str.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::ToString(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = JSTaggedValue(*thisStr);
    ASSERT_EQ(result.GetRawData(), test.GetRawData());
}

HWTEST_F_L0(BuiltinsStringTest, ValueOf)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();

    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("abcabcabc");
    JSHandle<JSFunction> stringObject(env->GetStringFunction());
    JSHandle<JSTaggedValue> value(thread, JSTaggedValue(thisStr.GetTaggedValue().GetTaggedObject()));
    JSHandle<JSPrimitiveRef> str = factory->NewJSPrimitiveRef(stringObject, value);

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(str.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::ValueOf(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    JSTaggedValue test = JSTaggedValue(*thisStr);
    ASSERT_EQ(result.GetRawData(), test.GetRawData());
}

static inline JSFunction *BuiltinsStringTestCreate(JSThread *thread)
{
    EcmaVM *ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> globalEnv = ecmaVM->GetGlobalEnv();
    return globalEnv->GetObjectFunction().GetObject<JSFunction>();
}

HWTEST_F_L0(BuiltinsStringTest, Raw)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSTaggedValue> foo(factory->NewFromASCII("foo"));
    JSHandle<JSTaggedValue> bar(factory->NewFromASCII("bar"));
    JSHandle<JSTaggedValue> baz(factory->NewFromASCII("baz"));
    JSHandle<JSTaggedValue> rawArray = JSHandle<JSTaggedValue>::Cast(JSArray::ArrayCreate(thread, JSTaggedNumber(0)));
    JSHandle<JSObject> obj(rawArray);
    JSHandle<JSTaggedValue> key0(thread, JSTaggedValue(0));
    PropertyDescriptor desc0(thread, foo);
    JSArray::DefineOwnProperty(thread, obj, key0, desc0);
    JSHandle<JSTaggedValue> key1(thread, JSTaggedValue(1));
    PropertyDescriptor desc1(thread, bar);
    JSArray::DefineOwnProperty(thread, obj, key1, desc1);
    JSHandle<JSTaggedValue> key2(thread, JSTaggedValue(2));
    PropertyDescriptor desc2(thread, baz);
    JSArray::DefineOwnProperty(thread, obj, key2, desc2);

    JSHandle<JSTaggedValue> constructor(thread, BuiltinsStringTestCreate(thread));
    JSHandle<JSTaggedValue> templateString(
        factory->NewJSObjectByConstructor(JSHandle<JSFunction>(constructor), constructor));
    JSHandle<JSTaggedValue> rawKey(factory->NewFromASCII("raw"));
    JSObject::SetProperty(thread, templateString, rawKey, rawArray);
    JSHandle<EcmaString> test = factory->NewFromASCII("foo5barJavaScriptbaz");

    JSHandle<EcmaString> javascript = factory->NewFromASCII("JavaScript");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(templateString.GetObject<EcmaString>()));
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(5)));
    ecmaRuntimeCallInfo->SetCallArg(2, javascript.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Raw(ecmaRuntimeCallInfo);
    ASSERT_TRUE(result.IsString());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(reinterpret_cast<EcmaString *>(result.GetRawData()), *test));
}

HWTEST_F_L0(BuiltinsStringTest, Replace)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("Twas the night before Xmas...");
    JSHandle<EcmaString> searchStr = factory->NewFromASCII("Xmas");
    JSHandle<EcmaString> replaceStr = factory->NewFromASCII("Christmas");
    JSHandle<EcmaString> expected = factory->NewFromASCII("Twas the night before Christmas...");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, searchStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, replaceStr.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Replace(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_TRUE(result.IsString());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(reinterpret_cast<EcmaString *>(result.GetRawData()), *expected));

    JSHandle<EcmaString> replaceStr1 = factory->NewFromASCII("abc$$");
    JSHandle<EcmaString> expected1 = factory->NewFromASCII("Twas the night before abc$...");

    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, searchStr.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(1, replaceStr1.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result1 = BuiltinsString::Replace(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<EcmaString> resultString1(thread, result1);
    ASSERT_TRUE(result1.IsString());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*resultString1, *expected1));

    JSHandle<EcmaString> replaceStr2 = factory->NewFromASCII("abc$$dd");
    JSHandle<EcmaString> expected2 = factory->NewFromASCII("Twas the night before abc$dd...");

    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, searchStr.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(1, replaceStr2.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result2 = BuiltinsString::Replace(ecmaRuntimeCallInfo2);
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<EcmaString> resultString2(thread, result2);
    ASSERT_TRUE(result2.IsString());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*resultString2, *expected2));

    JSHandle<EcmaString> replaceStr3 = factory->NewFromASCII("abc$&dd");
    JSHandle<EcmaString> expected3 = factory->NewFromASCII("Twas the night before abcXmasdd...");

    auto ecmaRuntimeCallInfo3 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo3->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo3->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo3->SetCallArg(0, searchStr.GetTaggedValue());
    ecmaRuntimeCallInfo3->SetCallArg(1, replaceStr3.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo3);
    JSTaggedValue result3 = BuiltinsString::Replace(ecmaRuntimeCallInfo3);
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<EcmaString> resultString3(thread, result3);
    ASSERT_TRUE(result3.IsString());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*resultString3, *expected3));

    JSHandle<EcmaString> replaceStr4 = factory->NewFromASCII("abc$`dd");
    JSHandle<EcmaString> expected4 =
        factory->NewFromASCII("Twas the night before abcTwas the night before dd...");

    auto ecmaRuntimeCallInfo4 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo4->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo4->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo4->SetCallArg(0, searchStr.GetTaggedValue());
    ecmaRuntimeCallInfo4->SetCallArg(1, replaceStr4.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo4);
    JSTaggedValue result4 = BuiltinsString::Replace(ecmaRuntimeCallInfo4);
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<EcmaString> resultString4(thread, result4);
    ASSERT_TRUE(result4.IsString());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*resultString4, *expected4));
}

HWTEST_F_L0(BuiltinsStringTest, Replace2)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("Twas the night before Xmas...");
    JSHandle<EcmaString> searchStr = factory->NewFromASCII("Xmas");
    JSHandle<EcmaString> replaceStr = factory->NewFromASCII("abc$\'dd");
    JSHandle<EcmaString> expected = factory->NewFromASCII("Twas the night before abc...dd...");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, searchStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, replaceStr.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Replace(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_TRUE(result.IsString());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(reinterpret_cast<EcmaString *>(result.GetRawData()), *expected));

    JSHandle<EcmaString> replaceStr2 = factory->NewFromASCII("abc$`dd$\'$ff");
    JSHandle<EcmaString> expected2 =
        factory->NewFromASCII("Twas the night before abcTwas the night before dd...$ff...");

    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, searchStr.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(1, replaceStr2.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result2 = BuiltinsString::Replace(ecmaRuntimeCallInfo2);
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<EcmaString> resultString2(thread, result2);
    ASSERT_TRUE(result2.IsString());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*resultString2, *expected2));

    JSHandle<EcmaString> replaceStr3 = factory->NewFromASCII("abc$`dd$\'$");
    JSHandle<EcmaString> expected3 =
        factory->NewFromASCII("Twas the night before abcTwas the night before dd...$...");

    auto ecmaRuntimeCallInfo3 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo3->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo3->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo3->SetCallArg(0, searchStr.GetTaggedValue());
    ecmaRuntimeCallInfo3->SetCallArg(1, replaceStr3.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo3);
    JSTaggedValue result3 = BuiltinsString::Replace(ecmaRuntimeCallInfo3);
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<EcmaString> resultString3(thread, result3);
    ASSERT_TRUE(result3.IsString());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*resultString3, *expected3));

    JSHandle<EcmaString> replaceStr4 = factory->NewFromASCII("abc$`dd$$");
    JSHandle<EcmaString> expected4 =
        factory->NewFromASCII("Twas the night before abcTwas the night before dd$...");

    auto ecmaRuntimeCallInfo4 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo4->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo4->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo4->SetCallArg(0, searchStr.GetTaggedValue());
    ecmaRuntimeCallInfo4->SetCallArg(1, replaceStr4.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo4);
    JSTaggedValue result4 = BuiltinsString::Replace(ecmaRuntimeCallInfo4);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_TRUE(result4.IsString());
    JSHandle<EcmaString> resultString4(thread, result4);
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*resultString4, *expected4));
}

HWTEST_F_L0(BuiltinsStringTest, Replace3)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("Twas the night before Xmas...");
    JSHandle<EcmaString> searchStr = factory->NewFromASCII("Xmas");
    JSHandle<EcmaString> replaceStr = factory->NewFromASCII("$&a $` $\' $2 $01 $$1 $21 $32 a");
    JSHandle<EcmaString> expected = factory->NewFromASCII(
        "Twas the night before Xmasa Twas the night before  ... $2 $01 $1 $21 $32 a...");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, searchStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, replaceStr.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Replace(ecmaRuntimeCallInfo);

    ASSERT_TRUE(result.IsString());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(reinterpret_cast<EcmaString *>(result.GetRawData()), *expected));
}

HWTEST_F_L0(BuiltinsStringTest, Replace4)
{
    // invoke RegExpConstructor method
    JSHandle<EcmaString> pattern1 =
        thread->GetEcmaVM()->GetFactory()->NewFromASCII("quick\\s(brown).+?(jumps)");
    JSHandle<EcmaString> flags1 = thread->GetEcmaVM()->GetFactory()->NewFromASCII("iug");
    JSTaggedValue result1 = CreateBuiltinsStringRegExpObjByPatternAndFlags(thread, pattern1, flags1);
    JSHandle<JSRegExp> searchStr(thread, reinterpret_cast<JSRegExp *>(result1.GetRawData()));
    JSHandle<EcmaString> expected = thread->GetEcmaVM()->GetFactory()->NewFromASCII(
        "The Quick Brown Fox Jumpsa The   Over The Lazy Dog Jumps Brown $1 Jumps1 $32 a Over The Lazy Dog");

    // make ecma_runtime_call_info2
    JSHandle<EcmaString> thisStr =
        thread->GetEcmaVM()->GetFactory()->NewFromASCII("The Quick Brown Fox Jumps Over The Lazy Dog");
    JSHandle<EcmaString> replaceStr =
        thread->GetEcmaVM()->GetFactory()->NewFromASCII("$&a $` $\' $2 $01 $$1 $21 $32 a");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, searchStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, replaceStr.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Replace(ecmaRuntimeCallInfo);

    ASSERT_TRUE(result.IsString());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(reinterpret_cast<EcmaString *>(result.GetRawData()), *expected));
}

HWTEST_F_L0(BuiltinsStringTest, Split)
{
    // invoke RegExpConstructor method
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("Hello World. How are you doing?");
    JSHandle<EcmaString> separatorStr = factory->NewFromASCII(" ");
    JSHandle<JSTaggedValue> limit(thread, JSTaggedValue(3));
    JSHandle<EcmaString> expected1 = factory->NewFromASCII("Hello");
    JSHandle<EcmaString> expected2 = factory->NewFromASCII("World.");
    JSHandle<EcmaString> expected3 = factory->NewFromASCII("How");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, separatorStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(3)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Split(ecmaRuntimeCallInfo);

    ASSERT_TRUE(result.IsECMAObject());
    JSHandle<JSArray> resultArray(thread, reinterpret_cast<JSArray *>(result.GetRawData()));
    ASSERT_TRUE(resultArray->IsJSArray());
    JSHandle<JSTaggedValue> resultObj(resultArray);
    JSHandle<EcmaString> string1(
        JSObject::GetProperty(thread, resultObj, JSHandle<JSTaggedValue>(thread, JSTaggedValue(0))).GetValue());
    JSHandle<EcmaString> string2(
        JSObject::GetProperty(thread, resultObj, JSHandle<JSTaggedValue>(thread, JSTaggedValue(1))).GetValue());
    JSHandle<EcmaString> string3(
        JSObject::GetProperty(thread, resultObj, JSHandle<JSTaggedValue>(thread, JSTaggedValue(2))).GetValue());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*string1, *expected1));
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*string2, *expected2));
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*string3, *expected3));
}

HWTEST_F_L0(BuiltinsStringTest, Split2)
{
    // invoke RegExpConstructor method
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<EcmaString> thisStr = factory->NewFromASCII("a-b-c");
    JSHandle<EcmaString> pattern1 = factory->NewFromASCII("-");
    JSHandle<EcmaString> flags1 = factory->NewFromASCII("iug");
    JSTaggedValue result1 = CreateBuiltinsStringRegExpObjByPatternAndFlags(thread, pattern1, flags1);
    JSHandle<JSRegExp> separatorObj(thread, result1);

    JSHandle<JSTaggedValue> limit(thread, JSTaggedValue(3));
    JSHandle<EcmaString> expected1 = factory->NewFromASCII("a");
    JSHandle<EcmaString> expected2 = factory->NewFromASCII("b");
    JSHandle<EcmaString> expected3 = factory->NewFromASCII("c");

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(thisStr.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, separatorObj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(3)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsString::Split(ecmaRuntimeCallInfo);

    ASSERT_TRUE(result.IsECMAObject());
    JSHandle<JSArray> resultArray(thread, result);
    ASSERT_TRUE(resultArray->IsJSArray());
    JSHandle<JSTaggedValue> resultObj(resultArray);
    JSHandle<EcmaString> string1(
        JSObject::GetProperty(thread, resultObj, JSHandle<JSTaggedValue>(thread, JSTaggedValue(0))).GetValue());
    JSHandle<EcmaString> string2(
        JSObject::GetProperty(thread, resultObj, JSHandle<JSTaggedValue>(thread, JSTaggedValue(1))).GetValue());
    JSHandle<EcmaString> string3(
        JSObject::GetProperty(thread, resultObj, JSHandle<JSTaggedValue>(thread, JSTaggedValue(2))).GetValue());
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*string1, *expected1));
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*string2, *expected2));
    ASSERT_TRUE(EcmaStringAccessor::StringsAreEqual(*string3, *expected3));
}
}  // namespace panda::test
