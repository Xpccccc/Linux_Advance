#pragma once
#include <iostream>
#include <memory>
#include "Socket.hpp"
#include "IOService.hpp"

using namespace socket_ns;

class Listener
{
public:
    Listener(uint16_t port, IOService &io) : _port(port), _listensock(std::make_unique<TcpSocket>()), _io(io)
    {
        InetAddr clientaddr("0", port);
        _listensock->BuildListenSocket(clientaddr);
        // LOG(DEBUG,"Listen sock : %d",_listensock->SockFd());
    }

    void Accepter(Connection *conn) // conn一定是listensock
    {
        LOG(DEBUG, "get new link , conn fd : %d", conn->SockFd());

        // 新连接到来
        while (true)
        {
            InetAddr clientaddr;
            int code = 0;

            int listensockfd = _listensock->Accepter(&clientaddr, &code); // 第二次卡住
            // listensockfd = _listensock->Accepter(&clientaddr, &code);

            if (listensockfd >= 0)
            {
                // 添加新连接
                conn->_R->AddConnection(listensockfd, EPOLLIN | EPOLLET,
                                        std::bind(&IOService::HandlerRecv, &_io, std::placeholders::_1),
                                        std::bind(&IOService::HandlerSend, &_io, std::placeholders::_1),
                                        std::bind(&IOService::HandlerExcept, &_io, std::placeholders::_1));
                // 这里就只是添加对应的处理函数（不懂跳过去可以看AddConnection函数），用不用看到时候到的是什么信号（EPOLLIN等）
                // 使用对应的函数会传conn，比如使用_recver(conn)。
            }
            else
            {
                if (code == EWOULDBLOCK || code == EAGAIN)
                {
                    // 读完了所有就绪文件描述符
                    LOG(DEBUG, "ready fd read complete!");
                    break;
                }
                else if (code == EINTR)
                {
                    LOG(DEBUG, "accpet interupt by signal ");
                    continue;
                }
                else
                {
                    LOG(WARNING, "accpet error");
                    break;
                }
            }
        }
    }

    int SockFd()
    {
        return _listensock->SockFd();
    }

    ~Listener()
    {
        ::close(_listensock->SockFd());
    }

private:
    uint16_t _port;
    std::unique_ptr<Socket> _listensock;
    IOService &_io;
};