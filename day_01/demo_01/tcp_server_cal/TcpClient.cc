#include <iostream>
#include <string>
#include <strings.h>
#include <unistd.h>

#include "Comm.hpp"
#include "Socket.hpp"
#include "Protocol.hpp"

using namespace socket_ns;
using namespace protocol_ns;

enum class Status
{
    NEW,
    CONNECTED,
    CONNECTING,
    DISCONNECTED,
    CLOSE
};

const int defaultsockfd = -1;
const int retryinterval = 1; // 重连间隔时间
const int retryamxtimes = 5; // 重连最大次数

class Connection
{
public:
    Connection(std::string serverip, uint16_t serverport)
        : _sockfdptr(std::make_unique<TcpSocket>()),
          _serverip(serverip),
          _serverport(serverport),
          _status(Status::NEW),
          _retry_interval(retryinterval),
          _retry_max_times(retryamxtimes)
    {
    }

    Status ConnectStatus()
    {
        return _status;
    }

    void Connect()
    {
        InetAddr server(_serverip, _serverport);
        bool ret = _sockfdptr->BuildClientSocket(server);
        if (!ret)
        {
            DisConnect(); //
            _status = Status::DISCONNECTED;
            return;
        }
        std::cout << "connect success" << std::endl;
        _status = Status::CONNECTED; // 已连接
    }

    void Process()
    {
        while (true)
        {
            // std::cout << "Please Enter#  ";
            // std::string sendmessage;// 不是{"": ,}类型
            // std::getline(std::cin, sendmessage);

            sleep(1);
            Request req;
            CalFactory cal;
            // 1.对需要发送的数据进行序列化
            std::string sendmessage;

            // 1.1.这里一下构建5个请求并放在一起
            // for (int i = 0; i < 5; ++i)
            // {
            //     cal.Product(req);
            //     std::string sub_sendstr;
            //     req.Serialize(&sub_sendstr);
            //     std::cout << "client Serialize:" << sub_sendstr << std::endl;
            //     // 2.对序列化后的数据进行加报头等打包
            //     sub_sendstr = Encode(sub_sendstr);
            //     std::cout << "client Encode:" << sub_sendstr << std::endl;
            //     sendmessage += sub_sendstr;
            // }

            cal.Product(req);
            req.Serialize(&sendmessage);
            std::cout << "client Serialize:" << sendmessage << std::endl;
            // 2.对序列化后的数据进行加报头等打包
            sendmessage = Encode(sendmessage);
            std::cout << "client Encode:" << sendmessage << std::endl;

            // std::cout << "sendmessage : " << sendmessage << std::endl;
            // 3.发送数据
            int n = _sockfdptr->Send(sendmessage);
            if (n < 0)
            {
                _status = Status::CLOSE; // 发送不成功就退出
                LOG(FATAL, "send error, errno : %d ,error string : %s", errno, strerror(errno));
                break;
            }
            // 发送成功
            std::string recvmessage;
            // 4.接收数据
            int m = _sockfdptr->Recv(&recvmessage);
            if (m <= 0)
            {
                _status = Status::DISCONNECTED; // 接收不成功就重连
                std::cerr << "recv error" << std::endl;
                break;
            }
            // 接收成功
            // 5.分析数据，确定完整报文
            std::string package;
            while (true)
            {
                package = Decode(recvmessage); // 可能为空
                if (package.empty())
                    break;
                // 完整的一条有效数据
                Response resp;
                // 6.反序列化
                resp.DeSerialize(package); // 把_result,_flag赋值
                // 7.处理返回数据
                std::cout << "Server Echo$ " << "result : " << resp._result << " , flag :" << resp._flag << " --- equation : " << resp._equation << std::endl;
            }
        }
    }

    void ReConnect()
    {
        _status = Status::CONNECTING;
        int cnt = 1;
        while (true)
        {
            Connect();
            if (_status == Status::CONNECTED)
            {
                break;
            }
            std::cout << "正在重连，重连次数 : " << cnt++ << std::endl;
            if (cnt > _retry_max_times)
            {
                _status = Status::CLOSE; // 重连失败
                std::cout << "重连失败，请检查网络.." << std::endl;
                break;
            }
            sleep(_retry_interval);
        }
    }

    void DisConnect()
    {
        if (_sockfdptr->SockFd() > defaultsockfd)
        {
            close(_sockfdptr->SockFd());
        }
    }

private:
    std::unique_ptr<Socket> _sockfdptr;
    std::string _serverip;
    uint16_t _serverport;
    Status _status;
    int _retry_interval;
    int _retry_max_times;
};

class TcpClient
{
public:
    TcpClient(std::string serverip, uint16_t serverport) : _connect(serverip, serverport)
    {
    }
    void Execute()
    {
        while (true)
        {
            switch (_connect.ConnectStatus())
            {
            case Status::NEW:
                _connect.Connect();
                break;
            case Status::CONNECTED:
                _connect.Process();
                break;
            case Status::DISCONNECTED:
                _connect.ReConnect();
                break;
            case Status::CLOSE:
                _connect.DisConnect();
                return; // 断开连接了，重连不管用了
            default:
                break;
            }
        }
    }

private:
    Connection _connect;
};

void Usage()
{
    std::cout << "Please use format : ./tcp_client serverip serverport" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        Usage();
        exit(USAGE_ERROR);
    }

    std::string serverip = argv[1];
    uint16_t serverport = std::stoi(argv[2]);

    TcpClient tcpclient(serverip, serverport);
    tcpclient.Execute();
    return 0;
}

// #include <iostream>
// #include <string>
// #include <strings.h>
// #include <unistd.h>

// #include "Comm.hpp"

// enum class Status
// {
//     NEW,
//     CONNECTED,
//     CONNECTING,
//     DISCONNECTED,
//     CLOSE
// };

// const int defaultsockfd = -1;
// const int retryinterval = 1; // 重连间隔时间
// const int retryamxtimes = 5; // 重连最大次数

// class Connection
// {
// public:
//     Connection(std::string serverip, uint16_t serverport)
//         : _sockfd(defaultsockfd),
//           _serverip(serverip),
//           _serverport(serverport),
//           _status(Status::NEW),
//           _retry_interval(retryinterval),
//           _retry_max_times(retryamxtimes)
//     {
//     }

//     Status ConnectStatus()
//     {
//         return _status;
//     }

//     void Connect()
//     {
//         _sockfd = socket(AF_INET, SOCK_STREAM, 0);
//         if (_sockfd < 0)
//         {
//             _status = Status::DISCONNECTED;
//             exit(CREATE_ERROR);
//         }
//         struct sockaddr_in server;
//         bzero(&server, sizeof(server));
//         server.sin_family = AF_INET;
//         server.sin_port = htons(_serverport);

//         // int inet_pton(int af, const char *src, void *dst);
//         inet_pton(AF_INET, _serverip.c_str(), &server.sin_addr.s_addr); // 也可以&server.sin_addr，因为sin_addr只有s_addr
//         int n = ::connect(_sockfd, CONV(&server), sizeof(server));
//         if (n < 0)
//         {
//             DisConnect(); //
//             _status = Status::DISCONNECTED;
//             return;
//         }
//         std::cout << "connect success" << std::endl;
//         _status = Status::CONNECTED; // 已连接
//     }

//     void Process()
//     {
//         while (true)
//         {
//             // 发送数据
//             std::string message;
//             std::cout << "Please Enter# ";
//             std::getline(std::cin, message);
//             int n = send(_sockfd, message.c_str(), message.size(), 0);
//             if (n > 0)
//             {
//                 // 接收数据
//                 char buff[1024];
//                 int m = recv(_sockfd, buff, sizeof(buff) - 0, 0);
//                 if (m > 0)
//                 {
//                     buff[m] = 0;
//                     std::cout << "Server Echo$ " << buff << std::endl;
//                 }
//                 else
//                 {
//                     _status = Status::DISCONNECTED; // 接收不成功就重连
//                     std::cerr << "recv error" << std::endl;
//                     break;
//                 }
//             }
//             else
//             {
//                 _status = Status::CLOSE; // 发送不成功就退出
//                 std::cerr << "send error" << std::endl;
//                 break;
//             }
//         }
//     }

//     void ReConnect()
//     {
//         _status = Status::CONNECTING;
//         int cnt = 1;
//         while (true)
//         {
//             Connect();
//             if (_status == Status::CONNECTED)
//             {
//                 break;
//             }
//             std::cout << "正在重连，重连次数 : " << cnt++ << std::endl;
//             if (cnt > _retry_max_times)
//             {
//                 _status = Status::CLOSE; // 重连失败
//                 std::cout << "重连失败，请检查网络.." << std::endl;
//                 break;
//             }
//             sleep(_retry_interval);
//         }
//     }

//     void DisConnect()
//     {
//         if (_sockfd > defaultsockfd)
//         {
//             close(_sockfd);
//             _sockfd = defaultsockfd;
//         }
//     }

// private:
//     int _sockfd;
//     std::string _serverip;
//     uint16_t _serverport;
//     Status _status;
//     int _retry_interval;
//     int _retry_max_times;
// };

// class TcpClient
// {
// public:
//     TcpClient(std::string serverip, uint16_t serverport) : _connect(serverip, serverport)
//     {
//     }
//     void Execute()
//     {
//         while (true)
//         {
//             switch (_connect.ConnectStatus())
//             {
//             case Status::NEW:
//                 _connect.Connect();
//                 break;
//             case Status::CONNECTED:
//                 _connect.Process();
//                 break;
//             case Status::DISCONNECTED:
//                 _connect.ReConnect();
//                 break;
//             case Status::CLOSE:
//                 _connect.DisConnect();
//                 return; // 断开连接了，重连不管用了
//             default:
//                 break;
//             }
//         }
//     }

// private:
//     Connection _connect;
// };

// void Usage()
// {
//     std::cout << "Please use format : ./tcp_client serverip serverport" << std::endl;
// }

// int main(int argc, char *argv[])
// {
//     if (argc != 3)
//     {
//         Usage();
//         exit(USAGE_ERROR);
//     }

//     std::string serverip = argv[1];
//     uint16_t serverport = std::stoi(argv[2]);

//     TcpClient tcpclient(serverip, serverport);
//     tcpclient.Execute();
//     return 0;
// }