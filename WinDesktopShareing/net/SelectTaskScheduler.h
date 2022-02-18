#ifndef XOP_SELECT_TASK_SCHEDULER_H
#define XOP_SELECT_TASK_SCHEDULER_H

#include "TaskScheduler.h"

namespace xop {
    class SelectTaskScheduler :public TaskScheduler {
    public:
        SelectTaskScheduler(int id = 0);
        virtual ~SelectTaskScheduler();

        void UpdateChannel(ChannelPtr channel) override;
        void RemoveChannel(ChannelPtr& channel) override;
        bool HandleEvent(int timeout) override;

    private:
        fd_set fd_read_backup_;
        fd_set fd_write_backup_;
        fd_set fd_exp_backup_;
        SOCKET maxfd_ = 0;

        bool is_fd_read_reset_ = false;
        bool is_fd_write_reset_ = false;
        bool is_fd_exp_reset_ = false;

        std::mutex mutex_;
        std::unordered_map<SOCKET, ChannelPtr> channels_;
    };
}

#endif
