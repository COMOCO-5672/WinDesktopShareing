﻿#include "SocketUtil.h"

using namespace xop;

bool SocketUtil::Bind(SOCKET sockfd, std::string ip, uint16_t port)
{
    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    if (::bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        return false;
    }
    return true;
}

void SocketUtil::SetNonBlock(SOCKET fd)
{
#if defined(__linux) || defined(__linux__)
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    unsigned long on = 1;
    ioctlsocket(fd, FIONBIO, &on);
#endif

}

void SocketUtil::SetBlock(SOCKET fd, int write_timeout)
{
#if defined(__linux) || defined(__linux__) 
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags&(~O_NONBLOCK));
#elif defined(WIN32) || defined(_WIN32)
    unsigned long on = 0;
    ioctlsocket(fd, FIONBIO, &on);
#else
#endif
    if (write_timeout > 0) {
#ifdef SO_SNDTIMEO
#if defined(__linux) || defined(__linux__) 
        struct timeval tv = { write_timeout / 1000, (write_timeout % 1000) * 1000 };
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof tv);
#elif defined(WIN32) || defined(_WIN32)
        unsigned long ms = (unsigned long)write_timeout;
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&ms, sizeof(unsigned long));
#else
#endif		
#endif
    }
}

void SocketUtil::SetReuseAddr(SOCKET fd)
{
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));
}

void SocketUtil::SetReusePort(SOCKET sockfd)
{
#ifdef SO_REUSEPORT
    int on = 1;
    int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
#endif
}

void SocketUtil::SetNoDelay(SOCKET sockfd)
{
#ifdef TCP_NODELAY
    int on = 1;
    int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
#endif
}

void SocketUtil::SetKeepAlive(SOCKET sockfd)
{
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));
}

void SocketUtil::SetNoSigpipe(SOCKET sockfd)
{
#ifdef SO_NOSIGPIPE
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, (char *)&on, sizeof(on));
#endif
}


void SocketUtil::SetSendBufSize(SOCKET sockfd, int size)
{
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));
}

void SocketUtil::SetRecvBufSize(SOCKET sockfd, int size)
{
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size));
}

std::string SocketUtil::GetPeerIp(SOCKET sockfd)
{
    struct sockaddr_in addr = { 0 };
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addrlen) == 0) {
        return inet_ntoa(addr.sin_addr);
    }
    return "0.0.0.0";
}

std::string SocketUtil::GetSocketIp(SOCKET sockfd)
{
    struct sockaddr_in addr = { 0 };
    char str[INET_ADDRSTRLEN] = "127.0.0.1";
    if (GetSocketAddr(sockfd, &addr) == 0) {
        inet_ntop(AF_INET, &addr.sin_addr, str, sizeof(str));
    }
    return str;
}

