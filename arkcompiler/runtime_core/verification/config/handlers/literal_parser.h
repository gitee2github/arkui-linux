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

#ifndef VERIFICATION_CONFIG_HANDLERS_LITERAL_PARSER_H
#define VERIFICATION_CONFIG_HANDLERS_LITERAL_PARSER_H

#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "verification/util/parser/parser.h"

#include <string>

namespace panda::verifier::debug {

template <typename Parser, typename Handler>
const auto &LiteralParser(Handler &handler)
{
    using panda::parser::action;
    using panda::parser::charset;
    using panda::parser::parser;

    struct Literal;

    using p = typename Parser::template next<Literal>;

    static const auto LITERAL_NAME_HANDLER = [&](action a, typename p::Ctx &c, auto from, auto to) {
        if (a == action::PARSED) {
            return handler(c, PandaString {from, to});
        }
        return true;
    };

    static const auto LITERAL_NAME = p::of_charset(charset {"abcdefghijklmnopqrstuvwxyz_-"}) |= LITERAL_NAME_HANDLER;

    return LITERAL_NAME;
}

inline const auto &LiteralsParser()
{
    using panda::parser::action;
    using panda::parser::charset;
    using panda::parser::parser;

    struct Literals;

    using Context = PandaVector<PandaString>;

    using p = typename parser<Context, const char, const char *>::template next<Literals>;
    using p1 = typename p::p;
    using p2 = typename p1::p;

    static const auto WS = p::of_charset(" \t");
    static const auto COMMA = p1::of_charset(",");

    static const auto LITERAL_HANDLER = [](Context &c, PandaString &&str) {
        c.emplace_back(std::move(str));
        return true;
    };

    static const auto LITERALS = ~WS >> *(~WS >> LiteralParser<p2>(LITERAL_HANDLER) >> ~WS >> ~COMMA) >> p::end();

    return LITERALS;
}

}  // namespace panda::verifier::debug

#endif  // VERIFICATION_CONFIG_HANDLERS_LITERAL_PARSER_H
