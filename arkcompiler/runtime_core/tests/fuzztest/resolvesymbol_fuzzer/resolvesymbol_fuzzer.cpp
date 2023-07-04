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

#include "resolvesymbol_fuzzer.h"

#include <cstdio>

#include "os/library_loader.h"

namespace OHOS {
    void ResolveSymbolFuzzTest(const uint8_t* data, size_t size)
    {
        /* Create data source */
        const char *name = "__ResolveSymbolFuzzTest.tmp";
        FILE *fp = fopen(name, "wb");
        if (fp == nullptr) {
            return;
        }
        (void)fwrite(data, sizeof(uint8_t), size, fp);
        (void)fclose(fp);

        {
            auto res = panda::os::library_loader::Load(name);
            if (res) {
                panda::os::library_loader::ResolveSymbol(*res, "StartDebug");
                panda::os::library_loader::ResolveSymbol(*res, "StopDebug");
            }
        }

        (void)remove(name);
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::ResolveSymbolFuzzTest(data, size);
    return 0;
}
