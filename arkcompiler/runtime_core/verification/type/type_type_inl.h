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

#ifndef PANDA_TYPE_TYPE_INL_HPP__
#define PANDA_TYPE_TYPE_INL_HPP__

#include "type_system.h"
#include "type_param.h"
#include "type_type.h"

#include "verification/util/lazy.h"

namespace panda::verifier {
// TODO(vdyadov): implement

template <typename Handler>
void Type::ForAllSupertypes(Handler &&handler) const
{
    GetTypeSystem().ForAllSupertypesOf(*this, std::move(handler));
}

template <typename Handler>
void Type::ForAllSupertypesOfSort(SortIdx sort, Handler &&handler) const
{
    ForAllSupertypes([this, &sort, &handler](const Type &type) {
        if (type.Sort() == sort) {
            return handler(type);
        }
        return true;
    });
}

template <typename Handler>
void Type::ForAllSubtypes(Handler &&handler) const
{
    GetTypeSystem().ForAllSubtypesOf(*this, std::move(handler));
}

template <typename Handler>
void Type::ForAllSubtypesOfSort(SortIdx sort, Handler &&handler) const
{
    ForAllSubtypes([this, &sort, &handler](const Type &type) {
        if (type.Sort() == sort) {
            return handler(type);
        }
        return true;
    });
}
}  // namespace panda::verifier

#endif  // !PANDA_TYPE_TYPE_INL_HPP__
