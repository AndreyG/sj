#include "sj/qt/qscheduler.h"
#include "sj/job_interface.h"

#include <QtCore/QPointer>
#include <QtCore/QCoreApplication>

#include <format>

namespace
{
    class Event : public QEvent
    {
    public:
        Event(std::coroutine_handle<> job, QObject* receiver, ITask& task)
            : QEvent(generated_type())
            , job_(job)
            , receiver_(receiver)
            , task_(task)
        {}

        void invoke()
        {
            if (receiver_.isNull() || task_.is_cancelled())
                return;

            job_();
        }

    private:
        static Type generated_type()
        {
            static int event_type = QEvent::registerEventType();
            return static_cast<QEvent::Type>(event_type);
        }

        std::coroutine_handle<> job_;
        QPointer<QObject> receiver_;
        ITask& task_;
    };
}

void QScheduler::post(std::coroutine_handle<> job, JobOwner* receiver, ITask& task)
{
    auto qojbect = dynamic_cast<QObject*>(receiver);
    if (!qojbect)
        throw std::runtime_error(std::format("QObject as receiver expected, but {} was gotten", typeid(*receiver).name()));
    QCoreApplication::postEvent(this, new Event{ job, qojbect, task });
}

bool QScheduler::event(QEvent* event)
{
    auto e = static_cast<Event*>(event);
    e->accept();
    e->invoke();
    return true;
}
