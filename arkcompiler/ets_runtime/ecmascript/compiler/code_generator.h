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

#ifndef ECMASCRIPT_COMPILER_CODE_GENERATOR_H
#define ECMASCRIPT_COMPILER_CODE_GENERATOR_H

#include "ecmascript/compiler/circuit.h"
#include "ecmascript/compiler/stub_builder.h"
#include "ecmascript/jspandafile/method_literal.h"

namespace panda::ecmascript::kungfu {
using ControlFlowGraph = std::vector<std::vector<GateRef>>;

class CodeGeneratorImpl {
public:
    CodeGeneratorImpl() = default;

    virtual ~CodeGeneratorImpl() = default;

    virtual void GenerateCodeForStub(Circuit *circuit, const ControlFlowGraph &graph, size_t index,
                                     const CompilationConfig *cfg) = 0;

    virtual void GenerateCode(Circuit *circuit, const ControlFlowGraph &graph, const CompilationConfig *cfg,
                              const MethodLiteral *methodLiteral, const JSPandaFile *jsPandaFile) = 0;
};

class CodeGenerator {
public:
    explicit CodeGenerator(std::unique_ptr<CodeGeneratorImpl> &impl, const std::string& methodName)
        : impl_(std::move(impl)), methodName_(methodName)
    {
    }

    ~CodeGenerator() = default;

    void RunForStub(Circuit *circuit, const ControlFlowGraph &graph, size_t index, const CompilationConfig *cfg)
    {
        impl_->GenerateCodeForStub(circuit, graph, index, cfg);
    }

    const std::string& GetMethodName() const
    {
        return methodName_;
    }

    void Run(Circuit *circuit, const ControlFlowGraph &graph, const CompilationConfig *cfg,
             const MethodLiteral *methodLiteral, const JSPandaFile *jsPandaFile)
    {
        impl_->GenerateCode(circuit, graph, cfg, methodLiteral, jsPandaFile);
    }

private:
    std::unique_ptr<CodeGeneratorImpl> impl_{nullptr};
    std::string methodName_;
};
} // namespace panda::ecmascript::kungfu
#endif // ECMASCRIPT_COMPILER_CODE_GENERATOR_H
