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

#include "ecmascript/builtins/builtins_promise.h"

#include "ecmascript/builtins/builtins_array.h"
#include "ecmascript/ecma_string.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/jobs/micro_job_queue.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_promise.h"
#include "ecmascript/js_tagged_value-inl.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::builtins;

namespace panda::test {
using BuiltinsBase = panda::ecmascript::base::BuiltinsBase;
using JSArray = panda::ecmascript::JSArray;

class BuiltinsPromiseTest : public testing::Test {
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

// native function for race2 then_on_rejected()
JSTaggedValue TestPromiseRaceThenOnRejectd(EcmaRuntimeCallInfo *argv)
{
    JSHandle<JSTaggedValue> result = BuiltinsBase::GetCallArg(argv, 0);
    // 12345 : test case
    EXPECT_EQ(JSTaggedValue::SameValue(result.GetTaggedValue(), JSTaggedValue(12345)), true);
    return JSTaggedValue::Undefined();
}

// native function for all then_on_resolved()
JSTaggedValue TestPromiseAllThenOnResolved(EcmaRuntimeCallInfo *argv)
{
    JSHandle<JSTaggedValue> array = BuiltinsBase::GetCallArg(argv, 0);
    JSHandle<JSObject> objectArray = JSHandle<JSObject>::Cast(array);
    [[maybe_unused]] PropertyDescriptor desc(argv->GetThread());
    [[maybe_unused]] bool result1 = JSObject::GetOwnProperty(
        argv->GetThread(), objectArray, JSHandle<JSTaggedValue>(argv->GetThread(), JSTaggedValue(0)), desc);
    EXPECT_TRUE(result1);
    JSHandle<JSTaggedValue> value1 = desc.GetValue();
    // 111 : test case
    EXPECT_EQ(JSTaggedValue::SameValue(value1.GetTaggedValue(), JSTaggedValue(111)), true);
    [[maybe_unused]] bool result2 = JSObject::GetOwnProperty(
        argv->GetThread(), objectArray, JSHandle<JSTaggedValue>(argv->GetThread(), JSTaggedValue(1)), desc);
    EXPECT_TRUE(result2);
    JSHandle<JSTaggedValue> value2 = desc.GetValue();
    // 222 : test case
    EXPECT_EQ(JSTaggedValue::SameValue(value2.GetTaggedValue(), JSTaggedValue(222)), true);
    return JSTaggedValue::Undefined();
}

// native function for catch catch_on_rejected()
JSTaggedValue TestPromiseCatch(EcmaRuntimeCallInfo *argv)
{
    JSHandle<JSTaggedValue> result = BuiltinsBase::GetCallArg(argv, 0);
    // 3 : test case
    EXPECT_EQ(JSTaggedValue::SameValue(result.GetTaggedValue(), JSTaggedValue(3)), true);
    return JSTaggedValue::Undefined();
}

// native function for then then_on_resolved()
JSTaggedValue TestPromiseThenOnResolved(EcmaRuntimeCallInfo *argv)
{
    auto factory = argv->GetThread()->GetEcmaVM()->GetFactory();
    JSHandle<JSTaggedValue> result = BuiltinsBase::GetCallArg(argv, 0);
    auto expect = factory->NewFromASCII("resolve");
    EXPECT_EQ(JSTaggedValue::SameValue(result.GetTaggedValue(), expect.GetTaggedValue()), true);
    return JSTaggedValue::Undefined();
}

// native function for then then_on_rejected()
JSTaggedValue TestPromiseThenOnRejected(EcmaRuntimeCallInfo *argv)
{
    auto factory = argv->GetThread()->GetEcmaVM()->GetFactory();
    JSHandle<JSTaggedValue> result = BuiltinsBase::GetCallArg(argv, 0);
    auto expect = factory->NewFromASCII("reject");
    EXPECT_EQ(JSTaggedValue::SameValue(result.GetTaggedValue(), expect.GetTaggedValue()), true);
    return JSTaggedValue::Undefined();
}

/*
 * @tc.name: Reject1
 * @tc.desc: The reject method receives a number.
 * @tc.type: FUNC
 */
HWTEST_F_L0(BuiltinsPromiseTest, Reject1)
{
    JSHandle<GlobalEnv> env = instance->GetGlobalEnv();

    JSHandle<JSFunction> promise = JSHandle<JSFunction>::Cast(env->GetPromiseFunction());
    JSHandle<JSTaggedValue> paramMsg(thread, JSTaggedValue(3));

    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo1->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, paramMsg.GetTaggedValue());

    /**
     * @tc.steps: var p1 = Promise.reject(3).
     */
    [[maybe_unused]] auto prevReject = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result = BuiltinsPromise::Reject(ecmaRuntimeCallInfo1);
    JSHandle<JSPromise> rejectPromise(thread, result);
    EXPECT_EQ(rejectPromise->GetPromiseState(), PromiseState::REJECTED);
    EXPECT_EQ(JSTaggedValue::SameValue(rejectPromise->GetPromiseResult(), JSTaggedValue(3)), true);
    TestHelper::TearDownFrame(thread, prevReject);
}

/*
 * @tc.name: Reject2
 * @tc.desc: The reject method receives a promise object.
 * @tc.type: FUNC
 */
HWTEST_F_L0(BuiltinsPromiseTest, Reject2)
{
    ObjectFactory *factory = instance->GetFactory();
    JSHandle<GlobalEnv> env = instance->GetGlobalEnv();

    // constructor promise1
    JSHandle<JSFunction> promise = JSHandle<JSFunction>::Cast(env->GetPromiseFunction());
    JSHandle<JSTaggedValue> paramMsg1 =
        JSHandle<JSTaggedValue>::Cast(factory->NewFromASCII("Promise reject"));

    /**
     * @tc.steps: step1. var p1 = Promise.reject("Promise reject")
     */
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, promise.GetTaggedValue(), 6);
    ecmaRuntimeCallInfo->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, paramMsg1.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsPromise::Reject(ecmaRuntimeCallInfo);
    JSHandle<JSPromise> promise1(thread, result);
    EXPECT_EQ(promise1->GetPromiseState(), PromiseState::REJECTED);
    EXPECT_EQ(JSTaggedValue::SameValue(promise1->GetPromiseResult(), paramMsg1.GetTaggedValue()), true);
    TestHelper::TearDownFrame(thread, prev);

    /**
     * @tc.steps: step2. var p2 = Promise.reject(p1)
     */
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, promise.GetTaggedValue(), 6);
    ecmaRuntimeCallInfo1->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, promise1.GetTaggedValue());
    [[maybe_unused]] auto prev1 = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);

    JSTaggedValue result1 = BuiltinsPromise::Reject(ecmaRuntimeCallInfo1);
    JSHandle<JSPromise> promise2(thread, result1);
    EXPECT_NE(*promise1, *promise2);
    EXPECT_EQ(promise2->GetPromiseState(), PromiseState::REJECTED);
    EXPECT_EQ(
        JSTaggedValue::SameValue(promise2->GetPromiseResult(), JSTaggedValue(promise1.GetTaggedValue().GetRawData())),
        true);
    TestHelper::TearDownFrame(thread, prev);
}

/*
 * @tc.name: Resolve1
 * @tc.desc: The resolve method receives a number.
 * @tc.type: FUNC
 */
HWTEST_F_L0(BuiltinsPromiseTest, Resolve1)
{
    JSHandle<GlobalEnv> env = instance->GetGlobalEnv();

    JSHandle<JSFunction> promise = JSHandle<JSFunction>::Cast(env->GetPromiseFunction());
    JSHandle<JSTaggedValue> paramMsg(thread, JSTaggedValue(5));

    /**
     * @tc.steps: step1. var p1 = Promise.resolve(12345)
     */
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo1->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, paramMsg.GetTaggedValue());

    [[maybe_unused]] auto prevReject = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result = BuiltinsPromise::Resolve(ecmaRuntimeCallInfo1);
    JSHandle<JSPromise> rejectPromise(thread, result);
    EXPECT_EQ(rejectPromise->GetPromiseState(), PromiseState::FULFILLED);
    EXPECT_EQ(JSTaggedValue::SameValue(rejectPromise->GetPromiseResult(), JSTaggedValue(5)), true);
    TestHelper::TearDownFrame(thread, prevReject);
}

/*
 * @tc.name: Resolve2
 * @tc.desc: The resolve method receives a promise object.
 * @tc.type: FUNC
 */
HWTEST_F_L0(BuiltinsPromiseTest, Resolve2)
{
    ObjectFactory *factory = instance->GetFactory();
    JSHandle<GlobalEnv> env = instance->GetGlobalEnv();

    // constructor promise1
    JSHandle<JSFunction> promise = JSHandle<JSFunction>::Cast(env->GetPromiseFunction());
    JSHandle<JSTaggedValue> paramMsg1 =
        JSHandle<JSTaggedValue>::Cast(factory->NewFromASCII("Promise reject"));

    /**
     * @tc.steps: step1. var p1 = Promise.reject("Promise reject")
     */
    auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, promise.GetTaggedValue(), 6);
    ecmaRuntimeCallInfo->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo->SetCallArg(0, paramMsg1.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
    JSTaggedValue result = BuiltinsPromise::Reject(ecmaRuntimeCallInfo);
    JSHandle<JSPromise> promise1(thread, result);
    EXPECT_EQ(promise1->GetPromiseState(), PromiseState::REJECTED);
    EXPECT_EQ(JSTaggedValue::SameValue(promise1->GetPromiseResult(), paramMsg1.GetTaggedValue()), true);
    TestHelper::TearDownFrame(thread, prev);

    // promise1 Enter Reject() as a parameter.
    /**
     * @tc.steps: step2. var p2 = Promise.resolve(p1)
     */
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, promise.GetTaggedValue(), 6);
    ecmaRuntimeCallInfo1->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, promise1.GetTaggedValue());
    [[maybe_unused]] auto prev1 = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);

    JSTaggedValue result1 = BuiltinsPromise::Resolve(ecmaRuntimeCallInfo1);
    JSHandle<JSPromise> promise2(thread, result1);
    EXPECT_EQ(*promise1, *promise2);
    EXPECT_EQ(promise2->GetPromiseState(), PromiseState::REJECTED);
    EXPECT_EQ(JSTaggedValue::SameValue(promise2->GetPromiseResult(), paramMsg1.GetTaggedValue()), true);
    TestHelper::TearDownFrame(thread, prev1);
}

/*
 * @tc.name: Race1
 * @tc.desc: The race method receives an array.
 * @tc.type: FUNC
 */
HWTEST_F_L0(BuiltinsPromiseTest, Race1)
{
    JSHandle<GlobalEnv> env = instance->GetGlobalEnv();

    JSHandle<JSFunction> promise = JSHandle<JSFunction>::Cast(env->GetPromiseFunction());
    JSHandle<JSTaggedValue> paramMsg1(thread, JSTaggedValue(12345));
    JSHandle<JSTaggedValue> paramMsg2(thread, JSTaggedValue(6789));

    /**
     * @tc.steps: step1. var p1 = Promise.reject(12345)
     */
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo1->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, paramMsg1.GetTaggedValue());

    [[maybe_unused]] auto prevReject = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result1 = BuiltinsPromise::Reject(ecmaRuntimeCallInfo1);
    JSHandle<JSPromise> rejectPromise(thread, result1);
    EXPECT_EQ(rejectPromise->GetPromiseState(), PromiseState::REJECTED);
    EXPECT_EQ(JSTaggedValue::SameValue(rejectPromise->GetPromiseResult(), JSTaggedValue(12345)), true);
    TestHelper::TearDownFrame(thread, prevReject);

    /**
     * @tc.steps: step2. var p2 = Promise.resolve(6789)
     */
    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo2->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, paramMsg2.GetTaggedValue());

    [[maybe_unused]] auto prevResolve = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result2 = BuiltinsPromise::Resolve(ecmaRuntimeCallInfo2);
    JSHandle<JSPromise> resolvePromise(thread, result2);
    EXPECT_EQ(resolvePromise->GetPromiseState(), PromiseState::FULFILLED);
    EXPECT_EQ(JSTaggedValue::SameValue(resolvePromise->GetPromiseResult(), JSTaggedValue(6789)), true);
    TestHelper::TearDownFrame(thread, prevResolve);

    /**
     * @tc.steps: step3. Construct an array with two elements p1 and p2. array = [p1. p2]
     */
    JSHandle<JSObject> array(JSArray::ArrayCreate(thread, JSTaggedNumber(2)));
    PropertyDescriptor desc(thread, JSHandle<JSTaggedValue>::Cast(rejectPromise));
    JSArray::DefineOwnProperty(thread, array, JSHandle<JSTaggedValue>(thread, JSTaggedValue(0)), desc);

    PropertyDescriptor desc1(thread, JSHandle<JSTaggedValue>::Cast(resolvePromise));
    JSArray::DefineOwnProperty(thread, array, JSHandle<JSTaggedValue>(thread, JSTaggedValue(1)), desc1);

    /**
     * @tc.steps: step4. var p3 = Promise.race([p1,p2]);
     */
    auto ecmaRuntimeCallInfo4 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo4->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo4->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo4->SetCallArg(0, array.GetTaggedValue());

    [[maybe_unused]] auto prev4 = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo4);
    JSTaggedValue result4 = BuiltinsPromise::Race(ecmaRuntimeCallInfo4);
    JSHandle<JSPromise> racePromise(thread, result4);
    EXPECT_EQ(racePromise->GetPromiseState(), PromiseState::PENDING);
    EXPECT_EQ(racePromise->GetPromiseResult().IsUndefined(), true);
    TestHelper::TearDownFrame(thread, prev4);
}

/*
 * @tc.name: Race2
 * @tc.desc: The Race method receives an array, uses the Then method to save the task in the task queue, and outputs
 * the execution result of the queue.
 * @tc.type: FUNC
 */
HWTEST_F_L0(BuiltinsPromiseTest, Race2)
{
    ObjectFactory *factory = instance->GetFactory();
    JSHandle<GlobalEnv> env = instance->GetGlobalEnv();

    JSHandle<JSFunction> promise = JSHandle<JSFunction>::Cast(env->GetPromiseFunction());
    JSHandle<JSTaggedValue> paramMsg1(thread, JSTaggedValue(12345));
    JSHandle<JSTaggedValue> paramMsg2(thread, JSTaggedValue(6789));

    /**
     * @tc.steps: step1. var p1 = Promise.reject(12345)
     */
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo1->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, paramMsg1.GetTaggedValue());

    [[maybe_unused]] auto prevReject = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result1 = BuiltinsPromise::Reject(ecmaRuntimeCallInfo1);
    JSHandle<JSPromise> rejectPromise(thread, result1);
    EXPECT_EQ(rejectPromise->GetPromiseState(), PromiseState::REJECTED);
    EXPECT_EQ(JSTaggedValue::SameValue(rejectPromise->GetPromiseResult(), JSTaggedValue(12345)), true);
    TestHelper::TearDownFrame(thread, prevReject);

    /**
     * @tc.steps: step2. var p2 = Promise.resolve(6789)
     */
    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo2->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, paramMsg2.GetTaggedValue());

    [[maybe_unused]] auto prevResolve = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result2 = BuiltinsPromise::Resolve(ecmaRuntimeCallInfo2);
    JSHandle<JSPromise> resolvePromise(thread, result2);
    EXPECT_EQ(resolvePromise->GetPromiseState(), PromiseState::FULFILLED);
    EXPECT_EQ(JSTaggedValue::SameValue(resolvePromise->GetPromiseResult(), JSTaggedValue(6789)), true);
    TestHelper::TearDownFrame(thread, prevResolve);

    /**
     * @tc.steps: step3. Construct an array with two elements p1 and p2. array = [p1. p2]
     */
    JSHandle<JSObject> array(JSArray::ArrayCreate(thread, JSTaggedNumber(2)));
    PropertyDescriptor desc(thread, JSHandle<JSTaggedValue>::Cast(rejectPromise));
    JSArray::DefineOwnProperty(thread, array, JSHandle<JSTaggedValue>(thread, JSTaggedValue(0)), desc);

    PropertyDescriptor desc1(thread, JSHandle<JSTaggedValue>::Cast(resolvePromise));
    JSArray::DefineOwnProperty(thread, array, JSHandle<JSTaggedValue>(thread, JSTaggedValue(1)), desc1);

    /**
     * @tc.steps: step4. var p3 = Promise.race([p1,p2]);
     */
    auto ecmaRuntimeCallInfo4 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo4->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo4->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo4->SetCallArg(0, array.GetTaggedValue());

    [[maybe_unused]] auto prev4 = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo4);
    JSTaggedValue result4 = BuiltinsPromise::Race(ecmaRuntimeCallInfo4);
    JSHandle<JSPromise> racePromise(thread, result4);
    EXPECT_EQ(racePromise->GetPromiseState(), PromiseState::PENDING);
    EXPECT_EQ(racePromise->GetPromiseResult().IsUndefined(), true);
    TestHelper::TearDownFrame(thread, prev4);

    /**
     * @tc.steps: step5. p3.then((resolve)=>{print(resolve)}, (reject)=>{print(reject)})
     */
    JSHandle<JSFunction> native_func_race_then_onrejected =
        factory->NewJSFunction(env, reinterpret_cast<void *>(TestPromiseRaceThenOnRejectd));
    auto ecmaRuntimeCallInfo5 = TestHelper::CreateEcmaRuntimeCallInfo(thread, racePromise.GetTaggedValue(), 8);
    ecmaRuntimeCallInfo5->SetFunction(racePromise.GetTaggedValue());
    ecmaRuntimeCallInfo5->SetThis(racePromise.GetTaggedValue());
    ecmaRuntimeCallInfo5->SetCallArg(0, JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo5->SetCallArg(1, native_func_race_then_onrejected.GetTaggedValue());

    [[maybe_unused]] auto prev5 = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo5);
    JSTaggedValue thenResult = BuiltinsPromise::Then(ecmaRuntimeCallInfo5);
    JSHandle<JSPromise> thenPromise(thread, thenResult);

    EXPECT_EQ(thenPromise->GetPromiseState(), PromiseState::PENDING);
    EXPECT_TRUE(thenPromise->GetPromiseResult().IsUndefined());
    TestHelper::TearDownFrame(thread, prev5);

    /**
     * @tc.steps: step6. execute promise queue
     */
    auto microJobQueue = instance->GetMicroJobQueue();
    if (!thread->HasPendingException()) {
        job::MicroJobQueue::ExecutePendingJob(thread, microJobQueue);
    }
}

/*
 * @tc.name: All
 * @tc.desc: The All method receives an array, uses the Then method to save the task in the task queue, and outputs the
 * execution result of the queue.
 * @tc.type: FUNC
 */
HWTEST_F_L0(BuiltinsPromiseTest, All)
{
    ObjectFactory *factory = instance->GetFactory();
    JSHandle<GlobalEnv> env = instance->GetGlobalEnv();

    JSHandle<JSFunction> promise = JSHandle<JSFunction>::Cast(env->GetPromiseFunction());
    JSHandle<JSTaggedValue> paramMsg1(thread, JSTaggedValue(111));
    JSHandle<JSTaggedValue> paramMsg2(thread, JSTaggedValue(222));

    /**
     * @tc.steps: step1. var p1 = Promise.resolve(111)
     */
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo1->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, paramMsg1.GetTaggedValue());

    [[maybe_unused]] auto prevReject = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result1 = BuiltinsPromise::Resolve(ecmaRuntimeCallInfo1);
    JSHandle<JSPromise> resolvePromise1(thread, result1);
    EXPECT_EQ(resolvePromise1->GetPromiseState(), PromiseState::FULFILLED);
    EXPECT_EQ(JSTaggedValue::SameValue(resolvePromise1->GetPromiseResult(), JSTaggedValue(111)), true);
    TestHelper::TearDownFrame(thread, prevReject);

    /**
     * @tc.steps: step2. var p2 = Promise.resolve(222)
     */
    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo2->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, paramMsg2.GetTaggedValue());

    [[maybe_unused]] auto prevResolve = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result2 = BuiltinsPromise::Resolve(ecmaRuntimeCallInfo2);
    JSHandle<JSPromise> resolvePromise2(thread, result2);
    EXPECT_EQ(resolvePromise2->GetPromiseState(), PromiseState::FULFILLED);
    EXPECT_EQ(JSTaggedValue::SameValue(resolvePromise2->GetPromiseResult(), JSTaggedValue(222)), true);
    TestHelper::TearDownFrame(thread, prevResolve);

    /**
     * @tc.steps: step3. Construct an array with two elements p1 and p2. array = [p1. p2]
     */
    JSHandle<JSObject> array(JSArray::ArrayCreate(thread, JSTaggedNumber(2)));
    PropertyDescriptor desc(thread, JSHandle<JSTaggedValue>::Cast(resolvePromise1));
    JSArray::DefineOwnProperty(thread, array, JSHandle<JSTaggedValue>(thread, JSTaggedValue(0)), desc);

    PropertyDescriptor desc1(thread, JSHandle<JSTaggedValue>::Cast(resolvePromise2));
    JSArray::DefineOwnProperty(thread, array, JSHandle<JSTaggedValue>(thread, JSTaggedValue(1)), desc1);

    /**
     * @tc.steps: step4. var p3 = Promise.all([p1,p2]);
     */
    auto ecmaRuntimeCallInfo4 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo4->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo4->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo4->SetCallArg(0, array.GetTaggedValue());

    [[maybe_unused]] auto prev4 = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo4);
    JSTaggedValue result4 = BuiltinsPromise::All(ecmaRuntimeCallInfo4);
    JSHandle<JSPromise> allPromise(thread, result4);
    EXPECT_EQ(allPromise->GetPromiseState(), PromiseState::PENDING);
    EXPECT_EQ(allPromise->GetPromiseResult().IsUndefined(), true);
    TestHelper::TearDownFrame(thread, prev4);

    /**
     * @tc.steps: step5. p3.then((resolve)=>{print(resolve)}, (reject)=>{print(reject)});
     */
    JSHandle<JSFunction> nativeFuncRaceThenOnResolved =
        factory->NewJSFunction(env, reinterpret_cast<void *>(TestPromiseAllThenOnResolved));
    auto ecmaRuntimeCallInfo5 = TestHelper::CreateEcmaRuntimeCallInfo(thread, allPromise.GetTaggedValue(), 8);
    ecmaRuntimeCallInfo5->SetFunction(allPromise.GetTaggedValue());
    ecmaRuntimeCallInfo5->SetThis(allPromise.GetTaggedValue());
    ecmaRuntimeCallInfo5->SetCallArg(0, nativeFuncRaceThenOnResolved.GetTaggedValue());
    ecmaRuntimeCallInfo5->SetCallArg(1, nativeFuncRaceThenOnResolved.GetTaggedValue());

    [[maybe_unused]] auto prev5 = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo5);
    JSTaggedValue thenResult = BuiltinsPromise::Then(ecmaRuntimeCallInfo5);
    JSHandle<JSPromise> thenPromise(thread, thenResult);

    EXPECT_EQ(thenPromise->GetPromiseState(), PromiseState::PENDING);
    EXPECT_TRUE(thenPromise->GetPromiseResult().IsUndefined());
    TestHelper::TearDownFrame(thread, prev5);

    /**
     * @tc.steps: step6. execute promise queue
     */
    auto microJobQueue = instance->GetMicroJobQueue();
    if (!thread->HasPendingException()) {
        job::MicroJobQueue::ExecutePendingJob(thread, microJobQueue);
    }
}

/*
 * @tc.name: Catch
 * @tc.desc: test Catch() method
 * @tc.type: FUNC
 */
HWTEST_F_L0(BuiltinsPromiseTest, Catch)
{
    auto env = instance->GetGlobalEnv();
    auto factory = instance->GetFactory();

    JSHandle<JSFunction> promise = JSHandle<JSFunction>::Cast(env->GetPromiseFunction());
    JSHandle<JSTaggedValue> paramMsg1(thread, JSTaggedValue(3));

    /**
     * @tc.steps: step1. var p1 = Promise.reject(3)
     */
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo1->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, paramMsg1.GetTaggedValue());

    [[maybe_unused]] auto prevReject = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result = BuiltinsPromise::Reject(ecmaRuntimeCallInfo1);
    JSHandle<JSPromise> rejectPromise(thread, result);
    EXPECT_EQ(rejectPromise->GetPromiseState(), PromiseState::REJECTED);
    EXPECT_EQ(JSTaggedValue::SameValue(rejectPromise->GetPromiseResult(), JSTaggedValue(3)), true);
    TestHelper::TearDownFrame(thread, prevReject);

    /**
     * @tc.steps: step2. p1 invokes catch()
     */
    JSHandle<JSFunction> testPromiseCatch = factory->NewJSFunction(env, reinterpret_cast<void *>(TestPromiseCatch));
    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, rejectPromise.GetTaggedValue(), 6);
    ecmaRuntimeCallInfo2->SetFunction(rejectPromise.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetThis(rejectPromise.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, testPromiseCatch.GetTaggedValue());

    [[maybe_unused]] auto prevCatch = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue catchResult = BuiltinsPromise::Catch(ecmaRuntimeCallInfo2);
    JSHandle<JSPromise> catchPromise(thread, catchResult);

    EXPECT_EQ(catchPromise->GetPromiseState(), PromiseState::PENDING);
    EXPECT_EQ(catchPromise->GetPromiseResult().IsUndefined(), true);
    TestHelper::TearDownFrame(thread, prevCatch);

    /**
     * @tc.steps: step3. execute promise queue
     */
    auto microJobQueue = instance->GetMicroJobQueue();
    if (!thread->HasPendingException()) {
        job::MicroJobQueue::ExecutePendingJob(thread, microJobQueue);
    }
}

/*
 * @tc.name: ThenResolve
 * @tc.desc: Testing the Then() function with the Resolve() function
 * @tc.type: FUNC
 */
HWTEST_F_L0(BuiltinsPromiseTest, ThenResolve)
{
    auto env = instance->GetGlobalEnv();
    auto factory = instance->GetFactory();

    JSHandle<JSFunction> promise = JSHandle<JSFunction>::Cast(env->GetPromiseFunction());
    JSHandle<JSTaggedValue> paramMsg = JSHandle<JSTaggedValue>::Cast(factory->NewFromASCII("resolve"));

    /**
     * @tc.steps: step1. var p1 = Promise.resolve("resolve")
     */
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo1->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, paramMsg.GetTaggedValue());

    [[maybe_unused]] auto prevReject = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result = BuiltinsPromise::Resolve(ecmaRuntimeCallInfo1);
    JSHandle<JSPromise> resolvePromise(thread, result);
    EXPECT_EQ(resolvePromise->GetPromiseState(), PromiseState::FULFILLED);
    EXPECT_EQ(JSTaggedValue::SameValue(resolvePromise->GetPromiseResult(), paramMsg.GetTaggedValue()), true);
    TestHelper::TearDownFrame(thread, prevReject);

    /**
     * @tc.steps: step2. p1 invokes then()
     */
    JSHandle<JSFunction> testPromiseThenOnResolved =
        factory->NewJSFunction(env, reinterpret_cast<void *>(TestPromiseThenOnResolved));
    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, resolvePromise.GetTaggedValue(), 8);
    ecmaRuntimeCallInfo2->SetFunction(resolvePromise.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetThis(resolvePromise.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, testPromiseThenOnResolved.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(1, JSTaggedValue::Undefined());

    [[maybe_unused]] auto prevThen = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue thenResult = BuiltinsPromise::Then(ecmaRuntimeCallInfo2);
    JSHandle<JSPromise> thenPromise(thread, thenResult);

    EXPECT_EQ(thenPromise->GetPromiseState(), PromiseState::PENDING);
    EXPECT_EQ(thenPromise->GetPromiseResult().IsUndefined(), true);
    TestHelper::TearDownFrame(thread, prevThen);

    /**
     * @tc.steps: step3.  execute promise queue
     */
    auto microJobQueue = instance->GetMicroJobQueue();
    if (!thread->HasPendingException()) {
        job::MicroJobQueue::ExecutePendingJob(thread, microJobQueue);
    }
}

/*
 * @tc.name: ThenReject
 * @tc.desc: Testing the Then() function with the Reject() function
 * @tc.type: FUNC
 */
HWTEST_F_L0(BuiltinsPromiseTest, ThenReject)
{
    auto env = instance->GetGlobalEnv();
    auto factory = instance->GetFactory();

    JSHandle<JSFunction> promise = JSHandle<JSFunction>::Cast(env->GetPromiseFunction());
    JSHandle<JSTaggedValue> paramMsg = JSHandle<JSTaggedValue>::Cast(factory->NewFromASCII("reject"));

    /**
     * @tc.steps: step1. var p1 = Promise.Reject(5)
     */
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*promise), 6);
    ecmaRuntimeCallInfo1->SetFunction(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetThis(promise.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, paramMsg.GetTaggedValue());

    [[maybe_unused]] auto prevReject = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result = BuiltinsPromise::Reject(ecmaRuntimeCallInfo1);
    JSHandle<JSPromise> rejectPromise(thread, result);
    EXPECT_EQ(rejectPromise->GetPromiseState(), PromiseState::REJECTED);
    EXPECT_EQ(JSTaggedValue::SameValue(rejectPromise->GetPromiseResult(), paramMsg.GetTaggedValue()), true);
    TestHelper::TearDownFrame(thread, prevReject);

    /**
     * @tc.steps: step1. p1 invokes then()
     */
    JSHandle<JSFunction> testPromiseThenOnRejected =
        factory->NewJSFunction(env, reinterpret_cast<void *>(TestPromiseThenOnRejected));
    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, rejectPromise.GetTaggedValue(), 8);
    ecmaRuntimeCallInfo2->SetFunction(rejectPromise.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetThis(rejectPromise.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, testPromiseThenOnRejected.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(1, testPromiseThenOnRejected.GetTaggedValue());

    [[maybe_unused]] auto prevThen = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue thenResult = BuiltinsPromise::Then(ecmaRuntimeCallInfo2);
    JSHandle<JSPromise> thenPromise(thread, thenResult);

    EXPECT_EQ(thenPromise->GetPromiseState(), PromiseState::PENDING);
    EXPECT_EQ(thenPromise->GetPromiseResult().IsUndefined(), true);
    TestHelper::TearDownFrame(thread, prevThen);
    /**
     * @tc.steps: step3.  execute promise queue
     */
    auto microJobQueue = instance->GetMicroJobQueue();
    if (!thread->HasPendingException()) {
        job::MicroJobQueue::ExecutePendingJob(thread, microJobQueue);
    }
}
}  // namespace panda::test
