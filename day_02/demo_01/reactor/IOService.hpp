#pragma once
#include "Connection.hpp"
#include "Comm.hpp"

// 处理IO，recver、sender、excepter
class IOService
{
public:
    IOService(func_t func) : _func(func) {}
    void HandlerRecv(Connection *conn)
    {
        // 处理读事件
        errno = 0;
        while (true)
        {
            char buff[1024];
            ssize_t n = ::recv(conn->SockFd(), buff, sizeof(buff) - 1, 0);
            SetNonBlock(conn->SockFd()); // 这里也得非阻塞，不然会阻塞
            if (n > 0)
            {
                buff[n] = 0;
                conn->AppendInbuff(buff);
            }
            else
            {
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    break;
                }
                else if (errno == EINTR)
                {
                    continue;
                }
                else
                {
                    conn->_excepter(conn); // 统一处理异常
                    return;                // 一定要提前返回
                }
            }
        }
        LOG(DEBUG, "debug");
        _func(conn);
    }

    void HandlerSend(Connection *conn)
    {
        // errno
        errno = 0;

        while (true)
        {
            ssize_t n = ::send(conn->SockFd(), conn->Outbuffer().c_str(), conn->Outbuffer().size(), 0);
            if (n > 0)
            {
                // 发送的数据的字节数小于Outbuffer的大小
                // n即实际发了多少
                conn->OutBufferRemove(n);
                if (conn->Outbuffer().empty())
                    break; // 发完了
            }
            else if (n == 0)
            {
                break;
            }
            else
            {
                // 写到文件结尾了
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    break;
                }
                else if (errno == EINTR)
                {
                    continue;
                }
                else
                {
                    conn->_excepter(conn); // 统一处理异常
                    return;                // 一定要提前返回
                }
            }
        }

        // 一定遇到了缓冲区被写满
        if (!conn->Outbuffer().empty())
        {
            // 开启对写事件关心
            conn->_R->EnableReadWrite(conn->SockFd(), true, true);
        }
        else
        {
            // 重置对写事件的不关心
            conn->_R->EnableReadWrite(conn->SockFd(), false, true);
        }
    }

    void HandlerExcept(Connection *conn)
    {
        conn->_R->RemoveConnection(conn->SockFd());
    }

private:
    func_t _func;
};