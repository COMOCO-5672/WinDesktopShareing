﻿#ifndef XOP_NET_INTERFACE_H
#define XOP_NET_INTERFACE_H

#include <string>

namespace xop
{
    class NetInterface
    {
    public:
        static std::string GetLocalIpAddress();
    };
}

#endif
