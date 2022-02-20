#ifndef XOP_EVENT_LOOP_H
#define XOP_EVENT_LOOP_H

#include <stdint.h>
#include <memory>
#include <thread>
#include <functional>

#include "SelectTaskScheduler.h"
#include "Timer.h"

#define TASK_SCHEDULER_PRIORITY_LOW       0
#define TASK_SCHEDULER_PRIORITY_NORMAL    1
#define TASK_SCHEDULER_PRIORITYO_HIGH     2 
#define TASK_SCHEDULER_PRIORITY_HIGHEST   3
#define TASK_SCHEDULER_PRIORITY_REALTIME  4

namespace xop {
    class EventLoop {
    public:
        EventLoop(const EventLoop &) = delete;
        EventLoop &operator = (const EventLoop &) = delete;
        EventLoop(uint32_t num_threads = 1);
        virtual ~EventLoop();

        std::shared_ptr<TaskScheduler> GetTaskScheduler();

        bool AddTriggerEvent(TriggerEvent callback);
        TimerId AddTimer(TimerEvent timer_event,uint32_t msec);
        void RemoveTimer(TimerId timer_id);
        void UpdateChannel(ChannelPtr channel);
        void RemoveChannel(ChannelPtr &channel);

        void Loop();
        void Quit();
    private:
        std::mutex mutex_;
        uint32_t num_threads = 1;
        uint32_t index_;
        std::vector<std::shared_ptr<TaskScheduler>> task_schedulers_;
        std::vector<std::shared_ptr<std::thread>> threads_;
    };
}
#endif
