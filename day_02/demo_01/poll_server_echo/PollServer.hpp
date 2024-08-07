#pragma once

#include <iostream>
#include <memory>
#include <poll.h>
#include <sys/time.h>
#include <string>
#include "Socket.hpp"

using namespace socket_ns;

class PollServer
{
    const static int defaultfd = -1;
    const static int N = 1024;
    const static int timeout = -1; // 负数阻塞式等待，整数等待的毫秒值

public:
    PollServer(uint16_t port) : _port(port), _listensock(std::make_unique<TcpSocket>())
    {
        InetAddr client("0", port);
        _listensock->BuildListenSocket(client);
        for (int i = 0; i < N; ++i)
        {
            _fds[i].fd = defaultfd;
            _fds[i].revents = 0;
            _fds[i].events = 0;
        }
        _fds[0].fd = _listensock->SockFd(); // 第一个肯定是listensock的文件描述符
        _fds[0].events = POLLIN;            // 对读事件关心
    }

    void AcceptClient()
    {
        InetAddr clientaddr;
        socket_sptr sockefd = _listensock->Accepter(&clientaddr);
        int fd = sockefd->SockFd();
        if (fd >= 0)
        {
            LOG(DEBUG, "Get new Link ,sockefd is :%d ,client info : %s:%d", fd, clientaddr.Ip().c_str(), clientaddr.Port());
        }
        // 把新到的文件描述符交给select托管，使用辅助数组
        int pos = 1;
        for (; pos < N; pos++)
        {
            if (_fds[pos].fd == defaultfd)
                break;
        }
        if (pos == N)
        {
            // 满了
            ::close(fd); // 这里就是比select更好，可以扩容，也可以直接关闭文件描述符
            LOG(WARNING, "server full ...");
            return;
        }
        else
        {
            _fds[pos].fd = fd; // 添加新文件描述符
            _fds[pos].events = POLLIN;// 对读事件关心
            _fds[pos].revents = 0;
            LOG(WARNING, "%d sockfd add to select array", fd);
        }
        LOG(DEBUG, "cur fdarr[] fd list : %s", RfdsToStr().c_str());
    }

    void ServiceIO(int pos)
    {
        char buff[1024];
        ssize_t n = ::recv(_fds[pos].fd, buff, sizeof(buff) - 1, 0);
        if (n > 0)
        {
            buff[n] = 0;
            LOG(DEBUG, "client # %s", buff);
            std::string message = "Server Echo# ";
            message += buff;
            ::send(_fds[pos].fd, message.c_str(), message.size(), 0);
        }
        else if (n == 0)
        {
            LOG(DEBUG, "%d socket closed!", _fds[pos].fd);
            ::close(_fds[pos].fd); // 有用户退出，把该文件描述符重置为默认值
            _fds[pos].fd = defaultfd;
            _fds[pos].events = 0;
            _fds[pos].revents = 0;
            LOG(DEBUG, "cur fdarr[] fd list : %s", RfdsToStr().c_str());
        }
        else
        {
            LOG(DEBUG, "%d recv error!", _fds[pos].fd);
            ::close(_fds[pos].fd);
            _fds[pos].fd = defaultfd;
            _fds[pos].events = 0;
            _fds[pos].revents = 0;
            LOG(DEBUG, "cur fdarr[] fd list : %s", RfdsToStr().c_str());
        }
    }

    void HandlerRead()
    {
        for (int i = 0; i < N; i++)
        {
            if (_fds[i].fd == defaultfd)
                continue;
            if (_fds[i].revents & POLLIN) // 读事件就绪
            {
                if (_fds[i].fd == _listensock->SockFd()) // listensock
                {
                    AcceptClient();
                }
                else // 真正的读事件就绪
                {
                    // socket读事件就绪
                    ServiceIO(i);
                }
            }
            else if(_fds[i].revents & POLLOUT)
            {
                // 写事件就绪，后面epoll再做
            }
        }
    }

    void Loop()
    {
        while (true)
        {
            int n = poll(_fds, N, timeout);
            if (n > 0)
            {
                // 处理读文件描述符
                HandlerRead();
            }
            else if (n == 0)
            {
                //  时间到了
                LOG(DEBUG, "time out ...");
            }
            else
            {
                // 错误
                LOG(FATAL, "select error ...");
            }
        }
    }

    std::string RfdsToStr()
    {
        std::string rfdstr;
        for (int i = 0; i < N; ++i)
        {
            if (_fds[i].fd != defaultfd)
            {
                rfdstr += std::to_string(_fds[i].fd);
                rfdstr += " ";
            }
        }
        return rfdstr;
    }

    ~PollServer() {}

private:
    uint16_t _port;
    std::unique_ptr<TcpSocket> _listensock;
    struct pollfd _fds[N]; // 可以设置成容量满自动扩容模式
};