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

#include "ecmascript/method.h"

#include "ecmascript/jspandafile/js_pandafile.h"
#include "ecmascript/jspandafile/program_object.h"

namespace panda::ecmascript {
std::string Method::ParseFunctionName() const
{
    const JSPandaFile *jsPandaFile = GetJSPandaFile();
    return MethodLiteral::ParseFunctionName(jsPandaFile, GetMethodId());
}

const char *Method::GetMethodName() const
{
    const JSPandaFile *jsPandaFile = GetJSPandaFile();
    return MethodLiteral::GetMethodName(jsPandaFile, GetMethodId());
}

const char *Method::GetMethodName(const JSPandaFile* file) const
{
    return MethodLiteral::GetMethodName(file, GetMethodId());
}

const CString Method::GetRecordName() const
{
    const JSPandaFile *jsPandaFile = GetJSPandaFile();
    return MethodLiteral::GetRecordName(jsPandaFile, GetMethodId());
}

uint32_t Method::GetCodeSize() const
{
    const JSPandaFile *jsPandaFile = GetJSPandaFile();
    return MethodLiteral::GetCodeSize(jsPandaFile, GetMethodId());
}

const JSPandaFile *Method::GetJSPandaFile() const
{
    JSTaggedValue constpool = GetConstantPool();
    if (constpool.IsUndefined()) {
        return nullptr;
    }

    const ConstantPool *taggedPool = ConstantPool::Cast(constpool.GetTaggedObject());
    return taggedPool->GetJSPandaFile();
}

const panda_file::File *Method::GetPandaFile() const
{
    const JSPandaFile *jsPandaFile = GetJSPandaFile();
    if (jsPandaFile == nullptr) {
        return nullptr;
    }
    return jsPandaFile->GetPandaFile();
}

MethodLiteral *Method::GetMethodLiteral() const
{
    if (IsAotWithCallField()) {
        ASSERT(!IsNativeWithCallField());
        const JSPandaFile *jsPandaFile = GetJSPandaFile();
        return jsPandaFile->FindMethodLiteral(GetMethodId().GetOffset());
    }
    return reinterpret_cast<MethodLiteral *>(GetCodeEntryOrLiteral());
}
} // namespace panda::ecmascript
