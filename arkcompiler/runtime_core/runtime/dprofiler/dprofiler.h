/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_RUNTIME_DPROFILER_DPROFILER_H_
#define PANDA_RUNTIME_DPROFILER_DPROFILER_H_

#include <memory>
#include <string>
#include <unordered_set>

#include "dprof/profiling_data.h"
#include "libpandabase/macros.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mem/panda_string.h"

namespace panda {

class Class;
class Method;
class Runtime;
class RuntimeListener;

/**
 * \brief The DProfiler class integrates the distributed profiling in the Panda.
 */
class DProfiler final {
public:
#ifdef PANDA_TARGET_UNIX
    DProfiler(std::string_view app_name, Runtime *runtime);
#else
    DProfiler([[maybe_unused]] std::string_view app_name, [[maybe_unused]] Runtime *runtime)
    {
        // Unsupported on windows platform
        UNREACHABLE();
    }
#endif  // PANDA_TARGET_UNIX
    ~DProfiler() = default;

    /**
     * \brief Add a panda::Class for the dump.
     * @param klass
     */
    void AddClass(const Class *klass);

    /**
     * \brief Send a dump of distributed profiling info
     */
    void Dump();

private:
    Runtime *runtime_;
    PandaUniquePtr<dprof::ProfilingData> profiling_data_;
    PandaUniquePtr<RuntimeListener> listener_;
    PandaUnorderedSet<const Method *> hot_methods_;

    NO_COPY_SEMANTIC(DProfiler);
    NO_MOVE_SEMANTIC(DProfiler);
};

}  // namespace panda

#endif  // PANDA_RUNTIME_DPROFILER_DPROFILER_H_
