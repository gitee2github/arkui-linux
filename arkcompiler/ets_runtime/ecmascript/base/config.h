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

#ifndef ECMASCRIPT_BASE_CONFIG_H
#define ECMASCRIPT_BASE_CONFIG_H

namespace panda::ecmascript {
#define ARK_INLINE __attribute__((always_inline))
#define ARK_NOINLINE __attribute__((noinline))

#define ECMASCRIPT_ENABLE_DEBUG_MODE 0
#define ECMASCRIPT_ENABLE_ARK_CONTAINER 1
#define ECMASCRIPT_ENABLE_VERBOSE_LEVEL_LOG 0

#define ECMASCRIPT_ENABLE_BUILTIN_LOG 0
#define ECMASCRIPT_ENABLE_ASM_FILE_LOAD_LOG 0

#define ECMASCRIPT_ENABLE_SNAPSHOT 0
#define ECMASCRIPT_ENABLE_HEAP_DETAIL_STATISTICS 0
#define ECMASCRIPT_ENABLE_OPT_CODE_PROFILER 0

#ifndef NDEBUG
#define ECMASCRIPT_ENABLE_INTERPRETER_LOG 1
#define ECMASCRIPT_ENABLE_RUNTIME_STAT 1
#define ECMASCRIPT_ENABLE_INTERPRETER_RUNTIME_STAT 1
#define ECMASCRIPT_ENABLE_BUILTINS_RUNTIME_STAT 1
#define ECMASCRIPT_ENABLE_ALLOCATE_AND_GC_RUNTIME_STAT 1
#else
#define ECMASCRIPT_ENABLE_INTERPRETER_LOG 0
#define ECMASCRIPT_ENABLE_RUNTIME_STAT 0
#define ECMASCRIPT_ENABLE_INTERPRETER_RUNTIME_STAT 0
#define ECMASCRIPT_ENABLE_BUILTINS_RUNTIME_STAT 0
#define ECMASCRIPT_ENABLE_ALLOCATE_AND_GC_RUNTIME_STAT 0
#endif
/*
 * 1. close ic
 * 2. close parallel gc
 * 3. enable gc logs
 * 4. enable handle-scope zap, zap reclaimed regions
 * 5. switch gc mode to full gc
 * 6. enable Cast() check
 * 7. enable verify heap
 * 9. enable Proactively interrogating and collecting information in the call stack
 */
#if ECMASCRIPT_ENABLE_DEBUG_MODE
    #define ECMASCRIPT_ENABLE_IC 0
    #define ECMASCRIPT_ENABLE_GC_LOG 1
    #define ECMASCRIPT_ENABLE_ZAP_MEM 1
    #define ECMASCRIPT_SWITCH_GC_MODE_TO_FULL_GC 1
    #define ECMASCRIPT_ENABLE_CAST_CHECK 1
    #define ECMASCRIPT_ENABLE_NEW_HANDLE_CHECK 1
    #define ECMASCRIPT_ENABLE_HEAP_VERIFY 1
#else
    #define ECMASCRIPT_ENABLE_IC 1
    #define ECMASCRIPT_ENABLE_GC_LOG 0
    #define ECMASCRIPT_ENABLE_ZAP_MEM 0
    #define ECMASCRIPT_SWITCH_GC_MODE_TO_FULL_GC 0
    #define ECMASCRIPT_ENABLE_CAST_CHECK 0
    #define ECMASCRIPT_ENABLE_NEW_HANDLE_CHECK 0
    #define ECMASCRIPT_ENABLE_HEAP_VERIFY 0
#endif

#if ECMASCRIPT_ENABLE_ZAP_MEM
    constexpr int INVALID_VALUE = 0x7;
#endif

#if defined(PANDA_TARGET_32)
    #define ECMASCRIPT_DISABLE_CONCURRENT_MARKING 1
#else
    #define ECMASCRIPT_DISABLE_CONCURRENT_MARKING 0
#endif
}  // namespace panda::ecmascript

#endif  // ECMASCRIPT_BASE_CONFIG_H
