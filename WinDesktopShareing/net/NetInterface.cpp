
#include "NetInterface.h"
#include "Socket.h"

using namespace xop;

std::string NetInterface::GetLocalIpAddress()
{
#if defined(__linux) || defined(__linux__)

#elif defined(WIN32) || defined(_WIN32)
    PIP_ADAPTER_INFO pIpAdapterInfo = new _IP_ADAPTER_INFO();
    unsigned long size = sizeof(IP_ADAPTER_INFO);

    int ret = GetAdaptersInfo(pIpAdapterInfo, &size);
    if (ret == ERROR_BUFFER_OVERFLOW) {
        delete pIpAdapterInfo;
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[size];
        ret = GetAdaptersInfo(pIpAdapterInfo, &size);
    }

    if (ret != ERROR_SUCCESS) {
        delete pIpAdapterInfo;
        return "0.0.0.0";
    }

    while (pIpAdapterInfo) {
        IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
        while (pIpAddrString) {
            if (strcmp(pIpAddrString->IpAddress.String, "127.0.0.1") != 0
                && strcmp(pIpAddrString->IpAddress.String, "0.0.0.0") != 0) {
                std::string ip(pIpAddrString->IpAddress.String);
                return ip;
            }
            pIpAddrString = pIpAddrString->Next;
        }
        pIpAdapterInfo = pIpAdapterInfo->Next;
    }

    delete pIpAdapterInfo;
    return "0.0.0.0";

#endif

}
