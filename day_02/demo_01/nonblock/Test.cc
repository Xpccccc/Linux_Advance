#include <iostream>

#include "NonBlock.hpp"

int main()
{
    char buff[1024];
    SetNonBlock(0);
    while (true)
    {
        ssize_t n = read(0, buff, sizeof(buff) - 1);
        if (n > 0)
        {
            buff[n] = 0;
            std::cout << "Echo # " << buff << std::endl;
        }
        else
        {
            // 问题：我怎么知道是IO条件不就绪还是读错误
            // 底层IO就绪和读错误是相同的返回值（-1？）
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                std::cout << "IO 没有就绪" << std::endl;
                continue;
            }
            else
            {
                std::cout << "read error ,errno : " << errno << std::endl;
            }
            sleep(1);
        }
    }

    return 0;
}