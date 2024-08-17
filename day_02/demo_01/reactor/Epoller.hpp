#pragma once
#include <sys/epoll.h>
#include "Log.hpp"

class Epoller
{
    bool EventCore(int sockfd, uint32_t event, int type)
    {
        struct epoll_event ep_event;
        ep_event.data.fd = sockfd;
        ep_event.events = event;
        int n = ::epoll_ctl(_epfd, type, sockfd, &ep_event);
        if (n < 0)
        {
            LOG(ERROR, "epoll_ctl error");
            return false;
        }
        LOG(DEBUG, "epoll_ctl add %d fd success", sockfd);
        return true;
    }

public:
    Epoller()
    {
        _epfd = ::epoll_create(128);
        if (_epfd < 0)
        {
            LOG(FATAL, "create epfd error");
            exit(EPOLL_CREATE_ERROR);
        }
        LOG(DEBUG, "create epfd success, epfd : %d", _epfd);
    }

    bool AddEvent(int sockfd, uint32_t event)
    {
        return EventCore(sockfd, event, EPOLL_CTL_ADD);
    }

    bool ModEvent(int sockfd, uint32_t event)
    {
        return EventCore(sockfd, event, EPOLL_CTL_MOD);
    }

    bool DelEvent(int sockfd)
    {
        return ::epoll_ctl(_epfd, EPOLL_CTL_DEL, sockfd, nullptr);
    }

    int Wait(struct epoll_event *recv, int num, int timeout)
    {
        int n = ::epoll_wait(_epfd, recv, num, timeout);
        return n;
    }
    ~Epoller()
    {
        if (_epfd > 0)
            ::close(_epfd);
    }

private:
    int _epfd;
};