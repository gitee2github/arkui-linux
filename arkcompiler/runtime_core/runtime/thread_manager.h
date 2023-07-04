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
#ifndef PANDA_RUNTIME_THREAD_MANAGER_H_
#define PANDA_RUNTIME_THREAD_MANAGER_H_

#include <bitset>

#include "libpandabase/os/mutex.h"
#include "libpandabase/utils/time.h"
#include "libpandabase/os/time.h"
#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mtmanaged_thread.h"
#include "runtime/include/thread_status.h"
#include "runtime/include/locks.h"

namespace panda {

// This interval is required for waiting for threads to stop.
static const int WAIT_INTERVAL = 10;
static constexpr int64_t K_MAX_DUMP_TIME_NS = UINT64_C(6 * 1000 * 1000 * 1000);   // 6s
static constexpr int64_t K_MAX_SINGLE_DUMP_TIME_NS = UINT64_C(50 * 1000 * 1000);  // 50ms

enum class EnumerationFlag {
    NONE = 0,                 // Nothing
    NON_CORE_THREAD = 1,      // Plugin type thread
    MANAGED_CODE_THREAD = 2,  // Thread which can execute managed code
    VM_THREAD = 4,            // Includes VM threads
    ALL = 8,                  // See the comment in the function SatisfyTheMask below
};

class ThreadManager {
public:
    NO_COPY_SEMANTIC(ThreadManager);
    NO_MOVE_SEMANTIC(ThreadManager);

    // For performance reasons don't exceed specified amount of bits.
    static constexpr size_t MAX_INTERNAL_THREAD_ID = std::min(0xffffU, ManagedThread::MAX_INTERNAL_THREAD_ID);

    explicit ThreadManager(mem::InternalAllocatorPtr allocator);

    virtual ~ThreadManager();

    template <class Callback>
    void EnumerateThreads(const Callback &cb, unsigned int mask = static_cast<unsigned int>(EnumerationFlag::ALL),
                          unsigned int xor_mask = static_cast<unsigned int>(EnumerationFlag::NONE)) const
    {
        os::memory::LockHolder lock(thread_lock_);

        EnumerateThreadsWithLockheld(cb, mask, xor_mask);
    }

    template <class Callback>
    void EnumerateThreadsWithLockheld(const Callback &cb,
                                      unsigned int inc_mask = static_cast<unsigned int>(EnumerationFlag::ALL),
                                      unsigned int xor_mask = static_cast<unsigned int>(EnumerationFlag::NONE)) const
        REQUIRES(thread_lock_)
    {
        for (auto t : threads_) {
            bool inc_target = SatisfyTheMask(t, inc_mask);
            bool xor_target = SatisfyTheMask(t, xor_mask);
            if (inc_target != xor_target) {
                if (!cb(t)) {
                    break;
                }
            }
        }
    }

    template <class Callback>
    void EnumerateThreadsForDump(const Callback &cb, std::ostream &os)
    {
        // TODO: can not get WriteLock() when other thread run code "while {}"
        // issue #3085
        SuspendAllThreads();
        Locks::mutator_lock->WriteLock();
        MTManagedThread *self = MTManagedThread::GetCurrent();
        {
            os << "ARK THREADS (" << threads_count_ << "):\n";
        }
        if (self != nullptr) {
            os::memory::LockHolder lock(thread_lock_);
            int64_t start = panda::os::time::GetClockTimeInThreadCpuTime();
            int64_t end;
            int64_t last_time = start;
            cb(self, os);
            for (const auto &thread : threads_) {
                if (thread != self) {
                    cb(thread, os);
                    end = panda::os::time::GetClockTimeInThreadCpuTime();
                    if ((end - last_time) > K_MAX_SINGLE_DUMP_TIME_NS) {
                        LOG(ERROR, RUNTIME) << "signal catcher: thread_list_dump thread : " << thread->GetId()
                                            << "timeout : " << (end - last_time);
                    }
                    last_time = end;
                    if ((end - start) > K_MAX_DUMP_TIME_NS) {
                        LOG(ERROR, RUNTIME) << "signal catcher: thread_list_dump timeout : " << end - start << "\n";
                        break;
                    }
                }
            }
        }
        Locks::mutator_lock->Unlock();
        ResumeAllThreads();
    }

    void RegisterThread(MTManagedThread *thread)
    {
        os::memory::LockHolder lock(thread_lock_);
        threads_count_++;
#ifndef NDEBUG
        registered_threads_count_++;
#endif  // NDEBUG
        threads_.emplace_back(thread);
        for (uint32_t i = suspend_new_count_; i > 0; i--) {
            thread->SuspendImpl(true);
        }
    }

    void IncPendingThreads()
    {
        os::memory::LockHolder lock(thread_lock_);
        pending_threads_++;
    }

    void DecPendingThreads()
    {
        os::memory::LockHolder lock(thread_lock_);
        pending_threads_--;
    }

    void AddDaemonThread()
    {
        daemon_threads_count_++;
    }

    int GetThreadsCount();

#ifndef NDEBUG
    uint32_t GetAllRegisteredThreadsCount();
#endif  // NDEBUG

    void WaitForDeregistration();

    void SuspendAllThreads();
    void ResumeAllThreads();

    uint32_t GetInternalThreadId();

    void RemoveInternalThreadId(uint32_t id);

    bool IsRunningThreadExist();

    // Returns true if unregistration succeeded; for now it can fail when we are trying to unregister main thread
    bool UnregisterExitedThread(MTManagedThread *thread);

    MTManagedThread *SuspendAndWaitThreadByInternalThreadId(uint32_t thread_id);

    void RegisterSensitiveThread() const;

    os::memory::Mutex *GetThreadsLock()
    {
        return &thread_lock_;
    }

    void SetMainThread(ManagedThread *thread)
    {
        main_thread_ = thread;
    }

private:
    bool HasNoActiveThreads() const REQUIRES(thread_lock_)
    {
        ASSERT(threads_count_ >= daemon_threads_count_);
        auto thread = static_cast<uint32_t>(threads_count_ - daemon_threads_count_);
        return thread < 2 && pending_threads_ == 0;
    }

    bool StopThreadsOnDeadlock(MTManagedThread *current) REQUIRES(thread_lock_);

    bool SatisfyTheMask(MTManagedThread *t, unsigned int mask) const
    {
        if ((mask & static_cast<unsigned int>(EnumerationFlag::ALL)) != 0) {
            // Some uninitialized threads may not have attached flag,
            // So, they are not included as MANAGED_CODE_THREAD.
            // Newly created threads are using flag suspend new count.
            // The case leads to deadlocks, when the thread can not be resumed.
            // To deal with it, just add a specific ALL case
            return true;
        }

        // For NONE mask
        bool target = false;
        if ((mask & static_cast<unsigned int>(EnumerationFlag::MANAGED_CODE_THREAD)) != 0) {
            target = t->IsAttached();
            if ((mask & static_cast<unsigned int>(EnumerationFlag::NON_CORE_THREAD)) != 0) {
                // Due to hyerarhical structure, we need to conjunct types
                bool non_core_thread = t->GetThreadLang() != panda::panda_file::SourceLang::PANDA_ASSEMBLY;
                target = target && non_core_thread;
            }
        }

        if ((mask & static_cast<unsigned int>(EnumerationFlag::VM_THREAD)) != 0) {
            target = target || t->IsVMThread();
        }

        return target;
    }

    /**
     * Tries to stop all daemon threads in case there are no active basic threads
     * returns false if we need to wait
     */
    void StopDaemonThreads() REQUIRES(thread_lock_);

    /**
     * Deregister all suspended threads including daemon threads.
     * Returns true on success and false otherwise.
     */
    bool DeregisterSuspendedThreads() REQUIRES(thread_lock_);

    void DecreaseCountersForThread(MTManagedThread *thread) REQUIRES(thread_lock_);

    MTManagedThread *GetThreadByInternalThreadIdWithLockHeld(uint32_t thread_id) REQUIRES(thread_lock_);

    bool CanDeregister(enum ThreadStatus status)
    {
        // Deregister thread only for IS_TERMINATED_LOOP.
        // In all other statuses we should wait:
        // * CREATED - wait until threads finish initializing which requires communication with ThreadManager;
        // * BLOCKED - it means we are trying to acquire lock in Monitor, which was created in internalAllocator;
        // * TERMINATING - threads which requires communication with Runtime;
        // * FINISHED threads should be deleted itself;
        // * NATIVE threads are either go to FINISHED status or considered a part of a deadlock;
        // * other statuses - should eventually go to IS_TERMINATED_LOOP or FINISHED status.
        return status == ThreadStatus::IS_TERMINATED_LOOP;
    }

    mutable os::memory::Mutex thread_lock_;
    ManagedThread *main_thread_ {nullptr};
    // Counter used to suspend newly created threads after SuspendAllThreads/SuspendDaemonThreads
    uint32_t suspend_new_count_ GUARDED_BY(thread_lock_) = 0;
    // We should delete only finished thread structures, so call delete explicitly on finished threads
    // and don't touch other pointers
    PandaList<MTManagedThread *> threads_ GUARDED_BY(thread_lock_);
    os::memory::Mutex ids_lock_;
    std::bitset<MAX_INTERNAL_THREAD_ID> internal_thread_ids_ GUARDED_BY(ids_lock_);
    uint32_t last_id_ GUARDED_BY(ids_lock_);
    PandaList<MTManagedThread *> daemon_threads_;

    os::memory::ConditionVariable stop_var_;
    std::atomic_uint32_t threads_count_ = 0;
#ifndef NDEBUG
    // This field is required for counting all registered threads (including finished daemons)
    // in AttachThreadTest. It is not needed in production mode.
    std::atomic_uint32_t registered_threads_count_ = 0;
#endif  // NDEBUG
    std::atomic_uint32_t daemon_threads_count_ = 0;
    // A specific counter of threads, which are not completely created
    // When the counter != 0, operations with thread set are permitted to avoid destruction of shared data (mutexes)
    // Synchronized with lock (not atomic) for mutual exclusion with thread operations
    int pending_threads_ GUARDED_BY(thread_lock_);
};

}  // namespace panda

#endif  // PANDA_RUNTIME_THREAD_MANAGER_H_
