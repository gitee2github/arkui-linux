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

#ifndef PANDA_RUNTIME_THREAD_POOL_H_
#define PANDA_RUNTIME_THREAD_POOL_H_

#include "libpandabase/os/mutex.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/thread_pool_queue.h"

static constexpr uint64_t TASK_WAIT_TIMEOUT = 500U;

namespace panda {

template <typename Task, typename ProcArg>
class ProcessorInterface {
public:
    NO_COPY_SEMANTIC(ProcessorInterface);
    NO_MOVE_SEMANTIC(ProcessorInterface);

    ProcessorInterface() = default;
    virtual ~ProcessorInterface() = default;

    explicit ProcessorInterface(ProcArg args);
    virtual bool Process(Task) = 0;
    // before main loop
    virtual bool Init()
    {
        return true;
    }
    // before thread exit
    virtual bool Destroy()
    {
        return true;
    }
};

class WorkerCreationInterface {
public:
    NO_COPY_SEMANTIC(WorkerCreationInterface);
    NO_MOVE_SEMANTIC(WorkerCreationInterface);
    WorkerCreationInterface() = default;
    virtual ~WorkerCreationInterface() = default;
    virtual void AttachWorker([[maybe_unused]] bool helper_thread)
    {
        // do nothing here
    }
    virtual void DetachWorker([[maybe_unused]] bool helper_thread)
    {
        // do nothing here
    }
};

template <typename Task, typename Proc, typename ProcArg>
class ThreadPool {
public:
    NO_COPY_SEMANTIC(ThreadPool);
    NO_MOVE_SEMANTIC(ThreadPool);

    explicit ThreadPool(mem::InternalAllocatorPtr allocator, TaskQueueInterface<Task> *queue, ProcArg args,
                        size_t n_threads = 1, const char *thread_name = nullptr,
                        WorkerCreationInterface *worker_creation_interface = nullptr)
        : allocator_(allocator),
          queue_(queue),
          workers_(allocator_->Adapter()),
          procs_(allocator_->Adapter()),
          args_(args),
          is_thread_active_(allocator_->Adapter()),
          worker_creation_interface_(worker_creation_interface)
    {
        is_active_ = true;
        thread_name_ = thread_name;
        Scale(n_threads);
    }

    ~ThreadPool()
    {
        os::memory::LockHolder lock(scale_lock_);
        DeactivateWorkers();
        WaitForWorkers();
    }

    void Scale(size_t new_n_threads)
    {
        os::memory::LockHolder scale_lock(scale_lock_);
        if (!IsActive()) {
            return;
        }
        LOG(DEBUG, RUNTIME) << "Scale thread pool for " << new_n_threads << " new threads";
        if (new_n_threads <= 0) {
            LOG(ERROR, RUNTIME) << "Incorrect number of threads " << new_n_threads << " for thread pool";
            return;
        }
        if (new_n_threads > threads_counter_) {
            // Need to add new threads.
            {
                os::memory::LockHolder queue_lock(queue_lock_);
                is_thread_active_.resize(new_n_threads);
            }
            for (size_t i = threads_counter_; i < new_n_threads; i++) {
                CreateNewThread(i);
            }
        } else if (new_n_threads < threads_counter_) {
            // Need to remove threads.
            for (size_t i = threads_counter_ - 1; i >= new_n_threads; i--) {
                StopWorker(workers_.back(), i);
                workers_.pop_back();
                allocator_->Delete(procs_.back());
                procs_.pop_back();
            }
            {
                os::memory::LockHolder queue_lock(queue_lock_);
                is_thread_active_.resize(new_n_threads);
            }
        } else {
            // Same number of threads - do nothing.
        }
        threads_counter_ = new_n_threads;
        LOG(DEBUG, RUNTIME) << "Scale has been completed";
    }

    void Help()
    {
        // Disallow scaling while the main thread processes the queue
        os::memory::LockHolder scale_lock(scale_lock_);
        if (!IsActive()) {
            return;
        }
        auto *proc = allocator_->New<Proc>(args_);
        ASSERT(proc != nullptr);
        WorkerCreationInterface *iface = GetWorkerCreationInterface();
        if (iface != nullptr) {
            iface->AttachWorker(true);
        }
        if (!proc->Init()) {
            LOG(FATAL, RUNTIME) << "Cannot initialize worker thread";
        }
        while (true) {
            Task task;
            {
                os::memory::LockHolder lock(queue_lock_);
                task = queue_->GetTask();
            }
            if (task.IsEmpty()) {
                break;
            }
            SignalTask();
            proc->Process(task);
        }
        if (!proc->Destroy()) {
            LOG(FATAL, RUNTIME) << "Cannot destroy worker thread";
        }
        if (iface != nullptr) {
            iface->DetachWorker(true);
        }
        allocator_->Delete(proc);
    }

    bool TryPutTask(Task task)
    {
        bool res = false;
        {
            os::memory::LockHolder lock(queue_lock_);
            if (!is_active_) {
                return false;
            }
            res = queue_->TryAddTask(std::move(task));
        }
        if (res) {
            // Task was added.
            SignalTask();
        }
        return res;
    }

    bool PutTask(Task task)
    {
        {
            os::memory::LockHolder lock(queue_lock_);
            if (!is_active_) {
                return false;
            }
            while (queue_->IsFull()) {
                WaitTask();
            }
            queue_->AddTask(std::move(task));
        }
        SignalTask();
        return true;
    }

    bool IsActive()
    {
        os::memory::LockHolder lock(queue_lock_);
        return is_active_;
    }

    void Shutdown(bool force = false)
    {
        os::memory::LockHolder lock(scale_lock_);
        DeactivateWorkers();
        if (force) {
            // Sync.
            WaitForWorkers();
        }
    }

    void WaitTask()
    {
        cond_var_.TimedWait(&queue_lock_, TASK_WAIT_TIMEOUT);
    }

    static void WorkerEntry(ThreadPool<Task, Proc, ProcArg> *thread_pool, Proc *proc, int i)
    {
        WorkerCreationInterface *iface = thread_pool->GetWorkerCreationInterface();
        if (iface != nullptr) {
            iface->AttachWorker(false);
        }
        if (!proc->Init()) {
            LOG(FATAL, RUNTIME) << "Cannot initialize worker thread";
        }
        while (true) {
            Task task;
            {
                os::memory::LockHolder lock(thread_pool->queue_lock_);
                if (!thread_pool->IsActive(i)) {
                    break;
                }
                task = std::move(thread_pool->queue_->GetTask());
                if (task.IsEmpty()) {
                    thread_pool->WaitTask();
                    continue;
                }
            }
            thread_pool->SignalTask();
            LOG(DEBUG, RUNTIME) << "Worker " << i << " started to process task";
            proc->Process(task);
        }
        if (!proc->Destroy()) {
            LOG(FATAL, RUNTIME) << "Cannot destroy worker thread";
        }
        if (iface != nullptr) {
            iface->DetachWorker(false);
        }
        LOG(DEBUG, RUNTIME) << "Worker " << i << " is finished";
    }

private:
    void SignalTask()
    {
        cond_var_.Signal();
    }

    void SignalAllTasks()
    {
        cond_var_.SignalAll();
    }

    void DeactivateWorkers()
    {
        os::memory::LockHolder lock(queue_lock_);
        is_active_ = false;
        queue_->Finalize();
        SignalAllTasks();
        for (size_t i = 0; i < is_thread_active_.size(); i++) {
            is_thread_active_.at(i) = false;
        }
    }

    bool IsActive(int i) REQUIRES(queue_lock_)
    {
        return is_thread_active_.at(i);
    }

    void WaitForWorkers() REQUIRES(scale_lock_)
    {
        for (auto worker : workers_) {
            StopWorker(worker);
        }
        {
            os::memory::LockHolder lock(queue_lock_);
            is_thread_active_.clear();
        }
        workers_.clear();
        for (auto proc : procs_) {
            allocator_->Delete(proc);
        }
        procs_.clear();
    }

    void StopWorker(std::thread *worker, size_t thread_id = 0) REQUIRES(scale_lock_)
    {
        if (worker != nullptr) {
            if (thread_id != 0) {
                os::memory::LockHolder lock(queue_lock_);
                is_thread_active_.at(thread_id) = false;
            }
            SignalAllTasks();
            worker->join();
            allocator_->Delete(worker);
            worker = nullptr;
        }
    }

    void CreateNewThread(int i) REQUIRES(scale_lock_)
    {
        {
            os::memory::LockHolder lock(queue_lock_);
            is_thread_active_.at(i) = true;
        }
        auto proc = allocator_->New<Proc>(args_);
        auto worker = allocator_->New<std::thread>(WorkerEntry, this, proc, i);
        if (worker == nullptr) {
            LOG(FATAL, RUNTIME) << "Cannot create a worker thread";
        }
        if (thread_name_ != nullptr) {
            int res = os::thread::SetThreadName(worker->native_handle(), thread_name_);
            if (res != 0) {
                LOG(ERROR, RUNTIME) << "Failed to set a name for the worker thread";
            }
        }
        workers_.emplace_back(worker);
        procs_.emplace_back(proc);
    }

    WorkerCreationInterface *GetWorkerCreationInterface()
    {
        return worker_creation_interface_;
    }

    mem::InternalAllocatorPtr allocator_;
    os::memory::ConditionVariable cond_var_;
    TaskQueueInterface<Task> *queue_ GUARDED_BY(queue_lock_);
    PandaList<std::thread *> workers_ GUARDED_BY(scale_lock_);
    size_t threads_counter_ GUARDED_BY(scale_lock_) = 0;
    PandaList<Proc *> procs_ GUARDED_BY(scale_lock_);
    ProcArg args_;
    bool is_active_ GUARDED_BY(queue_lock_) = false;
    os::memory::Mutex queue_lock_;
    os::memory::Mutex scale_lock_;
    PandaVector<bool> is_thread_active_ GUARDED_BY(queue_lock_);
    WorkerCreationInterface *worker_creation_interface_;
    const char *thread_name_;
};

}  // namespace panda

#endif  // PANDA_RUNTIME_THREAD_POOL_H_
