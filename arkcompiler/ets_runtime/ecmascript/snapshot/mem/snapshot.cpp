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

#include "ecmascript/snapshot/mem/snapshot.h"

#include <cerrno>

#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/jobs/micro_job_queue.h"
#include "ecmascript/jspandafile/program_object.h"
#include "ecmascript/js_hclass.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/aot_file_manager.h"
#include "ecmascript/jspandafile/js_pandafile_manager.h"
#include "ecmascript/mem/c_containers.h"
#include "ecmascript/mem/heap.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/platform/file.h"
#include "ecmascript/snapshot/mem/snapshot_env.h"
#include "ecmascript/ts_types/ts_manager.h"

namespace panda::ecmascript {
void Snapshot::Serialize(const CString &fileName)
{
    TSManager *tsManager = vm_->GetTSManager();
    JSTaggedValue root = tsManager->GetSnapshotCPList();
    Serialize(root.GetTaggedObject(), nullptr, fileName);
}

void Snapshot::Serialize(TaggedObject *objectHeader, const panda_file::File *pf, const CString &fileName)
{
    std::string realPath;
    if (!RealPath(std::string(fileName), realPath, false)) {
        LOG_FULL(FATAL) << "snapshot file path error";
    }
    std::fstream writer(realPath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
    if (!writer.good()) {
        writer.close();
        LOG_FULL(FATAL) << "snapshot open file failed";
    }

    SnapshotProcessor processor(vm_);
    processor.Initialize();

    std::unordered_map<uint64_t, std::pair<uint64_t, EncodeBit>> data;
    CQueue<TaggedObject *> objectQueue;

    if (objectHeader->GetClass()->GetObjectType() == JSType::PROGRAM) {
        processor.SetProgramSerializeStart();
    }

    processor.EncodeTaggedObject(objectHeader, &objectQueue, &data);
    size_t rootObjSize = objectQueue.size();
    processor.ProcessObjectQueue(&objectQueue, &data);
    WriteToFile(writer, pf, rootObjSize, processor);
}

void Snapshot::Serialize(uintptr_t startAddr, size_t size, const CString &fileName)
{
    std::string realPath;
    if (!RealPath(std::string(fileName), realPath, false)) {
        LOG_FULL(FATAL) << "snapshot file path error";
    }
    std::fstream writer(realPath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
    if (!writer.good()) {
        writer.close();
        LOG_FULL(FATAL) << "snapshot open file failed";
    }

    SnapshotProcessor processor(vm_);
    processor.Initialize();

    std::unordered_map<uint64_t, std::pair<uint64_t, EncodeBit>> data;
    CQueue<TaggedObject *> objectQueue;

    ObjectSlot start(startAddr);
    ObjectSlot end(startAddr + size * sizeof(JSTaggedType));
    processor.EncodeTaggedObjectRange(start, end, &objectQueue, &data);

    size_t rootObjSize = objectQueue.size();
    processor.ProcessObjectQueue(&objectQueue, &data);
    WriteToFile(writer, nullptr, rootObjSize, processor);
}

void Snapshot::SerializeBuiltins(const CString &fileName)
{
    std::string realPath;
    if (!RealPath(std::string(fileName), realPath, false)) {
        LOG_FULL(FATAL) << "snapshot file path error";
    }
    std::fstream write(realPath.c_str(), std::ios::out | std::ios::binary | std::ios::app);
    if (!write.good()) {
        write.close();
        LOG_FULL(FATAL) << "snapshot open file failed";
    }
    // if builtins.snapshot file has exist, return directly
    if (write.tellg()) {
        LOG_FULL(DEBUG) << "snapshot already exist";
        write.close();
        return;
    }

    SnapshotProcessor processor(vm_);
    processor.Initialize();
    processor.SetBuiltinsSerializeStart();

    std::unordered_map<uint64_t, std::pair<uint64_t, EncodeBit>> data;
    CQueue<TaggedObject *> objectQueue;

    auto globalEnvHandle = vm_->GetGlobalEnv();
    auto constant = const_cast<GlobalEnvConstants *>(vm_->GetJSThread()->GlobalConstants());
    constant->VisitRangeSlot([&objectQueue, &data, &processor]([[maybe_unused]]Root type,
                                                               ObjectSlot start, ObjectSlot end) {
        processor.EncodeTaggedObjectRange(start, end, &objectQueue, &data);
    });
    processor.EncodeTaggedObject(*globalEnvHandle, &objectQueue, &data);
    size_t rootObjSize = objectQueue.size();
    processor.ProcessObjectQueue(&objectQueue, &data);
    WriteToFile(write, nullptr, rootObjSize, processor);
}

const JSPandaFile *Snapshot::Deserialize(SnapshotType type, const CString &snapshotFile, bool isBuiltins)
{
    std::string realPath;
    if (!RealPath(std::string(snapshotFile), realPath, false)) {
        LOG_FULL(FATAL) << "snapshot file path error";
        UNREACHABLE();
    }

    SnapshotProcessor processor(vm_, snapshotFile);
    if (isBuiltins) {
        processor.SetBuiltinsDeserializeStart();
    }

    MemMap fileMap = FileMap(realPath.c_str(), FILE_RDONLY, PAGE_PROT_READWRITE);
    if (fileMap.GetOriginAddr() == nullptr) {
        LOG_FULL(FATAL) << "file mmap failed";
        UNREACHABLE();
    }
    auto readFile = ToUintPtr(fileMap.GetOriginAddr());
    auto hdr = *ToNativePtr<const SnapShotHeader>(readFile);
    if (!hdr.Verify()) {
        FileUnMap(fileMap);
        LOG_FULL(FATAL) << "file verify failed";
        UNREACHABLE();
    }
    uintptr_t oldSpaceBegin = readFile + sizeof(SnapShotHeader);
    processor.DeserializeObjectExcludeString(oldSpaceBegin, hdr.oldSpaceObjSize, hdr.nonMovableObjSize,
                                             hdr.machineCodeObjSize, hdr.snapshotObjSize, hdr.hugeObjSize);
    uintptr_t stringBegin = oldSpaceBegin + hdr.oldSpaceObjSize + hdr.nonMovableObjSize +
        hdr.machineCodeObjSize + hdr.snapshotObjSize + hdr.hugeObjSize;
    uintptr_t stringEnd = stringBegin + hdr.stringSize;
    processor.DeserializeString(stringBegin, stringEnd);

    FileUnMap(MemMap(fileMap.GetOriginAddr(), hdr.pandaFileBegin));
    const JSPandaFile *jsPandaFile = nullptr;
    if (static_cast<uint32_t>(fileMap.GetSize()) > hdr.pandaFileBegin) {
        uintptr_t panda_file_mem = readFile + hdr.pandaFileBegin;
        auto pf = panda_file::File::OpenFromMemory(os::mem::ConstBytePtr(ToNativePtr<std::byte>(panda_file_mem),
            static_cast<uint32_t>(fileMap.GetSize()) - hdr.pandaFileBegin, os::mem::MmapDeleter));
        jsPandaFile = JSPandaFileManager::GetInstance()->NewJSPandaFile(pf.release(), "");
    }
    // relocate object field
    processor.Relocate(type, jsPandaFile, hdr.rootObjectSize);
    LOG_COMPILER(INFO) << "loaded ai file: " << snapshotFile.c_str();
    return jsPandaFile;
}

size_t Snapshot::AlignUpPageSize(size_t spaceSize)
{
    if (spaceSize % Constants::PAGE_SIZE_ALIGN_UP == 0) {
        return spaceSize;
    }
    return Constants::PAGE_SIZE_ALIGN_UP * (spaceSize / Constants::PAGE_SIZE_ALIGN_UP + 1);
}

void Snapshot::WriteToFile(std::fstream &writer, const panda_file::File *pf, size_t size, SnapshotProcessor &processor)
{
    uint32_t totalStringSize = 0U;
    CVector<uintptr_t> stringVector = processor.GetStringVector();
    for (size_t i = 0; i < stringVector.size(); ++i) {
        auto str = reinterpret_cast<EcmaString *>(stringVector[i]);
        size_t objectSize = AlignUp(EcmaStringAccessor(str).ObjectSize(),
            static_cast<size_t>(MemAlignment::MEM_ALIGN_OBJECT));
        totalStringSize += objectSize;
    }

    std::vector<uint32_t> objSizeVector = processor.StatisticsObjectSize();
    size_t totalObjSize = totalStringSize;
    for (uint32_t objSize : objSizeVector) {
        totalObjSize += objSize;
    }
    uint32_t pandaFileBegin = RoundUp(totalObjSize + sizeof(SnapShotHeader), Constants::PAGE_SIZE_ALIGN_UP);
    SnapShotHeader hdr;
    hdr.oldSpaceObjSize = objSizeVector[0]; // 0:oldSpaceObj
    hdr.nonMovableObjSize = objSizeVector[1]; // 1:nonMovableObj
    hdr.machineCodeObjSize = objSizeVector[2]; // 2:machineCodeObj
    hdr.snapshotObjSize = objSizeVector[3]; // 3:snapshotObj
    hdr.hugeObjSize = objSizeVector[4]; // 4:hugeObj
    hdr.stringSize = totalStringSize;
    hdr.pandaFileBegin = pandaFileBegin;
    hdr.rootObjectSize = static_cast<uint32_t>(size);
    writer.write(reinterpret_cast<char *>(&hdr), sizeof(hdr));

    processor.WriteObjectToFile(writer);

    for (size_t i = 0; i < stringVector.size(); ++i) {
        auto str = reinterpret_cast<EcmaString *>(stringVector[i]);
        size_t strSize = AlignUp(EcmaStringAccessor(str).ObjectSize(),
            static_cast<size_t>(MemAlignment::MEM_ALIGN_OBJECT));
        writer.write(reinterpret_cast<char *>(str), strSize);
        writer.flush();
    }
    ASSERT(static_cast<size_t>(writer.tellp()) == totalObjSize + sizeof(SnapShotHeader));
    if (pf) {
        writer.seekp(pandaFileBegin);
        writer.write(reinterpret_cast<const char *>(pf->GetBase()), pf->GetHeader()->file_size);
    }
    writer.close();
}
}  // namespace panda::ecmascript
