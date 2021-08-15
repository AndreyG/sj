#include "sj/job.h"

void Job::cancel_active_tasks()
{
    for (auto task : active_tasks_)
        task->cancel();
}

Job::~Job()
{
    assert(active_tasks_.empty());
}

void Job::add_active_task(ITask& task)
{
    active_tasks_.push_back(&task);
}

void Job::remove_active_task(ITask& task)
{
    assert(!task.in_process());
    auto it = std::ranges::find(active_tasks_, &task);
    assert(it != active_tasks_.end());
    active_tasks_.erase(it);
}

Job::Job(JobOwner* owner)
    : owner_(owner)
{
    owner->set_job(JobPtr(this));
}

void JobPtr::Deleter::operator()(Job* job) const
{
    std::coroutine_handle<Job>::from_promise(*job).destroy();
}

void JobPtr::cancel()
{
    job_->cancel_active_tasks();
}
