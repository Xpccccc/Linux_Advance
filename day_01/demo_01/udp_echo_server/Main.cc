#include <iostream>
#include <memory>
#include "UdpServer.hpp"

void Usage()
{
    printf("./udp_server serverip serverport\n");
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        Usage();
        exit(USAGE_ERROR);
    }
    std::string ip = argv[1];
    uint16_t port = std::stoi(argv[2]);
    std::unique_ptr<UdpServer> usvr = std::make_unique<UdpServer>(ip, port);
    usvr->InitServer();
    usvr->Start();

    return 0;
}