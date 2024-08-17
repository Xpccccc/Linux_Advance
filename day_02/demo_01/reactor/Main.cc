#include <iostream>
#include <string>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>

#include "Comm.hpp"
#include "Reactor.hpp"

#include "Connection.hpp"
#include "Listener.hpp"
#include "PackageParse.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage : ./reactor port" << std::endl;
        exit(USAGE_ERROR);
    }
    uint16_t serverport = std::stoi(argv[1]);

    std::unique_ptr<Reactor> svr = std::make_unique<Reactor>(); // 主服务

    IOService io(PackageParse::Parse); // 这里回调函数可以对报文进行解析

    Listener listener(serverport, io); // 负责连接模块

    // 注册进入
    // EPOLLET是添加到uint32_t events中的，不是options
    svr->AddConnection(listener.SockFd(), EPOLLIN | EPOLLET, std::bind(&Listener::Accepter, &listener, std::placeholders::_1), nullptr, nullptr);

    svr->Despatcher();

    return 0;
}