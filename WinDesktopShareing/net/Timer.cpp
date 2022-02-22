#include "Timer.h"
#include <iostream>

using namespace xop;
using namespace std;
using namespace std::chrono;

TimerId TimerQueue::AddTimer(const TimerEvent& event, uint32_t msec)
{
    std::lock_guard<std::mutex> locker(mutex_);

    int64_t timeout = GetTimeNow();
    TimerId timer_id = ++last_timer_id_;

    auto timer = make_shared<Timer>(event, msec);
    timer->SetNextTimeout(timeout);
    timers_.emplace(timer_id, timer);
    events_.emplace(std::pair<int64_t, TimerId>(timeout + msec, timer_id), std::move(timer));
    return timer_id;
}

void TimerQueue::RemoveTimer(TimerId timer_id)
{
    std::lock_guard<std::mutex> locker(mutex_);

    auto iter = timers_.find(timer_id);
    if (iter != timers_.end()) {
        int64_t timeout = iter->second->getNextTimeout();
        //events_.erase(std::pair<int64_t, TimerId>(timeout, timer_id));
        timers_.erase(timer_id);
    }
}

int64_t TimerQueue::GetTimeNow()
{
    auto time_point = steady_clock::now();
    return duration_cast<microseconds>(time_point.time_since_epoch()).count();
}

int64_t TimerQueue::GetTimeRemaining()
{
    std::lock_guard<std::mutex> locker(mutex_);

    if (timers_.empty())
        return -1;

    int64_t msec = events_.begin()->first.first - GetTimeNow();
    if (msec < 0)
        msec = 0;

    return msec;
}

void TimerQueue::HandleTimerEvent()
{
    if (!timers_.empty()) {
        std::lock_guard<std::mutex> locker(mutex_);
        int64_t time_point = GetTimeNow();
        while (!timers_.empty() && events_.begin()->first.first <= time_point) {
            TimerId timer_id = events_.begin()->first.second;
            bool flag = events_.begin()->second->event_callback();
            if (flag == true) {
                events_.begin()->second->SetNextTimeout(time_point);
                auto timerPtr = std::move(events_.begin()->second);
                events_.erase(events_.begin());
                events_.emplace(std::pair<int64_t, TimerId>(timerPtr->getNextTimeout(), timer_id), timerPtr);
            } else {
                events_.erase(events_.begin());
                timers_.erase(timer_id);
            }
        }
    }
}


