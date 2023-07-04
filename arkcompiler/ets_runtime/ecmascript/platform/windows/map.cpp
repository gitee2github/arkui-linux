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

#include "ecmascript/platform/map.h"

#include <io.h>
#include <windows.h>

#ifdef ERROR
#undef ERROR
#endif

#include "ecmascript/log_wrapper.h"
#include "ecmascript/mem/mem.h"

namespace panda::ecmascript {
static constexpr int INSUFFICIENT_CONTINUOUS_MEM = 1455;

MemMap PageMap(size_t size, int prot, size_t alignment)
{
    ASSERT(size == AlignUp(size, PageSize()));
    ASSERT(alignment == AlignUp(alignment, PageSize()));
    size_t allocSize = size + alignment;
    void *result = VirtualAlloc(nullptr, allocSize, MEM_COMMIT, prot);
    if (result == nullptr) {
        int errCode = GetLastError();
        if (errCode == INSUFFICIENT_CONTINUOUS_MEM) {
            LOG_NO_TAG(ERROR) << "[ArkRuntime Log]Failed to request a continuous segment of " << size
                              << " virtual memory. Please clean up other heavy processes or restart the computer.";
        }
        LOG_ECMA(FATAL) << "PageMap "<< size << ", prot:" << prot << " fail, the error code is: " << errCode;
    }
    if (alignment != 0) {
        auto alignResult = AlignUp(reinterpret_cast<uintptr_t>(result), alignment);
        return MemMap(result, reinterpret_cast<void *>(alignResult), size);
    }
    return MemMap(result, result, size);
}

void PageUnmap(MemMap it)
{
    if (!VirtualFree(it.GetOriginAddr(), 0, MEM_RELEASE)) {
        int errCode = GetLastError();
        LOG_ECMA(ERROR) << "PageUnmap failed, error code is " << errCode;
    }
}

MemMap MachineCodePageMap(size_t size, int prot, size_t alignment)
{
    MemMap memMap = PageMap(size, prot, alignment);
    PageTag(memMap.GetMem(), memMap.GetSize());
    return memMap;
}

void MachineCodePageUnmap(MemMap it)
{
    PageTag(it.GetMem(), it.GetSize(), true);
    PageUnmap(it);
}

void PageRelease([[maybe_unused]] void *mem, [[maybe_unused]] size_t size)
{
}

void PageTag([[maybe_unused]] void *mem, [[maybe_unused]] size_t size, [[maybe_unused]] bool remove)
{
}

void PageProtect(void *mem, size_t size, int prot)
{
    [[maybe_unused]] DWORD oldProtect;
    if (!VirtualProtect(mem, size, prot, &oldProtect)) {
        int errCode = GetLastError();
        LOG_ECMA(ERROR) << "PageProtect change to " << prot << " failed, error code is " << errCode;
    }
}

size_t PageSize()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}
}  // namespace panda::ecmascript
