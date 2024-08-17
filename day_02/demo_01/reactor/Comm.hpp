#pragma once
#include <unistd.h>
#include <fcntl.h>

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
    USAGE_ERROR,
    EPOLL_CREATE_ERROR
};

#define CONV(ADDR) ((struct sockaddr *)ADDR)

std::string CombineIpAndPort(InetAddr addr)
{
    return "[" + addr.Ip() + ":" + std::to_string(addr.Port()) + "] ";
}


void SetNonBlock(int fd)
{
    int f1 = ::fcntl(fd, F_GETFL); // 获取标记位
    if (f1 < 0)
        return;
    ::fcntl(fd, F_SETFL, f1 | O_NONBLOCK);
}