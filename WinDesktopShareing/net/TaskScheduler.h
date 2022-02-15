#ifndef XOP_TASK_SCHEDULER_H
#define XOP_TASK_SCHEDULER_H

#include <functional>

namespace xop {
    typedef std::function<void(void)> TriggerEvent;

    class TaskScheduler {
    public:
        TaskScheduler(int id = 1);
        virtual ~TaskScheduler();

        void Start();
        void Stop();
        Time
    };

}

#endif
