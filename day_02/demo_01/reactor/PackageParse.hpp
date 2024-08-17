#pragma once
#include "Connection.hpp"
#include "Protocol.hpp"
#include "Calculate.hpp"

using namespace protocol_ns;
class PackageParse
{
public:
    static void Parse(Connection *conn)
    {
        // 对数据进行解析
        // LOG(DEBUG, "inbuff : %s" , conn->Inbuffer().c_str());
        std::string package;
        Request req;
        Calculate cal;
        while (true)
        {
            package = Decode(conn->Inbuffer()); // 可能为空
            if (package.empty())
                break;
            cout << "after Decode recvmessage : " << conn->Inbuffer() << std::endl;

            // 完整的一条有效数据
            std::cout << "server Decode:" << package << std::endl;

            // 3.反序列化
            req.DeSerialize(package); // 把_x,_y,_oper赋值

            // 4.业务处理
            std::unique_ptr<Response> resptr = cal.Execute(req);

            // 5.序列化
            std::string sendmessage;
            resptr->Serialize(&sendmessage);
            std::cout << "server Serialize:" << sendmessage << std::endl;

            // 6.加上报头数据封装
            sendmessage = Encode(sendmessage);
            std::cout << "server Encode:" << sendmessage << std::endl;

            // // 7.发送数据
            // int n = sockfd->Send(sendmessage);

            // 把解析的数据放到Outbuffer -- Outbuffer并不是内核的输出缓冲区！
            conn->AppendOutbuff(sendmessage);
        }

        // 到这里，说明解析完成
        if (!conn->Outbuffer().empty())
        {
            conn->_sender(conn);
        }
    }
};