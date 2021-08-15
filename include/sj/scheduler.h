#pragma once

#include <coroutine>

struct Scheduler
{
    virtual void post(std::coroutine_handle<> coro, class JobOwner*, class ITask&) = 0;

protected:
    ~Scheduler() = default;
};

namespace current_thread_scheduler
{
    Scheduler* get();
    void set(Scheduler*);
};
