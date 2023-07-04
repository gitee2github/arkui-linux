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

#include "ecmascript/builtins/builtins_collator.h"

#include "ecmascript/builtins/builtins_array.h"
#include "ecmascript/global_env.h"
#include "ecmascript/js_collator.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::builtins;

namespace panda::test {
using BuiltinsArray = ecmascript::builtins::BuiltinsArray;
class BuiltinsCollatorTest : public testing::Test {
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

HWTEST_F_L0(BuiltinsCollatorTest, CollatorConstructor)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> newTarget(env->GetCollatorFunction());
    JSHandle<JSTaggedValue> objFun = env->GetObjectFunction();

    JSHandle<JSTaggedValue> usageKey = thread->GlobalConstants()->GetHandledUsageString();
    JSHandle<JSTaggedValue> localeMatcherKey = thread->GlobalConstants()->GetHandledLocaleMatcherString();
    JSHandle<JSTaggedValue> numericKey = thread->GlobalConstants()->GetHandledNumericString();
    JSHandle<JSTaggedValue> caseFirstKey = thread->GlobalConstants()->GetHandledCaseFirstString();
    JSHandle<JSTaggedValue> sensitivityKey = thread->GlobalConstants()->GetHandledSensitivityString();
    JSHandle<JSTaggedValue> ignorePunctuationKey = thread->GlobalConstants()->GetHandledIgnorePunctuationString();

    JSHandle<JSTaggedValue> usageValue(factory->NewFromASCII("search"));
    JSHandle<JSTaggedValue> localeMatcherValue(factory->NewFromASCII("lookup"));
    JSHandle<JSTaggedValue> numericValue(factory->NewFromASCII("true"));
    JSHandle<JSTaggedValue> caseFirstValue(factory->NewFromASCII("lower"));
    JSHandle<JSTaggedValue> sensitivityValue(factory->NewFromASCII("variant"));
    JSHandle<JSTaggedValue> ignorePunctuationValue(factory->NewFromASCII("true"));
    JSHandle<JSTaggedValue> localesString(factory->NewFromASCII("en-Latn-US"));

    JSHandle<JSObject> optionsObj = factory->NewJSObjectByConstructor(JSHandle<JSFunction>(objFun), objFun);
    JSObject::SetProperty(thread, optionsObj, usageKey, usageValue);
    JSObject::SetProperty(thread, optionsObj, localeMatcherKey, localeMatcherValue);
    JSObject::SetProperty(thread, optionsObj, caseFirstKey, caseFirstValue);
    JSObject::SetProperty(thread, optionsObj, sensitivityKey, sensitivityValue);
    JSObject::SetProperty(thread, optionsObj, ignorePunctuationKey, ignorePunctuationValue);
    JSObject::SetProperty(thread, optionsObj, numericKey, numericValue);

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*newTarget), 8);
    ecmaRuntimeCallInfo->SetFunction(newTarget.GetTaggedValue());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, localesString.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, optionsObj.GetTaggedValue()); // set option tag

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsCollator::CollatorConstructor(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsJSCollator());
}

static JSTaggedValue JSCollatorCreateWithLocaleTest(JSThread *thread, JSHandle<JSTaggedValue> &locale)
{
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> newTarget(env->GetCollatorFunction());

    JSHandle<JSTaggedValue> localesString = locale;
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*newTarget), 8);
    ecmaRuntimeCallInfo->SetFunction(newTarget.GetTaggedValue());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, localesString.GetTaggedValue());
    // set no options
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue::Undefined());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsCollator::CollatorConstructor(ecmaRuntimeCallInfo);
    EXPECT_TRUE(result.IsJSCollator());
    TestHelper::TearDownFrame(thread, prev);
    return result;
}

static JSTaggedValue JSCollatorCreateWithLocaleAndOptionsTest(JSThread *thread, JSHandle<JSTaggedValue> &locale)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> newTarget(env->GetCollatorFunction());
    JSHandle<JSTaggedValue> objFun = env->GetObjectFunction();

    JSHandle<JSTaggedValue> localesString = locale;
    JSHandle<JSTaggedValue> usageKey = thread->GlobalConstants()->GetHandledUsageString();
    JSHandle<JSTaggedValue> sensitivityKey = thread->GlobalConstants()->GetHandledSensitivityString();
    JSHandle<JSTaggedValue> usageValue(factory->NewFromASCII("search"));
    JSHandle<JSTaggedValue> sensitivityValue(factory->NewFromASCII("base"));

    JSHandle<JSObject> optionsObj = factory->NewJSObjectByConstructor(JSHandle<JSFunction>(objFun), objFun);
    JSObject::SetProperty(thread, optionsObj, usageKey, usageValue);
    JSObject::SetProperty(thread, optionsObj, sensitivityKey, sensitivityValue);

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*newTarget), 8);
    ecmaRuntimeCallInfo->SetFunction(newTarget.GetTaggedValue());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, localesString.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, optionsObj.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsCollator::CollatorConstructor(ecmaRuntimeCallInfo);
    EXPECT_TRUE(result.IsJSCollator());
    TestHelper::TearDownFrame(thread, prev);
    return result;
}

// compare with sort(de)
HWTEST_F_L0(BuiltinsCollatorTest, Compare_001)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSTaggedValue> locale(factory->NewFromASCII("de"));
    JSHandle<JSCollator> jsCollator = JSHandle<JSCollator>(thread, JSCollatorCreateWithLocaleTest(thread, locale));

    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(jsCollator.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result1 = BuiltinsCollator::Compare(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);
    JSHandle<JSFunction> jsFunction(thread, result1);

    JSArray *jsArray =
        JSArray::Cast(JSArray::ArrayCreate(thread, JSTaggedNumber(0)).GetTaggedValue().GetTaggedObject());
    JSHandle<JSObject> jsObject(thread, jsArray);

    JSHandle<JSTaggedValue> key0(thread, JSTaggedValue(0));
    JSHandle<JSTaggedValue> value0(factory->NewFromASCII("Z"));
    PropertyDescriptor desc0(thread, value0, true, true, true);
    JSArray::DefineOwnProperty(thread, jsObject, key0, desc0);
    JSHandle<JSTaggedValue> key1(thread, JSTaggedValue(1));
    JSHandle<JSTaggedValue> value1(factory->NewFromASCII("a"));
    PropertyDescriptor desc1(thread, value1, true, true, true);
    JSArray::DefineOwnProperty(thread, jsObject, key1, desc1);
    JSHandle<JSTaggedValue> key2(thread, JSTaggedValue(2));
    JSHandle<JSTaggedValue> value2(factory->NewFromUtf8("ä"));
    PropertyDescriptor desc2(thread, value2, true, true, true);
    JSArray::DefineOwnProperty(thread, jsObject, key2, desc2);

    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(jsObject.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, jsFunction.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result2 = BuiltinsArray::Sort(ecmaRuntimeCallInfo2); // sort in language(de)
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<JSTaggedValue> resultArr =
        JSHandle<JSTaggedValue>(thread, JSTaggedValue(static_cast<JSTaggedType>(result2.GetRawData())));
    EXPECT_EQ(JSTaggedValue::SameValue(JSArray::GetProperty(thread, resultArr, key0).GetValue(), value1), true);
    EXPECT_EQ(JSTaggedValue::SameValue(JSArray::GetProperty(thread, resultArr, key1).GetValue(), value2), true);
    EXPECT_EQ(JSTaggedValue::SameValue(JSArray::GetProperty(thread, resultArr, key2).GetValue(), value0), true);
}

// // compare with sort(sv)
HWTEST_F_L0(BuiltinsCollatorTest, Compare_002)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSTaggedValue> locale(factory->NewFromASCII("sv"));
    JSHandle<JSCollator> jsCollator = JSHandle<JSCollator>(thread, JSCollatorCreateWithLocaleTest(thread, locale));

    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(jsCollator.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result1 = BuiltinsCollator::Compare(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<JSFunction> jsFunction(thread, result1);
    JSArray *jsArray =
        JSArray::Cast(JSArray::ArrayCreate(thread, JSTaggedNumber(0)).GetTaggedValue().GetTaggedObject());
    JSHandle<JSObject> jsObject(thread, jsArray);

    JSHandle<JSTaggedValue> key0(thread, JSTaggedValue(0));
    JSHandle<JSTaggedValue> value0(factory->NewFromASCII("Z"));
    PropertyDescriptor desc0(thread, value0, true, true, true);
    JSArray::DefineOwnProperty(thread, jsObject, key0, desc0);
    JSHandle<JSTaggedValue> key1(thread, JSTaggedValue(1));
    JSHandle<JSTaggedValue> value1(factory->NewFromASCII("a"));
    PropertyDescriptor desc1(thread, value1, true, true, true);
    JSArray::DefineOwnProperty(thread, jsObject, key1, desc1);
    JSHandle<JSTaggedValue> key2(thread, JSTaggedValue(2));
    JSHandle<JSTaggedValue> value2(factory->NewFromUtf8("ä"));
    PropertyDescriptor desc2(thread, value2, true, true, true);
    JSArray::DefineOwnProperty(thread, jsObject, key2, desc2);

    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(jsObject.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, jsFunction.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result2 = BuiltinsArray::Sort(ecmaRuntimeCallInfo2); // sort in language(sv)
    TestHelper::TearDownFrame(thread, prev);
    JSHandle<JSObject> resultObj(thread, result2);

    JSHandle<EcmaString> str = thread->GetEcmaVM()->GetFactory()->NewFromUtf8("a,Z,ä");
    auto ecmaRuntimeCallInfo3 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo3->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo3->SetThis(resultObj.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo3);
    JSTaggedValue result = BuiltinsArray::Join(ecmaRuntimeCallInfo3);
    TestHelper::TearDownFrame(thread, prev);
    JSHandle<EcmaString> resultHandle(thread, reinterpret_cast<EcmaString *>(result.GetRawData()));
    EXPECT_EQ(EcmaStringAccessor::Compare(*resultHandle, *str), 0);
}

// compare with options("search")
HWTEST_F_L0(BuiltinsCollatorTest, Compare_003)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSTaggedValue> locale(factory->NewFromASCII("sv"));
    JSHandle<JSCollator> jsCollator =
        JSHandle<JSCollator>(thread, JSCollatorCreateWithLocaleAndOptionsTest(thread, locale));

    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(jsCollator.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result1 = BuiltinsCollator::Compare(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<JSFunction> jsFunction(thread, result1);
    JSArray *jsArray =
        JSArray::Cast(JSArray::ArrayCreate(thread, JSTaggedNumber(0)).GetTaggedValue().GetTaggedObject());
    JSHandle<JSObject> jsObject(thread, jsArray);

    JSHandle<JSTaggedValue> value0(factory->NewFromUtf8("Congrès"));
    JSHandle<JSTaggedValue> value1(factory->NewFromASCII("congres"));
    PropertyDescriptor desc(thread, JSHandle<JSTaggedValue>(jsFunction), true, true, true);
    JSHandle<JSTaggedValue> joinKey(factory->NewFromASCII("join"));
    JSArray::DefineOwnProperty(thread, jsObject, joinKey, desc);

    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(jsObject.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, value0.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(1, value1.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result2 = BuiltinsArray::ToString(ecmaRuntimeCallInfo2); // search in language(sv)
    TestHelper::TearDownFrame(thread, prev);
    JSHandle<JSTaggedValue> resultHandle(thread, result2);
    EXPECT_EQ(resultHandle->GetInt(), 0); // Congrès and congres is matching
}

HWTEST_F_L0(BuiltinsCollatorTest, ResolvedOptions)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    auto globalConst = thread->GlobalConstants();
    JSHandle<JSTaggedValue> locale(factory->NewFromASCII("de"));
    JSHandle<JSCollator> jsCollator = JSHandle<JSCollator>(thread, JSCollatorCreateWithLocaleTest(thread, locale));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(jsCollator.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsCollator::ResolvedOptions(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<JSTaggedValue> resultObj =
        JSHandle<JSTaggedValue>(thread, JSTaggedValue(static_cast<JSTaggedType>(result.GetRawData())));
    // judge whether the properties of the object are the same as those of jscollator tag
    JSHandle<JSTaggedValue> localeKey = globalConst->GetHandledLocaleString();
    EXPECT_EQ(JSTaggedValue::SameValue(JSObject::GetProperty(thread, resultObj, localeKey).GetValue(), locale), true);
    JSHandle<JSTaggedValue> usageKey = globalConst->GetHandledUsageString();
    JSHandle<JSTaggedValue> defaultUsageValue(factory->NewFromASCII("sort"));
    EXPECT_EQ(JSTaggedValue::SameValue(
        JSObject::GetProperty(thread, resultObj, usageKey).GetValue(), defaultUsageValue), true);
    JSHandle<JSTaggedValue> handledCaseFirstKey = globalConst->GetHandledCaseFirstString();
    JSHandle<JSTaggedValue> handledCaseFirstValue(factory->NewFromASCII("upper"));
    EXPECT_EQ(JSTaggedValue::SameValue(JSObject::GetProperty(thread, resultObj, handledCaseFirstKey).GetValue(),
                                       handledCaseFirstValue), true);
}

HWTEST_F_L0(BuiltinsCollatorTest, SupportedLocalesOf)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSTaggedValue> objFun = env->GetObjectFunction();

    JSHandle<JSTaggedValue> localeMatcherKey = thread->GlobalConstants()->GetHandledLocaleMatcherString();
    JSHandle<JSTaggedValue> localeMatcherValue(factory->NewFromASCII("lookup"));
    JSHandle<JSObject> optionsObj = factory->NewJSObjectByConstructor(JSHandle<JSFunction>(objFun), objFun);
    JSObject::SetProperty(thread, optionsObj, localeMatcherKey, localeMatcherValue);
    JSHandle<JSTaggedValue> locale(factory->NewFromASCII("id-u-co-pinyin-de-ID"));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, locale.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, optionsObj.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue resultArr = BuiltinsCollator::SupportedLocalesOf(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<JSArray> resultHandle(thread, resultArr);
    JSHandle<TaggedArray> elements(thread, resultHandle->GetElements());
    EXPECT_EQ(elements->GetLength(), 1U);
    JSHandle<EcmaString> handleEcmaStr(thread, elements->Get(0));
    EXPECT_STREQ("id-u-co-pinyin-de-id", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}
}  // namespace panda::test
