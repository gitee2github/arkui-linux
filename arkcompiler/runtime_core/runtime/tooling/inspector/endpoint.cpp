/**
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

#include "endpoint.h"

#include "utils/json_parser.h"
#include "utils/logger.h"

#include <optional>

namespace panda::tooling::inspector {
void EndpointBase::HandleMessage(const std::string &message)
{
    JsonObject request(message);
    if (!request.IsValid()) {
        LOG(INFO, DEBUGGER) << "Invalid request: " << message;
        return;
    }

    LOG(DEBUG, DEBUGGER) << "Received " << message;

    auto id = request.GetValue<JsonObject::NumT>("id");
    auto method = request.GetValue<JsonObject::StringT>("method");
    auto result = request.GetValue<JsonObject::JsonObjPointer>("result");

    if (method != nullptr && result == nullptr) {
        if (auto handler = methodHandlers_.find(*method); handler != methodHandlers_.end()) {
            auto *params = request.GetValue<JsonObject::JsonObjPointer>("params");

            JsonObject empty;

            handler->second(id != nullptr ? std::make_optional(*id) : std::nullopt,
                            params != nullptr ? **params : empty);
        } else {
            LOG(WARNING, DEBUGGER) << "Unsupported method: " << message;
        }
    } else if (result != nullptr && method == nullptr) {
        if (id == nullptr) {
            LOG(INFO, DEBUGGER) << "Response object with no \"id\": " << message;
            return;
        }

        if (auto handler = resultHandlers_.extract(*id)) {
            handler.mapped()(**result);
        }
    } else {
        LOG(INFO, DEBUGGER) << "Expected either 'method' or 'result' parameter";
    }
}
}  // namespace panda::tooling::inspector
