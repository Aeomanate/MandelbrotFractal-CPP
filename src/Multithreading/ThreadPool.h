#ifndef MANDELBROT_CPP_THREADPOOL_H
#define MANDELBROT_CPP_THREADPOOL_H

#include <thread>
#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>

template<class Result>
class TaskPool
{
public:
    using CallableTask = std::function<Result()>;

    template<class TasksIterator>
    void add(TasksIterator tasks_iterator)
    {
        {
            std::lock_guard lock(mutex);
            do
            {
                task_queue.push(*tasks_iterator);
            } while (++tasks_iterator);
        }
        notifyAll();
    }

    CallableTask getTask()
    {
        CallableTask task;
        if (std::lock_guard lock(mutex); !task_queue.empty())
        {
            task = task_queue.front();
            task_queue.pop();
        }
        return task;
    }

    void notifyAll()
    {
        condition_variable.notify_all();
    }

    bool isEmpty()
    {
        std::lock_guard lock(mutex);
        return task_queue.empty();
    }

    void waitForTasks(bool const& is_program_work)
    {
        std::unique_lock lock(mutex);
        condition_variable.wait(lock, [&]
        { return !task_queue.empty() || !is_program_work; });
    }

private:
    std::queue<CallableTask> task_queue;
    std::mutex mutex;
    std::condition_variable condition_variable;
};

template<class Result, class Worker, bool is_not_void>
class WorkerHelper
{
public:
    template<class Task>
    void runTask(Task task)
    {
        finished_work.push_back(std::move(task()));
    }

    auto getResults()
    {
        if (!static_cast<Worker*>(this)->IsWorkDone())
        {
            throw std::logic_error("Work not done");
        }
        return std::move(finished_work);
    }

protected:
    std::vector<Result> finished_work;
};

template<class Result, class Worker>
class WorkerHelper<Result, Worker, true>
{
public:
    template<class Task>
    void runTask(Task task)
    {
        task();
    }
};

template<class Result, bool is_result_void = std::is_same_v<Result, void>>
class Worker : public WorkerHelper<Result, Worker<Result>, is_result_void>
{
public:
    Worker() = default;

    Worker(bool const& is_program_work, TaskPool<Result>& task_pool)
        : thread(&Worker::work, this, std::ref(is_program_work), std::ref(task_pool))
    { }

    ~Worker()
    {
        if (thread.joinable())
        { thread.join(); }
    }

    void work(bool const& is_program_work, TaskPool<Result>& task_pool)
    {
        while (is_program_work)
        {
            typename TaskPool<Result>::CallableTask task = task_pool.getTask();
            if (task)
            {
                this->runTask(task);
            }
            else
            {
                done_work = true;
                task_pool.waitForTasks(is_program_work);
                done_work = false;
            }
        }
    }

    void workMain(TaskPool<Result>& task_pool)
    {
        typename TaskPool<Result>::CallableTask task;
        while ((task = task_pool.getTask()))
        {
            this->runTask(task);
        }

    }

    [[nodiscard]] bool done() const
    {
        return done_work;
    }

private:
    std::thread thread;
    std::atomic<bool> done_work = true;
};

template<class TaskResult>
class ThreadPool
{
public:
    ThreadPool()
    {
        for (size_t i = 0; i != std::thread::hardware_concurrency() - 1; ++i)
        {
            workers.push_back(std::make_unique<Worker<TaskResult>>(is_program_work, task_pool));
        }
        workers.push_back(std::make_unique<Worker<TaskResult>>());
    }

    ~ThreadPool()
    {
        is_program_work = false;
        task_pool.notifyAll();
        workers.clear();
    }

    template<class TasksIterator>
    void addTasks(TasksIterator task_iterator)
    {
        if (!task_iterator)
        {
            return;
        }
        task_pool.add(task_iterator);
    }

    void joinMainToWorkers()
    {
        workers.back()->workMain(task_pool);
    }

    bool IsWorkDone()
    {
        bool work_done = task_pool.isEmpty();
        for (std::unique_ptr<Worker<TaskResult>> const& w: workers)
        {
            if (work_done &= w->done(); !work_done)
            {
                break;
            }
        }
        return work_done;
    }

    template<class ResultHandler>
    void handleResults(ResultHandler result_handler)
    {
        while (!IsWorkDone())
        { }
        for (auto& worker: workers)
        {
            for (auto& result: worker->getResults())
            {
                result_handler(result);
            }
        }
    }

private:
    bool is_program_work = true;
    TaskPool<TaskResult> task_pool;
    std::vector<std::unique_ptr<Worker<TaskResult>>> workers;
};

#endif //MANDELBROT_CPP_THREADPOOL_H
