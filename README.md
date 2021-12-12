The repostitory consists of 2 libraries.
* `sj` is collection of primitives for launching async activities, this library doesn't have external dependencies except C++20 standard library,
* [`sj-qt`](https://github.com/AndreyG/sj/tree/master/qt-integration) contains 
   * class QScheduler -- Qt-specific implementation of [`Scheduler`](https://github.com/AndreyG/sj/blob/master/include/sj/scheduler.h#L5) interface;
   * function `IProgress* create_progress_dialog(...)` for creating progress dialog, interface `IProgress` extends `JobOwner`.

`sj-qt` depends on `sj` and QtWidgets.
  
## Example of usage
* CMakeLists.txt: `target_link_libraries(... sj-qt)`.
* Somewhere in `int main(...)`:
```
QScheduler scheduler;
current_thread_scheduler::set(&scheduler);
```
* Usage of job and future:
```
void calc_in_background_and_update(IProgress* progress, MyWidget* view)
{
    auto & job = co_await Job::GetCurrent;
    auto future = job.async([...] {
        return do_heavy_calculation();
    });
    auto result = co_await future;
    view->update_ui(std::move(result));
    progress->stop();
}

void MyWidget::calc_and_update()
{
    auto progress = create_progress_dialog("Activity Name", this);
    calc_in_background_and_update(progress, this);
    progress->start();
}
```
