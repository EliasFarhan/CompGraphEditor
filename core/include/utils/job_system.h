#pragma once

#include <memory>
#include <functional>
#include <thread>
#include <queue>
#include <shared_mutex>

namespace core
{
class Job
{
public:
    virtual ~Job() = default;
    void Execute();
    bool HasStarted() const;
    bool IsDone() const;
    virtual bool ShouldStart() const;
protected:
    virtual void ExecuteImpl() = 0;
private:
    std::atomic<bool> hasStarted_{ false };
    std::atomic<bool> isDone_{ false };
};

class FuncJob : public Job
{
public:
    FuncJob(const std::function<void(void)>& func): func_(func){}
protected:
    void ExecuteImpl() override;

private:
    std::function<void(void)> func_;
};

class DependencyJob : public Job
{
public:
    DependencyJob(const std::weak_ptr<Job>& dependency): dependency_(dependency){}
    bool ShouldStart() const override;

private:
    std::weak_ptr<Job> dependency_{};
};
class DependenciesJob : public Job
{
public:
    DependenciesJob(std::initializer_list<std::weak_ptr<Job>> dependencies) : dependencies_(dependencies) {}
    [[nodiscard]] bool ShouldStart() const override;

    void AddDependency(const std::weak_ptr<Job>& dependency);

private:
    std::vector<std::weak_ptr<Job>> dependencies_{};
};

class FuncDependentJob : public FuncJob, public DependencyJob
{
    using DependencyJob::DependencyJob;
};

class FuncDependenciesJob: public FuncJob, public DependenciesJob
{
    using DependenciesJob::DependenciesJob;
};

class WorkerQueue
{
public:
    WorkerQueue() = default;
    WorkerQueue(const WorkerQueue&) = delete;
    WorkerQueue& operator= (const WorkerQueue&) = delete;
    WorkerQueue(WorkerQueue&&) noexcept{}
    WorkerQueue& operator= (WorkerQueue&&) noexcept{}
    void Begin();
    void AddJob(const std::shared_ptr<Job>& newJob);
    bool IsEmpty() const;
    bool IsRunning() const;
    std::shared_ptr<Job> PopNextTask();
    void WaitForTask();
    void End();
private:
    mutable std::shared_mutex mutex_;
    std::queue<std::shared_ptr<Job>> jobsQueue_;
    std::condition_variable_any conditionVariable_;
    std::atomic<bool> isRunning_{ false };
};

class Worker
{
public:
    Worker(WorkerQueue& queue) : queue_(queue){}
    void Begin();
    void End();
private:
    void Run();
    std::thread thread_;
    WorkerQueue& queue_;
};


class JobSystem
{
public:
    JobSystem();
    int SetupNewQueue(int threadCount);
    void Begin();
    void AddJob(const std::shared_ptr<Job>& newJob, int queueIndex);
    void End();
    void ExecuteMainThread();
private:
    WorkerQueue mainThreadQueue_{};
    std::vector<WorkerQueue> queues_{};
    std::vector<Worker> workers_{};
};

JobSystem* GetJobSystem();
}