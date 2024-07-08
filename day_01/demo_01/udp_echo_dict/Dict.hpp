#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <fstream>
#include <unordered_map>

#include "Log.hpp"

const std::string dict_path = "./dict.txt";
const std::string sep = ": ";

class Dict
{
private:
    bool Load()
    {
        std::ifstream in(_file_path);
        if (!in.is_open())
        {
            LOG(FATAL, "open %s error", _file_path.c_str());
            return false;
        }
        std::string line;
        while (std::getline(in, line))
        {
            auto pos = line.find(sep);
            if (pos == std::string::npos)
                continue;
            std::string english = line.substr(0, pos);
            std::string chinese = line.substr(pos + sep.size()); // abc: cc -- pos:3
            LOG(DEBUG, "add %s : %s success", english.c_str(), chinese.c_str());
            _dict[english] = chinese;
        }
        LOG(DEBUG, "Load %s success", _file_path.c_str());
        return true;
    }

public:
    Dict(const std::string file_path = dict_path) : _file_path(file_path)
    {
        Load(); // 加载文件到内存
    }
    std::string Translate(std::string &str, bool *ok)
    {
        *ok = false;
        for (auto &e : _dict)
        {
            if (e.first == str)
            {
                *ok = true;
                return e.second;
            }
        }

        return "未找到";
    }
    ~Dict() {}

private:
    const std::string _file_path;
    std::unordered_map<std::string, std::string> _dict;
};