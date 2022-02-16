#ifndef XOP_ACCEPTOR_H
#define XOP_ACCEPTOR_H

#include <functional>
#include <memory>
#include <mutex>
#include "Channel.h"

namespace xop {
    typedef std::function<void(SOCKET)>NewConnectionCallback;

    class EventLoop;

    class Acceptor {
    public:
        Acceptor(EventLoop *eventLoop);
        virtual ~Acceptor();

        void SetNewConnectionCallback(const NewConnectionCallback &cb)
        {
            
        }

    private:
        void OnAccept();

        EventLoop *event_loop_{ nullptr };
        std::mutex mutex_;
        std::unique_ptr<>;
        ChannelPtr channel_ptr_;
    };
}

#endif
