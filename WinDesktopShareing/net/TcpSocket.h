#ifndef XOP_TCPSOCKET_H
#define XOP_TCPSOCKET_H

#include <cstdint>
#include <string>
#include "Socket.h"

namespace xop {
    class TcpSocket {
    public:
        TcpSocket(SOCKET sockfd = -1);
        virtual ~TcpSocket();

        SOCKET  Create();
        bool    Bind();
        bool    Listen();
        SOCKET  Accept();
        bool    Connect();
    };
}

#endif
