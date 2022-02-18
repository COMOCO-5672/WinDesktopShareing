#include "TcpConnection.h"
#include "SocketUtil.h"

using namespace xop;

TcpConnection::TcpConnection(TaskScheduler* task_scheduler, SOCKET sockfd)
    : task_scheduler_(task_scheduler)
    , reader_buffer_(new BufferReader)
    , writer_buffer_(new BufferWriter(500))
    , channel_(new Channel(sockfd))
{
    is_closed_ = false;

    channel_->SetReadCallback([this] {
        this->HandleRead();
    });
    channel_->SetCloseCallback([this] {
        this->HandleClose();
    });
    channel_->SetWriteCallback([this] {
        this->HandleWrite();
    });
    channel_->SetErrorCallback([this] {
        this->HandleError();
    });

    SocketUtil::SetNonBlock(sockfd);
    SocketUtil::SetSendBufSize(sockfd, 100 * 1024);
    SocketUtil::SetKeepAlive(sockfd);

    channel_->EnableReading();
    task_scheduler->UpdateChannel(channel_);
}

TcpConnection::~TcpConnection()
{
    SOCKET fd = channel_->GetSocket();
    if (fd > 0)
        SocketUtil::Close(fd);
}

void TcpConnection::Send(const char* data, uint32_t size)
{
    
}

void TcpConnection::Send(std::shared_ptr<char> data, uint32_t size)
{
    
}


