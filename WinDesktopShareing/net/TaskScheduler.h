#ifndef XOP_TASK_SCHEDULER_H
#define XOP_TASK_SCHEDULER_H

#include <functional>
#include "Timer.h"

namespace xop {
    typedef std::function<void(void)> TriggerEvent;

    class TaskScheduler {
    public:
        TaskScheduler(int id = 1);
        virtual ~TaskScheduler();

        void Start();
        void Stop();
        TimerId AddTimer(TimerEvent timer_event,uint32_t msec);
        void RemoveTimer(TimerId timer_id);
        bool AddTriggerEvent(TriggerEvent callback);

        virtual void UpdateChannel();

    };

}

#endif
