#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>

bool isSave = false; // 默认向显示器打印
#define FILEPATH "./log.txt"

enum level
{
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

void SaveToFile(const string &message)
{
    ofstream out(FILEPATH, ios_base::app);
    if (!out.is_open())
        return;
    out << message;
    out.close();
}

std::string LevelToString(int level)
{
    switch (level)
    {
    case DEBUG:
        return "Debug";
    case INFO:
        return "Info";
    case WARNING:
        return "Warning";
    case ERROR:
        return "Error";
    case FATAL:
        return "Fatal";
    default:
        return "Unknow";
    }
}

std::string GetTimeString()
{
    time_t curr_time = time(nullptr);
    struct tm *format_time = localtime(&curr_time);
    if (format_time == nullptr)
        return "None";
    char buff[1024];
    snprintf(buff, sizeof(buff), "%d-%d-%d %d:%d:%d",
             format_time->tm_year + 1900,
             format_time->tm_mon + 1,
             format_time->tm_mday,
             format_time->tm_hour,
             format_time->tm_min,
             format_time->tm_sec);
    return buff;
}

void LogMessage(const std::string filename, int line, bool issave, int level, const char *format, ...)
{
    std::string levelstr = LevelToString(level);
    std::string timestr = GetTimeString();
    pid_t pid = getpid();

    char buff[1024];
    va_list arg;
    // int vsnprintf(char *str, size_t size, const char *format, va_list ap); // 使用可变参数
    va_start(arg, format);
    vsnprintf(buff, sizeof(buff), format, arg);
    va_end(arg);

    std::string message = "[" + timestr + "]" + "[" + levelstr + "]" + "[pid:" + std::to_string(pid) + "]" + "[" + filename + "]" + "[" + std::to_string(line) + "] " + buff + '\n';
    if (issave == false)
        std::cout << message;
    else
        SaveToFile(message);
}

// 固定文件名和行数
#define LOG(level, format, ...)                                               \
    do                                                                        \
    {                                                                         \
        LogMessage(__FILE__, __LINE__, isSave, level, format, ##__VA_ARGS__); \
    } while (0)

#define EnableScreen()  \
    do                  \
    {                   \
        isSave = false; \
    } while (0)

#define EnableFile()   \
    do                 \
    {                  \
        isSave = true; \
    } while (0)

void Test(int num, ...)
{
    va_list arg;
    va_start(arg, num);
    while (num--)
    {
        int data = va_arg(arg, int);
        std::cout << data << " ";
    }
    std::cout << std::endl;
    va_end(arg);
}