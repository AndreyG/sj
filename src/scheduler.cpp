#include "sj/scheduler.h"

#include <cassert>

namespace current_thread_scheduler
{
    thread_local Scheduler* instance = nullptr;

    Scheduler* get()
    {
        assert(instance);
        return instance;
    }

    void set(Scheduler* scheduler)
    {
        assert(!instance);
        instance = scheduler;
    }
}
