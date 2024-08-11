#include <iostream>
#include "EpollServer.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " serverport" << std::endl;
        exit(USAGE_ERROR);
    }

    uint16_t serverport = std::stoi(argv[1]);
    std::shared_ptr<EpollServer> selectserverpter = std::make_shared<EpollServer>(serverport);
    selectserverpter->Loop();
}