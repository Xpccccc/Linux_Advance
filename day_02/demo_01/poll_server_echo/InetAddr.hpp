#pragma once

#include <iostream>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

class InetAddr
{
    void GetAddress(std::string *ip, uint16_t *port)
    {
        // char *inet_ntoa(struct in_addr in);
        *ip = inet_ntoa(_addr.sin_addr);
        *port = ntohs(_addr.sin_port);
    }

public:
    InetAddr(const struct sockaddr_in &addr) : _addr(addr)
    {
        GetAddress(&_ip, &_port);
    }

    InetAddr(std::string ip, uint16_t port) : _ip(ip), _port(port)
    {
        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(_port);
        _addr.sin_addr.s_addr = inet_addr(_ip.c_str());
    }
    InetAddr() {}

    std::string Ip()
    {
        return _ip;
    }

    uint16_t Port()
    {
        return _port;
    }
    bool operator==(InetAddr &addr)
    {
        return _ip == addr.Ip() && _port == addr.Port();
    }

    const struct sockaddr_in &GetAddr()
    {
        return _addr;
    }

    ~InetAddr() {}

private:
    struct sockaddr_in _addr;
    std::string _ip;
    uint16_t _port;
};
