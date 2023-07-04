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

#include "ecmascript/free_object.h"
#include "ecmascript/object_factory.h"

namespace panda::ecmascript {
FreeObject *FreeObject::FillFreeObject(EcmaVM *vm, uintptr_t address, size_t size)
{
    ASAN_UNPOISON_MEMORY_REGION(reinterpret_cast<void *>(address), size);
    FreeObject *freeObject = vm->GetFactory()->FillFreeObject(address, size);
    ASAN_POISON_MEMORY_REGION(reinterpret_cast<void *>(address), size);
    return freeObject;
}
}  // namespace panda::ecmascript
