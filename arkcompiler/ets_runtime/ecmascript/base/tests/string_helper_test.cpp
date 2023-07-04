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

#include "ecmascript/base/string_helper.h"
#include "ecmascript/base/utf_helper.h"
#include "ecmascript/global_env.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::base;
using namespace panda::ecmascript::base::utf_helper;

namespace panda::test {
class StringHelperTest : public testing::Test {
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

HWTEST_F_L0(StringHelperTest, RepalceAll)
{
    CString sourceStr = "H\\e\\l\\l\\o\\W\\o\\r\\l\\d!\0";
    const CString oldValue1 = "\\";
    const CString oldValue2 = "World";
    CString result = StringHelper::RepalceAll(sourceStr, oldValue1, "");
    result = StringHelper::RepalceAll(result, oldValue2, " OpenHarmony");
    EXPECT_STREQ(result.c_str(), "Hello OpenHarmony!");
}

HWTEST_F_L0(StringHelperTest, Utf16ToU16String_Utf8ToU16String)
{
    const uint32_t utf16DataLen1 = 11;
    const uint16_t utf16Data1[utf16DataLen1] = { // "OpenHarmony"
        0x4f, 0x70, 0x65, 0x6e,
        0x48, 0x61, 0x72, 0x6d,
        0x6f, 0x6e, 0x79
    };
    std::u16string u16Str1 = StringHelper::Utf16ToU16String(utf16Data1, utf16DataLen1);
    std::string u16Value1 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16Str1);
    EXPECT_STREQ(u16Value1.c_str(), "OpenHarmony");

    const uint32_t utf16DataLen2 = 2;
    const uint16_t utf16Data2[utf16DataLen2] = { 0x9e3f, 0x8499 }; // "鸿蒙"
    std::u16string u16Str2 = StringHelper::Utf16ToU16String(utf16Data2, utf16DataLen2);
    std::string u16Value2 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16Str2);
    EXPECT_STREQ(u16Value2.c_str(), "鸿蒙");

    const uint32_t utf8DataLen1 = 11;
    const uint8_t utf8Data1[utf8DataLen1] = { // "OpenHarmony"
        0x4f, 0x70, 0x65, 0x6e,
        0x48, 0x61, 0x72, 0x6d,
        0x6f, 0x6e, 0x79
    };
    std::u16string u8Str1 = StringHelper::Utf8ToU16String(utf8Data1, utf8DataLen1);
    std::string u8Value1 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u8Str1);
    EXPECT_STREQ(u8Value1.c_str(), "OpenHarmony");

    const uint32_t utf8DataLen2 = 6;
    const uint8_t utf8Data2[utf8DataLen2] = { 0xe9, 0xb8, 0xbf, 0xe8, 0x92, 0x99 }; // "鸿蒙"
    std::u16string u8Str2 = StringHelper::Utf8ToU16String(utf8Data2, utf8DataLen2);
    std::string u8Value2 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u8Str2);
    EXPECT_STREQ(u8Value2.c_str(), "鸿蒙");
}

HWTEST_F_L0(StringHelperTest, Find_RFind)
{
    const std::string valueStr = "Hello worldworld";
    const std::string searchStr1 = "world";
    const std::string searchStr2 = "undefined";
    const std::u16string u16ValueStr = StringHelper::StringToU16string(valueStr);
    const std::u16string u16SearchStr1 = StringHelper::StringToU16string(searchStr1);
    const std::u16string u16SearchStr2 = StringHelper::StringToU16string(searchStr2);

    size_t pos1 = StringHelper::Find(valueStr, searchStr1, 0);
    size_t pos2 = StringHelper::Find(valueStr, searchStr2, 0);
    size_t pos3 = StringHelper::Find(u16ValueStr, u16SearchStr1, 0);
    size_t pos4 = StringHelper::Find(u16ValueStr, u16SearchStr2, 0);
    size_t pos5 = StringHelper::RFind(u16ValueStr, u16SearchStr1, 17); // 17 : Search to the end
    size_t pos6 = StringHelper::RFind(u16ValueStr, u16SearchStr2, 17); // 17 : Search to the end
    EXPECT_EQ(pos1, 6U);
    EXPECT_EQ(pos2, std::string::npos);
    EXPECT_EQ(pos3, 6U);
    EXPECT_EQ(pos4, std::string::npos);
    EXPECT_EQ(pos5, 11U);
    EXPECT_EQ(pos6, std::string::npos);
}

HWTEST_F_L0(StringHelperTest, ToUpper_ToLower_ToLocaleUpper_ToLocaleLower)
{
    const std::u16string u16SourceStr1 = StringHelper::StringToU16string("AbCdEfGhIjKlMnOpQrStUvWxYz");
    std::string upperStr = StringHelper::ToUpper(u16SourceStr1);
    std::string lowerStr = StringHelper::ToLower(u16SourceStr1);
    EXPECT_STREQ(upperStr.c_str(), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    EXPECT_STREQ(lowerStr.c_str(), "abcdefghijklmnopqrstuvwxyz");

    icu::Locale locale("el", "Latn", "GR");
    const std::u16string u16SourceStr2 = StringHelper::StringToU16string("Greek : Αυτό είναι ένα δοκιμαστικό κείμενο.");
    std::string localeUpperStr = StringHelper::ToLocaleUpper(u16SourceStr2, locale);
    EXPECT_STREQ(localeUpperStr.c_str(), "GREEK : ΑΥΤΟ ΕΙΝΑΙ ΕΝΑ ΔΟΚΙΜΑΣΤΙΚΟ ΚΕΙΜΕΝΟ.");

    const std::u16string u16SourceStr3 = StringHelper::StringToU16string("GREEK : ΑΥΤΌ ΕΊΝΑΙ ΈΝΑ ΔΟΚΙΜΑΣΤΙΚΌ ΚΕΊΜΕΝΟ.");
    std::string localeLowerStr = StringHelper::ToLocaleLower(u16SourceStr3, locale);
    EXPECT_STREQ(localeLowerStr.c_str(), "greek : αυτό είναι ένα δοκιμαστικό κείμενο.");
}

HWTEST_F_L0(StringHelperTest, FindFromU16ToUpper)
{
    const std::u16string u16SourceStr = StringHelper::StringToU16string("HELLO WORLD!");
    const uint32_t utf16DataLen = 5;
    const uint32_t utf16DataLenUpper = 9;
    uint16_t utf16Data[utf16DataLen] = { // "world"
        0x77, 0x6f, 0x72, 0x6c, 0x64
    };
    uint16_t utf16DataUpper[utf16DataLenUpper] = { // "WORLD"
        0x57, 0x4f, 0x52, 0x4c, 0x44
    };
    size_t pos1 = StringHelper::FindFromU16ToUpper(u16SourceStr, utf16Data);
    size_t pos2 = StringHelper::FindFromU16ToUpper(u16SourceStr, utf16DataUpper);
    EXPECT_EQ(pos1, 6U);
    EXPECT_EQ(pos2, 6U);
}

HWTEST_F_L0(StringHelperTest, UnicodeFromUtf8)
{
    int maxLen = 1;
    const uint8_t *p = new uint8_t[6] { 0x00 };
    const uint8_t **pp = &p;
    const uint8_t oneByteVaild1[1] = { 0x00 };
    const uint8_t oneByteVaild2[1] = { BIT_MASK_1 - 0x01 };
    const uint8_t oneByteUnvaild[1] = { BIT_MASK_1 };
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(oneByteVaild1, maxLen, pp), 0);
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(oneByteVaild2, maxLen, pp), 127);
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(oneByteUnvaild, maxLen, pp), -1);

    maxLen = 2;
    const uint8_t twoByteVaild[2] = { BIT_MASK_3 - 0x01, BIT_MASK_2 - 0x01 };
    const uint8_t twoByteUnvaild1[2] = { BIT_MASK_2, BIT_MASK_2 };
    const uint8_t twoByteUnvaild2[2] = { BIT_MASK_3, BIT_MASK_1 };
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(twoByteVaild, maxLen, pp), 2047); // 2047 : utf8 [0xDF, 0xBF]
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(twoByteUnvaild1, maxLen, pp), -1);
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(twoByteUnvaild2, maxLen, pp), -1);

    maxLen = 3;
    const uint8_t threeByteVaild[3] = { BIT_MASK_4 - 0x01, BIT_MASK_2 - 0x01, BIT_MASK_2 - 0x01 };
    const uint8_t threeByteUnvaild1[3] = { BIT_MASK_3, BIT_MASK_1, BIT_MASK_2 };
    const uint8_t threeByteUnvaild2[3] = { BIT_MASK_3, BIT_MASK_2, BIT_MASK_1 };
    const uint8_t threeByteUnvaild3[3] = { BIT_MASK_4, BIT_MASK_1, BIT_MASK_1 };
    const uint8_t threeByteUnvaild4[3] = { BIT_MASK_4, BIT_MASK_2, BIT_MASK_2 };
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(threeByteVaild, maxLen, pp), 65535); // 65535 : utf8 [0xEF, 0xBF, 0xBF]
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(threeByteUnvaild1, maxLen, pp), -1);
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(threeByteUnvaild2, maxLen, pp), -1);
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(threeByteUnvaild3, maxLen, pp), -1);
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(threeByteUnvaild4, maxLen, pp), -1);

    maxLen = 4;
    const uint8_t fourByteVaild[4] = {
        BIT_MASK_5 - 0x01, BIT_MASK_2 - 0x01, BIT_MASK_2 - 0x01, BIT_MASK_2 - 0x01
    };
    const uint8_t fourByteUnvaild1[4] = { BIT_MASK_4, BIT_MASK_1, BIT_MASK_1, BIT_MASK_2 };
    const uint8_t fourByteUnvaild2[4] = { BIT_MASK_4, BIT_MASK_1, BIT_MASK_2, BIT_MASK_1 };
    const uint8_t fourByteUnvaild3[4] = { BIT_MASK_4, BIT_MASK_2, BIT_MASK_1, BIT_MASK_1 };
    const uint8_t fourByteUnvaild4[4] = { BIT_MASK_5, BIT_MASK_1, BIT_MASK_1, BIT_MASK_1 };
    const uint8_t fourByteUnvaild5[4] = { BIT_MASK_5, BIT_MASK_2, BIT_MASK_2, BIT_MASK_2 };
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(
        fourByteVaild, maxLen, pp), 2097151); // 2097151 : utf [0xF7, 0xBF, 0xBF, 0xBF]
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(fourByteUnvaild1, maxLen, pp), -1);
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(fourByteUnvaild2, maxLen, pp), -1);
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(fourByteUnvaild3, maxLen, pp), -1);
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(fourByteUnvaild4, maxLen, pp), -1);
    EXPECT_EQ(StringHelper::UnicodeFromUtf8(fourByteUnvaild5, maxLen, pp), -1);
}

HWTEST_F_L0(StringHelperTest, Append_SplitString)
{
    const std::u16string str1 = StringHelper::StringToU16string("Hello ");
    const std::u16string str2 = StringHelper::StringToU16string("world!");
    std::u16string u16Result = StringHelper::Append(str1, str2);
    std::string result = StringHelper::U16stringToString(u16Result);
    EXPECT_STREQ(result.c_str(), "Hello world!");

    const std::string delimiter = " ";
    std::vector<std::string> resultList = StringHelper::SplitString(result, delimiter);
    EXPECT_STREQ(resultList[0].c_str(), "Hello");
    EXPECT_STREQ(resultList[1].c_str(), "world!");
}

HWTEST_F_L0(StringHelperTest, GetSpecifiedLine)
{
    const std::string srcStr = "Hello\nworld\n!";
    std::string resLine1 = StringHelper::GetSpecifiedLine(srcStr, 1);
    std::string resLine2 = StringHelper::GetSpecifiedLine(srcStr, 2);
    std::string resLine3 = StringHelper::GetSpecifiedLine(srcStr, 3);
    EXPECT_STREQ(resLine1.c_str(), "Hello");
    EXPECT_STREQ(resLine2.c_str(), "world");
    EXPECT_STREQ(resLine3.c_str(), "!");
}
}  // namespace panda::test
