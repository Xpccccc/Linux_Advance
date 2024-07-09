#include <iostream>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include "Thread.hpp"

using namespace ThreadModule;

enum errorcode
{
    USAGE_ERROR = 1,
};

void Usage()
{
    printf("Usage : ./udp_client serverip serverport\n");
}

class ThreadData
{
public:
    ThreadData(int sock, struct sockaddr_in &server) : _sockfd(sock), _server(server) {}
    ~ThreadData() {}

public:
    int _sockfd;
    struct sockaddr_in _server;
};

void RecverRoute(ThreadData &td, std::string &threadname)
{
    while (true)
    {
        // 获取回应数据
        char buff[1024];
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        // ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen); // src_addr
        ssize_t n = recvfrom(td._sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&peer, &len);
        if (n > 0)
        {
            buff[n] = 0;
            std::cerr << threadname << " | " << buff << std::endl; // 方便重定位，分离发送窗口和接收窗口
        }
    }
}

void SenderRoute(ThreadData &td, std::string &threadname)
{
    pthread_detach(pthread_self());
    while (true)
    {
        std::string message;
        std::cout << threadname << " | Please Enter:# ";
        std::getline(std::cin, message);
        if (message == "QUIT")
        {
            std::cout <<threadname << " : 不玩了！" << std::endl;
            break;
        }
        // 发送数据
        // ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen); // dest_addr
        int n = sendto(td._sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&td._server, sizeof(td._server));
        if (n <= 0)
        {
            std::cout << "sendto error" << std::endl;
            break;
        }
    }
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

    // 1. 创建socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "socket error" << std::endl;
    }
    // 2. client要不要bind？一定要，client也要有自己的IP和PORT。要不要显式[和server一样用bind函数]的bind？不能！不建议！！
    // a. 如何bind呢？udp client首次发送数据的时候，OS会自己自动随机的给client进行bind --- 为什么？防止client port冲突。要bind，必然要和port关联！
    // b. 什么时候bind呢？首次发送数据的时候
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(serverport); // 主机序列转网络序列 -- 网络序列是大端
    // a.字符串序列点分十进制IP地址 转换为 4字节IP
    // b.主机序列转网络序列
    // in_addr_t inet_addr(const char *cp);
    server.sin_addr.s_addr = inet_addr(serverip.c_str()); // sin_addr 是一个结构体，里面的成员是s_addr
    // 直接通信即可，已经自动绑定

    // 创建两个线程，一个收消息，一个发消息
    ThreadData td(sockfd, server);
    auto boundRecv = std::bind(RecverRoute, td, std::placeholders::_1);
    auto boundSend = std::bind(SenderRoute, td, std::placeholders::_1);
    Thread recver(boundRecv, "recver");
    recver.Start();
    Thread sender(boundSend, "sender");
    sender.Start();

    // // udp是全双工的
    // // 下面代码只能半双工，不能不能同时收发
    // while (true)
    // {
    //     std::cout << "Please Enter:# ";
    //     std::getline(std::cin, message);
    //     // 发送数据
    //     // ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen); // dest_addr
    //     sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&server, sizeof(server));

    //     // 获取回应数据
    //     char buff[1024];
    //     struct sockaddr_in peer;
    //     socklen_t len = sizeof(peer);
    //     // ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen); // src_addr
    //     ssize_t n = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&peer, &len);
    //     if (n > 0)
    //     {
    //         buff[n] = 0;
    //         std::cout << "Server Echo:# " << buff << std::endl;
    //     }
    // }

    sender.Join();
    recver.Join();
    return 0;
}