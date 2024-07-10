#include <iostream>
#include <memory>
#include "TcpServer.hpp"

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

    uint16_t serverport = std::stoi(argv[1]);
    std::unique_ptr<TcpServer> tsvr = std::make_unique<TcpServer>(serverport);
    tsvr->InitServer();
    tsvr->Start();

    return 0;
}