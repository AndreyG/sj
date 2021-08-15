#pragma once

#include "task.h"

#include <list>
#include <memory>

class JobOwner;

struct Job
{
    void cancel_active_tasks();

    void add_active_task(ITask&);
    void remove_active_task(ITask&);

    Job(JobOwner*);
    ~Job();

    static constexpr struct get_current_job_tag {} GetCurrent;

    JobOwner* owner() const { return owner_; }

private:
    JobOwner* owner_;
    std::list<ITask*> active_tasks_;
};

class JobPtr
{
    struct Deleter
    {
        void operator() (Job*) const;
    };

    std::unique_ptr<Job, Deleter> job_;

public:
    explicit JobPtr(Job* promise)
        : job_(promise)
    {}

    void cancel();
};

class JobOwner
{
public:
    virtual void set_job(JobPtr) = 0;
    virtual void on_cancelled_task_finished() = 0;

protected:
    ~JobOwner() = default;
};

