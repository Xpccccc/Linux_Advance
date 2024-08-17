#pragma once
#include <string>
#include <functional>
#include <sys/epoll.h>
#include "InetAddr.hpp"

class Reactor;
class Connection;
using func_t = std::function<void(Connection *)>;

class Connection
{
public:
    Connection(int sockfd) : _sockfd(sockfd), _R(nullptr) {}

    void SetEvent(uint32_t events)
    {
        _events = events;
    }

    void Register(func_t recver, func_t sender, func_t excepter)
    {
        _recver = recver;
        _sender = sender;
        _excepter = excepter;
    }

    void SetSelf(Reactor *R)
    {
        _R = R;
    }

    int SockFd()
    {
        return _sockfd;
    }

    void AppendInbuff(const std::string &buff)
    {
        _inbuffer += buff;
    }

    void AppendOutbuff(const std::string &buff)
    {
        _outbuffer += buff;
    }

    std::string &Inbuffer() // 返回引用，后面Decode得字符串切割
    {
        return _inbuffer;
    }

    std::string &Outbuffer() // 返回引用，后面Decode得字符串切割
    {
        return _outbuffer;
    }

    void OutBufferRemove(int n)
    {
        _outbuffer.erase(0, n);
    }

    uint32_t Events()
    {
        return _events;
    }

    ~Connection() {}

private:
    int _sockfd;

    // 输入输出缓冲区
    std::string _inbuffer;
    std::string _outbuffer;

    // 已经准备好的事件
    uint32_t _events;

    InetAddr _clientaddr;

public:
    // 处理事件
    func_t _recver;
    func_t _sender;
    func_t _excepter;

    Reactor *_R;
};
