#pragma once

#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>

class Session
{
public:
    Session(std::string username, std::string status) : _username(username), _status(status)
    {
        _create_time = time(nullptr);
    }

    ~Session() {}

public:
    std::string _username;
    std::string _status;
    uint32_t _create_time;
};

class SessionManager
{
public:
    SessionManager()
    {
        srand(time(nullptr) ^ getpid());
    }

    std::string AddSession(Session *sptr)
    {
        uint32_t randnum = rand() + time(nullptr);
        std::string sessionid = std::to_string(randnum);
        _sessions.insert(std::make_pair(sessionid, sptr));
        return sessionid;
    }

    Session *GetSession(const std::string sessionid)
    {
        if (_sessions.find(sessionid) == _sessions.end())
            return nullptr;
        return _sessions[sessionid];
    }
    ~SessionManager() {}

private:
    std::unordered_map<std::string, Session *> _sessions;
};