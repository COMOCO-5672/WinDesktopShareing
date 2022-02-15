#ifndef XOP_EVENT_LOOP_H
#define XOP_EVENT_LOOP_H

#include <stdint.h>
#include <memory>
#include <thread>
#include <functional>

namespace xop {
    class EventLoop {
    public:
        EventLoop(const EventLoop &) = delete;
        EventLoop &operator = (const EventLoop &) = delete;
        EventLoop(uint32_t num_threads = 1);
        virtual ~EventLoop();
        std::shared_ptr<TaskSc>

    };
}
#endif
