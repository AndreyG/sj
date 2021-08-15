#include "sj/qt/progress.h"

#include <QtWidgets/QProgressDialog>

namespace
{
    class ProgressDialog final : public QProgressDialog, public IProgress
    {
    public:
        ProgressDialog(QString const& title, QWidget* parent)
            : QProgressDialog(title, "Cancel", 0, 0, parent)
        {
            setWindowModality(Qt::WindowModal);
        }

        // JobOwner
        void set_job(JobPtr)                override;
        void on_cancelled_task_finished()   override;

        // IProgress
        void start() override { exec();  }
        void stop()  override { close(); }
    };

    void ProgressDialog::set_job(JobPtr job)
    {
        connect(this, &QProgressDialog::canceled, [job = std::move(job), this]() mutable
        {
            job.cancel();
            close();
        });
    }

    void ProgressDialog::on_cancelled_task_finished()
    {
        deleteLater();
    }
}

IProgress* create_progress_dialog(QString const& title, QWidget* parent)
{
    return new ProgressDialog(title, parent);
}
