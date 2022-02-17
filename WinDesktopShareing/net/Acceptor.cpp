#include "Acceptor.h"

using namespace xop;

Acceptor::Acceptor(EventLoop* eventLoop)
    : event_loop_(eventLoop)
    , tcp_socket_(new TcpSocket)
{
    
}

Acceptor::~Acceptor()
{
    
}

int Acceptor::Listen(std::string ip, uint16_t port)
{
    std::lock_guard<std::mutex> locker(mutex_);

    if(tcp_socket_->GetSocket() > 0)
    {
        tcp_socket_->Close();
    }

    SOCKET sockfd = tcp_socket_->Create();
    channel_ptr_.reset(new Channel);


}

void Acceptor::Close()
{
    
}




