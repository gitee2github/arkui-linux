/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef ECMASCRIPT_JSPANDAFILE_ACCESSOR_MODULE_DATA_ACCESSOR_H
#define ECMASCRIPT_JSPANDAFILE_ACCESSOR_MODULE_DATA_ACCESSOR_H

#include <cstddef>
#include <cstdint>
#include <variant>
#include <vector>

#include "libpandabase/macros.h"
#include "libpandabase/utils/span.h"
#include "libpandafile/file.h"
#include "libpandafile/file_items.h"
#include "libpandafile/helpers.h"

#include "ecmascript/module/js_module_source_text.h"

namespace panda::ecmascript {
using StringData = panda_file::File::StringData;

class ModuleDataAccessor {
public:
    ModuleDataAccessor(const panda_file::File &panda_file, panda_file::File::EntityId module_data_id);
    ~ModuleDataAccessor() = default;
    DEFAULT_MOVE_CTOR(ModuleDataAccessor)
    DEFAULT_COPY_CTOR(ModuleDataAccessor)
    NO_MOVE_OPERATOR(ModuleDataAccessor);
    NO_COPY_OPERATOR(ModuleDataAccessor);

    void EnumerateImportEntry(JSThread *thread, const JSHandle<TaggedArray> &requestModuleArray,
                              JSHandle<SourceTextModule> &moduleRecord);

    void EnumerateLocalExportEntry(JSThread *thread, JSHandle<SourceTextModule> &moduleRecord);

    void EnumerateIndirectExportEntry(JSThread *thread, const JSHandle<TaggedArray> &requestModuleArray,
                                      JSHandle<SourceTextModule> &moduleRecord);

    void EnumerateStarExportEntry(JSThread *thread, const JSHandle<TaggedArray> &requestModuleArray,
                                  JSHandle<SourceTextModule> &moduleRecord);

    const panda_file::File &GetPandaFile() const
    {
        return pandaFile_;
    }

    panda_file::File::EntityId GetModuleDataId() const
    {
        return moduleDataId_;
    }

    const std::vector<uint32_t>& getRequestModules() const
    {
        return moduleRequests_;
    }

    using ModuleValue = std::variant<uint32_t, StringData>;

private:
    const panda_file::File &pandaFile_;
    panda_file::File::EntityId moduleDataId_;
    uint32_t numModuleRequests_;
    std::vector<uint32_t> moduleRequests_;
    Span<const uint8_t> entryDataSp_ {nullptr, nullptr};
};
}  // namespace panda::ecmascript
#endif  // ECMASCRIPT_JSPANDAFILE_ACCESSOR_MODULE_DATA_ACCESSOR_H
