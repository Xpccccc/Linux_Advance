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
#include "Socket.hpp"

using namespace socket_ns;

using service_t = std::function<void(socket_sptr sockfd, InetAddr client)>;

// 会话层
// 声明
class TcpServer;

class ThreadData
{
public:
    ThreadData(socket_sptr sockfd, InetAddr addr, TcpServer *self)
        : _sockfd(sockfd), _addr(addr), _self(self) {}
    ~ThreadData() = default;

public:
    socket_sptr _sockfd;
    InetAddr _addr;
    TcpServer *_self;
};

class TcpServer
{
public:
    TcpServer(uint16_t port, service_t service)
        : _localaddr("0", port),
          _listensock(std::make_unique<TcpSocket>()),
          _service(service),
          _isrunning(false)
    {
        _listensock->BuildListenSocket(_localaddr);
    }

    static void *HandlerService(void *args)
    {
        pthread_detach(pthread_self()); // 分离线程
        ThreadData *td = static_cast<ThreadData *>(args);
        td->_self->_service(td->_sockfd, td->_addr);
        ::close(td->_sockfd->SockFd()); // 服务结束，关闭文件描述符，避免文件描述符泄漏
        delete td;
        return nullptr;
    }

    void Start()
    {
        _isrunning = true;
        while (_isrunning)
        {
            InetAddr peerAddr;
            socket_sptr normalsock = _listensock->Accepter(&peerAddr);

            // v2 -- 多线程
            pthread_t tid;
            ThreadData *td = new ThreadData(normalsock, peerAddr, this); // 传指针
            pthread_create(&tid, nullptr, HandlerService, td);           // 这里创建线程后，线程去做执行任务，主线程继续向下执行 , 并且线程不能关闭sockf，线程和进程共享文件描述符表
        }
        _isrunning = false;
    }

    ~TcpServer()
    {
    }

private:
    service_t _service;
    InetAddr _localaddr;
    std::unique_ptr<Socket> _listensock;
    bool _isrunning;
};