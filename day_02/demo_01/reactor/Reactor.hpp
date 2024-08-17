#pragma once
#include <string>
#include <unordered_map>
#include "Connection.hpp"
#include "Epoller.hpp"

// TcpServer就是Reactor（反应堆）
// class TcpServer // 对Connection和Epoller管理就行
class Reactor
{
    const static int gnum = 64;

public:
    Reactor() : _is_running(false) {}

    void AddConnection(int sockfd, uint32_t events, func_t recver, func_t sender, func_t excepter)
    {
        // 1.构建Connection对象
        Connection *conn = new Connection(sockfd);
        conn->SetEvent(events);
        conn->Register(recver, sender, excepter);
        conn->SetSelf(this);

        // 2.向内核表示对fd的关心
        _epoller.AddEvent(conn->SockFd(), conn->Events());

        // std::cout << "sockfd : " << sockfd << " , events : " << (events & EPOLLIN) << std::endl;

        // 3.向_connections添加Connection对象
        _connections.insert(std::make_pair(conn->SockFd(), conn));
    }

    bool ConnectionIsExist(int sockfd)
    {
        auto iter = _connections.find(sockfd);

        return iter != _connections.end();
    }

    void EnableReadWrite(int sockfd, bool wr, bool rd)
    {
        uint32_t events = (wr ? EPOLLOUT : 0) | (rd ? EPOLLIN : 0) | EPOLLET;
        if (ConnectionIsExist(sockfd))
        {
            // 修改对事件的关心
            _connections[sockfd]->SetEvent(events);
            // 设置到内核
            _epoller.ModEvent(sockfd, events);
        }
    }

    void RemoveConnection(int sockfd)
    {
        if (!ConnectionIsExist(sockfd))
            return;
        // 解除对文件描述符的关心
        _epoller.DelEvent(sockfd);
        // 关闭文件描述符
        ::close(sockfd);
        // 去除该连接
        delete _connections[sockfd];
        _connections.erase(sockfd);
    }

    // 一次派发
    void LoopOnce(int timeout)
    {
        int n = _epoller.Wait(recv, gnum, timeout); // n个事件就绪
        for (int i = 0; i < n; i++)
        {
            int sockfd = recv[i].data.fd;
            uint32_t revents = recv[i].events;

            // std::cout << "sockfd : " << sockfd << " , revents : " << revents << std::endl;

            // 挂起或者出错了转为读写事件就绪
            if (revents & EPOLLHUP)
                revents |= (EPOLLIN | EPOLLOUT);
            if (revents & EPOLLERR)
                revents |= (EPOLLIN | EPOLLOUT);

            // 读事件就绪
            if (revents & EPOLLIN)
            {
                // 文件描述符得在_connections存在（比如客户端可能退出了，这个文件描述符就没有了）
                if (ConnectionIsExist(sockfd) && (_connections[sockfd]->_recver != nullptr))
                    _connections[sockfd]->_recver(_connections[sockfd]); // 处理读事件就绪，这里_recver已经在AddConnection注册了！
            }
            // 写事件就绪
            if (revents & EPOLLOUT)
            {
                if (ConnectionIsExist(sockfd) && (_connections[sockfd]->_sender != nullptr))
                    _connections[sockfd]->_sender(_connections[sockfd]); // 处理写事件就绪，这里_sender已经在AddConnection注册了！
            }
        }
    }

    // 只负责事件派发
    void Despatcher()
    {
        _is_running = true;
        int timeout = -1; // 阻塞等
        while (true)
        {
            LoopOnce(timeout);
            // 处理其他事情
            Debug();
        }
        _is_running = false;
    }

    void Debug()
    {
        for (auto &connection : _connections)
        {
            std::cout << "------------------------------------" << std::endl;
            std::cout << "fd : " << connection.second->SockFd() << " , ";
            uint32_t events = connection.second->Events();
            if ((events & EPOLLIN) && (events & EPOLLET))
                std::cout << "EPOLLIN | EPOLLET";
            if ((events & EPOLLIN) && (events & EPOLLET))
                std::cout << "EPOLLIN | EPOLLET";
            std::cout << std::endl;
        }
        std::cout << "------------------------------------" << std::endl;
    }
    ~Reactor() {}

private:
    std::unordered_map<int, Connection *> _connections; // 保存fd 和 对应的连接
    Epoller _epoller;

    struct epoll_event recv[gnum];
    bool _is_running;
};