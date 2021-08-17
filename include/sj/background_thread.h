#pragma once

#include "task.h"

#include <boost/intrusive/slist_hook.hpp>

#include <stop_token>
#include <thread>

class BackgroundThread
{
public:
    BackgroundThread();
    ~BackgroundThread();

private:
    class TasksQueue;

    class Task
        : public ITask
        , public boost::intrusive::slist_base_hook<>
    {
    public:
        enum class Status;

        std::atomic<Status> status;
        std::atomic_bool cancelled_after_start = false;

        void start()        override;
        void cancel()       override;
        bool is_cancelled() override;
        bool in_process()   override;

        void join();

        void invoke_user_action();

        virtual ~Task() = default;

        Task           (Task const&) = delete;
        Task           (Task &&)     = delete;
        Task& operator=(Task const&) = delete;
        Task& operator=(Task &&)     = delete;

    protected:
        Task(TasksQueue& queue)
            : queue_(queue)
        {}

    private:
        virtual void invoke_user_action_impl() = 0;

        TasksQueue& queue_;
    };

    void invoke_tasks(std::stop_token);

public:
    template<typename UserAction>
    class ConcreteTask final : public Task
    {
        UserAction user_action_;

    public:
        void invoke_user_action_impl() override
        {
            user_action_(*this);
        }

        ConcreteTask(UserAction&& user_action, TasksQueue& queue)
            : Task(queue)
            , user_action_(std::move(user_action))
        {}
    };

    template<typename UserAction>
    [[nodiscard]] ConcreteTask<UserAction> create_task_inplace(UserAction user_action)
    {
        return { std::move(user_action), *tasks_queue_ };
    }

private:
    std::unique_ptr<TasksQueue> tasks_queue_;
    std::jthread worker_;
};

extern BackgroundThread g_background_thread;
