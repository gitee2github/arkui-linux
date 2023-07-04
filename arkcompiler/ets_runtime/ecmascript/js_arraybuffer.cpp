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

#include "ecmascript/js_arraybuffer.h"

#include "ecmascript/base/builtins_base.h"
#include "ecmascript/ecma_macros.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tagged_array.h"

#include "securec.h"

namespace panda::ecmascript {
void JSArrayBuffer::CopyDataBlockBytes(JSTaggedValue toBlock, JSTaggedValue fromBlock, int32_t fromIndex, int32_t count)
{
    void *fromBuf = JSNativePointer::Cast(fromBlock.GetTaggedObject())->GetExternalPointer();
    void *toBuf = JSNativePointer::Cast(toBlock.GetTaggedObject())->GetExternalPointer();
    CopyDataPointBytes(toBuf, fromBuf, fromIndex, count);
}

void JSArrayBuffer::CopyDataPointBytes(void *toBuf, void *fromBuf, int32_t fromIndex, int32_t count)
{
    auto *from = static_cast<uint8_t *>(fromBuf);
    auto *to = static_cast<uint8_t *>(toBuf);
    if (memcpy_s(to, count, from + fromIndex, count) != EOK) {  // NOLINT
        LOG_FULL(FATAL) << "memcpy_s failed";
        UNREACHABLE();
    }
}

void JSArrayBuffer::Attach(JSThread *thread, uint32_t arrayBufferByteLength, JSTaggedValue arrayBufferData)
{
    ASSERT(arrayBufferData.IsNativePointer());
    SetArrayBufferByteLength(arrayBufferByteLength);
    SetArrayBufferData(thread, arrayBufferData);
}

void JSArrayBuffer::Detach(JSThread *thread)
{
    JSTaggedValue arrayBufferData = GetArrayBufferData();
    // already detached.
    if (arrayBufferData.IsNull()) {
        return;
    }

    SetArrayBufferData(thread, JSTaggedValue::Null());
    SetArrayBufferByteLength(0);
}
}  // namespace panda::ecmascript
