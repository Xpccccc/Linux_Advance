#pragma once

#include <iostream>
#include <memory>
#include <sys/select.h>
#include <sys/time.h>
#include <string>
#include "Socket.hpp"

using namespace socket_ns;

// select需要一个第三方数组保存文件描述符
class SelectServer
{
    const static int N = sizeof(fd_set) * 8;
    const static int defaultfd = -1;

public:
    SelectServer(uint16_t port) : _port(port), _listensock(std::make_unique<TcpSocket>())
    {
        InetAddr client("0", port);
        _listensock->BuildListenSocket(client);
        for (int i = 0; i < N; ++i)
            _fd_array[i] = defaultfd;
        _fd_array[0] = _listensock->SockFd(); // listen文件描述符一定是第一个
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
            if (_fd_array[pos] == defaultfd)
                break;
        }
        if (pos == N)
        {
            ::close(fd);
            // 满了
            LOG(WARNING, "server full ...");
            return;
        }
        else
        {
            _fd_array[pos] = fd;
            LOG(WARNING, "%d sockfd add to select array", fd);
        }
        LOG(DEBUG, "cur fdarr[] fd list : %s", RfdsToStr().c_str());
    }

    void ServiceIO(int pos)
    {
        char buff[1024];
        ssize_t n = ::recv(_fd_array[pos], buff, sizeof(buff) - 1, 0);
        if (n > 0)
        {
            buff[n] = 0;
            LOG(DEBUG, "client # %s", buff);
            std::string message = "Server Echo# ";
            message += buff;
            ::send(_fd_array[pos], message.c_str(), message.size(), 0);
        }
        else if (n == 0)
        {
            LOG(DEBUG, "%d socket closed!", _fd_array[pos]);
            ::close(_fd_array[pos]);
            _fd_array[pos] = defaultfd;
            LOG(DEBUG, "cur fdarr[] fd list : %s", RfdsToStr().c_str());
        }
        else
        {
            LOG(DEBUG, "%d recv error!", _fd_array[pos]);
            ::close(_fd_array[pos]);
            _fd_array[pos] = defaultfd;
            LOG(DEBUG, "cur fdarr[] fd list : %s", RfdsToStr().c_str());
        }
    }

    void HandlerRead(fd_set &rfds)
    {
        for (int i = 0; i < N; i++)
        {
            if (_fd_array[i] == defaultfd)
                continue;
            if (FD_ISSET(_fd_array[i], &rfds))
            {
                if (_fd_array[i] == _listensock->SockFd())
                {
                    AcceptClient();
                }
                else
                {
                    // socket读事件就绪
                    ServiceIO(i);
                }
            }
        }
    }

    void Loop()
    {
        while (true)
        {
            // int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
            fd_set rfds;
            FD_ZERO(&rfds);
            int maxfd = defaultfd;
            for (int i = 0; i < N; ++i)
            {
                if (_fd_array[i] == defaultfd)
                    continue;
                FD_SET(_fd_array[i], &rfds);
                if (_fd_array[i] > maxfd)
                    maxfd = _fd_array[i];
            }
            struct timeval t = {2, 0}; // 隔一段时间阻塞
            // struct timeval t = {0, 0}; // 不阻塞
            // int n = select(maxfd + 1, &rfds, nullptr, nullptr, &t);
            int n = select(maxfd + 1, &rfds, nullptr, nullptr, nullptr); // 永久阻塞
            if (n > 0)
            {
                // 处理读文件描述符
                HandlerRead(rfds);
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
            if (_fd_array[i] != defaultfd)
            {
                rfdstr += std::to_string(_fd_array[i]);
                rfdstr += " ";
            }
        }
        return rfdstr;
    }

    ~SelectServer() {}

private:
    uint16_t _port;
    std::unique_ptr<TcpSocket> _listensock;
    int _fd_array[N];
};