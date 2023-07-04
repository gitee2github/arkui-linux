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

#include "ecmascript/builtins/builtins_locale.h"

#include "ecmascript/global_env.h"
#include "ecmascript/js_locale.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::builtins;

namespace panda::test {
class BuiltinsLocaleTest : public testing::Test {
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

// new locale( [ options ] )
HWTEST_F_L0(BuiltinsLocaleTest, LocaleConstructor)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> newTarget(env->GetLocaleFunction());
    JSHandle<JSTaggedValue> objFun = env->GetObjectFunction();

    JSHandle<JSTaggedValue> languageKey = thread->GlobalConstants()->GetHandledLanguageString();
    JSHandle<JSTaggedValue> regionKey = thread->GlobalConstants()->GetHandledRegionString();
    JSHandle<JSTaggedValue> numericKey =  thread->GlobalConstants()->GetHandledNumericString();
    JSHandle<JSTaggedValue> scriptKey =  thread->GlobalConstants()->GetHandledScriptString();
    JSHandle<JSTaggedValue> languageValue(factory->NewFromASCII("cn"));
    JSHandle<JSTaggedValue> regionValue(factory->NewFromASCII("CN"));
    JSHandle<JSTaggedValue> scriptValue(factory->NewFromASCII("Chin"));
    JSHandle<JSTaggedValue> numericValue(thread, JSTaggedValue::True());
    JSHandle<JSTaggedValue> localeString(factory->NewFromASCII("zh-Hans-CN"));

    JSHandle<JSObject> optionsObj = factory->NewJSObjectByConstructor(JSHandle<JSFunction>(objFun), objFun);
    JSObject::SetProperty(thread, optionsObj, numericKey, numericValue);
    JSObject::SetProperty(thread, optionsObj, regionKey, regionValue);
    JSObject::SetProperty(thread, optionsObj, languageKey, languageValue);
    JSObject::SetProperty(thread, optionsObj, scriptKey, scriptValue);

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*newTarget), 8);
    ecmaRuntimeCallInfo->SetFunction(newTarget.GetTaggedValue());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, localeString.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, optionsObj.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::LocaleConstructor(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsJSLocale());
}

static JSTaggedValue JSLocaleCreateWithOptionTest(JSThread *thread)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> newTarget(env->GetLocaleFunction());
    JSHandle<JSTaggedValue> objFun = env->GetObjectFunction();

    JSHandle<JSTaggedValue> languageKey = thread->GlobalConstants()->GetHandledLanguageString();
    JSHandle<JSTaggedValue> regionKey = thread->GlobalConstants()->GetHandledRegionString();
    JSHandle<JSTaggedValue> scriptKey =  thread->GlobalConstants()->GetHandledScriptString();
    JSHandle<JSTaggedValue> languageValue(factory->NewFromASCII("en"));
    JSHandle<JSTaggedValue> regionValue(factory->NewFromASCII("US"));
    JSHandle<JSTaggedValue> scriptValue(factory->NewFromASCII("Latn"));
    JSHandle<JSTaggedValue> locale(factory->NewFromASCII("en-Latn-US"));
    // set option(language, region, script)
    JSHandle<JSObject> optionsObj = factory->NewJSObjectByConstructor(JSHandle<JSFunction>(objFun), objFun);
    JSObject::SetProperty(thread, optionsObj, languageKey, languageValue);
    JSObject::SetProperty(thread, optionsObj, regionKey, regionValue);
    JSObject::SetProperty(thread, optionsObj, scriptKey, scriptValue);

    JSHandle<JSTaggedValue> calendarValue(factory->NewFromASCII("chinese"));
    JSHandle<JSTaggedValue> calendarKey = thread->GlobalConstants()->GetHandledCalendarString();
    JSObject::SetProperty(thread, optionsObj, calendarKey, calendarValue); // test chinese calendar

    JSHandle<JSTaggedValue> hourCycleKey = thread->GlobalConstants()->GetHandledHourCycleString();
    JSHandle<JSTaggedValue> hourCycleValue(factory->NewFromASCII("h24"));
    JSObject::SetProperty(thread, optionsObj, hourCycleKey, hourCycleValue); // test h24

    JSHandle<JSTaggedValue> numericKey = thread->GlobalConstants()->GetHandledNumericString();
    JSHandle<JSTaggedValue> numericValue(factory->NewFromASCII("true"));
    JSObject::SetProperty(thread, optionsObj, numericKey, numericValue); // test true

    JSHandle<JSTaggedValue> numberingSystemKey = thread->GlobalConstants()->GetHandledNumberingSystemString();
    JSHandle<JSTaggedValue> numberingSystemValue(factory->NewFromASCII("mong"));
    JSObject::SetProperty(thread, optionsObj, numberingSystemKey, numberingSystemValue); // test mong

    JSHandle<JSTaggedValue> collationKey = thread->GlobalConstants()->GetHandledCollationString();
    JSHandle<JSTaggedValue> collationValue(factory->NewFromASCII("compat"));
    JSObject::SetProperty(thread, optionsObj, collationKey, collationValue); // test compat

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*newTarget), 8);
    ecmaRuntimeCallInfo->SetFunction(newTarget.GetTaggedValue());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, locale.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, optionsObj.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::LocaleConstructor(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    return result;
}

HWTEST_F_L0(BuiltinsLocaleTest, ToString)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(jsLocale.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::ToString(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    JSHandle<EcmaString> resultStr(
        factory->NewFromASCII("en-Latn-US-u-ca-chinese-co-compat-hc-h24-kn-nu-mong"));
    EXPECT_EQ(EcmaStringAccessor::Compare(*handleEcmaStr, *resultStr), 0);
}

HWTEST_F_L0(BuiltinsLocaleTest, GetBaseName)
{
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(jsLocale.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::GetBaseName(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("en-Latn-US", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

HWTEST_F_L0(BuiltinsLocaleTest, GetHourCycle)
{
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(jsLocale.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::GetHourCycle(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("h24", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

HWTEST_F_L0(BuiltinsLocaleTest, GetCalendar)
{
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(jsLocale.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::GetCalendar(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("chinese", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

HWTEST_F_L0(BuiltinsLocaleTest, GetCaseFirst)
{
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(jsLocale.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::GetCaseFirst(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("undefined", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

HWTEST_F_L0(BuiltinsLocaleTest, GetCollation)
{
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(jsLocale.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::GetCollation(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("compat", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

HWTEST_F_L0(BuiltinsLocaleTest, GetNumeric)
{
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(jsLocale.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::GetNumeric(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_EQ(result.GetRawData(), JSTaggedValue::True().GetRawData());
}

HWTEST_F_L0(BuiltinsLocaleTest, GetNumberingSystem)
{
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(jsLocale.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::GetNumberingSystem(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("mong", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

HWTEST_F_L0(BuiltinsLocaleTest, GetLanguage)
{
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(jsLocale.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::GetLanguage(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("en", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

HWTEST_F_L0(BuiltinsLocaleTest, GetScript)
{
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(jsLocale.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::GetScript(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("Latn", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

HWTEST_F_L0(BuiltinsLocaleTest, GetRegion)
{
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(jsLocale.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::GetRegion(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("US", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

static JSTaggedValue JSLocaleCreateWithOptionsTagsTest(JSThread *thread, JSHandle<JSTaggedValue> &value)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> newTarget(env->GetLocaleFunction());
    JSHandle<JSTaggedValue> objFun = env->GetObjectFunction();

    JSHandle<JSTaggedValue> languageKey = thread->GlobalConstants()->GetHandledLanguageString();
    JSHandle<JSTaggedValue> languageValue = JSHandle<JSTaggedValue>(value);
    JSHandle<EcmaString> locale = factory->NewFromASCII("zh");
    // set option(language)
    JSHandle<JSObject> optionsObj = factory->NewJSObjectByConstructor(JSHandle<JSFunction>(objFun), objFun);
    JSObject::SetProperty(thread, optionsObj, languageKey, languageValue);

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*newTarget), 8);
    ecmaRuntimeCallInfo->SetFunction(newTarget.GetTaggedValue());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, locale.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, optionsObj.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::LocaleConstructor(ecmaRuntimeCallInfo);
    EXPECT_TRUE(result.IsJSLocale());
    TestHelper::TearDownFrame(thread, prev);
    return result;
}

HWTEST_F_L0(BuiltinsLocaleTest, Maximize_001)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    // set language,then call Maximize function get language,script and region
    JSHandle<JSTaggedValue> languageValue(factory->NewFromASCII("zh"));
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionsTagsTest(thread, languageValue));

    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(jsLocale.GetTaggedValue());
    // test "zh" to "zh-Hans-CN"
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue resultObj = BuiltinsLocale::Maximize(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);
    JSHandle<JSLocale> resultLocale(thread, resultObj);

    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(resultLocale.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result = BuiltinsLocale::GetBaseName(ecmaRuntimeCallInfo2);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("zh-Hans-CN", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

HWTEST_F_L0(BuiltinsLocaleTest, Maximize_002)
{
    // set language,script,region and numeric and it's maximized
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(jsLocale.GetTaggedValue());
    // test "en-Latn-US" to "en-Latn-US"
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue resultObj = BuiltinsLocale::Maximize(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);
    JSHandle<JSLocale> resultLocale(thread, resultObj);

    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(resultLocale.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result = BuiltinsLocale::GetBaseName(ecmaRuntimeCallInfo2);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("en-Latn-US", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

HWTEST_F_L0(BuiltinsLocaleTest, Minimize_001)
{
    // set language, script, region, and it's maximized,then call Minimize function get language
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionTest(thread));

    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(jsLocale.GetTaggedValue());
    // test "en-Latn-US" to "en"
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue resultObj = BuiltinsLocale::Minimize(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);
    JSHandle<JSLocale> resultLocale(thread, resultObj);

    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(resultLocale.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result = BuiltinsLocale::GetBaseName(ecmaRuntimeCallInfo2);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("en", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

HWTEST_F_L0(BuiltinsLocaleTest, Minimize_002)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    // set language and it's minimized
    JSHandle<JSTaggedValue> languageValue(factory->NewFromASCII("zh"));
    JSHandle<JSLocale> jsLocale = JSHandle<JSLocale>(thread, JSLocaleCreateWithOptionsTagsTest(thread, languageValue));

    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(jsLocale.GetTaggedValue());
    // test "zh" to "zh"
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue resultObj = BuiltinsLocale::Minimize(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);
    JSHandle<JSLocale> resultLocale(thread, resultObj);

    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(resultLocale.GetTaggedValue());

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result = BuiltinsLocale::ToString(ecmaRuntimeCallInfo2);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsString());
    JSHandle<EcmaString> handleEcmaStr(thread, result);
    EXPECT_STREQ("zh", EcmaStringAccessor(handleEcmaStr).ToCString().c_str());
}

// test NormalizeKeywordValue(kf is not setted "yes")
HWTEST_F_L0(BuiltinsLocaleTest, NormalizeKeywordValue)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> newTarget(env->GetLocaleFunction());
    JSHandle<JSTaggedValue> objFun = env->GetObjectFunction();

    JSHandle<JSTaggedValue> caseFirstKey = thread->GlobalConstants()->GetHandledCaseFirstString();
    JSHandle<JSTaggedValue> caseFirstValue(factory->NewFromASCII("false"));
    JSHandle<EcmaString> locale = factory->NewFromASCII("en-US");

    JSHandle<JSObject> optionsObj = factory->NewJSObjectByConstructor(JSHandle<JSFunction>(objFun), objFun);
    JSObject::SetProperty(thread, optionsObj, caseFirstKey, caseFirstValue);

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*newTarget), 8);
    ecmaRuntimeCallInfo->SetFunction(newTarget.GetTaggedValue());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, locale.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, optionsObj.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsLocale::LocaleConstructor(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);

    JSHandle<JSLocale> jsInitLocale(thread, result);
    JSHandle<EcmaString> keyWords = JSLocale::NormalizeKeywordValue(thread, jsInitLocale, "kf");
    EXPECT_STREQ("false", EcmaStringAccessor(keyWords).ToCString().c_str());
}
}  // namespace panda::test
