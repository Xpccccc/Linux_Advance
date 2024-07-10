#include <iostream>

#include <strings.h>
#include <string.h>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Comm.hpp"
#include "Log.hpp"

void Usage()
{
    printf("Usage : ./udp_client serverip serverport\n");
}
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        Usage();
        exit(USAGE_ERROR);
    }
    std::string serverip = argv[1];
    uint16_t serverport = std::stoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        LOG(FATAL, "create sockfd error, error code : %d, error string : %s", errno, strerror(errno));
        exit(CREATE_ERROR);
    }
    LOG(INFO, "create sockfd success");

    struct sockaddr_in local;
    bzero(&local, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(serverport);
    local.sin_addr.s_addr = inet_addr(serverip.c_str());

    // 发起连接
    int n = ::connect(sockfd, CONV(&local), sizeof(local));
    if (n < 0)
    {
        LOG(WARNING, "create connect error, error code : %d, error string : %s", errno, strerror(errno));
        exit(CONNECT_ERROR);
    }
    LOG(INFO, "create connect success");

    // 发收数据
    while (true)
    {
        std::cout << "Please Enter# ";
        // 发送数据
        std::string message;
        std::getline(cin, message);

        int ret = ::send(sockfd, message.c_str(), message.size(), 0);
        if (ret < 0)
        {
            LOG(FATAL, "send message error, error code : %d, error string : %s", errno, strerror(errno));
            exit(SEND_ERROR);
        }

        char buff[1024];
        // 接收数据
        int m = ::recv(sockfd, buff, sizeof(buff) - 1, 0);
        if (m > 0)
        {
            buff[m] = 0;
            std::cout << "Server Echo$ " << buff << std::endl;
        }
    }
}