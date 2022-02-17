#ifndef XOP_TCP_CONNECTION_H
#define XOP_TCP_CONNECTION_H

#include "TaskScheduler.h"
#include "BufferReader.h"
#include "BufferWriter.h"
#include "SocketUtil.h"
#include "Channel.h"

namespace xop
{
    class TcpConnection : public std::enable_shared_from_this<TcpConnection>
    {
    public:
        using Ptr = std::shared_ptr<TcpConnection>;
        using DisconnectCallback = std::function<void(std::shared_ptr<TcpConnection> conn)>;
        using CloseCallback = std::function<void(std::shared_ptr<TcpConnection> conn)>;
        using ReadCallback = std::function<bool(std::shared_ptr<TcpConnection> conn, xop::BufferReader &buffer)>;

        TcpConnection();
    };
}

#endif
