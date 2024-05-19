#include "utils/job_system.h"

namespace core
{

void Job::Execute()
{
    hasStarted_.store(true, std::memory_order_release);
    ExecuteImpl();
    isDone_.store(true, std::memory_order_release);
}

bool Job::HasStarted() const
{
    return hasStarted_.load(std::memory_order_acquire);
}

bool Job::IsDone() const
{
    return isDone_.load(std::memory_order_acquire);
}

bool Job::ShouldStart() const
{
    return true;
}

void Job::Reset()
{
    hasStarted_.store(false, std::memory_order_release);
    isDone_.store(false, std::memory_order_release);
}

bool Job::CheckDependency(const Job *ptr) const
{
    return false;
}

void FuncJob::ExecuteImpl()
{
    func_();
}

bool FuncDependentJob::ShouldStart() const
{
    const auto dependencyJob = dependency_.lock();
    if(dependencyJob != nullptr)
    {
        return dependencyJob->HasStarted();
    }
    return false;
}

bool FuncDependentJob::CheckDependency(const Job *ptr) const
{
    if(ptr == this)
    {
        return true;
    }
    auto dep = dependency_.lock();
    if(dep != nullptr) {
        return dep->CheckDependency(ptr);
    }
    return false;
}

bool FuncDependenciesJob::ShouldStart() const
{
    bool shouldStart = true;
    for (auto& dependency : dependencies_)
    {
        const auto dependencyJob = dependency.lock();
        if (dependencyJob != nullptr && !dependencyJob->HasStarted())
        {
            shouldStart = false;
            break;
        }
    }
    return shouldStart;
}

bool FuncDependenciesJob::AddDependency(const std::weak_ptr<Job>& dependency)
{
    auto newDependency = dependency.lock();
    if(newDependency == nullptr || newDependency->CheckDependency(this))
    {
        return false;
    }
    dependencies_.push_back(dependency);
    return true;
}

bool FuncDependenciesJob::CheckDependency(const Job *ptr) const
{
    if(ptr == this)
    {
        return true;
    }
    for(auto& dependency: dependencies_)
    {
        if(dependency.lock()->CheckDependency(ptr))
        {
            return true;
        }
    }
    return false;
}

void WorkerQueue::Begin()
{
    isRunning_.store(true, std::memory_order_release);
}

void WorkerQueue::AddJob(const std::shared_ptr<Job>& newJob)
{
    std::scoped_lock lock(mutex_);
    jobsQueue_.push(newJob);
    conditionVariable_.notify_one();
}

bool WorkerQueue::IsEmpty() const
{
    std::shared_lock lock(mutex_);
    return jobsQueue_.empty();
}

bool WorkerQueue::IsRunning() const
{
    return isRunning_.load(std::memory_order_acquire);
}

std::shared_ptr<Job> WorkerQueue::PopNextTask()
{
    if (IsEmpty())
        return nullptr;

    std::scoped_lock lock(mutex_);
    if (jobsQueue_.empty())
        return nullptr;
    auto newTask = jobsQueue_.front();
    jobsQueue_.pop();
    return newTask;
}

void WorkerQueue::WaitForTask()
{
    std::unique_lock lock(mutex_);
    conditionVariable_.wait(lock);
}

void WorkerQueue::End()
{
    isRunning_.store(false, std::memory_order::release);
    conditionVariable_.notify_all();
}

void Worker::Begin()
{
    thread_ = std::thread(&Worker::Run, this);
}

void Worker::End()
{
    if(thread_.joinable())
    {
        thread_.join();
    }
}

void Worker::Run()
{
    while(queue_.IsRunning())
    {
        if (queue_.IsEmpty())
        {
            if (!queue_.IsRunning())
            {
                break;
            }
            queue_.WaitForTask();
        }
        else
        {
            while (!queue_.IsEmpty())
            {
                auto newTask = queue_.PopNextTask();
                if (newTask == nullptr)
                    continue;
                if (!newTask->ShouldStart())
                {
                    queue_.AddJob(std::move(newTask));
                }
                else
                {
                    newTask->Execute();
                }
            }
        }
    }
}

static JobSystem* instance = nullptr;

JobSystem::JobSystem()
{
    instance = this;
}

int JobSystem::SetupNewQueue(int threadCount)
{
    const int newQueueIndex = queues_.size();
    queues_.emplace_back();
    for(int i = 0; i < threadCount; i++)
    {
        workers_.emplace_back(queues_.back());
    }
    return newQueueIndex;
}

void JobSystem::Begin()
{
    for(auto& queue: queues_)
    {
        queue.Begin();
    }
    for(auto& worker : workers_)
    {
        worker.Begin();
    }
}

void JobSystem::AddJob(const std::shared_ptr<Job>& newJob, int queueIndex)
{
    newJob->Reset();
    if(queueIndex == MAIN_QUEUE_INDEX)
    {
        mainThreadQueue_.AddJob(newJob);
        return;
    }
    queues_[queueIndex].AddJob(newJob);
}

void JobSystem::End()
{
    for(auto& queue: queues_)
    {
        queue.End();
    }
    for(auto& worker: workers_)
    {
        worker.End();
    }
}

void JobSystem::ExecuteMainThread()
{
    while (!mainThreadQueue_.IsEmpty())
    {
        auto newTask = mainThreadQueue_.PopNextTask();
        if (!newTask->ShouldStart())
        {
            mainThreadQueue_.AddJob(std::move(newTask));
        }
        else
        {
            newTask->Execute();
        }
    }
}

JobSystem* GetJobSystem()
{
    if(instance == nullptr)
    {
        // Instance should not be nullptr
        // It means the JobSystem was not instanced somewhere
        std::terminate();
    }
    return instance;
}

}