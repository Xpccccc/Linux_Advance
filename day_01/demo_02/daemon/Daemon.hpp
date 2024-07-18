#pragma once

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>

const std::string work_dir = "/";
const std::string path = "/dev/null";

bool Daemon(bool ischdir, bool isclose)
{
    // 1.忽略不要的信号
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    // 2.创建子进程，父进程退出
    // 因为进程组长不能创建新会话，那就子进程创建新会话
    if (fork() > 0)
        exit(0);

    // 3.创建新会话
    setsid();

    // 4.是否需要更改工作目录
    if (ischdir)
    {
        chdir(work_dir.c_str());
    }

    // 5.是否需要重定向012
    if (isclose)
    {
        ::close(0);
        ::close(1);
        ::close(2);
    }
    else
    {
        int fd = open(path.c_str(), O_RDONLY);
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);
    }
    return true;
}