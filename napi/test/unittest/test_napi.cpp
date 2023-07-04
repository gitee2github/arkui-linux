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

#include "test.h"
#include "gtest/gtest.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"
#include "utils/log.h"

class NapiBasicTest : public NativeEngineTest {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "NapiBasicTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "NapiBasicTest TearDownTestCase";
    }

    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: UndefinedTest001
 * @tc.desc: Test undefined type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, UndefinedTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_undefined);
}

/**
 * @tc.name: NullTest001
 * @tc.desc: Test null type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NullTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_null(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_null);
}

/**
 * @tc.name: BooleanTest001
 * @tc.desc: Test boolean type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, BooleanTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_boolean);

    bool resultValue = false;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, result, &resultValue));
    ASSERT_TRUE(resultValue);
}

/**
 * @tc.name: NumberTest001
 * @tc.desc: Test number type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NumberTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    {
        int32_t testValue = INT32_MAX;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_int32(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_number);

        int32_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_int32(env, result, &resultValue));
        ASSERT_EQ(resultValue, INT32_MAX);
    }
    {
        uint32_t testValue = UINT32_MAX;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_uint32(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_number);

        uint32_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_uint32(env, result, &resultValue));
        ASSERT_EQ(resultValue, UINT32_MAX);
    }
    {
        int64_t testValue = 9007199254740991;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_int64(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_number);

        int64_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_int64(env, result, &resultValue));
        ASSERT_EQ(resultValue, testValue);
    }
    {
        double testValue = DBL_MAX;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_double(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_number);

        double resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_double(env, result, &resultValue));
        ASSERT_EQ(resultValue, DBL_MAX);
    }
}

/**
 * @tc.name: StringTest001
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char testStr[] = "中文,English,123456,!@#$%$#^%&";
    size_t testStrLength = strlen(testStr);
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, testStr, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, result, nullptr, 0, &bufferSize));
    ASSERT_GT(bufferSize, (size_t)0);
    buffer = new char[bufferSize + 1]{ 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, result, buffer, bufferSize + 1, &strLength));
    ASSERT_STREQ(testStr, buffer);
    ASSERT_EQ(testStrLength, strLength);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: SymbolTest001
 * @tc.desc: Test symbol type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SymbolTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    const char testStr[] = "testSymbol";
    napi_value result = nullptr;

    napi_create_string_latin1(env, testStr, strlen(testStr), &result);

    napi_value symbolVal = nullptr;
    napi_create_symbol(env, result, &symbolVal);

    ASSERT_CHECK_VALUE_TYPE(env, symbolVal, napi_symbol);
}

/**
 * @tc.name: ExternalTest001
 * @tc.desc: Test external type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ExternalTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char testStr[] = "test";
    napi_value external = nullptr;
    napi_create_external(
        env, (void*)testStr,
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)testStr, &external);

    ASSERT_CHECK_VALUE_TYPE(env, external, napi_external);
    void* tmpExternal = nullptr;
    napi_get_value_external(env, external, &tmpExternal);
    ASSERT_TRUE(tmpExternal);
    ASSERT_EQ(tmpExternal, testStr);
}

/**
 * @tc.name: ObjectTest001
 * @tc.desc: Test object type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ObjectTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);

    const char testStr[] = "1234567";
    napi_value strAttribute = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, testStr, strlen(testStr), &strAttribute));
    ASSERT_CHECK_VALUE_TYPE(env, strAttribute, napi_string);
    ASSERT_CHECK_CALL(napi_set_named_property(env, result, "strAttribute", strAttribute));

    napi_value retStrAttribute = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, result, "strAttribute", &retStrAttribute));
    ASSERT_CHECK_VALUE_TYPE(env, retStrAttribute, napi_string);

    int32_t testNumber = 12345;
    napi_value numberAttribute = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, testNumber, &numberAttribute));
    ASSERT_CHECK_VALUE_TYPE(env, numberAttribute, napi_number);
    ASSERT_CHECK_CALL(napi_set_named_property(env, result, "numberAttribute", numberAttribute));

    napi_value propNames = nullptr;
    ASSERT_CHECK_CALL(napi_get_property_names(env, result, &propNames));
    ASSERT_CHECK_VALUE_TYPE(env, propNames, napi_object);
    bool isArray = false;
    ASSERT_CHECK_CALL(napi_is_array(env, propNames, &isArray));
    ASSERT_TRUE(isArray);
    uint32_t arrayLength = 0;
    ASSERT_CHECK_CALL(napi_get_array_length(env, propNames, &arrayLength));
    ASSERT_EQ(arrayLength, (uint32_t)2);

    for (uint32_t i = 0; i < arrayLength; i++) {
        bool hasElement = false;
        ASSERT_CHECK_CALL(napi_has_element(env, propNames, i, &hasElement));

        napi_value propName = nullptr;
        ASSERT_CHECK_CALL(napi_get_element(env, propNames, i, &propName));
        ASSERT_CHECK_VALUE_TYPE(env, propName, napi_string);

        bool hasProperty = false;
        napi_has_property(env, result, propName, &hasProperty);
        ASSERT_TRUE(hasProperty);

        napi_value propValue = nullptr;
        napi_get_property(env, result, propName, &propValue);
        ASSERT_TRUE(propValue != nullptr);
    }
}

/**
 * @tc.name: FunctionTest001
 * @tc.desc: Test function type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, FunctionTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar;
        napi_value* argv = nullptr;
        size_t argc = 0;
        void* data = nullptr;

        napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
        if (argc > 0) {
            argv = new napi_value[argc];
        }
        napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

        napi_value result = nullptr;
        napi_create_object(env, &result);

        napi_value messageKey = nullptr;
        const char* messageKeyStr = "message";
        napi_create_string_latin1(env, messageKeyStr, strlen(messageKeyStr), &messageKey);
        napi_value messageValue = nullptr;
        const char* messageValueStr = "OK";
        napi_create_string_latin1(env, messageValueStr, strlen(messageValueStr), &messageValue);
        napi_set_property(env, result, messageKey, messageValue);

        if (argv != nullptr) {
            delete []argv;
        }

        return result;
    };

    napi_value recv = nullptr;
    napi_value funcValue = nullptr;
    napi_get_undefined(env, &recv);
    ASSERT_NE(recv, nullptr);

    napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    ASSERT_NE(funcValue, nullptr);

    napi_handle_scope parentScope = nullptr;
    napi_open_handle_scope(env, &parentScope);
    ASSERT_NE(parentScope, nullptr);

    napi_escapable_handle_scope childScope = nullptr;
    napi_open_escapable_handle_scope(env, &childScope);
    ASSERT_NE(childScope, nullptr);

    napi_value funcResultValue = nullptr;
    napi_value newFuncResultValue = nullptr;
    napi_call_function(env, recv, funcValue, 0, nullptr, &funcResultValue);
    ASSERT_NE(funcResultValue, nullptr);

    napi_escape_handle(env, childScope, funcResultValue, &newFuncResultValue);
    napi_close_escapable_handle_scope(env, childScope);
    ASSERT_TRUE(newFuncResultValue != nullptr);
    ASSERT_CHECK_VALUE_TYPE(env, newFuncResultValue, napi_object);
    napi_close_handle_scope(env, parentScope);
}

/**
 * @tc.name: ArrayTest001
 * @tc.desc: Test array type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ArrayTest001, testing::ext::TestSize.Level1) {
    napi_env env = (napi_env) engine_;

    napi_value array = nullptr;
    napi_create_array(env, &array);
    ASSERT_NE(array, nullptr);
    bool isArray = false;
    napi_is_array(env, array, &isArray);
    ASSERT_TRUE(isArray);

    for (size_t i = 0; i < 10; i++) {
        napi_value num = nullptr;
        napi_create_uint32(env, i, &num);
        napi_set_element(env, array, i, num);
    }

    uint32_t arrayLength = 0;
    napi_get_array_length(env, array, &arrayLength);

    ASSERT_EQ(arrayLength, (uint32_t) 10);

    for (size_t i = 0; i < arrayLength; i++) {
        bool hasIndex = false;
        napi_has_element(env, array, i, &hasIndex);
        ASSERT_TRUE(hasIndex);
    }

    for (size_t i = 0; i < arrayLength; i++) {
        bool isDelete = false;
        napi_delete_element(env, array, i, &isDelete);
        ASSERT_TRUE(isDelete);
    }
}

/**
 * @tc.name: ArrayBufferTest001
 * @tc.desc: Test array buffer type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ArrayBufferTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 1024;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    void* tmpArrayBufferPtr = nullptr;
    size_t arrayBufferLength = 0;
    napi_get_arraybuffer_info(env, arrayBuffer, &tmpArrayBufferPtr, &arrayBufferLength);

    ASSERT_EQ(arrayBufferPtr, tmpArrayBufferPtr);
    ASSERT_EQ(arrayBufferSize, arrayBufferLength);
}

/**
 * @tc.name: TypedArrayTest001
 * @tc.desc: Test typed array type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, TypedArrayTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    {
        napi_value arrayBuffer = nullptr;
        void* arrayBufferPtr = nullptr;
        size_t arrayBufferSize = 1024;
        napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

        void* tmpArrayBufferPtr = nullptr;
        size_t arrayBufferLength = 0;
        napi_get_arraybuffer_info(env, arrayBuffer, &tmpArrayBufferPtr, &arrayBufferLength);

        ASSERT_EQ(arrayBufferPtr, tmpArrayBufferPtr);
        ASSERT_EQ(arrayBufferSize, arrayBufferLength);

        napi_value typedarray = nullptr;
        napi_create_typedarray(env, napi_int8_array, arrayBufferSize, arrayBuffer, 0, &typedarray);
        ASSERT_NE(typedarray, nullptr);
        bool isTypedArray = false;
        napi_is_typedarray(env, typedarray, &isTypedArray);
        ASSERT_TRUE(isTypedArray);

        napi_typedarray_type typedarrayType;
        size_t typedarrayLength = 0;
        void* typedarrayBufferPtr = nullptr;
        napi_value tmpArrayBuffer = nullptr;
        size_t byteOffset = 0;

        napi_get_typedarray_info(env, typedarray, &typedarrayType, &typedarrayLength, &typedarrayBufferPtr,
                                 &tmpArrayBuffer, &byteOffset);

        ASSERT_EQ(typedarrayBufferPtr, arrayBufferPtr);
        ASSERT_EQ(arrayBufferSize, typedarrayLength);
    }
}

/**
 * @tc.name: DataViewTest001
 * @tc.desc: Test data view type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, DataViewTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 1024;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_NE(arrayBufferPtr, nullptr);
    bool isArrayBuffer = false;
    napi_is_arraybuffer(env, arrayBuffer, &isArrayBuffer);
    ASSERT_TRUE(isArrayBuffer);

    napi_value result = nullptr;
    napi_create_dataview(env, arrayBufferSize, arrayBuffer, 0, &result);

    bool isDataView = false;
    napi_is_dataview(env, result, &isDataView);

    napi_value retArrayBuffer = nullptr;
    void* data = nullptr;
    size_t byteLength = 0;
    size_t byteOffset = 0;
    napi_get_dataview_info(env, result, &byteLength, &data, &retArrayBuffer, &byteOffset);

    bool retIsArrayBuffer = false;
    napi_is_arraybuffer(env, arrayBuffer, &retIsArrayBuffer);
    ASSERT_TRUE(retIsArrayBuffer);
    ASSERT_EQ(arrayBufferPtr, data);
    ASSERT_EQ(arrayBufferSize, byteLength);
    ASSERT_EQ((size_t)0, byteOffset);
}

/**
 * @tc.name: PromiseTest001
 * @tc.desc: Test promise type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, PromiseTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    {
        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        napi_create_promise(env, &deferred, &promise);
        ASSERT_NE(deferred, nullptr);
        ASSERT_NE(promise, nullptr);

        bool isPromise = false;
        napi_is_promise(env, promise, &isPromise);
        ASSERT_TRUE(isPromise);

        napi_value undefined = nullptr;
        napi_get_undefined(env, &undefined);
        napi_resolve_deferred(env, deferred, undefined);
    }
    {
        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        napi_create_promise(env, &deferred, &promise);
        ASSERT_NE(deferred, nullptr);
        ASSERT_NE(promise, nullptr);

        bool isPromise = false;
        napi_is_promise(env, promise, &isPromise);
        ASSERT_TRUE(isPromise);

        napi_value undefined = nullptr;
        napi_get_undefined(env, &undefined);
        napi_reject_deferred(env, deferred, undefined);
    }
}

/**
 * @tc.name: ErrorTest001
 * @tc.desc: Test error type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ErrorTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    {
        napi_value code = nullptr;
        napi_value message = nullptr;

        napi_create_string_latin1(env, "500", NAPI_AUTO_LENGTH, &code);
        napi_create_string_latin1(env, "common error", NAPI_AUTO_LENGTH, &message);

        napi_value error = nullptr;
        napi_create_error(env, code, message, &error);
        ASSERT_TRUE(error != nullptr);
        bool isError = false;
        napi_is_error(env, error, &isError);
        ASSERT_TRUE(isError);
        napi_throw(env, error);
    }

    {
        napi_value code = nullptr;
        napi_value message = nullptr;
        napi_create_string_latin1(env, "500", NAPI_AUTO_LENGTH, &code);
        napi_create_string_latin1(env, "range error", NAPI_AUTO_LENGTH, &message);
        napi_value error = nullptr;
        napi_create_range_error(env, code, message, &error);
        ASSERT_TRUE(error != nullptr);
        bool isError = false;
        napi_is_error(env, error, &isError);
        ASSERT_TRUE(isError);
    }

    {
        napi_value code = nullptr;
        napi_value message = nullptr;
        napi_create_string_latin1(env, "500", NAPI_AUTO_LENGTH, &code);
        napi_create_string_latin1(env, "type error", NAPI_AUTO_LENGTH, &message);
        napi_value error = nullptr;
        napi_create_type_error(env, code, message, &error);
        ASSERT_TRUE(error != nullptr);
        bool isError = false;
        napi_is_error(env, error, &isError);
        ASSERT_TRUE(isError);
    }

    napi_throw_error(env, "500", "Common error");
    napi_throw_range_error(env, "500", "Range error");
    napi_throw_type_error(env, "500", "Type error");
    bool isExceptionPending = false;
    napi_is_exception_pending(env, &isExceptionPending);
    ASSERT_TRUE(isExceptionPending);
}

/**
 * @tc.name: ReferenceTest001
 * @tc.desc: Test reference type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ReferenceTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_ref resultRef = nullptr;

    napi_create_object(env, &result);
    napi_create_reference(env, result, 1, &resultRef);

    uint32_t resultRefCount = 0;

    napi_reference_ref(env, resultRef, &resultRefCount);
    ASSERT_EQ(resultRefCount, (uint32_t)2);

    napi_reference_unref(env, resultRef, &resultRefCount);
    ASSERT_EQ(resultRefCount, (uint32_t)1);

    napi_value refValue = nullptr;
    napi_get_reference_value(env, resultRef, &refValue);

    ASSERT_NE(refValue, nullptr);

    napi_delete_reference(env, resultRef);
}

/**
 * @tc.name: CustomClassTest001
 * @tc.desc: Test define class.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CustomClassTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    auto constructor = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar = nullptr;
        napi_value* argv = nullptr;
        size_t argc = 0;
        void* data = nullptr;
        napi_value constructor = nullptr;
        napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
        if (argc > 0) {
            argv = new napi_value[argc];
        }
        napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
        napi_get_new_target(env, info, &constructor);
        if (constructor == nullptr) {
            napi_throw_error(env, nullptr, "is not new instance");
        }
        if (argv != nullptr) {
            delete []argv;
        }
        return thisVar;
    };

    napi_value ln2 = nullptr;
    napi_value e = nullptr;

    napi_create_double(env, 2.718281828459045, &e);
    napi_create_double(env, 0.6931471805599453, &ln2);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("add", [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
        DECLARE_NAPI_FUNCTION("sub", [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
        DECLARE_NAPI_FUNCTION("mul", [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
        DECLARE_NAPI_FUNCTION("div", [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
        DECLARE_NAPI_STATIC_FUNCTION("getTime",
                                     [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
        DECLARE_NAPI_GETTER_SETTER(
            "pi", [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
            [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),

    };

    napi_value customClass = nullptr;

    ASSERT_CHECK_CALL(napi_define_class(env, "CustomClass", NAPI_AUTO_LENGTH, constructor, nullptr,
                                        sizeof(desc) / sizeof(desc[0]), desc, &customClass));
    ASSERT_CHECK_VALUE_TYPE(env, customClass, napi_function);
    napi_value customClassPrototype = nullptr;
    napi_get_prototype(env, customClass, &customClassPrototype);
    ASSERT_CHECK_VALUE_TYPE(env, customClassPrototype, napi_function);

    napi_value customInstance = nullptr;
    ASSERT_CHECK_CALL(napi_new_instance(env, customClass, 0, nullptr, &customInstance));

    bool isInstanceOf = false;
    ASSERT_CHECK_CALL(napi_instanceof(env, customInstance, customClass, &isInstanceOf));
    ASSERT_TRUE(isInstanceOf);
}

/**
 * @tc.name: AsyncWorkTest001
 * @tc.desc: Test async work.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncWorkTest001, testing::ext::TestSize.Level1)
{
    struct AsyncWorkContext {
        napi_async_work work = nullptr;
    };
    napi_env env = (napi_env)engine_;
    {
        auto asyncWorkContext = new AsyncWorkContext();
        napi_value resourceName = nullptr;
        napi_create_string_utf8(env, "AsyncWorkTest", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(
            env, nullptr, resourceName, [](napi_env value, void* data) {},
            [](napi_env env, napi_status status, void* data) {
                AsyncWorkContext* asyncWorkContext = (AsyncWorkContext*)data;
                napi_delete_async_work(env, asyncWorkContext->work);
                delete asyncWorkContext;
            },
            asyncWorkContext, &asyncWorkContext->work);
        napi_queue_async_work(env, asyncWorkContext->work);
    }
    {
        auto asyncWorkContext = new AsyncWorkContext();
        napi_value resourceName = nullptr;
        napi_create_string_utf8(env, "AsyncWorkTest", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(
            env, nullptr, resourceName, [](napi_env value, void* data) {},
            [](napi_env env, napi_status status, void* data) {
                AsyncWorkContext* asyncWorkContext = (AsyncWorkContext*)data;
                napi_delete_async_work(env, asyncWorkContext->work);
                delete asyncWorkContext;
            },
            asyncWorkContext, &asyncWorkContext->work);
        napi_queue_async_work(env, asyncWorkContext->work);
        napi_cancel_async_work(env, asyncWorkContext->work);
    }
}

/**
 * @tc.name: ObjectWrapperTest001
 * @tc.desc: Test object wrapper.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ObjectWrapperTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value testClass = nullptr;
    napi_define_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, &testClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testClass, 0, nullptr, &instanceValue);

    const char* testStr = "test";
    napi_wrap(
        env, instanceValue, (void*)testStr, [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);

    char* tmpTestStr = nullptr;
    napi_unwrap(env, instanceValue, (void**)&tmpTestStr);
    ASSERT_STREQ(testStr, tmpTestStr);

    char* tmpTestStr1 = nullptr;
    napi_remove_wrap(env, instanceValue, (void**)&tmpTestStr1);
    ASSERT_STREQ(testStr, tmpTestStr1);
}

/**
 * @tc.name: StrictEqualsTest001
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StrictEqualsTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    const char* testStringStr = "test";
    napi_value testString = nullptr;
    napi_create_string_utf8(env, testStringStr, strlen(testStringStr), &testString);
    bool isStrictEquals = false;
    napi_strict_equals(env, testString, testString, &isStrictEquals);
    ASSERT_TRUE(isStrictEquals);

    napi_value testObject = nullptr;
    napi_create_object(env, &testObject);
    isStrictEquals = false;
    napi_strict_equals(env, testObject, testObject, &isStrictEquals);
    ASSERT_TRUE(isStrictEquals);
}

/**
 * @tc.name: CreateRuntimeTest001
 * @tc.desc: Test create runtime.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateRuntimeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_env newEnv = nullptr;
    napi_create_runtime(env, &newEnv);
#ifdef USE_V8_ENGINE
    ASSERT_NE(newEnv, nullptr);
#elif USE_QUICKJS_ENGINE
    ASSERT_EQ(newEnv, nullptr);
#endif
}

/**
 * @tc.name: SerializeDeSerializeTest001
 * @tc.desc: Test serialize & deserialize.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SerializeDeSerializeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value num = nullptr;
    uint32_t value = 1000;
    napi_create_uint32(env, value, &num);
    napi_value data = nullptr;
    napi_serialize(env, num, undefined, &data);
    ASSERT_NE(data, nullptr);

    napi_value result = nullptr;
    napi_deserialize(env, data, &result);
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_number);
    int32_t resultData = 0;
    napi_get_value_int32(env, result, &resultData);
    ASSERT_EQ(resultData, 1000);
}

/**
 * @tc.name: IsCallableTest001
 * @tc.desc: Test is callable.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, IsCallableTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar;
        napi_value* argv = nullptr;
        size_t argc = 0;
        void* data = nullptr;

        napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
        if (argc > 0) {
            argv = new napi_value[argc];
        }
        napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

        napi_value result = nullptr;
        napi_create_object(env, &result);

        napi_value messageKey = nullptr;
        const char* messageKeyStr = "message";
        napi_create_string_latin1(env, messageKeyStr, strlen(messageKeyStr), &messageKey);
        napi_value messageValue = nullptr;
        const char* messageValueStr = "OK";
        napi_create_string_latin1(env, messageValueStr, strlen(messageValueStr), &messageValue);
        napi_set_property(env, result, messageKey, messageValue);

        if (argv != nullptr) {
            delete []argv;
        }
        return result;
    };

    napi_value funcValue = nullptr;
    napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    ASSERT_NE(funcValue, nullptr);

    bool result = false;
    napi_is_callable(env, funcValue, &result);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: EncodeToUtf8Test001
 * @tc.desc: Test EncodeToUtf8 Func.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EncodeToUtf8Test001, testing::ext::TestSize.Level1)
{
    std::string str = "encode";
    auto testStr = engine_->CreateString(str.c_str(), str.length());
    char* buffer = new char[str.length()];
    size_t bufferSize = str.length();
    int32_t written = 0;
    int32_t nchars = 0;
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 6);
    ASSERT_EQ(nchars, 6);
    delete[] buffer;

    str = "encode\xc2\xab\xe2\x98\x80";
    testStr = engine_->CreateString(str.c_str(), str.length());
    buffer = new char[str.length()];
    bufferSize = str.length();
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 11);
    ASSERT_EQ(nchars, 8);
    delete[] buffer;

    buffer = new char[str.length()];
    bufferSize = str.length();
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    bufferSize--;
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 8);
    ASSERT_EQ(nchars, 7);
    delete[] buffer;

    buffer = new char[str.length()];
    bufferSize = str.length();
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    bufferSize -= 4;
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 6);
    ASSERT_EQ(nchars, 6);
    delete[] buffer;

    str = "encode\xc2\xab\xe2\x98\x80t";
    testStr = engine_->CreateString(str.c_str(), str.length());
    buffer = new char[str.length()];
    bufferSize = str.length();
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    bufferSize--;
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 11);
    ASSERT_EQ(nchars, 8);
    delete[] buffer;

    str = "";
    testStr = engine_->CreateString(str.c_str(), str.length());
    buffer = new char[str.length() + 1];
    bufferSize = str.length() + 1;
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 0);
    ASSERT_EQ(nchars, 0);
    delete[] buffer;
}

/**
 * @tc.name: WrapWithSizeTest001
 * @tc.desc: Test wrap with size.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, WrapWithSizeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value testWrapClass = nullptr;
    napi_define_class(
        env, "TestWrapClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, &testWrapClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testWrapClass, 0, nullptr, &instanceValue);

    const char* testWrapStr = "testWrapStr";
    size_t size = sizeof(*testWrapStr) / sizeof(char);
    napi_wrap_with_size(
        env, instanceValue, (void*)testWrapStr, [](napi_env env, void* data, void* hint) {}, nullptr, nullptr, size);

    char* tempTestStr = nullptr;
    napi_unwrap(env, instanceValue, (void**)&tempTestStr);
    ASSERT_STREQ(testWrapStr, tempTestStr);

    char* tempTestStr1 = nullptr;
    napi_remove_wrap(env, instanceValue, (void**)&tempTestStr1);
    ASSERT_STREQ(testWrapStr, tempTestStr1);
    
}

/**
 * @tc.name: CreateExternalWithSizeTest001
 * @tc.desc: Test create external with size.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateExternalWithSizeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char testStr[] = "test";
    size_t size = sizeof(testStr) / sizeof(char);
    napi_value external = nullptr;
    napi_create_external_with_size(
        env, (void*)testStr,
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)testStr, &external, size);

    ASSERT_CHECK_VALUE_TYPE(env, external, napi_external);
    void* tempExternal = nullptr;
    napi_get_value_external(env, external, &tempExternal);
    ASSERT_TRUE(tempExternal);
    ASSERT_EQ(tempExternal, testStr);
}

/**
 * @tc.name: BigArrayTest001
 * @tc.desc: Test is big int64 array and big uint64 array.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, BigArrayTest001, testing::ext::TestSize.Level1) {
    napi_env env = (napi_env) engine_;

    napi_value array = nullptr;
    napi_create_array(env, &array);
    ASSERT_NE(array, nullptr);
    bool isArray = false;
    napi_is_array(env, array, &isArray);
    ASSERT_TRUE(isArray);

    bool isBigInt64Array = true;
    napi_is_big_int64_array(env, array, &isBigInt64Array);
    ASSERT_EQ(isBigInt64Array, false);
    
    bool isBigUInt64Array = true;
    napi_is_big_uint64_array(env, array, &isBigUInt64Array);
    ASSERT_EQ(isBigUInt64Array, false);
}

/**
 * @tc.name: SharedArrayBufferTest001
 * @tc.desc: Test is shared array buffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SharedArrayBufferTest001, testing::ext::TestSize.Level1) {
    napi_env env = (napi_env) engine_;
    
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 1024;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    bool isSharedArrayBuffer = true;
    napi_is_shared_array_buffer(env, arrayBuffer, &isSharedArrayBuffer);
    ASSERT_EQ(isSharedArrayBuffer, false);
}