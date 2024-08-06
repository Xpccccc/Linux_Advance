#include <iostream>
#include "SelectServer.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " serverport" << std::endl;
        exit(USAGE_ERROR);
    }

    uint16_t serverport = std::stoi(argv[1]);

    std::shared_ptr<SelectServer> selectserverpter = std::make_shared<SelectServer>(serverport);
    selectserverpter->Loop();
}