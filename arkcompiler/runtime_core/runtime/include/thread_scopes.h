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

#ifndef PANDA_RUNTIME_THREAD_SCOPES_H_
#define PANDA_RUNTIME_THREAD_SCOPES_H_

#include "mtmanaged_thread.h"

namespace panda {

class ScopedNativeCodeThread {
public:
    explicit ScopedNativeCodeThread(ManagedThread *thread) : thread_(thread)
    {
        ASSERT(thread_ != nullptr);
        ASSERT(thread_ == ManagedThread::GetCurrent());
        thread_->NativeCodeBegin();
    }

    ~ScopedNativeCodeThread()
    {
        thread_->NativeCodeEnd();
    }

private:
    ManagedThread *thread_;

    NO_COPY_SEMANTIC(ScopedNativeCodeThread);
    NO_MOVE_SEMANTIC(ScopedNativeCodeThread);
};

class ScopedManagedCodeThread {
public:
    explicit ScopedManagedCodeThread(ManagedThread *thread) : thread_(thread)
    {
        ASSERT(thread_ != nullptr);
        ASSERT(thread_ == ManagedThread::GetCurrent());
        thread_->ManagedCodeBegin();
    }

    ~ScopedManagedCodeThread()
    {
        thread_->ManagedCodeEnd();
    }

private:
    ManagedThread *thread_;

    NO_COPY_SEMANTIC(ScopedManagedCodeThread);
    NO_MOVE_SEMANTIC(ScopedManagedCodeThread);
};

class ScopedChangeThreadStatus {
public:
    explicit ScopedChangeThreadStatus(ManagedThread *thread, ThreadStatus new_status) : thread_(thread)
    {
        old_status_ = thread_->GetStatus();
        thread_->UpdateStatus(new_status);
    }

    ~ScopedChangeThreadStatus()
    {
        thread_->UpdateStatus(old_status_);
    }

private:
    ManagedThread *thread_;
    ThreadStatus old_status_;

    NO_COPY_SEMANTIC(ScopedChangeThreadStatus);
    NO_MOVE_SEMANTIC(ScopedChangeThreadStatus);
};

}  // namespace panda

#endif  // PANDA_RUNTIME_THREAD_SCOPES_H_
