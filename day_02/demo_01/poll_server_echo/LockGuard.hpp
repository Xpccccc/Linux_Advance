# pragma once

#include <pthread.h>


class LockGuard
{
public:
    LockGuard(pthread_mutex_t *mutex) : _mutex(mutex)
    {
        pthread_mutex_lock(_mutex); // 构造加锁
    }
    ~LockGuard()
    {
        pthread_mutex_unlock(_mutex); // 析构解锁
    }

private:
    pthread_mutex_t *_mutex;
};
