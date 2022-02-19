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
    if (!is_closed_) {
        mutex_.lock();
        writer_buffer_->Append(data, size);
        mutex_.unlock();

        this->HandleWrite();
    }
}

void TcpConnection::Send(std::shared_ptr<char> data, uint32_t size)
{
    if (!is_closed_) {
        mutex_.lock();
        writer_buffer_->Append(data, size);
        mutex_.unlock();

        this->HandleWrite();
    }
}

void TcpConnection::Disconnect()
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto conn = shared_from_this();
    task_scheduler_->AddTriggerEvent([conn]() {
        conn->Close();
    });
}

void TcpConnection::HandleRead()
{
    {
        std::lock_guard<std::mutex> locker(mutex_);
        if (is_closed_) {
            return;
        }

        int ret = reader_buffer_->Read(channel_->GetSocket());
        if (ret <= 0) {
            this->Close();
            return;
        }
    }

    if (read_cb_) {
        bool ret = read_cb_(shared_from_this(), *reader_buffer_);
        if (ret == false) {
            std::lock_guard<std::mutex> locker(mutex_);
            this->Close();
        }
    }
}

void TcpConnection::HandleWrite()
{
    if (is_closed_) {
        return;
    }

    if (!mutex_.try_lock()) {
        return;
    }

    int ret = 0;
    bool empty = false;
    do {
        ret = writer_buffer_->Send(channel_->GetSocket());
        if (ret < 0) {
            this->Close();
            mutex_.unlock();
            return;
        }
        empty = writer_buffer_->IsEmpty();

    } while (0);

    if (empty) {
        if (channel_->IsWriting()) {
            channel_->DisableWriting();
            task_scheduler_->UpdateChannel(channel_);
        }
    }
    else if (!channel_->IsWriting()) {
        channel_->EnableWriting();
        task_scheduler_->UpdateChannel(channel_);
    }
    mutex_.unlock();
}

void TcpConnection::Close()
{
    if (!is_closed_) {
        is_closed_ = true;
        task_scheduler_->RemoveChannel(channel_);

        if (close_cb_)
            close_cb_(shared_from_this());

        if (disconnect_cb_)
            disconnect_cb_(shared_from_this());
    }
}

void TcpConnection::HandleClose()
{
    std::lock_guard<std::mutex> locker(mutex_);
    this->Close();
}


void TcpConnection::HandleError()
{
    std::lock_guard<std::mutex> locker(mutex_);
    this->Close();
}


