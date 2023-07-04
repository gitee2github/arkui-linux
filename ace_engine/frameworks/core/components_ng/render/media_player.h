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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_MEDIA_PLAYER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_MEDIA_PLAYER_H

#include <cstdint>

#include "base/memory/ace_type.h"
#include "base/utils/noncopyable.h"
#include "core/components/video/video_utils.h"
#include "core/components_ng/render/render_surface.h"

namespace OHOS::Ace::NG {
using PositionUpdatedEvent = std::function<void(uint32_t)>;
using StateChangedEvent = std::function<void(PlaybackStatus)>;
using CommonEvent = std::function<void()>;
// MediaPlayer is used to show and play meida
class MediaPlayer : public virtual AceType {
    DECLARE_ACE_TYPE(NG::MediaPlayer, AceType)

public:
    ~MediaPlayer() override = default;

    static RefPtr<MediaPlayer> Create();

    virtual void CreateMediaPlayer() {}

    virtual void ResetMediaPlayer() {}

    virtual bool IsMediaPlayerValid()
    {
        return false;
    }

    virtual void SetVolume(float leftVolume, float rightVolume) {}

    virtual bool SetSource(const std::string& /*src*/)
    {
        return false;
    }

    virtual void SetRenderSurface(const RefPtr<RenderSurface>& renderSurface) {}

    virtual void RegisterMediaPlayerEvent(PositionUpdatedEvent&& positionUpdatedEvent,
        StateChangedEvent&& stateChangedEvent, CommonEvent&& errorEvent, CommonEvent&& resolutionChangeEvent)
    {}

    virtual int32_t GetDuration(int32_t& /*duration*/)
    {
        return -1;
    }

    virtual int32_t GetVideoWidth()
    {
        return -1;
    }

    virtual int32_t GetVideoHeight()
    {
        return -1;
    }

    virtual int32_t SetLooping(bool /*loop*/)
    {
        return -1;
    }

    virtual int32_t SetPlaybackSpeed(float /*speed*/)
    {
        return -1;
    }

    virtual int32_t SetSurface()
    {
        return -1;
    }

    virtual int32_t PrepareAsync()
    {
        return -1;
    }

    virtual bool IsPlaying()
    {
        return false;
    }

    virtual int32_t Play()
    {
        return -1;
    }

    virtual int32_t Pause()
    {
        return -1;
    }

    virtual int32_t Stop()
    {
        return -1;
    }

    virtual int32_t Seek(int32_t /*mSeconds*/, SeekMode /*mode*/)
    {
        return -1;
    }

protected:
    MediaPlayer() = default;

    ACE_DISALLOW_COPY_AND_MOVE(MediaPlayer);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_MEDIA_PLAYER_H
