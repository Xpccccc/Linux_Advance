#include <iostream>
#include <memory>
#include "TcpServer.hpp"
#include "Protocol.hpp"
#include "Calculate.hpp"

using namespace protocol_ns;

using cal_t = std::function<std::unique_ptr<Response>(const Request &req)>;

void Usage()
{
    // printf("./udp_server serverip serverport\n");
    printf("Usage : ./udp_server serverport\n"); // ip 已经设置为0
}

class Service
{
public:
    Service(cal_t cb) : _cb(cb) {}
    void AddService(socket_sptr sockfd, InetAddr client)
    {
        // TCP是字节流（可以使用write和read接口），UDP是数据报
        std::string clientaddr = CombineIpAndPort(client);
        std::string recvmessage;

        while (true)
        {
            sleep(5);

            Request req; // 注意多线程问题，不能放在里面while

            // 1.接收数据
            int n = sockfd->Recv(&recvmessage);
            std::cout << "server recv:" << recvmessage << std::endl;

            if (n <= 0)
            {
                LOG(INFO, "client %s quit", clientaddr.c_str());
                break;
            }
            // 2.分析数据，确定完整报文
            std::string package;
            while (true)
            {
                package = Decode(recvmessage); // 可能为空
                if (package.empty())
                    break;
                cout << "after Decode recvmessage : " << recvmessage << std::endl;

                // 完整的一条有效数据
                std::cout << "server Decode:" << package << std::endl;

                // 3.反序列化
                req.DeSerialize(package); // 把_x,_y,_oper赋值

                // 4.业务处理
                std::unique_ptr<Response> resptr = _cb(req);

                // 5.序列化
                std::string sendmessage;
                resptr->Serialize(&sendmessage);
                std::cout << "server Serialize:" << sendmessage << std::endl;

                // 6.加上报头数据封装
                sendmessage = Encode(sendmessage);
                std::cout << "server Encode:" << sendmessage << std::endl;

                // 7.发送数据
                int n = sockfd->Send(sendmessage);
            }
        }
    }
    ~Service() {}

private:
    cal_t _cb;
};

int main(int argc, char *argv[])
{
    // if (argc != 3)
    if (argc != 2)
    {
        Usage();
        exit(USAGE_ERROR);
    }

    uint16_t serverport = std::stoi(argv[1]);

    // __nochdir = 1:在当前工作目录执行
    // __nochdir = 0:在根目录/工作目录执行
    // __noclose = 1:不进行重定向
    // __noclose = 0:进行重定向 /dev/null
    // int daemon(int __nochdir, int __noclose)
    // if(fork > 0) exit(0);
    // setsid();
    // 先创建子进程，再父进程退出，因为组长不能直接调用setsid();
    daemon(0, 0);
    // 执行下面的代码不是当前进程，而是当前进程的子进程

    EnableFile();

    Calculate cal; // 应用层
    cal_t servercal = std::bind(&Calculate::Execute, &cal, placeholders::_1);
    Service sev(servercal);
    service_t service = std::bind(&Service::AddService, &sev, placeholders::_1, placeholders::_2); // 表示层
    std::unique_ptr<TcpServer> tsvr = std::make_unique<TcpServer>(serverport, service);            //  会话层
    tsvr->Start();

    return 0;
}