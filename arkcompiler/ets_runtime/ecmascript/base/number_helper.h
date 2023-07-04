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

#ifndef ECMASCRIPT_BASE_NUMBER_HELPER_H
#define ECMASCRIPT_BASE_NUMBER_HELPER_H

#include <cstdint>

#include "ecmascript/ecma_string.h"
#include "ecmascript/js_tagged_value.h"

namespace panda::ecmascript::base {
constexpr double MIN_RADIX = 2;
constexpr double MAX_RADIX = 36;
constexpr double MIN_FRACTION = 0;
constexpr double MAX_FRACTION = 100;

// Coversion flags
static constexpr uint32_t NO_FLAGS = 0U;
static constexpr uint32_t ALLOW_BINARY = 1U << 0U;
static constexpr uint32_t ALLOW_OCTAL = 1U << 1U;
static constexpr uint32_t ALLOW_HEX = 1U << 2U;
static constexpr uint32_t IGNORE_TRAILING = 1U << 3U;

static constexpr uint32_t MAX_PRECISION = 16;
static constexpr uint8_t BINARY = 2;
static constexpr uint8_t OCTAL = 8;
static constexpr uint8_t DECIMAL = 10;
static constexpr uint8_t HEXADECIMAL = 16;
static constexpr double HALF = 0.5;
static constexpr double EPSILON = std::numeric_limits<double>::epsilon();
static constexpr int64_t MAX_SAFE_INTEGER = 9007199254740991;
static constexpr double MAX_VALUE = std::numeric_limits<double>::max();
static constexpr double MIN_VALUE = std::numeric_limits<double>::min();
static constexpr double POSITIVE_INFINITY = std::numeric_limits<double>::infinity();
static constexpr double NAN_VALUE = std::numeric_limits<double>::quiet_NaN();

// Helper defines for double
static constexpr int DOUBLE_MAX_PRECISION = 17;
static constexpr int DOUBLE_EXPONENT_BIAS = 0x3FF;
static constexpr size_t DOUBLE_SIGNIFICAND_SIZE = 52;
static constexpr uint64_t DOUBLE_SIGN_MASK = 0x8000000000000000ULL;
static constexpr uint64_t DOUBLE_EXPONENT_MASK = 0x7FFULL << DOUBLE_SIGNIFICAND_SIZE;
static constexpr uint64_t DOUBLE_SIGNIFICAND_MASK = 0x000FFFFFFFFFFFFFULL;
static constexpr uint64_t DOUBLE_HIDDEN_BIT = 1ULL << DOUBLE_SIGNIFICAND_SIZE;
static constexpr int32_t MINUS_ZERO_LOBITS = static_cast<int32_t>(0);
static constexpr int32_t MINUS_ZERO_HIBITS = static_cast<int32_t>(1) << 31;
static constexpr int64_t MINUS_ZERO_BITS = (static_cast<uint64_t>(MINUS_ZERO_HIBITS) << 32) | MINUS_ZERO_LOBITS;

static constexpr size_t INT64_BITS = 64;
static constexpr size_t INT32_BITS = 32;
static constexpr size_t INT16_BITS = 16;
static constexpr size_t INT8_BITS = 8;
static constexpr size_t JS_DTOA_BUF_SIZE = 128;

// help defines for random
static constexpr int LEFT52 = 52 ;
static constexpr int RIGHT12 = 12;
static constexpr uint32_t USE_LEFT = 0x3ff;
static constexpr int SECONDS_TO_SUBTLE = 1000000;
static constexpr int RIGHT27 = 27;
static constexpr int LEFT25 = 25;
static constexpr uint64_t  GET_MULTIPLY = 0x2545F4914F6CDD1D;

class NumberHelper {
public:
    static inline JSTaggedType GetNaN()
    {
        return JSTaggedValue(NAN_VALUE).GetRawData();
    }

    static inline JSTaggedType GetPositiveInfinity()
    {
        return JSTaggedValue(POSITIVE_INFINITY).GetRawData();
    }

    static bool IsFinite(JSTaggedValue number)
    {
        return number.IsInt() || (number.IsDouble() && std::isfinite(number.GetDouble()));
    }
    static bool IsNaN(JSTaggedValue number)
    {
        return number.IsDouble() && std::isnan(number.GetDouble());
    }
    static JSTaggedValue DoubleToString(JSThread *thread, double number, int radix);
    static bool IsEmptyString(const uint8_t *start, const uint8_t *end);
    static JSHandle<EcmaString> NumberToString(const JSThread *thread, JSTaggedValue number);
    static double TruncateDouble(double d);
    static int64_t DoubleToInt64(double d);
    static double StringToDouble(const uint8_t *start, const uint8_t *end, uint8_t radix, uint32_t flags = NO_FLAGS);
    static int32_t DoubleToInt(double d, size_t bits);
    static int32_t DoubleInRangeInt32(double d);
    static JSTaggedValue DoubleToExponential(JSThread *thread, double number, int digit);
    static JSTaggedValue DoubleToFixed(JSThread *thread, double number, int digit);
    static JSTaggedValue DoubleToPrecision(JSThread *thread, double number, int digit);
    static JSTaggedValue StringToDoubleWithRadix(const uint8_t *start, const uint8_t *end, int radix);
    static CString IntToString(int number);
    static CString IntegerToString(int64_t number, int radix);
    static JSTaggedValue StringToBigInt(JSThread *thread, JSHandle<JSTaggedValue> strVal);

private:
    static char Carry(char current, int radix);
    static double Strtod(const char *str, int exponent, uint8_t radix);
    static CString DecimalsToString(int64_t *numberInteger, double fraction, int radix, double delta);
    static bool GotoNonspace(uint8_t **ptr, const uint8_t *end);
    static void GetBase(double d, int digits, int *decpt, char *buf, char *bufTmp, int size);
    static int GetMinmumDigits(double d, int *decpt, char *buf);
};
class RandomGenerator {
public:
    static uint64_t& GetRandomState();
    static uint64_t XorShift64(uint64_t *pVal);
    static void InitRandom();
private:
    static thread_local uint64_t randomState;
};
}  // namespace panda::ecmascript::base
#endif  // ECMASCRIPT_BASE_NUMBER_HELPER_H
