#pragma once

class ITask
{
protected:
    ~ITask() = default;

public:
    virtual void start()        = 0;
    virtual void cancel()       = 0;
    virtual bool is_cancelled() = 0;
    virtual bool in_process()   = 0;
};
