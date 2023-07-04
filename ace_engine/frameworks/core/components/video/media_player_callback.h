/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_MEDIA_PLAYER_CALLBACK_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_MEDIA_PLAYER_CALLBACK_H

#include "base/log/log.h"

#include "core/common/container_scope.h"
#include "core/components/video/video_utils.h"
#include "foundation/multimedia/player_framework/interfaces/inner_api/native/player.h"

namespace OHOS::Ace {
namespace {

constexpr int32_t MILLISECONDS_TO_SECONDS = 1000;

PlaybackStatus ConvertToPlaybackStatus(int32_t status)
{
    PlaybackStatus result = PlaybackStatus::NONE;
    switch (status) {
        case OHOS::Media::PLAYER_STATE_ERROR:
            result = PlaybackStatus::ERROR;
            break;
        case OHOS::Media::PLAYER_IDLE:
            result = PlaybackStatus::IDLE;
            break;
        case OHOS::Media::PLAYER_PREPARED:
            result = PlaybackStatus::PREPARED;
            break;
        case OHOS::Media::PLAYER_STARTED:
            result = PlaybackStatus::STARTED;
            break;
        case OHOS::Media::PLAYER_PAUSED:
            result = PlaybackStatus::PAUSED;
            break;
        case OHOS::Media::PLAYER_STOPPED:
            result = PlaybackStatus::STOPPED;
            break;
        case OHOS::Media::PLAYER_PLAYBACK_COMPLETE:
            result = PlaybackStatus::PLAYBACK_COMPLETE;
            break;
        default:
            LOGE("status is not supported");
            break;
    }
    return result;
}

} // namespace

struct MediaPlayerCallback : public Media::PlayerCallback {

public:
    using PositionUpdatedEvent = std::function<void(uint32_t)>;
    using StateChangedEvent = std::function<void(PlaybackStatus)>;
    using CommonEvent = std::function<void()>;

    MediaPlayerCallback() = default;
    explicit MediaPlayerCallback(int32_t instanceId)
    {
        instanceId_ = instanceId;
    }

    ~MediaPlayerCallback() = default;

    // Below api9
    void OnError(Media::PlayerErrorType errorType, int32_t errorCode) override
    {
        LOGE("OnError callback, errorType: %{public}d, errorCode: %{public}d", errorType, errorCode);
        ContainerScope scope(instanceId_);
        if (errorEvent_) {
            errorEvent_();
        }
    }

    // Above api9
    void OnError(int32_t errorCode, const std::string &errorMsg) override
    {
        LOGE("OnError callback, errorCode: %{public}d, error message: %{public}s", errorCode, errorMsg.c_str());
        ContainerScope scope(instanceId_);
        if (errorEvent_) {
            errorEvent_();
        }
    }

    void OnInfo(Media::PlayerOnInfoType type, int32_t extra, const Media::Format &InfoBody = {}) override
    {
        LOGI("OnInfo type: %{public}d, extra: %{public}d", type, extra);
        ContainerScope scope(instanceId_);
        switch (type) {
            case OHOS::Media::INFO_TYPE_SEEKDONE:
                LOGI("OnSeekDone callback");
                if (positionUpdatedEvent_) {
                    positionUpdatedEvent_(extra / MILLISECONDS_TO_SECONDS);
                }
                break;
            case OHOS::Media::INFO_TYPE_EOS:
                LOGI("OnEndOfStream callback");
                if (endOfStreamEvent_) {
                    endOfStreamEvent_();
                }
                break;
            case OHOS::Media::INFO_TYPE_STATE_CHANGE:
                LOGI("OnStateChanged callback");
                PrintState(static_cast<OHOS::Media::PlayerStates>(extra));
                if (stateChangedEvent_) {
                    stateChangedEvent_(ConvertToPlaybackStatus(extra));
                }
                break;
            case OHOS::Media::INFO_TYPE_POSITION_UPDATE:
                if (positionUpdatedEvent_) {
                    positionUpdatedEvent_(extra / MILLISECONDS_TO_SECONDS);
                }
                break;
            case OHOS::Media::INFO_TYPE_RESOLUTION_CHANGE:
                if (resolutionChangeEvent_) {
                    resolutionChangeEvent_();
                }
                break;
            case OHOS::Media::INFO_TYPE_MESSAGE:
                LOGI("OnMessage callback type: %{public}d", extra);
                break;
            default:
                break;
            }
    }

    void PrintState(OHOS::Media::PlayerStates state) const
    {
        switch (state) {
            case OHOS::Media::PLAYER_STOPPED:
                LOGI("State: Stopped");
                break;
            case OHOS::Media::PLAYER_PREPARED:
                LOGI("State: Buffering");
                break;
            case OHOS::Media::PLAYER_PAUSED:
                LOGI("State: Paused");
                break;
            case OHOS::Media::PLAYER_STARTED:
                LOGI("State: Playing");
                break;
            default:
                LOGI("Invalid state");
                break;
        }
    }

    void SetPositionUpdatedEvent(PositionUpdatedEvent&& positionUpdatedEvent)
    {
        positionUpdatedEvent_ = std::move(positionUpdatedEvent);
    }

    void SetEndOfStreamEvent(CommonEvent&& endOfStreamEvent)
    {
        endOfStreamEvent_ = std::move(endOfStreamEvent);
    }

    void SetStateChangedEvent(StateChangedEvent&& stateChangedEvent)
    {
        stateChangedEvent_ = std::move(stateChangedEvent);
    }

    void SetErrorEvent(CommonEvent&& errorEvent)
    {
        errorEvent_ = std::move(errorEvent);
    }

    void SetResolutionChangeEvent(CommonEvent&& resolutionChangeEvent)
    {
        resolutionChangeEvent_ = std::move(resolutionChangeEvent);
    }

private:
    PositionUpdatedEvent positionUpdatedEvent_;
    CommonEvent endOfStreamEvent_;
    StateChangedEvent stateChangedEvent_;
    CommonEvent errorEvent_;
    CommonEvent resolutionChangeEvent_;
    int32_t instanceId_ = -1;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_MEDIA_PLAYER_CALLBACK_H
