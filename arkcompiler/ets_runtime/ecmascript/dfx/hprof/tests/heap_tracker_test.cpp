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

#include <cstdio>
#include <fstream>
#include <fcntl.h>

#include "ecmascript/dfx/hprof/heap_profiler_interface.h"
#include "ecmascript/dfx/hprof/heap_profiler.h"
#include "ecmascript/dfx/hprof/heap_snapshot_json_serializer.h"
#include "ecmascript/dfx/hprof/heap_snapshot.h"
#include "ecmascript/ecma_string.h"
#include "ecmascript/global_env.h"

#include "ecmascript/js_tagged_value.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/mem/heap.h"
#include "ecmascript/tests/test_helper.h"
#include "ecmascript/dfx/hprof/file_stream.h"

using namespace panda::ecmascript;

namespace panda::ecmascript {
class TestProgress : public Progress {
public:
    TestProgress() = default;
    ~TestProgress() = default;

    void ReportProgress([[maybe_unused]]int32_t done, [[maybe_unused]]int32_t total) override {}
};

class TestStream : public Stream {
public:
    TestStream() = default;
    ~TestStream() = default;

    void EndOfStream() override {}
    int GetSize() override
    {
        static const int heapProfilerChunkSise = 100_KB;
        return heapProfilerChunkSise;
    }
    bool WriteChunk([[maybe_unused]]char *data, [[maybe_unused]]int32_t size) override
    {
        return true;
    }
    bool Good() override
    {
        return testStream_.good();
    }

    void UpdateHeapStats([[maybe_unused]]HeapStat* updateData, [[maybe_unused]]int32_t count) override
    {
    }

    void UpdateLastSeenObjectId([[maybe_unused]]int32_t lastSeenObjectId, [[maybe_unused]]int64_t timeStampUs) override
    {
    }

    void Clear()
    {
        testStream_.clear(std::ios::badbit);
    }

private:
    std::fstream testStream_;
};
}

namespace panda::test {
class HeapTrackerTest : public testing::Test {
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
        instance->SetEnableForceGC(false);
    }

    void TearDown() override
    {
        TestHelper::DestroyEcmaVMWithScope(instance, scope);
    }

    EcmaVM *instance {nullptr};
    EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};
};

HWTEST_F_L0(HeapTrackerTest, HeapTracker)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    HeapProfilerInterface *heapProfile = HeapProfilerInterface::GetInstance(instance);
    heapProfile->StartHeapTracking(50);
    sleep(1);
    int count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSAsyncFuncObject();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSSymbol();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        JSHandle<EcmaString> string = instance->GetFactory()->NewFromASCII("Hello World");
        instance->GetFactory()->NewJSString(JSHandle<JSTaggedValue>(string));
    }

    // Create file test.heaptimeline
    std::string fileName = "test.heaptimeline";
    fstream outputString(fileName, std::ios::out);
    outputString.close();
    outputString.clear();

    FileStream stream(fileName.c_str());
    heapProfile->StopHeapTracking(&stream, nullptr);
    HeapProfilerInterface::Destroy(instance);

    // Check
    fstream inputStream(fileName, std::ios::in);
    std::string line;
    std::string emptySample = "\"samples\":";
    std::string firstSample = "\"samples\":[0, ";
    uint32_t emptySize = emptySample.size();
    bool isFind = false;
    while (getline(inputStream, line)) {
        if (line.substr(0U, emptySize) == emptySample) {
            ASSERT_TRUE(line.substr(0, firstSample.size()) == firstSample);
            isFind = true;
        }
    }
    ASSERT_TRUE(isFind);

    inputStream.close();
    inputStream.clear();
    std::remove(fileName.c_str());
}

HWTEST_F_L0(HeapTrackerTest, HeapTrackerTraceAllocation)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    HeapProfilerInterface *heapProfile = HeapProfilerInterface::GetInstance(instance);
    TestStream testStream;
    heapProfile->StartHeapTracking(50, true, &testStream, true);
    sleep(1);
    int count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSAsyncFuncObject();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSSymbol();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        JSHandle<EcmaString> string = instance->GetFactory()->NewFromASCII("Hello World");
        instance->GetFactory()->NewJSString(JSHandle<JSTaggedValue>(string));
    }

    // Create file test.heaptimeline
    std::string fileName = "test.heaptimeline";
    fstream outputString(fileName, std::ios::out);
    outputString.close();
    outputString.clear();

    FileStream stream(fileName.c_str());
    TestProgress testProgress;
    heapProfile->StopHeapTracking(&stream, &testProgress);
    HeapProfilerInterface::Destroy(instance);

    // Check
    fstream inputStream(fileName, std::ios::in);
    std::string line;
    std::string emptyTraceFunctionInfo = "\"trace_function_infos\":[";
    std::string emptyTraceNode = "\"trace_tree\":[";
    std::string firstTraceFunctionInfo = "\"trace_function_infos\":[0,";
    std::string firstTraceNode = "\"trace_tree\":[1,0";
    uint32_t emptyTraceFunctionInfoSize = emptyTraceFunctionInfo.size();
    uint32_t emptyTraceNodeSize = emptyTraceNode.size();
    bool traceFunctionInfoIsFind = false;
    bool traceNodeIsFind = false;
    while (getline(inputStream, line)) {
        if (line.substr(0U, emptyTraceFunctionInfoSize) == emptyTraceFunctionInfo) {
            ASSERT_TRUE(line.substr(0, firstTraceFunctionInfo.size()) == firstTraceFunctionInfo);
            traceFunctionInfoIsFind = true;
        }

        if (line.substr(0U, emptyTraceNodeSize) == emptyTraceNode) {
            ASSERT_TRUE(line.substr(0, firstTraceNode.size()) == firstTraceNode);
            traceNodeIsFind = true;
        }
    }
    ASSERT_TRUE(traceFunctionInfoIsFind);
    ASSERT_TRUE(traceNodeIsFind);

    inputStream.close();
    inputStream.clear();
    std::remove(fileName.c_str());
}

HWTEST_F_L0(HeapTrackerTest, DumpHeapSnapshot)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    HeapProfilerInterface *heapProfile = HeapProfilerInterface::GetInstance(instance);

    sleep(1);
    int count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSAsyncFuncObject();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSSymbol();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        JSHandle<EcmaString> string = instance->GetFactory()->NewFromASCII("Hello World");
        instance->GetFactory()->NewJSString(JSHandle<JSTaggedValue>(string));
    }

    // Create file test.heapsnapshot
    std::string fileName = "test.heapsnapshot";
    fstream outputString(fileName, std::ios::out);
    outputString.close();
    outputString.clear();

    FileStream stream(fileName.c_str());
    TestProgress testProgress;
    heapProfile->DumpHeapSnapshot(DumpFormat::JSON, &stream, &testProgress, true, true);
    HeapProfilerInterface::Destroy(instance);

    // Check
    fstream inputStream(fileName, std::ios::in);
    std::string line;
    std::string nodes = "\"nodes\":[";
    std::string sample = "\"samples\":[]";
    uint32_t nodesSize = nodes.size();
    uint32_t sampleSize = sample.size();
    bool isNodesFind = false;
    bool isSampleFind = false;
    while (getline(inputStream, line)) {
        if (line.substr(0U, nodesSize) == nodes) {
            isNodesFind = true;
        }

        if (line.substr(0U, sampleSize) == sample) {
            isSampleFind = true;
        }
    }
    ASSERT_TRUE(isNodesFind);
    ASSERT_TRUE(isSampleFind);

    inputStream.close();
    inputStream.clear();
    std::remove(fileName.c_str());
}

HWTEST_F_L0(HeapTrackerTest, HeapSnapshotBuildUp)
{
    bool isVmMode = true;
    bool isPrivate = false;
    bool traceAllocation = false;
    HeapSnapshot heapSnapshot(instance, isVmMode, isPrivate, traceAllocation, instance->GetChunk());
    EXPECT_TRUE(heapSnapshot.BuildUp());
}

HWTEST_F_L0(HeapTrackerTest, HeapSnapshotUpdateNode)
{
    bool isVmMode = true;
    bool isPrivate = false;
    bool traceAllocation = false;
    HeapSnapshot heapSnapshot(instance, isVmMode, isPrivate, traceAllocation, instance->GetChunk());
    size_t beginNode = heapSnapshot.GetNodeCount();
    heapSnapshot.UpdateNodes();
    size_t endNode = heapSnapshot.GetNodeCount();
    EXPECT_TRUE(beginNode != endNode);
}

HWTEST_F_L0(HeapTrackerTest, GenDumpFileName_001)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    HeapProfilerInterface *heapProfile = HeapProfilerInterface::GetInstance(instance);

    sleep(1);
    int count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSAsyncFuncObject();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSSymbol();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        JSHandle<EcmaString> string = instance->GetFactory()->NewFromASCII("Hello World");
        instance->GetFactory()->NewJSString(JSHandle<JSTaggedValue>(string));
    }

    TestStream stream;
    stream.Clear();
    EXPECT_TRUE(!stream.Good());
    TestProgress testProgress;
    heapProfile->DumpHeapSnapshot(DumpFormat::JSON, &stream, &testProgress, true, true);
    HeapProfilerInterface::Destroy(instance);
}

HWTEST_F_L0(HeapTrackerTest, GenDumpFileName_002)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    HeapProfilerInterface *heapProfile = HeapProfilerInterface::GetInstance(instance);

    sleep(1);
    int count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSAsyncFuncObject();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSSymbol();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        JSHandle<EcmaString> string = instance->GetFactory()->NewFromASCII("Hello World");
        instance->GetFactory()->NewJSString(JSHandle<JSTaggedValue>(string));
    }

    TestStream stream;
    stream.Clear();
    EXPECT_TRUE(!stream.Good());
    TestProgress testProgress;
    heapProfile->DumpHeapSnapshot(DumpFormat::BINARY, &stream, &testProgress, true, true);
    HeapProfilerInterface::Destroy(instance);
}

HWTEST_F_L0(HeapTrackerTest, GenDumpFileName_003)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    HeapProfilerInterface *heapProfile = HeapProfilerInterface::GetInstance(instance);

    sleep(1);
    int count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSAsyncFuncObject();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSSymbol();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        JSHandle<EcmaString> string = instance->GetFactory()->NewFromASCII("Hello World");
        instance->GetFactory()->NewJSString(JSHandle<JSTaggedValue>(string));
    }

    TestStream stream;
    stream.Clear();
    EXPECT_TRUE(!stream.Good());
    TestProgress testProgress;
    heapProfile->DumpHeapSnapshot(DumpFormat::OTHER, &stream, &testProgress, true, true);
    HeapProfilerInterface::Destroy(instance);
}

HWTEST_F_L0(HeapTrackerTest, GenDumpFileName_004)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    HeapProfilerInterface *heapProfile = HeapProfilerInterface::GetInstance(instance);

    sleep(1);
    int count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSAsyncFuncObject();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSSymbol();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        JSHandle<EcmaString> string = instance->GetFactory()->NewFromASCII("Hello World");
        instance->GetFactory()->NewJSString(JSHandle<JSTaggedValue>(string));
    }

    TestStream stream;
    stream.Clear();
    EXPECT_TRUE(!stream.Good());
    TestProgress testProgress;
    DumpFormat dumFormat = static_cast<DumpFormat>(5);
    heapProfile->DumpHeapSnapshot(dumFormat, &stream, &testProgress, true, true);
    HeapProfilerInterface::Destroy(instance);
}

HWTEST_F_L0(HeapTrackerTest, FileDescriptorStreamEndOfStream)
{
    int fd = 3;
    FileDescriptorStream fileStream(fd);
    EXPECT_TRUE(fileStream.Good());
    fileStream.EndOfStream();
}

HWTEST_F_L0(HeapTrackerTest, StreamWriterEnd)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    HeapProfilerInterface *heapProfile = HeapProfilerInterface::GetInstance(instance);

    sleep(1);
    int count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSAsyncFuncObject();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSSymbol();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        JSHandle<EcmaString> string = instance->GetFactory()->NewFromASCII("Hello World");
        instance->GetFactory()->NewJSString(JSHandle<JSTaggedValue>(string));
    }

    // Create file test.heapsnapshot
    std::string fileName = "test.heapsnapshot";
    fstream outputString(fileName, std::ios::out);
    outputString.close();
    outputString.clear();

    FileStream stream(fileName.c_str());
    CVector<HeapStat> statsBuffer;
    statsBuffer.emplace_back(1, 2, 4);
    stream.UpdateHeapStats(&statsBuffer.front(), static_cast<int32_t>(statsBuffer.size()));
    stream.UpdateLastSeenObjectId(1, 1677567644913058);

    TestProgress testProgress;
    heapProfile->DumpHeapSnapshot(DumpFormat::JSON, &stream, &testProgress, true, true);
    StreamWriter streamWriter(&stream);
    streamWriter.End();
    HeapProfilerInterface::Destroy(instance);

    // Check
    fstream inputStream(fileName, std::ios::in);
    std::string line;
    std::string nodes = "\"nodes\":[";
    std::string sample = "\"samples\":[]";
    uint32_t nodesSize = nodes.size();
    uint32_t sampleSize = sample.size();
    bool isNodesFind = false;
    bool isSampleFind = false;
    while (getline(inputStream, line)) {
        if (line.substr(0U, nodesSize) == nodes) {
            isNodesFind = true;
        }

        if (line.substr(0U, sampleSize) == sample) {
            isSampleFind = true;
        }
    }
    ASSERT_TRUE(isNodesFind);
    ASSERT_TRUE(isSampleFind);

    inputStream.close();
    inputStream.clear();
    std::remove(fileName.c_str());
}

HWTEST_F_L0(HeapTrackerTest, GetStringByKey)
{
    StringKey key = static_cast<StringKey>(2);
    StringHashMap stringHashMap(instance);
    CString *hashMap = stringHashMap.GetStringByKey(key);
    EXPECT_TRUE(hashMap == nullptr);
}

HWTEST_F_L0(HeapTrackerTest, FormatString)
{
    bool isVmMode = true;
    bool isPrivate = false;
    bool traceAllocation = false;
    HeapSnapshot heapSnapshot(instance, isVmMode, isPrivate, traceAllocation, instance->GetChunk());

    StringHashMap stringHashMap(instance);
    CString ret = "H\"e\rl\nl\\o\t W\fo\31rld!";
    stringHashMap.GetString(ret);
    StringKey retKey = std::hash<std::string>{} (std::string(ret));

    CString *tmpResult = nullptr;
    tmpResult = stringHashMap.GetStringByKey(retKey);
    EXPECT_TRUE(tmpResult != nullptr);
    EXPECT_TRUE(*tmpResult == "H`e`l`l`o` W`o`rld!");
}

HWTEST_F_L0(HeapTrackerTest, FileDescriptorStreamWriteChunk)
{
    int32_t fd = -1;
    FileDescriptorStream testFileStream(fd);
    CVector<HeapStat> statsBuffer;
    statsBuffer.emplace_back(1, 2, 4);
    testFileStream.UpdateHeapStats(&statsBuffer.front(), static_cast<int32_t>(statsBuffer.size()));
    testFileStream.UpdateLastSeenObjectId(1, 1677567644913058);
    testFileStream.GetSize();
    std::string testString = "Hello!";
    int strSize = testString.size();
    bool isFileStream = testFileStream.WriteChunk(testString.data(), strSize);
    EXPECT_TRUE(!isFileStream);

    fd = 5;
    FileDescriptorStream tmpFileStream(fd);
    tmpFileStream.Good();
    testString = "Hello!";
    strSize = testString.size();
    isFileStream = tmpFileStream.WriteChunk(testString.data(), strSize);
    close(fd);
    EXPECT_TRUE(!isFileStream);

    std::string fileName = "test.StreamWriteChunk";
    fd = open(fileName.c_str(), O_RDONLY);
    if (fd < 0) {
        fd = open(fileName.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IROTH);
    }
    FileDescriptorStream fileStream(fd);
    testString = "Hello!";
    strSize = testString.size();
    isFileStream = fileStream.WriteChunk(testString.data(), strSize);
    EXPECT_TRUE(isFileStream);
    std::remove(fileName.c_str());
    close(fd);
}

HWTEST_F_L0(HeapTrackerTest, AddNodeToTree)
{
    CVector<uint32_t> traceNodeIndex;
    for (int i = 0; i < 3; i++) {
        traceNodeIndex.push_back(i + 1);
    }
    TraceTree traceTree;
    TraceNode *traceNode = traceTree.AddNodeToTree(traceNodeIndex);
    EXPECT_TRUE(traceNode != nullptr);
}

HWTEST_F_L0(HeapTrackerTest, FindOrAddChild)
{
    TraceTree traceTree;
    uint32_t index = 1;
    TraceNode traceNode(&traceTree, index);
    TraceNode *node = traceNode.FindOrAddChild(index);
    EXPECT_TRUE(node->GetNodeIndex() == 1);

    TraceNode *tmpNode = traceNode.FindOrAddChild(2);
    EXPECT_TRUE(tmpNode->GetNodeIndex() == 2);
}
}  // namespace panda::test
