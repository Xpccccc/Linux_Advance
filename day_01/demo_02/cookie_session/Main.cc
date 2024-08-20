#include <iostream>
#include <memory>
#include "TcpServer.hpp"
#include "Http.hpp"

std::shared_ptr<HttpResponse> Login(std::shared_ptr<HttpRequest> reqptr)
{
    LOG(DEBUG, "sep:==============================");
    std::string userdata;
    // 登录逻辑
    if (strcasecmp(reqptr->Method().c_str(), "get") == 0)
    {
        // get方法
        userdata = reqptr->Args();
    }
    else if (strcasecmp(reqptr->Method().c_str(), "post") == 0)
    {
        // post 方法
        userdata = reqptr->ReqText();
    }
    else
    {
    }

    LOG(DEBUG, "get data : %s", userdata.c_str());

    // 1.进程间通信 把数据交给子进程 pipe / 环境变量
    // 2.fork()
    // 3.exec() -- python/C++/Java/php

    // 这里也可以把数据交给数据库进行验证

    // 处理数据
    Factory fac;
    auto resptr = fac.BuildResponse();
    resptr->ADDStatusLine(200, "OK");
    // std::cout << "text404" << text404 << std::endl;
    std::string text = "<html><h1>haha,no content!</h1></html>";
    resptr->ADDHeader("Content-Type", "text/html");
    resptr->ADDHeader("Content-Length", std::to_string(text.size()));
    resptr->ADDText(text);

    LOG(DEBUG, "sep:==============================");

    return resptr;
}

void Usage()
{
    // printf("./udp_server serverip serverport\n");
    printf("Usage : ./udp_server serverport\n"); // ip 已经设置为0
}

int main(int argc, char *argv[])
{
    // if (argc != 3)
    if (argc != 2)
    {
        Usage();
        exit(USAGE_ERROR);
    }

    uint16_t serverport = std::stoi(argv[1]);

    HttpService httpservice;

    httpservice.AddHandler("login", Login);
    // httpservice.AddHandler("login.html", Login);
    // httpservice.AddHandler("login.html", Login);

    TcpServer tcpserver(serverport, std::bind(&HttpService::HandlerHttpService, &httpservice, placeholders::_1));
    tcpserver.Start();
    return 0;
}