#pragma once

#include <string>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <string>
#include <unistd.h>

// #define SELF 1; // SELF=1就用自定义的序列化和反序列化，否则用默认的

// 表示层
namespace protocol_ns
{
    const std::string SEP = "\r\n";
    const std::string CAL_SEP = " ";

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
#ifdef SELF
            // "len\r\nx op y\r\n" -- 自定义序列化和反序列化
            std::string data_x = std::to_string(_x);
            std::string data_y = std::to_string(_y);
            *out = data_x + CAL_SEP + _oper + CAL_SEP + data_y;
#else
            Json::Value root;
            root["x"] = _x;
            root["y"] = _y;
            root["oper"] = _oper;

            Json::FastWriter writer;
            std::string str = writer.write(root);

            *out = str;
#endif
        }

        // 反序列化 -- 解析
        bool DeSerialize(const std::string &in)
        {
#ifdef SELF
            auto left_blank_pos = in.find(CAL_SEP);
            if (left_blank_pos == std::string::npos)
                return false;
            std::string x_str = in.substr(0, left_blank_pos);
            if (x_str.empty())
                return false;
            auto right_blank_pos = in.rfind(CAL_SEP);

            if (right_blank_pos == std::string::npos)
                return false;
            std::string y_str = in.substr(right_blank_pos + 1);
            if (y_str.empty())
                return false;
            if (left_blank_pos + 1 + CAL_SEP.size() != right_blank_pos)
                return false;

            _x = std::stoi(x_str);
            _y = std::stoi(y_str);
            _oper = in[right_blank_pos - 1];
            return true;

#else
            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(in, root))
                return false;
            _x = root["x"].asInt();
            _y = root["y"].asInt();
            _oper = root["oper"].asInt();
            return true;
#endif
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
#ifdef SELF
            // "len\r\nresult flag equation\r\n"
            std::string data_res = std::to_string(_result);
            std::string data_flag = std::to_string(_flag);
            *out = data_res + CAL_SEP + data_flag + CAL_SEP + _equation;
#else
            Json::Value root;
            root["result"] = _result;
            root["flag"] = _flag;
            root["equation"] = _equation;

            Json::FastWriter writer;
            std::string str = writer.write(root);

            *out = str;
#endif
        }

        // 反序列化 -- 解析
        bool DeSerialize(const std::string &in)
        {
#ifdef SELF
            // "result flag equation"

            auto left_blank_pos = in.find(CAL_SEP);
            if (left_blank_pos == std::string::npos)
                return false;
            std::string res_str = in.substr(0, left_blank_pos);
            if (res_str.empty())
                return false;

            auto second_blank_pos = in.find(CAL_SEP, left_blank_pos + 1);
            if (second_blank_pos == std::string::npos)
                return false;
            std::string equation = in.substr(second_blank_pos + 1);
            if (equation.empty())
                return false;

            if (left_blank_pos + 1 + CAL_SEP.size() != second_blank_pos)
                return false;
            _result = std::stoi(res_str);
            _flag = in[second_blank_pos - 1] - '0';
            _equation = equation;
            return true;
#else
            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(in, root))
                return false;
            _result = root["result"].asInt();
            _flag = root["flag"].asInt();
            _equation = root["equation"].asString();
            return true;
#endif
        }
        ~Response() {}

    public:
        int _result = 0;
        int _flag = 0;                              // 0表示操作符正确，1表示除0错误，2表示取模0错误，3表示操作符错误
        std::string _equation = "操作符不符合要求"; // 等式
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
            // req._y = 0; // 测试
            usleep(req._x * req._y + 20);
            req._oper = opers[(rand() % opers.size())];
        }
        ~CalFactory() {}

    private:
    };
}
