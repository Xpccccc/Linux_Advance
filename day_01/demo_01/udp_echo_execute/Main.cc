#include <iostream>
#include <memory>
#include <stdio.h>
#include <vector>
#include "UdpServer.hpp"

void Usage()
{
    // printf("./udp_server serverip serverport\n");
    printf("Usage : ./udp_server serverport\n"); // ip 已经设置为0
}

std::string OnMessage(std::string request)
{
    return request + " got you!";
}

bool CheckCommand(std::string command)
{
    vector<string> cmd = {
        "kill",
        "rm",
        "dd",
        "top",
        "reboot",
        "shutdown",
        "mv",
        "cp",
        "halt",
        "unlink",
        "exit",
        "chmod"};
    for (auto &e : cmd)
    {
        if (command.find(e) != std::string::npos)
            return false;
    }
    return true;
}

std::string OnCommand(std::string command)
{
    if (!CheckCommand(command))
    {
        return "bad man!";
    }
    // FILE *popen(const char *command, const char *type);
    FILE *pp = popen(command.c_str(), "r");
    if (!pp)
    {
        return "popen error!";
    }

    std::string response;
    char buff[1024];
    while (true)
    {
        // char *fgets(char *s, int size, FILE *stream);
        char *s = fgets(buff, sizeof(buff), pp);
        if (!s)
            break;
        else
            response += buff;
    }
    pclose(pp);
    return response.empty() ? "not command" : response;
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
    std::unique_ptr<UdpServer> usvr = std::make_unique<UdpServer>(OnCommand, port);
    usvr->InitServer();
    usvr->Start();

    return 0;
}