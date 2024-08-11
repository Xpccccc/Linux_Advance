#pragma once

#include <iostream>
#include <memory>
#include <sys/epoll.h>
#include <sys/time.h>
#include <string>
#include "Socket.hpp"

using namespace socket_ns;


class EpollServer
{
    const static int defaultfd = -1;
    const static int N = 64;
    const static int timeout = -1; // 负数阻塞式等待，整数等待的毫秒值

public:
    EpollServer(uint16_t port) : _port(port), _listensock(std::make_unique<TcpSocket>()), _epfd(defaultfd)
    {
        InetAddr client("0", port);
        _listensock->BuildListenSocket(client);

        memset(_events, 0, sizeof(_events));

        _epfd = epoll_create(128);
        if (_epfd < 0)
        {
            LOG(FATAL, "epoll create error...");
            exit(-1);
        }
        LOG(DEBUG, "epoll create sucess, epoll fd : %d", _epfd);
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = _listensock->SockFd();
        // _events[0].events = EPOLLIN;
        // _events[0].data.fd = _listensock->SockFd();
        epoll_ctl(_epfd, EPOLL_CTL_ADD, _listensock->SockFd(), &ev);
    }

    void AcceptClient()
    {
        InetAddr clientaddr;
        socket_sptr sockefd = _listensock->Accepter(&clientaddr);
        int fd = sockefd->SockFd();
        if(fd < 0) return;
        if (fd >= 0)
        {
            LOG(DEBUG, "Get new Link ,sockefd is :%d ,client info : %s:%d", fd, clientaddr.Ip().c_str(), clientaddr.Port());
        }
        // 把新到的文件描述符交给select托管，使用辅助数组
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = fd;
        epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev);
        LOG(DEBUG, "%d sockfd add to epoll rbtree", fd);
    }

    void ServiceIO(int fd)
    {
        char buff[1024];
        ssize_t n = ::recv(fd, buff, sizeof(buff) - 1, 0);
        if (n > 0)
        {
            buff[n] = 0;
            LOG(DEBUG, "client # %s", buff);
            std::string message = "Server Echo# ";
            message += buff;
            ::send(fd, message.c_str(), message.size(), 0);
        }
        else if (n == 0)
        {
            LOG(DEBUG, "%d socket closed!", fd);
            epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr); // 得保证要删除的fd是合法的
            ::close(fd);                                  // 有用户退出，把该文件描述符重置为默认值
        }
        else
        {
            LOG(DEBUG, "%d recv error!", fd);
            epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr);
            ::close(fd); // 有用户退出，把该文件描述符重置为默认值
        }
    }

    void HandlerRead(int num)
    {
        for (int i = 0; i < num; i++)
        {
            uint32_t events = _events[i].events;
            int sockfd = _events[i].data.fd;
            if (events & EPOLLIN) // 读事件就绪
            {
                if (sockfd == _listensock->SockFd()) // listensock
                {
                    AcceptClient();
                }
                else // 真正的读事件就绪
                {
                    // socket读事件就绪
                    ServiceIO(sockfd);
                }
            }
            else if (events & EPOLLOUT)
            {
                // 写事件就绪，后面epoll再做
            }
        }
    }

    void Loop()
    {
        while (true)
        {
            int n = epoll_wait(_epfd, _events, N, timeout); // 返回请求I/O文件描述符的个数
            switch (n)
            {
            case -1:
                // 错误
                LOG(FATAL, "epoll wait error ...");
                break;
            case 0:
                //  时间到了
                LOG(DEBUG, "time out ...");
                break;
            default:
                // 处理读文件描述符
                HandlerRead(n);
                break;
            }
        }
    }

 

    ~EpollServer()
    {
        ::close(_listensock->SockFd());
        if (_epfd >= 0)
            ::close(_epfd);
    }

private:
    uint16_t _port;
    std::unique_ptr<TcpSocket> _listensock;
    int _epfd;
    struct epoll_event _events[N];
};