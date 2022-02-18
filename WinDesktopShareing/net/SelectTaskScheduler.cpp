#include "SelectTaskScheduler.h"
#include "Timer.h"
#include <forward_list>

using namespace xop;

SelectTaskScheduler::SelectTaskScheduler(int id) :TaskScheduler(id)
{
    FD_ZERO(&fd_read_backup_);
    FD_ZERO(&fd_write_backup_);
    FD_ZERO(&fd_exp_backup_);

    this->UpdateChannel(wakeup_channel_);
}

SelectTaskScheduler::~SelectTaskScheduler()
{

}