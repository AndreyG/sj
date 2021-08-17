#pragma once

#include "job_interface.h"
#include "scheduler.h"

#include <variant>

namespace detail
{
    template<typename F>
    struct Future : std::suspend_always
    {
        using Result = std::invoke_result_t<F>;

        struct invoker
        {
            Future& future;
            F f;
            Scheduler* scheduler = current_thread_scheduler::get();

            void operator() (ITask& task)
            {
                try
                {
                    future.result_ = f();
                }
                catch (...)
                {
                    future.result_ = std::current_exception();
                }
                auto owner = future.job_.owner();
                if (task.is_cancelled())
                    owner->on_cancelled_task_finished();
                else
                    scheduler->post(std::coroutine_handle<Job>::from_promise(future.job_), owner, task);
            }
        };

        void await_suspend(std::coroutine_handle<>)
        {
            task_.start();
        }

        Result await_resume()
        {
            if (auto err = std::get_if<std::exception_ptr>(&result_))
                std::rethrow_exception(*err);

            return std::move(std::get<Result>(result_));
        }

        Future(Job& job, BackgroundThread& bg_thread, F f)
            : job_(job)
            , task_(bg_thread.create_task_inplace(invoker(*this, std::move(f))))
        {
            job.add_active_task(task_);
        }

        ~Future()
        {
            task_.join();
            job_.remove_active_task(task_);
        }

    private:
        Job& job_;
        std::variant<std::exception_ptr, Result> result_;
        BackgroundThread::ConcreteTask<invoker> task_;
    };
}
