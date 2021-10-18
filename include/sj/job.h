#pragma once

#include "job_interface.h"
#include "background_thread.h"

#include "detail/Future.h"

namespace detail
{
    class Promise : public Job
    {
    public:
        // coroutine promise_type interface

        static std::suspend_never   initial_suspend() noexcept { return {}; }
        static std::suspend_always  final_suspend()   noexcept { return {}; }

        static void return_void() noexcept {}

        static void unhandled_exception()
        {
            throw;
        }

        static void get_return_object() noexcept {}

        // implementation of `co_await Job::GetCurrent`

        auto await_transform(get_current_job_tag)
        {
            struct job_getter : std::suspend_never
            {
                Promise& promise;

                Promise& await_resume() const noexcept
                {
                    return promise;
                }
            };

            return job_getter{ {}, *this };
        }

        // implementation of `async(...)`

        template<std::invocable F>
        Future<F> async(F f, BackgroundThread& bg_thread = g_background_thread)
        {
            return { std::move(f), bg_thread };
        }

        template<std::invocable F>
        typename Future<F>::Awaiter await_transform(Future<F> tag)
        {
            return { *this, tag.bg_thread, std::move(tag.f) };
        }

        //
        Promise(JobOwner* owner, auto...)
            : Job(owner)
        {
        }
    };
}

template<std::derived_from<JobOwner> Owner, typename... Args>
struct std::coroutine_traits<void, Owner*, Args...>
{
    using promise_type = detail::Promise;
};
