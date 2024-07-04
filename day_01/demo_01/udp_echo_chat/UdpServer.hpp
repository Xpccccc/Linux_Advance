#pragma once

#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string.h>
#include <error.h>
#include <pthread.h>
#include <functional>

using namespace std;

#include "Log.hpp"
#include "InetAddr.hpp"
#include "Threadpool.hpp"

const int Defaultfd = -1;

enum errorcode
{
    CREATE_ERROR = 1,
    BIND_ERROR,
    USAGE_ERROR
};

using task_t = function<void()>;

class UdpServer
{
public:
    // UdpServer(std::string ip, uint16_t port) : _sockfd(Defaultfd), _ip(ip), _port(port), _isrunning(false)
    UdpServer(uint16_t port) : _sockfd(Defaultfd), _port(port), _isrunning(false)
    {
        pthread_mutex_init(&_mutex, nullptr);
    }
    void InitServer()
    {
        // 1. 创建udp套接字 -- 必须要做的
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0); // udp通信
        if (_sockfd < 0)
        {
            LOG(FATAL, "socke create error ! error string %s error %d", strerror(errno), errno);
            exit(CREATE_ERROR);
        }
        // 创建socket成功
        LOG(INFO, "create socket success ! sockfd:", _sockfd);
        // 2.0. 填充sockaddr_in结构
        struct sockaddr_in local;
        bzero(&local, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(_port); // 主机序列转网络序列 -- 网络序列是大端
        // a.字符串序列点分十进制IP地址 转换为 4字节IP
        // b.主机序列转网络序列
        // in_addr_t inet_addr(const char *cp);
        // local.sin_addr.s_addr = inet_addr(_ip.c_str()); // sin_addr 是一个结构体，里面的成员是s_addr
        local.sin_addr.s_addr = INADDR_ANY; // 0 -- 链接当前服务器的所有ip都接受
        // 2.1. 绑定网络信息
        int n = bind(_sockfd, (struct sockaddr *)&local, sizeof(local));
        if (n < 0)
        {
            LOG(FATAL, "socke bind error ! error string %s error %d", strerror(errno), errno);
            exit(BIND_ERROR);
        }
        // 绑定成功
        LOG(INFO, "bind socket success !");

        Threadpool<task_t>::GetInstance()->Start(); // 启动线程池
    }

    void AddOnlineUser(InetAddr user)
    {
        LockGuard lockguard(&_mutex);
        for (auto &e : _online_user)
        {
            if (e == user)
                return;
        }
        _online_user.push_back(user);
    }

    void Route(std::string message)
    {
        LockGuard lockguard(&_mutex);
        for (auto &user : _online_user)
        {
            sendto(_sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&user.GetAddr(), sizeof(user.GetAddr()));
        }
    }

    void Start()
    {
        // 一直运行，直到管理者不想运行了， 服务器都是死循环
        // UDP是面向数据报的协议
        _isrunning = true;
        while (true)
        {
            // 获取数据
            // ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen); // src_addr
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer); // 既是输入（必须初始化） ，也是输出（peer的实际长度）
            char buff[1024];
            ssize_t n = recvfrom(_sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&peer, &len);
            if (n > 0)
            {
                buff[n] = 0;
                // 回答发送方
                // ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen); // dest_addr
                InetAddr addr(peer);
                AddOnlineUser(addr);

                // 转发
                std::string message = "[";
                message += addr.Ip();
                message += ":";
                message += std::to_string(addr.Port());
                message += "] ";
                message += buff;
                auto task = std::bind(&UdpServer::Route, this, message); // 这里绑定后，task的参数就是void()
                Threadpool<task_t>::GetInstance()->Enqueue(task);        // 启动线程池

                LOG(DEBUG, "get message , sender:[%s:%d] , content: %s", addr.Ip().c_str(), addr.Port(), buff);
                sendto(_sockfd, buff, strlen(buff), 0, (struct sockaddr *)&peer, len);
            }
        }
        _isrunning = false;
    }
    ~UdpServer()
    {
        pthread_mutex_destroy(&_mutex);
    }

private:
    int _sockfd;
    // std::string _ip; // 暂时先这样写 -- 不需要
    uint16_t _port;
    bool _isrunning;

    pthread_mutex_t _mutex;
    std::vector<InetAddr> _online_user;
};
