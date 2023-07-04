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

#include "ecmascript/builtins/builtins_atomics.h"

#include "ecmascript/builtins/builtins_array.h"
#include "ecmascript/builtins/builtins_typedarray.h"
#include "ecmascript/builtins/builtins_sharedarraybuffer.h"
#include "ecmascript/js_arraybuffer.h"
#include "ecmascript/base/atomic_helper.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/js_typed_array.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::builtins;

namespace panda::test {
using TypedArray = ecmascript::builtins::BuiltinsTypedArray;
class BuiltinsAtomicsTest : public testing::Test {
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

JSTypedArray *CreateTypedArray(JSThread *thread, const JSHandle<TaggedArray> &array)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();

    JSHandle<JSTaggedValue> jsarray(JSArray::CreateArrayFromList(thread, array));
    JSHandle<JSFunction> int8_array(env->GetInt8ArrayFunction());
    JSHandle<JSObject> globalObject(thread, env->GetGlobalObject());
    //  6 : test case
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*int8_array), 6);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue(*int8_array));
    ecmaRuntimeCallInfo1->SetThis(JSTaggedValue(*globalObject));
    ecmaRuntimeCallInfo1->SetCallArg(0, jsarray.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result = TypedArray::Int8ArrayConstructor(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsECMAObject());
    JSTypedArray *int8arr = JSTypedArray::Cast(reinterpret_cast<TaggedObject *>(result.GetRawData()));
    return int8arr;
}

JSTypedArray *CreateTypedArray(JSThread *thread, const JSHandle<TaggedArray> &array, DataViewType type)
{
    auto vm = thread->GetEcmaVM();
    auto env = vm->GetGlobalEnv();
    JSHandle<JSTaggedValue> jsarray(JSArray::CreateArrayFromList(thread, array));
    JSHandle<JSObject> globalObject(thread, env->GetGlobalObject());
    JSHandle<JSFunction> arrayFunc;
    JSTaggedValue result = JSTaggedValue::Hole();
    switch (type) {
        case DataViewType::BIGINT64: {
            arrayFunc = JSHandle<JSFunction>(env->GetBigInt64ArrayFunction());
            auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*arrayFunc), 6);
            ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue(*arrayFunc));
            ecmaRuntimeCallInfo1->SetThis(JSTaggedValue(*globalObject));
            ecmaRuntimeCallInfo1->SetCallArg(0, jsarray.GetTaggedValue());

            [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
            result = TypedArray::BigInt64ArrayConstructor(ecmaRuntimeCallInfo1);
            TestHelper::TearDownFrame(thread, prev);
            break;
        }
        case DataViewType::BIGUINT64: {
            arrayFunc = JSHandle<JSFunction>(env->GetBigUint64ArrayFunction());
            auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*arrayFunc), 6);
            ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue(*arrayFunc));
            ecmaRuntimeCallInfo1->SetThis(JSTaggedValue(*globalObject));
            ecmaRuntimeCallInfo1->SetCallArg(0, jsarray.GetTaggedValue());

            [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
            result = TypedArray::BigUint64ArrayConstructor(ecmaRuntimeCallInfo1);
            TestHelper::TearDownFrame(thread, prev);
            break;
        }
        case DataViewType::INT16: {
            arrayFunc = JSHandle<JSFunction>(env->GetInt16ArrayFunction());
            auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*arrayFunc), 6);
            ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue(*arrayFunc));
            ecmaRuntimeCallInfo1->SetThis(JSTaggedValue(*globalObject));
            ecmaRuntimeCallInfo1->SetCallArg(0, jsarray.GetTaggedValue());

            [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
            result = TypedArray::Int16ArrayConstructor(ecmaRuntimeCallInfo1);
            TestHelper::TearDownFrame(thread, prev);
            break;
        }
        case DataViewType::INT32: {
            arrayFunc = JSHandle<JSFunction>(env->GetInt32ArrayFunction());
            auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*arrayFunc), 6);
            ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue(*arrayFunc));
            ecmaRuntimeCallInfo1->SetThis(JSTaggedValue(*globalObject));
            ecmaRuntimeCallInfo1->SetCallArg(0, jsarray.GetTaggedValue());

            [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
            result = TypedArray::Int32ArrayConstructor(ecmaRuntimeCallInfo1);
            TestHelper::TearDownFrame(thread, prev);
            break;
        }
        case DataViewType::INT8: {
            arrayFunc = JSHandle<JSFunction>(env->GetInt8ArrayFunction());
            auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*arrayFunc), 6);
            ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue(*arrayFunc));
            ecmaRuntimeCallInfo1->SetThis(JSTaggedValue(*globalObject));
            ecmaRuntimeCallInfo1->SetCallArg(0, jsarray.GetTaggedValue());

            [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
            result = TypedArray::Int8ArrayConstructor(ecmaRuntimeCallInfo1);
            TestHelper::TearDownFrame(thread, prev);
            break;
        }
        case DataViewType::UINT16: {
            arrayFunc = JSHandle<JSFunction>(env->GetUint16ArrayFunction());
            auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*arrayFunc), 6);
            ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue(*arrayFunc));
            ecmaRuntimeCallInfo1->SetThis(JSTaggedValue(*globalObject));
            ecmaRuntimeCallInfo1->SetCallArg(0, jsarray.GetTaggedValue());

            [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
            result = TypedArray::Uint16ArrayConstructor(ecmaRuntimeCallInfo1);
            TestHelper::TearDownFrame(thread, prev);
            break;
        }
        case DataViewType::UINT32: {
            arrayFunc = JSHandle<JSFunction>(env->GetUint32ArrayFunction());
            auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*arrayFunc), 6);
            ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue(*arrayFunc));
            ecmaRuntimeCallInfo1->SetThis(JSTaggedValue(*globalObject));
            ecmaRuntimeCallInfo1->SetCallArg(0, jsarray.GetTaggedValue());

            [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
            result = TypedArray::Uint32ArrayConstructor(ecmaRuntimeCallInfo1);
            TestHelper::TearDownFrame(thread, prev);
            break;
        }
        case DataViewType::UINT8: {
            arrayFunc = JSHandle<JSFunction>(env->GetUint8ArrayFunction());
            auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*arrayFunc), 6);
            ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue(*arrayFunc));
            ecmaRuntimeCallInfo1->SetThis(JSTaggedValue(*globalObject));
            ecmaRuntimeCallInfo1->SetCallArg(0, jsarray.GetTaggedValue());

            [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
            result = TypedArray::Uint8ArrayConstructor(ecmaRuntimeCallInfo1);
            TestHelper::TearDownFrame(thread, prev);
            break;
        }
        default: {
            JSHandle<JSTaggedValue> undefined(thread, JSTaggedValue::Undefined());
            arrayFunc = JSHandle<JSFunction>(undefined);
            break;
        }
    }
    EXPECT_TRUE(result.IsECMAObject());
    JSTypedArray *arr = JSTypedArray::Cast(reinterpret_cast<TaggedObject *>(result.GetRawData()));
    return arr;
}

JSTaggedValue CreateSharedArrayBuffer(JSThread *thread, int32_t length)
{
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> sharedArrayBuffer(thread, env->GetSharedArrayBufferFunction().GetTaggedValue());
    JSHandle<JSObject> globalObject(thread, env->GetGlobalObject());
    // 6 : test case
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, sharedArrayBuffer.GetTaggedValue(), 6);
    ecmaRuntimeCallInfo->SetFunction(sharedArrayBuffer.GetTaggedValue());
    ecmaRuntimeCallInfo->SetThis(globalObject.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(length));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsSharedArrayBuffer::SharedArrayBufferConstructor(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    return result;
}

JSTypedArray *CreateInt32TypedArray(JSThread *thread, const JSHandle<JSArrayBuffer> &arrBuf)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();

    JSHandle<JSFunction> int32_array(env->GetInt32ArrayFunction());
    JSHandle<JSObject> globalObject(thread, env->GetGlobalObject());
    //  6 : test case
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*int32_array), 6);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue(*int32_array));
    ecmaRuntimeCallInfo1->SetThis(JSTaggedValue(*globalObject));
    ecmaRuntimeCallInfo1->SetCallArg(0, arrBuf.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result = TypedArray::Int32ArrayConstructor(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsECMAObject());
    JSTypedArray *int32arr = JSTypedArray::Cast(reinterpret_cast<TaggedObject *>(result.GetRawData()));
    return int32arr;
}

HWTEST_F_L0(BuiltinsAtomicsTest, Add_1)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(7));
    array->Set(thread, 1, JSTaggedValue(8));
    array->Set(thread, 2, JSTaggedValue(9));

    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array, DataViewType::UINT8));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(5)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Add(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 7);
}

HWTEST_F_L0(BuiltinsAtomicsTest, Add_2)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(10));

    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array, DataViewType::INT8));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Add(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 0);
}

HWTEST_F_L0(BuiltinsAtomicsTest, Add_3)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));

    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array,
                                                                                   DataViewType::UINT16));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Add(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 0);


    auto ecmaRuntimeCallInfos = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfos->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfos->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfos);
    JSTaggedValue results = BuiltinsAtomics::Load(ecmaRuntimeCallInfos);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(results.GetInt(), 2);
}

HWTEST_F_L0(BuiltinsAtomicsTest, SubAndAdd_1)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(5));
    array->Set(thread, 1, JSTaggedValue(0));
    array->Set(thread, 2, JSTaggedValue(0));
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array, DataViewType::INT16));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    BuiltinsAtomics::Sub(ecmaRuntimeCallInfo);
    JSTaggedValue addResult = BuiltinsAtomics::Add(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(addResult.GetInt(), 3);
}

HWTEST_F_L0(BuiltinsAtomicsTest, And_1)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(10));
    array->Set(thread, 0, JSTaggedValue(7));
    array->Set(thread, 1, JSTaggedValue(0));
    array->Set(thread, 2, JSTaggedValue(0));

    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array, DataViewType::INT32));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Add(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 7);
}

HWTEST_F_L0(BuiltinsAtomicsTest, And_2)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(10));
    array->Set(thread, 0, JSTaggedValue(7));
    array->Set(thread, 1, JSTaggedValue(0));
    array->Set(thread, 2, JSTaggedValue(0));

    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array,
                                                                                   DataViewType::UINT32));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::And(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 7);
    
    
    auto ecmaRuntimeCallInfos = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfos->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfos->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfos);
    result = BuiltinsAtomics::Load(ecmaRuntimeCallInfos);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 2);
}

HWTEST_F_L0(BuiltinsAtomicsTest, CompareExchange_1)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(5));
    array->Set(thread, 1, JSTaggedValue(0));
    array->Set(thread, 2, JSTaggedValue(0));
    
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array, DataViewType::UINT8));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 12);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(5)));
    ecmaRuntimeCallInfo->SetCallArg(3, JSTaggedValue(static_cast<int32_t>(2)));
    
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::CompareExchange(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 5);
}

HWTEST_F_L0(BuiltinsAtomicsTest, CompareExchange_2)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(5));
    array->Set(thread, 1, JSTaggedValue(0));
    array->Set(thread, 2, JSTaggedValue(0));
    
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array, DataViewType::INT8));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 12);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(5)));
    ecmaRuntimeCallInfo->SetCallArg(3, JSTaggedValue(static_cast<int32_t>(2)));
    
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::CompareExchange(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 5);
    
    
    auto ecmaRuntimeCallInfos = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfos->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfos->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfos);
    result = BuiltinsAtomics::Load(ecmaRuntimeCallInfos);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 2);
}

HWTEST_F_L0(BuiltinsAtomicsTest, TypedArrayCover)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(2));
    array->Set(thread, 1, JSTaggedValue(0));
    array->Set(thread, 2, JSTaggedValue(0));
    // UINT16
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array,
                                                                                   DataViewType::UINT16));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 12);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    ecmaRuntimeCallInfo->SetCallArg(3, JSTaggedValue(static_cast<int32_t>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::CompareExchange(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 2);
    // INT16
    obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array, DataViewType::INT16));
    ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 12);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    ecmaRuntimeCallInfo->SetCallArg(3, JSTaggedValue(static_cast<int32_t>(2)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    result = BuiltinsAtomics::CompareExchange(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 2);
    // UINT32
    obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array, DataViewType::UINT32));
    ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 12);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    ecmaRuntimeCallInfo->SetCallArg(3, JSTaggedValue(static_cast<int32_t>(2)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    result = BuiltinsAtomics::CompareExchange(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 2);
    // INT32
    obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array, DataViewType::INT32));
    ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 12);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    ecmaRuntimeCallInfo->SetCallArg(3, JSTaggedValue(static_cast<int32_t>(2)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    result = BuiltinsAtomics::CompareExchange(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 2);

    // Detached Buffer
    JSTaggedValue tagged = CreateSharedArrayBuffer(thread, 0);
    JSHandle<JSArrayBuffer> arrBuf(thread, JSArrayBuffer::Cast(reinterpret_cast<TaggedObject *>(tagged.GetRawData())));
    obj = JSHandle<JSTaggedValue>(thread, CreateInt32TypedArray(thread, arrBuf));
    arrBuf->SetArrayBufferData(thread, JSTaggedValue::Null());
    ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 12);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    ecmaRuntimeCallInfo->SetCallArg(3, JSTaggedValue(static_cast<int32_t>(2)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    result = BuiltinsAtomics::CompareExchange(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    EXPECT_TRUE(thread->HasPendingException());
    EXPECT_EQ(result, JSTaggedValue::Exception());
    thread->ClearException();
}

HWTEST_F_L0(BuiltinsAtomicsTest, Exchange_1)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(3));
    array->Set(thread, 1, JSTaggedValue(0));
    array->Set(thread, 2, JSTaggedValue(0));
    
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(6)));
    
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Exchange(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 3);
}

HWTEST_F_L0(BuiltinsAtomicsTest, Exchange_2)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(3));
    array->Set(thread, 1, JSTaggedValue(0));
    array->Set(thread, 2, JSTaggedValue(0));
    
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(6)));
    
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Exchange(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 3);
    
    
    auto ecmaRuntimeCallInfos = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfos->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfos->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfos);
    result = BuiltinsAtomics::Load(ecmaRuntimeCallInfos);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 6);
}

HWTEST_F_L0(BuiltinsAtomicsTest, Or_1)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(5));
    array->Set(thread, 1, JSTaggedValue(0));
    array->Set(thread, 2, JSTaggedValue(0));
    
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Or(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 5);
}

HWTEST_F_L0(BuiltinsAtomicsTest, Or_2)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(5));
    array->Set(thread, 1, JSTaggedValue(0));
    array->Set(thread, 2, JSTaggedValue(0));
    
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 12);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Or(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 5);
    
    
    auto ecmaRuntimeCallInfos = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfos->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfos->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfos);
    result = BuiltinsAtomics::Load(ecmaRuntimeCallInfos);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 7);
}

HWTEST_F_L0(BuiltinsAtomicsTest, Sub_1)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(5));
    array->Set(thread, 1, JSTaggedValue(0));
    array->Set(thread, 2, JSTaggedValue(0));
    
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Sub(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 5);
}

HWTEST_F_L0(BuiltinsAtomicsTest, Sub_2)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(0));
    array->Set(thread, 1, JSTaggedValue(5));
    array->Set(thread, 2, JSTaggedValue(0));
    
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(1)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Sub(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 5);
    
    auto ecmaRuntimeCallInfos = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfos->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfos->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(1)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfos);
    result = BuiltinsAtomics::Load(ecmaRuntimeCallInfos);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 3);
}

HWTEST_F_L0(BuiltinsAtomicsTest, Xor_1)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(5));
    array->Set(thread, 1, JSTaggedValue(7));
    array->Set(thread, 2, JSTaggedValue(0));
    
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(1)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Xor(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 7);
}

HWTEST_F_L0(BuiltinsAtomicsTest, Xor_2)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(5));
    array->Set(thread, 1, JSTaggedValue(7));
    array->Set(thread, 2, JSTaggedValue(0));
    
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(1)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    
    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Xor(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 7);
    
    auto ecmaRuntimeCallInfos = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfos->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfos->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(1)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfos);
    result = BuiltinsAtomics::Load(ecmaRuntimeCallInfos);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 5);
}

HWTEST_F_L0(BuiltinsAtomicsTest, IsLockFree_1)
{
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(1)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::IsLockFree(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_TRUE(result.ToBoolean());
}

HWTEST_F_L0(BuiltinsAtomicsTest, IsLockFree_2)
{
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::IsLockFree(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_TRUE(result.ToBoolean());
}

HWTEST_F_L0(BuiltinsAtomicsTest, IsLockFree_3)
{
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(4)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::IsLockFree(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_TRUE(result.ToBoolean());
}

HWTEST_F_L0(BuiltinsAtomicsTest, IsLockFree_4)
{
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(-3)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::IsLockFree(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_FALSE(result.ToBoolean());
}

HWTEST_F_L0(BuiltinsAtomicsTest, IsLockFree_5)
{
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(8)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::IsLockFree(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_TRUE(result.ToBoolean());
}


HWTEST_F_L0(BuiltinsAtomicsTest, Store_1)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(5));
    array->Set(thread, 1, JSTaggedValue(6));
    array->Set(thread, 2, JSTaggedValue(7));

    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Store(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetDouble(), 2);
}

HWTEST_F_L0(BuiltinsAtomicsTest, Store_2)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(5));
    array->Set(thread, 1, JSTaggedValue(6));
    array->Set(thread, 2, JSTaggedValue(7));

    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArray(thread, array));
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Store(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetDouble(), 2);

    auto ecmaRuntimeCallInfos = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfos->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfos->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfos->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfos);
    result = BuiltinsAtomics::Load(ecmaRuntimeCallInfos);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result.GetInt(), 2);
}

HWTEST_F_L0(BuiltinsAtomicsTest, Wait)
{
    ASSERT_NE(thread, nullptr);
    JSTaggedValue tagged = CreateSharedArrayBuffer(thread, 4);
    JSHandle<JSArrayBuffer> arrBuf(thread, JSArrayBuffer::Cast(reinterpret_cast<TaggedObject *>(tagged.GetRawData())));
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateInt32TypedArray(thread, arrBuf));

    // Not Equal
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 12);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));
    ecmaRuntimeCallInfo->SetCallArg(3, JSTaggedValue(static_cast<int32_t>(2)));

    auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsAtomics::Wait(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result, thread->GlobalConstants()->GetNotEqualString());

    // timeout
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(3, JSTaggedValue(static_cast<int32_t>(100)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    result = BuiltinsAtomics::Wait(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result, thread->GlobalConstants()->GetTimeoutString());
}

HWTEST_F_L0(BuiltinsAtomicsTest, Notify)
{
    ASSERT_NE(thread, nullptr);
    JSTaggedValue tagged = CreateSharedArrayBuffer(thread, 4);
    JSHandle<JSArrayBuffer> arrBuf(thread, JSArrayBuffer::Cast(reinterpret_cast<TaggedObject *>(tagged.GetRawData())));
    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateInt32TypedArray(thread, arrBuf));

    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 10);
    ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo->SetCallArg(0, obj.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(0)));
    ecmaRuntimeCallInfo->SetCallArg(2, JSTaggedValue(static_cast<int32_t>(2)));

    auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    [[maybe_unused]]JSTaggedValue result = BuiltinsAtomics::Notify(ecmaRuntimeCallInfo);
    TestHelper::TearDownFrame(thread, prev);
    ASSERT_EQ(result, JSTaggedValue(0));
}
}