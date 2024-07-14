#pragma once

#include <string>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>

namespace protocol_ns
{
    const std::string SEP = "\r\n";

    // 对发送数据进行封装
    // "len\r\n{有效载荷}\r\n" -- 其中len是有效载荷的长度
    std::string Encode(const std::string &inbuff)
    {
        int inbuff_len = inbuff.size();
        std::string newstr = std::to_string(inbuff_len);
        newstr += SEP;
        newstr += inbuff;
        newstr += SEP;
        return newstr;
    }

    // 解析字符串
    std::string Decode(std::string &outbuff)
    {
        int pos = outbuff.find(SEP);
        if (pos == std::string::npos)
        {
            // 没找到分隔符
            return std::string(); // 返回空串，等待接收到完整数据
        }
        // 找到分隔符
        std::string len_str = outbuff.substr(0, pos);
        if (len_str.empty())
            return std::string(); // 返回空串，等待接收到完整数据
        int data_len = std::stoi(len_str);
        // 判断长度是否符合要求
        int total_len = pos + SEP.size() * 2 + data_len; // 包装好的一条数据的长度
        if (outbuff.size() < total_len)
        {
            return std::string(); // 小于包装好的一条数据的长度，返回空串，等待接收到完整数据
        }
        // 大于等于包装好的一条数据的长度
        std::string message = outbuff.substr(pos + SEP.size(), data_len); // 有效数据
        outbuff.erase(0, total_len);                                      // 数据长度减少包装好的一条数据的长度，从前面开始移除
        return message;
    }

    class Request
    {
    public:
        Request() {}
        Request(int x, int y, char oper)
            : _x(x),
              _y(y),
              _oper(oper)
        {
        }

        // 序列化 -- 转化为字符串发送
        // {"x":_x,"y":_y,"oper":_oper}
        // 这样发送可以吗？不行，不一定一次到达的数据刚好是1条，可能是半条，也可能是2条，因此我们需要对发送的数据进行封装：
        // "len\r\n{有效载荷}\r\n" -- 其中len是有效载荷的长度
        void Serialize(std::string *out) // 要带出来
        {
            Json::Value root;
            root["x"] = _x;
            root["y"] = _y;
            root["oper"] = _oper;

            Json::FastWriter writer;
            std::string str = writer.write(root);

            *out = str;
        }

        // 反序列化 -- 解析
        bool DeSerialize(const std::string &in)
        {
            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(in, root))
                return false;
            _x = root["x"].asInt();
            _y = root["y"].asInt();
            _oper = root["oper"].asInt();
            return true;
        }
        ~Request() {}

    public:
        int _x;
        int _y;
        char _oper; // +-*/% 如果不是这些操作法那就是非法的
    };

    class Response
    {
    public:
        Response() {}
        // 序列化 -- 转化为字符串发送
        void Serialize(std::string *out) // 要带出来
        {
            Json::Value root;
            root["result"] = _result;
            root["flag"] = _flag;

            Json::FastWriter writer;
            std::string str = writer.write(root);

            *out = str;
        }

        // 反序列化 -- 解析
        bool DeSerialize(const std::string &in)
        {
            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(in, root))
                return false;
            _result = root["result"].asInt();
            _flag = root["flag"].asInt();
            return true;
        }
        ~Response() {}

    public:
        int _result = 0;
        int _flag = 0; // 0表示操作符正确，1表示除0错误，2表示取模0错误，3表示操作符错误
    };

    const std::string opers = "+-*/%&^";

    class CalFactory
    {
    public:
        CalFactory()
        {
            srand(time(nullptr) ^ getpid() ^ 2);
        }
        void Product(Request &req)
        {
            req._x = rand() & 5 + 1;
            usleep(req._x * 20);
            req._y = rand() % 10 + 5;
            usleep(req._x * req._y + 20);
            req._oper = opers[(rand() % opers.size())];
        }
        ~CalFactory() {}

    private:
    };
}
