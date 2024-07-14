#pragma once

#include <string.h>
#include <memory>

#include "Log.hpp"
#include "Comm.hpp"

namespace socket_ns
{
    const static int gbacklog = 8;

    class Socket;
    using socket_sptr = std::shared_ptr<Socket>; // 定义智能指针，以便于后面多态

    // 使用
    // std::unique_ptr<Socket> listensocket = std::make_unique<TcpSocket>();
    // listensocket->BuildListenSocket();
    // socket_sptr retsock = listensocket->Accepter();
    // retsock->Recv();
    // retsock->Send();

    // std::unique_ptr<Socket> clientsocket = std::make_unique<TcpSocket>();
    // clientsocket->BuildClientSocket();
    // clientsocket->Send();
    // clientsocket->Recv();

    class Socket
    {
    public:
        virtual void CreateSocketOrDie() = 0;
        virtual void BindSocketOrDie(InetAddr &addr) = 0;
        virtual void ListenSocketOrDie() = 0;
        virtual socket_sptr Accepter(InetAddr *addr) = 0;
        virtual bool Connector(InetAddr &addr) = 0;

        virtual int SockFd() = 0;

        virtual ssize_t Recv(std::string *out) = 0;
        virtual ssize_t Send(std::string &in) = 0;
        // virtual void Other() = 0;

    public:
        void BuildListenSocket(InetAddr &addr)
        {
            CreateSocketOrDie();
            BindSocketOrDie(addr);
            ListenSocketOrDie();
        }

        bool BuildClientSocket(InetAddr &addr)
        {
            CreateSocketOrDie();
            return Connector(addr);
        }
    };

    class TcpSocket : public Socket
    {
    public:
        TcpSocket(int sockfd = -1) : _socktfd(sockfd)
        {
        }
        virtual void CreateSocketOrDie() override
        {
            // 创建
            _socktfd = socket(AF_INET, SOCK_STREAM, 0); // 这个就是文件描述符
            if (_socktfd < 0)
            {
                LOG(FATAL, "create sockfd error, error code : %d, error string : %s", errno, strerror(errno));
                exit(CREATE_ERROR);
            }
            LOG(INFO, "create sockfd success");
        }
        virtual void BindSocketOrDie(InetAddr &addr) override
        {
            struct sockaddr_in local;
            bzero(&local, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(addr.Port());
            local.sin_addr.s_addr = INADDR_ANY;
            // 绑定
            int n = ::bind(_socktfd, CONV(&local), sizeof(local));
            if (n < 0)
            {
                LOG(FATAL, "bind sockfd error, error code : %d, error string : %s", errno, strerror(errno));
                exit(BIND_ERROR);
            }
            LOG(INFO, "bind sockfd success");
        }
        virtual void ListenSocketOrDie() override
        {
            // 监听
            int ret = ::listen(_socktfd, gbacklog);
            if (ret < 0)
            {
                LOG(FATAL, "listen error, error code : %d , error string : %s", errno, strerror(errno));
                exit(LISTEN_ERROR);
            }
            LOG(INFO, "listen success!");
        }
        virtual socket_sptr Accepter(InetAddr *addr) override
        {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            // 获取新连接
            int newsockfd = accept(_socktfd, CONV(&peer), &len); // 建立连接成功，创建新文件描述符进行通信
            if (newsockfd < 0)
            {
                LOG(WARNING, "accept error, error code : %d , error string : %s", errno, strerror(errno));
                return nullptr;
            }
            LOG(INFO, "accept success! new sockfd : %d", newsockfd);
            *addr = peer;
            socket_sptr sock = std::make_shared<TcpSocket>(newsockfd); // 创建新的文件描述符，传出去以便于后面的Recv和Send
            return sock;
        }

        virtual bool Connector(InetAddr &addr) override
        {
            struct sockaddr_in local;
            bzero(&local, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(addr.Port());
            local.sin_addr.s_addr = inet_addr(addr.Ip().c_str());

            // 发起连接
            int n = ::connect(_socktfd, CONV(&local), sizeof(local));
            if (n < 0)
            {
                LOG(WARNING, "create connect error, error code : %d, error string : %s", errno, strerror(errno));
                return false;
            }
            LOG(INFO, "create connect success");
            return true;
        }

        virtual int SockFd() override
        {
            return _socktfd;
        }

        virtual ssize_t Recv(std::string *out) override
        {
            char buff[1024];
            ssize_t n = recv(_socktfd, buff, sizeof(buff) - 1, 0);
            if (n > 0)
            {
                buff[n] = 0;
                *out += buff; // 方便当数据到来不是刚好1条数据的时候，进行合并后来的数据
            }
            return n;
        }
        virtual ssize_t Send(std::string &in) override
        {
            ssize_t n = send(_socktfd, in.c_str(), in.size(),0);
            return n;
        }

    private:
        int _socktfd; // 用同一个_socket
    };
}
