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

#ifndef ECMASCRIPT_SAMPLES_RECORD_H
#define ECMASCRIPT_SAMPLES_RECORD_H

#include <atomic>
#include <ctime>
#include <cstring>
#include <fstream>
#include <semaphore.h>

#include "ecmascript/compiler/gate_meta_data.h"
#include "ecmascript/deoptimizer/deoptimizer.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/jspandafile/method_literal.h"
#include "ecmascript/mem/c_containers.h"
#include "libpandabase/os/mutex.h"

namespace panda::ecmascript {
const int MAX_STACK_SIZE = 128; // 128:the maximum size of the js stack
const int MAX_NODE_COUNT = 20000; // 20000:the maximum size of the array
const int MIN_TIME_DELTA = 10; // 10: the minimum value of the time delta
const int QUEUE_CAPACITY = 51; // the capacity of the circular queue is QUEUE_CAPACITY - 1
const size_t NAPI_CALL_SETP = 2; // 2: step size of the variable napiCallIdx in while loop
const size_t PRE_IDX_RANGE = 5; // 5: length of variable preIdx looping backward
enum class RunningState : size_t {
    OTHER = 0,
    GC,
    CINT,
    AINT,
    AOT,
    BUILTIN,
    NAPI,
    ARKUI_ENGINE,
    RUNTIME
};

struct MethodKey {
    void *methodIdentifier = nullptr;
    RunningState state = RunningState::OTHER;
    kungfu::DeoptType deoptType = kungfu::DeoptType::NOTCHECK;
    bool operator < (const MethodKey &methodKey) const
    {
        return state < methodKey.state ||
               (state == methodKey.state && methodIdentifier < methodKey.methodIdentifier) ||
               (state == methodKey.state && methodIdentifier == methodKey.methodIdentifier &&
               deoptType < methodKey.deoptType);
    }
};

struct NodeKey {
    struct MethodKey methodKey = {0};
    int parentId = 0;
    bool operator < (const NodeKey &nodeKey) const
    {
        return parentId < nodeKey.parentId ||
               (parentId == nodeKey.parentId && methodKey < nodeKey.methodKey);
    }
};

struct FrameInfo {
    std::string codeType = "";
    std::string functionName = "";
    int columnNumber = 0;
    int lineNumber = 0;
    int scriptId = 0;
    std::string url = "";
};

struct CpuProfileNode {
    int id = 0;
    int parentId = 0;
    int hitCount = 0;
    struct FrameInfo codeEntry;
    CVector<int> children;
};

struct ProfileInfo {
    uint64_t tid = 0;
    uint64_t startTime = 0;
    uint64_t stopTime = 0;
    struct CpuProfileNode nodes[MAX_NODE_COUNT];
    int nodeCount = 0;
    CVector<int> samples;
    CVector<int> timeDeltas;
    // state time statistic
    uint64_t gcTime = 0;
    uint64_t cInterpreterTime = 0;
    uint64_t asmInterpreterTime = 0;
    uint64_t aotTime = 0;
    uint64_t builtinTime = 0;
    uint64_t napiTime = 0;
    uint64_t arkuiEngineTime = 0;
    uint64_t runtimeTime = 0;
    uint64_t otherTime = 0;
};

struct FrameInfoTemp {
    char codeType[20] = {0}; // 20:the maximum size of the codeType
    char functionName[100] = {0}; // 100:the maximum size of the functionName
    int columnNumber = 0;
    int lineNumber = 0;
    int scriptId = 0;
    char url[500] = {0}; // 500:the maximum size of the url
    struct MethodKey methodKey = {0};
};

struct FrameStackAndInfo {
    struct FrameInfoTemp frameInfoTemps[MAX_STACK_SIZE] = {};
    struct MethodKey frameStack[MAX_STACK_SIZE] = {};
    int frameInfoTempsLength {};
    int frameStackLength {};
    uint64_t timeStamp {};
};

class SamplesQueue {
public:
    SamplesQueue() = default;
    ~SamplesQueue() = default;

    NO_COPY_SEMANTIC(SamplesQueue);
    NO_MOVE_SEMANTIC(SamplesQueue);

    void PostFrame(FrameInfoTemp *frameInfoTemps, MethodKey *frameStack,
                   int frameInfoTempsLength, int frameStackLength);
    void PostNapiFrame(CVector<FrameInfoTemp> &napiFrameInfoTemps,
                       CVector<MethodKey> &napiFrameStack);
    FrameStackAndInfo *PopFrame();
    bool IsEmpty();
    bool IsFull();
    int GetSize();
    int GetFrontIndex();
    int GetRearIndex();
    bool CheckAndCopy(char *dest, size_t length, const char *src) const;
    FrameStackAndInfo GetFront();
    FrameStackAndInfo GetRear();
    uint64_t GetLastPostTime();

private:
    FrameStackAndInfo frames_[QUEUE_CAPACITY] = {};
    int front_ = 0;
    int rear_ = 0;
    os::memory::Mutex mtx_;
    uint64_t lastPostTime_ = 0;
};

class SamplesRecord {
public:
    explicit SamplesRecord();
    virtual ~SamplesRecord();

    void AddSample(FrameStackAndInfo *frame);
    void AddRootSample();
    void StringifySampleData();
    int GetMethodNodeCount() const;
    int GetframeStackLength() const;
    std::string GetSampleData() const;
    void SetThreadStartTime(uint64_t threadStartTime);
    void SetThreadStopTime();
    void SetStartsampleData(std::string sampleData);
    void SetFileName(std::string &fileName);
    const std::string GetFileName() const;
    void ClearSampleData();
    std::unique_ptr<struct ProfileInfo> GetProfileInfo();
    bool GetIsStart() const;
    void SetIsStart(bool isStart);
    bool GetGcState() const;
    void SetGcState(bool gcState);
    bool GetRuntimeState() const;
    void SetRuntimeState(bool runtimeState);
    void SetIsBreakSampleFlag(bool sampleFlag);
    int SemInit(int index, int pshared, int value);
    int SemPost(int index);
    int SemWait(int index);
    int SemDestroy(int index);
    const CMap<struct MethodKey, struct FrameInfo> &GetStackInfo() const;
    void InsertStackInfo(struct MethodKey &methodKey, struct FrameInfo &codeEntry);
    bool PushFrameStack(struct MethodKey &methodKey);
    bool PushStackInfo(const FrameInfoTemp &frameInfoTemp);
    bool GetBeforeGetCallNapiStackFlag();
    void SetBeforeGetCallNapiStackFlag(bool flag);
    bool GetAfterGetCallNapiStackFlag();
    void SetAfterGetCallNapiStackFlag(bool flag);
    bool GetCallNapiFlag();
    void SetCallNapiFlag(bool flag);
    bool PushNapiFrameStack(struct MethodKey &methodKey);
    bool PushNapiStackInfo(const FrameInfoTemp &frameInfoTemp);
    int GetNapiFrameStackLength();
    void ClearNapiStack();
    void ClearNapiCall();
    void RecordCallNapiTime(uint64_t currentTime);
    void RecordCallNapiAddr(const std::string &methodAddrName);
    void FinetuneSampleData();
    void FindSampleAndFinetune(size_t findIdx, size_t napiCallIdx, size_t &sampleIdx,
                               uint64_t startSampleTime, uint64_t &sampleTime);
    void FinetuneTimeDeltas(size_t idx, uint64_t napiTime, uint64_t &sampleTime, bool isEndSample);
    void PostFrame();
    void PostNapiFrame();
    void ResetFrameLength();
    uint64_t GetCallTimeStamp();
    void SetCallTimeStamp(uint64_t timeStamp);
    std::ofstream fileHandle_;
    SamplesQueue *samplesQueue_ {nullptr};

    void SetEnableVMTag(bool flag)
    {
        enableVMTag_ = flag;
    }

private:
    void StringifyStateTimeStatistic();
    void StringifyNodes();
    void StringifySamples();
    struct FrameInfo GetMethodInfo(struct MethodKey &methodKey);
    std::string AddRunningState(char *functionName, RunningState state, kungfu::DeoptType type);
    void FrameInfoTempToMap(FrameInfoTemp *frameInfoTemps, int frameInfoTempLength);
    void NapiFrameInfoTempToMap();
    void StatisticStateTime(int timeDelta, RunningState state);

    int previousId_ = 0;
    RunningState previousState_ = RunningState::OTHER;
    uint64_t previousTimeStamp_ = 0;
    std::atomic_bool isBreakSample_ = false;
    std::atomic_bool gcState_ = false;
    std::atomic_bool runtimeState_ = false;
    std::atomic_bool isStart_ = false;
    std::atomic_bool beforeCallNapi_ = false;
    std::atomic_bool afterCallNapi_ = false;
    std::atomic_bool callNapi_ = false;
    std::unique_ptr<struct ProfileInfo> profileInfo_;
    CVector<int> stackTopLines_;
    CMap<struct NodeKey, int> nodeMap_;
    std::string sampleData_ = "";
    std::string fileName_ = "";
    sem_t sem_[3]; // 3 : sem_ size is three.
    CMap<struct MethodKey, struct FrameInfo> stackInfoMap_;
    struct MethodKey frameStack_[MAX_STACK_SIZE] = {};
    int frameStackLength_ = 0;
    CMap<std::string, int> scriptIdMap_;
    FrameInfoTemp frameInfoTemps_[MAX_STACK_SIZE] = {};
    int frameInfoTempLength_ = 0;
    // napi stack
    CVector<struct MethodKey> napiFrameStack_;
    CVector<FrameInfoTemp> napiFrameInfoTemps_;
    CVector<uint64_t> napiCallTimeVec_;
    CVector<std::string> napiCallAddrVec_;
    bool enableVMTag_ {false};
    uint64_t callTimeStamp_ = 0;
};
} // namespace panda::ecmascript
#endif // ECMASCRIPT_SAMPLES_RECORD_H