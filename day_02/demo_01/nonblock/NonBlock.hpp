#pragma once

#include <unistd.h>
#include <fcntl.h>
void SetNonBlock(int fd)
{
    int f1 = ::fcntl(fd, F_GETFL); // 获取标记位
    if (f1 < 0)
        return;
    ::fcntl(fd, F_SETFL, f1 | O_NONBLOCK);
}