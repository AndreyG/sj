#pragma once

#include "sj/job_interface.h"

class IProgress : public JobOwner
{
protected:
    ~IProgress() = default;

public:
    virtual void start() = 0;
    virtual void stop()  = 0;
};

IProgress* create_progress_dialog(class QString const& title, class QWidget* parent);
