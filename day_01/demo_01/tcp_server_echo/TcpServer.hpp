#pragma once

#include <sys/types.h> /* See NOTES */
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <error.h>
#include <string.h>
#include <pthread.h>
#include <functional>

#include "Log.hpp"
#include "InetAddr.hpp"
#include "Comm.hpp"
#include "Threadpool.hpp"

const int defaultsockfd = -1;
int gbacklog = 16; // 暂时先用

using task_t = std::function<void()>;

// 声明
class TcpServer;

class ThreadData
{
public:
    ThreadData(int sockfd, InetAddr addr, TcpServer *self)
        : _sockfd(sockfd), _addr(addr), _self(self) {}
    ~ThreadData() = default;

public:
    int _sockfd;
    InetAddr _addr;
    TcpServer *_self;
};

class TcpServer
{
public:
    TcpServer(uint16_t port) : _port(port), _listensock(defaultsockfd), _isrunning(false)
    {
    }

    void InitServer()
    {
        // 创建
        _listensock = socket(AF_INET, SOCK_STREAM, 0); // 这个就是文件描述符
        if (_listensock < 0)
        {
            LOG(FATAL, "create sockfd error, error code : %d, error string : %s", errno, strerror(errno));
            exit(CREATE_ERROR);
        }
        LOG(INFO, "create sockfd success");

        struct sockaddr_in local;
        bzero(&local, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(_port);
        local.sin_addr.s_addr = INADDR_ANY;
        // 绑定
        int n = ::bind(_listensock, CONV(&local), sizeof(local));
        if (n < 0)
        {
            LOG(FATAL, "bind sockfd error, error code : %d, error string : %s", errno, strerror(errno));
            exit(BIND_ERROR);
        }
        LOG(INFO, "bind sockfd success");
    }

    void Service(int sockfd, InetAddr client)
    {
        while (true)
        {
            // TCP是字节流（可以使用write和read接口），UDP是数据报
            char buff[1024];
            // 接收消息
            int n = ::read(sockfd, buff, sizeof(buff)); // bug，接收数据可能收到的不完整，比如1+100，可能先收到1+1，再收到00 -- 按序到达
            std::string clientAddr = CombineIpAndPort(client);

            if (n > 0)
            {
                buff[n] = 0;
                std::string message = clientAddr + buff;
                LOG(INFO, "get message : \n %s", message.c_str());

                // 发送消息
                int m = ::write(sockfd, buff, strlen(buff)); 
                if (m < 0)
                {
                    LOG(FATAL, "send message error ,error code : %d , error string : %s", errno, strerror(errno));
                    exit(SEND_ERROR);
                }
            }
            else if (n == 0)
            {
                // 发送端不发送数据了
                LOG(INFO, "%s quit", clientAddr.c_str());
                break;
            }
            else
            {
                LOG(FATAL, "recv message error ,error code : %d , error string : %s", errno, strerror(errno));
                exit(RECV_ERROR);
            }
        }
        ::close(sockfd); // 服务结束，关闭文件描述符，避免文件描述符泄漏
    }

    static void *HandlerService(void *args)
    {
        pthread_detach(pthread_self()); // 分离线程
        ThreadData *td = static_cast<ThreadData *>(args);
        td->_self->Service(td->_sockfd, td->_addr);
        delete td;
        return nullptr;
    }

    void Start()
    {
        _isrunning = true;
        while (_isrunning)
        {
            // 监听
            int ret = ::listen(_listensock, gbacklog);
            if (ret < 0)
            {
                LOG(FATAL, "listen error, error code : %d , error string : %s", errno, strerror(errno));
                exit(LISTEN_ERROR);
            }
            LOG(INFO, "listen success!");

            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            // 获取新连接
            int sockfd = accept(_listensock, CONV(&peer), &len); // 建立连接成功，创建新文件描述符进行通信
            if (sockfd < 0)
            {
                LOG(WARNING, "accept error, error code : %d , error string : %s", errno, strerror(errno));
                continue;
            }
            LOG(INFO, "accept success! new sockfd : %d", sockfd);

            InetAddr addr(peer); // 给后面提供传入的ip、port
            // 服务 -- 发送和接收数据
            // V0 -- 单进程
            // Service(sockfd, addr); // 这里是while死循环，没有运行完就一直运行，下一个请求来的时候得这个while退出才能执行

            // v1 -- 多进程
            // int id = fork();
            // if (id == 0)
            // {
            //     // 子进程
            //     ::close(_listensock); // 子进程对监听文件描述符不关心
            //     if (fork() > 0)
            //         exit(0);           // 子进程创建进程后退出，孙子进程被系统领养，不用等待
            //     Service(sockfd, addr); // 孙子进程执行任务
            //     exit(0);
            // }
            // else if (id > 0)
            // {
            //     ::close(sockfd); // 这里每次关闭的文件描述符都是4，使得每次accept创建的文件描述符都是4，这个4是留个各个子进程（子进程再给孙子进程）的（互不影响）
            //     // 父进程
            //     pid_t rid = waitpid(id, nullptr, 0); // 虽然是阻塞等待，但是子进程是刚创建就退出来了，让孙子进程（孤儿进程）执行任务，接下来继续监听和建立连接
            //     if (rid == id)
            //     {
            //         LOG(INFO, "wait child process success");
            //     }
            // }
            // else
            // {
            //     // 异常
            //     LOG(FATAL, "fork error ,error code : %d , error string : %s", errno, strerror(errno));
            //     exit(FORK_ERROR);
            // }

            // v2 -- 多线程
            // pthread_t tid;
            // ThreadData *td = new ThreadData(sockfd, addr, this); // 传指针
            // pthread_create(&tid, nullptr, HandlerService, td); // 这里创建线程后，线程去做执行任务，主线程继续向下执行 , 并且线程不能关闭sockf，线程和进程共享文件描述符表

            // v3 -- 线程池
            task_t t = std::bind(&TcpServer::Service, this, sockfd, addr);
            Threadpool<task_t>::GetInstance()->Enqueue(t);

            // v4 -- 进程池 -- 不推荐，需要传递文件描述符！
        }
        _isrunning = false;
    }

    ~TcpServer()
    {
        if (_listensock > defaultsockfd)
            ::close(_listensock); // 不用了关闭监听
    }

private:
    uint16_t _port;
    int _listensock;
    bool _isrunning;
};