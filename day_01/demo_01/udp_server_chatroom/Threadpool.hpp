#pragma once

#include <vector>
#include <queue>
#include <queue>
#include "Thread.hpp"
#include <pthread.h>
#include "LockGuard.hpp"

using namespace ThreadModule;

const int NUM = 3;

template <typename T>
class Threadpool
{
    void LockQueue(pthread_mutex_t &mutex)
    {
        pthread_mutex_lock(&mutex);
    }

    void UnLockQueue(pthread_mutex_t &mutex)
    {
        pthread_mutex_unlock(&mutex);
    }

    void SleepThread(pthread_cond_t &cond, pthread_mutex_t &mutex)
    {
        pthread_cond_wait(&cond, &mutex);
    }
    void WakeUpThread(pthread_cond_t &cond)
    {
        pthread_cond_signal(&cond);
    }
    void WakeUpAll(pthread_cond_t &cond)
    {
        pthread_cond_broadcast(&_cond);
    }
    Threadpool(const int threadnum = NUM) : _threadnum(threadnum), _waitnum(0), _isrunning(false)
    {
        pthread_mutex_init(&_mutex, nullptr);
        pthread_cond_init(&_cond, nullptr);
        LOG(INFO, "Threadpool Constructor successful ! ");
    }

    void TaskHandler(string &name)
    {
        // sleep(1);
        // cout  << name << " : hh " << endl;
        // sleep(1);
        LOG(DEBUG, "%s is running", name.c_str());
        while (true)
        {
            LockQueue(_mutex);
            while (_task_queue.empty() && _isrunning)
            {
                // 等待
                ++_waitnum;
                SleepThread(_cond, _mutex);
                --_waitnum;
            }
            // 此时一定大于一个线程没有休眠
            if (_task_queue.empty() && !_isrunning)
            {
                // 此时任务队列已经没有内容，且此时线程池已经停止
                UnLockQueue(_mutex);
                cout << name << " quit ... " << endl;
                break;
            }
            LOG(DEBUG, "%s get task sucessful !", name.c_str());
            //  其他情况就得处理任务
            T t = _task_queue.front();
            _task_queue.pop();
            UnLockQueue(_mutex);

            // 处理任务
            t();
            // cout << name << " : " << t.stringResult() << endl;
            // LOG(DEBUG, "%s handler task sucessful ! Result is %s", name.c_str(), t.stringResult().c_str());
            sleep(1);
        }
    }
    void InitThreadPool()
    {
        for (int i = 0; i < _threadnum; ++i)
        {
            string name = "Thread - " + to_string(i + 1);
            _threads.emplace_back(bind(&Threadpool::TaskHandler, this, placeholders::_1), name);
        }
        _isrunning = true;
        LOG(INFO, "Init Threadpool successful !");
    }

public:
    static Threadpool<T> *GetInstance(int threadnum = NUM)
    {

        if (_instance == nullptr)
        {
            LockGuard lockguard(&_lock);
            if (_instance == nullptr)
            {
                // pthread_mutex_lock(&_lock);
                // 第一次创建线程池
                _instance = new Threadpool<T>(threadnum);
                _instance->InitThreadPool();
                _instance->Start();
                LOG(DEBUG, "第一次创建线程池");
                // pthread_mutex_unlock(&_lock);
                return _instance;
            }
        }

        LOG(DEBUG, "获取线程池");
        return _instance;
    }

    bool Enqueue(const T &in)
    {
        bool ret = false;
        LockQueue(_mutex);
        if (_isrunning)
        {
            _task_queue.push(in);
            if (_waitnum > 0)
                WakeUpThread(_cond);
            LOG(DEBUG, "enqueue sucessful...");
            ret = true;
        }

        UnLockQueue(_mutex);
        return ret;
    }

    void Stop()
    {
        LockQueue(_mutex);
        _isrunning = false;
        if (_waitnum > 0)
            WakeUpAll(_cond);
        UnLockQueue(_mutex);
    }

    void Start()
    {
        for (auto &thread : _threads)
        {
            thread.Start();
            LOG(INFO, "%s is start sucessful...", thread.name().c_str());
        }
    }

    void Wait()
    {
        for (auto &thread : _threads)
        {
            thread.Join();
            LOG(INFO, "%s is quit...", thread.name().c_str());
        }
    }
    ~Threadpool()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_cond);
        LOG(INFO, "delete mutex sucessful !");
    }

private:
    vector<Thread> _threads;
    queue<T> _task_queue;
    int _threadnum;
    int _waitnum;

    pthread_mutex_t _mutex; // 互斥访问任务队列
    pthread_cond_t _cond;

    bool _isrunning;

    // 懒汉模式
    static Threadpool<T> *_instance;
    static pthread_mutex_t _lock;
};

template <typename T>
Threadpool<T> *Threadpool<T>::_instance = nullptr;
template <typename T>
pthread_mutex_t Threadpool<T>::_lock = PTHREAD_MUTEX_INITIALIZER;

// #pragma once

// #include <vector>
// #include <queue>
// #include <queue>
// #include "Thread.hpp"
// #include <pthread.h>

// using namespace ThreadModule;

// template <typename T>
// class Threadpool
// {
//     void LockQueue(pthread_mutex_t &mutex)
//     {
//         pthread_mutex_lock(&mutex);
//     }

//     void UnLockQueue(pthread_mutex_t &mutex)
//     {
//         pthread_mutex_unlock(&mutex);
//     }

//     void SleepThread(pthread_cond_t &cond, pthread_mutex_t &mutex)
//     {
//         pthread_cond_wait(&cond, &mutex);
//     }
//     void WakeUpThread(pthread_cond_t &cond)
//     {
//         pthread_cond_signal(&cond);
//     }
//     void WakeUpAll(pthread_cond_t &cond)
//     {
//         pthread_cond_broadcast(&_cond);
//     }

// public:
//     Threadpool(const int threadnum = 3) : _threadnum(threadnum), _waitnum(0), _isrunning(false)
//     {
//         pthread_mutex_init(&_mutex, nullptr);
//         pthread_cond_init(&_cond, nullptr);
//         LOG(INFO, "Threadpool Constructor successful ! ");
//     }

//     void TaskHandler(string &name)
//     {
//         // sleep(1);
//         // cout  << name << " : hh " << endl;
//         // sleep(1);
//         LOG(DEBUG, "%s is running", name.c_str());
//         while (true)
//         {
//             LockQueue(_mutex);
//             while (_task_queue.empty() && _isrunning)
//             {
//                 // 等待
//                 ++_waitnum;
//                 SleepThread(_cond, _mutex);
//                 --_waitnum;
//             }
//             // 此时一定大于一个线程没有休眠
//             if (_task_queue.empty() && !_isrunning)
//             {
//                 // 此时任务队列已经没有内容，且此时线程池已经停止
//                 UnLockQueue(_mutex);
//                 cout << name << " quit ... " << endl;
//                 break;
//             }
//             LOG(DEBUG, "%s get task sucessful !", name.c_str());
//             //  其他情况就得处理任务
//             T t = _task_queue.front();
//             _task_queue.pop();
//             UnLockQueue(_mutex);

//             // 处理任务
//             t();
//             // cout << name << " : " << t.stringResult() << endl;
//             LOG(DEBUG, "%s handler task sucessful ! Result is %s", name.c_str(), t.stringResult().c_str());
//             sleep(1);
//         }
//     }
//     void InitThreadPool()
//     {
//         for (int i = 0; i < _threadnum; ++i)
//         {
//             string name = "Thread - " + to_string(i + 1);
//             _threads.emplace_back(bind(&Threadpool::TaskHandler, this, placeholders::_1), name);
//         }
//         _isrunning = true;
//         LOG(INFO, "Init Threadpool successful !");
//     }

//     bool Enqueue(const T &in)
//     {
//         bool ret = false;
//         LockQueue(_mutex);
//         if (_isrunning)
//         {
//             _task_queue.push(in);
//             if (_waitnum > 0)
//                 WakeUpThread(_cond);
//             LOG(DEBUG, "enqueue sucessful...");
//             ret = true;
//         }

//         UnLockQueue(_mutex);
//         return ret;
//     }

//     void Stop()
//     {
//         LockQueue(_mutex);
//         _isrunning = false;
//         if (_waitnum > 0)
//             WakeUpAll(_cond);
//         UnLockQueue(_mutex);
//     }

//     void Start()
//     {
//         for (auto &thread : _threads)
//         {
//             thread.Start();
//             LOG(INFO, "%s is start sucessful...", thread.name().c_str());
//         }
//     }

//     void Wait()
//     {
//         for (auto &thread : _threads)
//         {
//             thread.Join();
//             LOG(INFO, "%s is quit...", thread.name().c_str());
//         }
//     }
//     ~Threadpool()
//     {
//         pthread_mutex_destroy(&_mutex);
//         pthread_cond_destroy(&_cond);
//         LOG(INFO, "delete mutex sucessful !");
//     }

// private:
//     vector<Thread> _threads;
//     queue<T> _task_queue;
//     int _threadnum;
//     int _waitnum;

//     pthread_mutex_t _mutex; // 互斥访问任务队列
//     pthread_cond_t _cond;

//     bool _isrunning;

// };


