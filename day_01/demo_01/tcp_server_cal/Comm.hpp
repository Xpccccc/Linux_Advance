#pragma once
#include "InetAddr.hpp"

enum errorcode
{
    CREATE_ERROR = 1,
    BIND_ERROR,
    LISTEN_ERROR,
    SEND_ERROR,
    RECV_ERROR,
    CONNECT_ERROR,
    FORK_ERROR,
    USAGE_ERROR
};

#define CONV(ADDR) ((struct sockaddr *)ADDR)

std::string CombineIpAndPort(InetAddr addr)
{
    return "[" + addr.Ip() + ":" + std::to_string(addr.Port()) + "] ";
}