#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "Session.hpp"

const std::string SEP = "\r\n";
const std::string REQ_HEADER_SEP = ": ";
const std::string wwwroot = "wwwroot";
const std::string homepage = "index.html";
const std::string httpverison = "HTTP/1.0";
const std::string SPACE = " ";
const std::string CONF_MIME_TYPE_FILE = "conf/Mime_Type.txt";
const std::string CONF_STATUS_CODE_DESC_FILE = "conf/Status_Code_Desc.txt";
const std::string CONF_SEP = ": ";
const std::string REQ_PATH_SUFFIX_SEP = ".";
const std::string errorpage = "404.html";
const std::string URI_SEP = "?";
const std::string PATH_SEP = "/";

class HttpResponse;
class HttpRequest;
using func_t = std::function<std::shared_ptr<HttpResponse>(std::shared_ptr<HttpRequest>)>;

class HttpRequest
{
    // GET / HTTP/1.1
    // Host: 129.204.171.204:8888
    // User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/15.6.1 Safari/605.1.15
    // Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
    // Accept-Encoding: gzip, deflate
    // Accept-Language: zh-CN,zh-Hans;q=0.9
    // Upgrade-Insecure-Requests: 1
    // 空行
    // 请求正文

private:
    std::string GetOneLine(std::string &reqstr)
    {
        if (reqstr.empty())
            return std::string();
        auto pos = reqstr.find(SEP);
        if (pos == std::string::npos)
            return std::string();
        std::string line = reqstr.substr(0, pos); // 读到空行就是substr(0, 0)，就是空
        // 删除该行，准备读取下一行
        reqstr.erase(0, pos + SEP.size());
        // 读取成功
        return line.empty() ? SEP : line;
    }

public:
    HttpRequest() : _blank_line(SEP), _path(wwwroot) {}
    bool DeSerialize(std::string &reqstr)
    {
        // 先读取第一行，是请求行
        _req_line = GetOneLine(reqstr);
        while (true)
        {
            // 这里读取请求报头行
            std::string line = GetOneLine(reqstr);
            if (line.empty())
            {
                break; // 读取不成功
            }
            else if (line == SEP)
            {
                // 读到空行
                _req_text = reqstr; // 剩下的都是正文
                break;
            }
            else
            {
                _req_header.emplace_back(line);
            }
        }
        ParseReqLine();
        ParseReqHeader();
        return true;
    }

    bool ParseReqLine()
    {
        if (_req_line.empty())
            return false;
        std::stringstream ss(_req_line); // 按空格把数据分开
        ss >> _req_method >> _uri >> _version;

        // http://129.204.171.204:8888/index.html?username=xp&password=12354
        if (strcasecmp(_req_method.c_str(), "get") == 0) // 如果请求方法是get，那可能需要分离uri，因为提交参数会放到uri中
        {
            auto pos = _uri.find(URI_SEP);
            if (pos != std::string::npos)
            {
                _args = _uri.substr(pos + 1);
                _uri.resize(pos); // 只要前面那部分
            }
            else
            {
                _args = std::string(); // 没有提交参数
            }
            LOG(INFO, "uri: %s, args : %s", _uri.c_str(), _args.c_str());
        }
        else if (strcasecmp(_req_method.c_str(), "post") == 0)
        {
        }
        _path += _uri;

        if (_path[_path.size() - 1] == '/')
        {
            _path += homepage;
        }

        auto pos = _path.rfind(REQ_PATH_SUFFIX_SEP); // 找后缀
        if (pos == std::string::npos)
        {
            return false;
        }
        _suffix = _path.substr(pos);
        return true;
    }

    std::string Path()
    {
        return _path;
    }

    std::string Suffix()
    {
        return _suffix;
    }

    std::string Method()
    {
        return _req_method;
    }

    std::string Args()
    {
        return _args;
    }

    std::string ReqText()
    {
        return _req_text;
    }

    bool ParseReqHeader()
    {
        // 这里注意：这是服务器的Request！在服务器发送了响应报头Set-Cookie: sessionid=1234后，客户端（浏览器）会对其进行处理，下一次请求就会带上Cookie: sessionid=1234
        std::string prefix = "Cookie";
        for (auto header : _req_header)
        {
            auto pos = header.find(REQ_HEADER_SEP);
            if (pos == std::string::npos)
                return false;
            _headerskv[header.substr(0, pos)] = header.substr(pos + REQ_HEADER_SEP.size()); // 添加成功
            if (header.substr(0, pos) == prefix) // 添加Cookie到vector中
            {
                _sessionid = header.substr(pos + REQ_HEADER_SEP.size());
                _cookies.push_back(_sessionid);
            }
        }
        return true;
    }

    std::string &SessionId()
    { // 这里只测试1个的情况，如果是多个，就返回vector
        return _sessionid;
    }

    bool IsExec()
    {
        return !_args.empty() || !_req_text.empty(); // 只要有提交参数或者有正文数据（post会把提交参数放正文）就执行回调函数（所以这个成员函数必须在AddText前执行，防止正文数据混淆）
    }

    void DebugPrint()
    {
        std::cout << "request: " << std::endl;
        std::cout << "$$$ " << _req_line << std::endl;
        for (auto &header : _req_header)
        {
            std::cout << "### " << header << std::endl;
        }
        std::cout << "*** " << _blank_line << std::endl;
        std::cout << "@@@ " << _req_text << std::endl;

        // std::cout << "---------------------" << std::endl;
        // std::cout << "$$$ " << _req_method << " " << _uri << " " << _version << " " << _path << std::endl;
        // for (auto &header : _headerskv)
        // {
        //     std::cout << "### " << header.first << "--" << header.second << std::endl;
        // }
        // std::cout << "*** " << _blank_line << std::endl;
        // std::cout << "@@@ " << _req_text << std::endl;
    }
    ~HttpRequest() {}

private:
    std::string _req_line;                // 请求行
    std::vector<std::string> _req_header; // 请求报头
    std::string _blank_line;              // 空行
    std::string _req_text;                // 请求正文

    // 细分
    std::string _req_method;                                 // 请求行的请求方法
    std::string _uri;                                        // 请求行的uri
    std::string _args;                                       // uri分离出来的提交参数（比如账号密码）
    std::string _path;                                       // 请求路径
    std::string _suffix;                                     // 请求后缀
    std::string _version;                                    // 请求行的版本
    std::unordered_map<std::string, std::string> _headerskv; // 请求报头的kv结构

    std::vector<std::string> _cookies; // 其实cookie可以有多个，因为Set-Cookie可以被写多条，测试，一条够了。
    std::string _sessionid;            // 请求携带的sessionid，仅仅用来测试
};

class HttpResponse
{
public:
    HttpResponse() : _version(httpverison), _blank_line(SEP) {}

    void ADDStatusLine(int status_code, std::string desc)
    {
        _status_code = status_code;
        // _desc = "OK";
        _desc = desc;
        _status_line = _version + SPACE + std::to_string(_status_code) + SPACE + _desc + SEP;
    }

    void ADDHeader(const std::string &k, const std::string &v)
    {
        _headerskv[k] = v;
    }
    void ADDText(std::string text)
    {
        _resp_text = text;
    }

    void Serialize(std::string *out)
    {
        std::string status_line = _version + SPACE + std::to_string(_status_code) + SPACE + _desc + SEP;
        for (auto &header : _headerskv)
        {
            _resp_header.emplace_back(header.first + REQ_HEADER_SEP + header.second);
        }

        // 序列化
        std::string resptr;
        resptr = status_line;
        for (auto &header : _resp_header)
        {
            resptr += header + SEP;
        }
        resptr += _blank_line;
        resptr += _resp_text;
        *out = resptr;
    }

    void DebugPrint()
    {
        std::cout << "response: " << std::endl;
        std::cout << "$$$ " << _status_line;
        for (auto &header : _resp_header)
        {
            std::cout << "### " << header << std::endl;
        }
        std::cout << "*** " << _blank_line << std::endl;
        // std::cout << "@@@ " << "_resp_text" << std::endl; // 正文是二进制
    }

    ~HttpResponse() {}

private:
    // 构建应答的必要字段
    int _status_code;     // 状态码
    std::string _desc;    // 状态描述
    std::string _version; // 版本

    std::string _status_line;              // 状态行
    std::vector<std::string> _resp_header; // 响应报头
    std::string _blank_line;               // 空行
    std::string _resp_text;                // 响应正文

    std::unordered_map<std::string, std::string> _headerskv; // 响应报头的kv结构
};

class Factory
{
public:
    // static?
    std::shared_ptr<HttpRequest> BuildRequest()
    {
        return std::make_shared<HttpRequest>();
    }

    std::shared_ptr<HttpResponse> BuildResponse()
    {
        return std::make_shared<HttpResponse>();
    }

private:
};

class HttpService
{
private:
    bool LoadMimeType()
    {
        std::ifstream in(CONF_MIME_TYPE_FILE); // 默认是读
        if (!in.is_open())
        {
            std::cerr << "load mime error" << std::endl;
            return false;
        }
        std::string line;
        while (std::getline(in, line))
        {
            auto pos = line.find(CONF_SEP);
            if (pos == std::string::npos)
            {
                std::cerr << "find CONF_SEP error" << std::endl;
                return false;
            }
            _mime_type[line.substr(0, pos)] = line.substr(pos + CONF_SEP.size());
        }
        return true;
    }

    bool LoadStatusDesc()
    {
        std::ifstream in(CONF_STATUS_CODE_DESC_FILE); // 默认是读
        if (!in.is_open())
        {
            std::cerr << "load status desc error" << std::endl;
            return false;
        }
        std::string line;
        while (std::getline(in, line))
        {
            auto pos = line.find(CONF_SEP);
            if (pos == std::string::npos)
            {
                std::cerr << "find CONF_SEP error" << std::endl;
                return false;
            }
            _status_code_to_desc[std::stoi(line.substr(0, pos))] = line.substr(pos + CONF_SEP.size());
        }
        return true;
    }

public:
    HttpService()
    {
        LoadMimeType();
        // for (auto &e : _mime_type)
        // {
        //     std::cout << e.first << "--" << e.second << std::endl;
        // }
        // std::cout << "-----------------------" << std::endl;

        LoadStatusDesc();
        // for (auto &e : _status_to_desc)
        // {
        //     std::cout << e.first << "--" << e.second << std::endl;
        // }
    }

    std::string ReadFileRequest(const std::string &path, int *size)
    {
        // 要按二进制打开
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open())
        {
            std::cerr << "open file error" << std::endl;
            return std::string();
        }
        in.seekg(0, in.end);
        int filesize = in.tellg();
        in.seekg(0, in.beg);

        std::string content;
        content.resize(filesize);
        in.read((char *)content.c_str(), filesize);
        in.close();
        *size = filesize;
        return content;
    }

    void AddHandler(std::string functionname, func_t f)
    {
        // 添加方法处理 提交参数 的方法
        std::string fn = wwwroot + PATH_SEP + functionname; // wwwroot/方法名
        _funcs[fn] = f;
    }

    std::string GetMonName(int month)
    {
        std::vector<std::string> months = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        return months[month];
    }

    std::string GetWeekname(int day)
    {
        std::vector<std::string> weekdays = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        return weekdays[day];
    }

    std::string ExpireTimeUseRFC1123(int t) // t是秒级别
    {
        time_t timeout = time(nullptr) + t; // 当前时间加t
        struct tm *tm = gmtime(&timeout);   // 这里不能用 localtime，因为 localtime 是默认带了时区的. gmtime 获取的就是 UTC 统一时间
        char buff[1024];
        // 时间格式如: expires=Thu, 18 Dec 2024 12:00:00 UTC
        snprintf(buff, sizeof(buff), "%s, %02d %s %d %02d:%02d:%02d UTC",
                 GetWeekname(tm->tm_wday).c_str(), // wday -- 一周的第几天 ， mday -- 一月的第几天
                 tm->tm_mday,
                 GetMonName(tm->tm_mon).c_str(),
                 tm->tm_year + 1900,
                 tm->tm_hour,
                 tm->tm_min,
                 tm->tm_sec);
        return buff;
    }

    std::string ProveCookieWrite() // 证明 cookie 能被写入浏览器
    {
        return "username=xp;";
    }
    std::string ProveCookieTimeOut()
    {
        return "username=xp; expires=" + ExpireTimeUseRFC1123(2) + ";"; // 让 cookie 2秒 后过期
    }
    std::string ProvePath()
    {
        return "username=xp; path=/a/b;";
    }

    std::string ProveSession(const std::string &sessionid)
    {
        return sessionid + ";";
    }

    std::string HandlerHttpService(std::string &request)
    {
#ifdef HTTP
        std::cout << "-------------------------" << std::endl;
        std::cout << request;
        std::string response = "HTTP/1.0 200 OK\r\n"; // 404 NOT Found
        response += "\r\n";
        response += "<html><body><h1>hello ynu!</h1></body></html>";
        return response;
#else
        // std::cout << "request : " << request;

        // 对请求进行反序列化
        Factory fac; // 也可以给Build静态
        auto reqptr = fac.BuildRequest();
        reqptr->DeSerialize(request);
        reqptr->DebugPrint();
        // DebugPrint();

        std::string resource = reqptr->Path();
        std::string suffix = reqptr->Suffix();
        auto resptr = fac.BuildResponse();

        static int number = 0;

        // std::cout << "text :" << reqptr->ReqText() << std::endl;
        if (reqptr->IsExec())
        {
            // 提交参数了 --  相当于进入login了 --  用/login path向指定浏览器写入sessionid，并在服务器维护对应的session对象
            // 处理参数

            resptr = _funcs[resource](reqptr);
            
            std::string sessionid = reqptr->SessionId();
            if (sessionid.empty())
            {
                std::string user = "user-" + std::to_string(number++);
                Session *sptr = new Session(user, "logined");
                std::string sessionid = _session_manager.AddSession(sptr);
                resptr->ADDHeader("Set-Cookie", ProveSession(sessionid));
                LOG(DEBUG, "%s add done, sessionid is : %s ", user.c_str(), sessionid.c_str());
            }
        }
        else
        {
            // 当浏览器活跃在本站点任何路径中活跃，都会自动提交sessionid，我就知道谁活跃了
            std::string sessionid = reqptr->SessionId();
            if (!sessionid.empty())
            {
                Session *sptr = _session_manager.GetSession(sessionid);
                if (sptr != nullptr)
                {
                    LOG(DEBUG, "%s 正在活跃...", sptr->_username.c_str());
                }
                else
                {
                    LOG(DEBUG, "cookie : %s 已经过期, 需要清理\n", sessionid.c_str());
                }
            }

            // std::string newurl = "https://github.com/Xpccccc";
            std::string newurl = "http://129.204.171.204:8888/Image/2.png";
            int status_code = 0;

            // 对请求的地址进行判断
            if (resource == "wwwroot/redir")
            {
                cout << "test========================" << endl;
                status_code = 302;
                resptr->ADDStatusLine(status_code, _status_code_to_desc[status_code]);
                resptr->ADDHeader("Content-Type", _mime_type[suffix]);

                resptr->ADDHeader("Location", newurl);
            }
            else if (resource == "wwwroot/404.html")
            {
            }
            else
            {
                status_code = 200;
                int filesize = 0;
                // std::cout << "debug resource : " << resource << std::endl;

                std::string textcontent = ReadFileRequest(resource, &filesize); // 不存在文件会输出open file error
                // std::cout << "debug" << std::endl;
                if (textcontent.empty())
                {
                    // 空内容说明读到不存在的页面
                    status_code = 404;
                    resptr->ADDStatusLine(status_code, _status_code_to_desc[status_code]);
                    std::string text404 = ReadFileRequest(wwwroot + PATH_SEP + errorpage, &filesize);
                    // std::cout << "text404" << text404 << std::endl;
                    resptr->ADDHeader("Content-Length", std::to_string(filesize));
                    resptr->ADDHeader("Content-Type", _mime_type[".html"]);
                    resptr->ADDText(text404);
                }
                else
                {
                    resptr->ADDStatusLine(status_code, _status_code_to_desc[status_code]);
                    resptr->ADDHeader("Content-Length", std::to_string(filesize));
                    resptr->ADDHeader("Content-Type", _mime_type[suffix]);

                    resptr->ADDText(textcontent);
                }
            }

            // std::cout << "suffix : " << suffix << std::endl;

            // resptr->ADDStatusLine(200);
            // resptr->ADDStatusLine(301);
            // resptr->ADDHeader("Content-Length", std::to_string(filesize));
            // resptr->ADDHeader("Content-Type", _mime_type[suffix]);
            // resptr->ADDHeader("Location", "https://github.com/Xpccccc");
            // std::cout << "suffix Content-Type : " << _mime_type[suffix] << std::endl;

            // resptr->ADDText(textcontent);
        }

        // 添加Cookie
        // resptr->ADDHeader("Set-Cookie", ProveCookieWrite());
        // resptr->ADDHeader("Set-Cookie", ProveCookieTimeOut());
        // resptr->ADDHeader("Set-Cookie", ProvePath());

        std::string response;

        resptr->Serialize(&response);
        resptr->DebugPrint();

        return response;

#endif
    }

    ~HttpService() {}

private:
    std::unordered_map<std::string, std::string> _mime_type;   // Content-Type
    std::unordered_map<int, std::string> _status_code_to_desc; // 状态码及其描述
    std::unordered_map<std::string, func_t> _funcs;            //
    SessionManager _session_manager;                           // Session管理
};