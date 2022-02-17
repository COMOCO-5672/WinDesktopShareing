#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include <cstdint>

using namespace xop;

TcpServer::TcpServer(EventLoop* event_loop)
    :event_loop_(event_loop)
    , port_(0)
    , acceptor_(new Acceptor(event_loop_))
    , is_started_(false)
{
    acceptor_->SetNewConnectionCallback([this](SOCKET sockfd) {
        TcpConnection::Ptr conn = this->OnConnect(sockfd);
        if (conn) {
            this->AddConnection(sockfd, conn);
        }
        });
}

