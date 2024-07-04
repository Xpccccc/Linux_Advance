#include <iostream>
#include <memory>

#include "UdpServer.hpp"

void Usage()
{
    // printf("./udp_server serverip serverport\n");
    printf("Usage : ./udp_server serverport\n"); // ip 已经设置为0
}

int main(int argc, char *argv[])
{
    // if (argc != 3)
    if (argc != 2)
    {
        Usage();
        exit(USAGE_ERROR);
    }
    // std::string ip = argv[1];
    // uint16_t port = std::stoi(argv[2]);
    uint16_t port = std::stoi(argv[1]);
    // std::unique_ptr<UdpServer> usvr = std::make_unique<UdpServer>(ip, port);
    std::unique_ptr<UdpServer> usvr = std::make_unique<UdpServer>(port);
    usvr->InitServer();
    usvr->Start();

    return 0;
}