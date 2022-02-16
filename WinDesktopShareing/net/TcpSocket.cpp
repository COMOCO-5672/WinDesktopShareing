
#include "TcpSocket.h"
#include "Socket.h"

using namespace xop;

TcpSocket::TcpSocket(SOCKET sockfd) :sockfd_(sockfd)
{

}

TcpSocket::~TcpSocket()
{

}

SOCKET TcpSocket::Create()
{
    sockfd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    return sockfd_;
}

bool TcpSocket::Bind(std::string ip, uint16_t port)
{
    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    if (::bind(sockfd_, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        return false;
    }
    return true;
}



