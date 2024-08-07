#include <iostream>
#include "PollServer.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " serverport" << std::endl;
        exit(USAGE_ERROR);
    }

    uint16_t serverport = std::stoi(argv[1]);

    std::shared_ptr<PollServer> selectserverpter = std::make_shared<PollServer>(serverport);
    selectserverpter->Loop();
}