#pragma once

#include "sj/task.h"
#include "sj/scheduler.h"

#include <QtCore/QObject>

class QScheduler : QObject, public Scheduler
{
public:
    void post(std::coroutine_handle<> job, JobOwner* receiver, ITask& task) override;

    bool event(QEvent* event) override;
};
