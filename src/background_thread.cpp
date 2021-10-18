#include "sj/background_thread.h"

#include <boost/intrusive/list.hpp>

#include <cassert>
#include <condition_variable>
#include <functional>

BackgroundThread g_background_thread;

enum class BackgroundThread::Task::Status
{
    Queued,
    InProcess,
    Cancelled,
    Finished,
};

class BackgroundThread::TasksQueue
{
    std::mutex mutex_;
    std::condition_variable_any cv_;
    boost::intrusive::list<Task> queue_;

public:
    void  push (Task*);
    Task* pop  (std::stop_token const &);
    void  erase(Task*);
};

bool BackgroundThread::Task::is_cancelled()
{
    switch (status.load())
    {
    case Status::Cancelled: return true;
    case Status::InProcess:
    case Status::Finished:
        return cancelled_after_start;
    default:
#ifdef _MSC_VER
        __assume(false);
#else
        __builtin_unreachable();
#endif
    }
}

bool BackgroundThread::Task::in_process()
{
    return status == Status::InProcess;
}

void BackgroundThread::Task::invoke_user_action()
{
    invoke_user_action_impl();
    status = Status::Finished;
    status.notify_one();
}

void BackgroundThread::Task::join()
{
    status.wait(Status::InProcess);
#ifndef NDEBUG
    const auto status_value = status.load();
    assert(status_value == Status::Finished ||
           status_value == Status::Cancelled);
#endif
}

void BackgroundThread::Task::cancel()
{
    Status expected = Status::Queued;
    if (status.compare_exchange_strong(expected, /*desired:*/ Status::Cancelled))
    {
        queue_.erase(this);
    }
    else
    {
        assert(expected == Task::Status::InProcess ||
               expected == Task::Status::Finished);
        cancelled_after_start = true;
    }
}

void BackgroundThread::Task::start()
{
    queue_.push(this);
}

void BackgroundThread::TasksQueue::push(Task* task)
{
    task->status = Task::Status::Queued;
    std::lock_guard lock(mutex_);
    queue_.push_back(*task);
    cv_.notify_one();
}

BackgroundThread::Task* BackgroundThread::TasksQueue::pop(std::stop_token const & stop_token)
{
    std::unique_lock lock(mutex_);
    for (;;)
    {
        cv_.wait(lock, stop_token, [this] { return !queue_.empty(); });
        if (stop_token.stop_requested())
            return nullptr;

        for (auto it = queue_.begin(); it != queue_.end(); ++it)
        {
            Task& task = *it;
            Task::Status expected = Task::Status::Queued;
            if (task.status.compare_exchange_strong(expected, Task::Status::InProcess))
            {
                queue_.erase(it);
                return &task;
            }
            assert(expected == Task::Status::Cancelled);
        }
    }
}

void BackgroundThread::TasksQueue::erase(Task* task)
{
    assert(task->status == Task::Status::Cancelled);
    std::lock_guard lock(mutex_);
    queue_.erase(queue_.iterator_to(*task));
}

void BackgroundThread::invoke_tasks(std::stop_token stop_token)
{
    for (;;) {
        if (stop_token.stop_requested())
            break;

        Task* task = tasks_queue_->pop(stop_token);
        if (!task)
        {
            assert(stop_token.stop_requested());
            break;
        }
        assert(task->status == Task::Status::InProcess); // Queue postcondition #2
        task->invoke_user_action();
    }
}

BackgroundThread::BackgroundThread()
    : tasks_queue_(new TasksQueue)
    , worker_(std::bind(&BackgroundThread::invoke_tasks, this, std::placeholders::_1))
{}

BackgroundThread::~BackgroundThread()
{
}
