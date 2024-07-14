#include <iostream>
#include <fstream>
#include <string>
#include <jsoncpp/json/json.h>

struct stu
{
    int id;
    std::string name;
    double grade;
    char c;

    void DebugPrint()
    {
        std::cout << id << " " << name << " " << grade << " " << c << std::endl;
    }
};

int main()
{
    // 读
    char buff[1024];
    std::ifstream ifs("./text.txt");
    if (!ifs.is_open())
    {
        return 1;
    }
    ifs.getline(buff, sizeof(buff) - 1);
    std::string res = buff;
    Json::Value root;
    struct stu s;
    Json::Reader reader;
    // 反序列化
    bool n = reader.parse(res, root);
    if (!n)
        return 2;
    s.id = root["id"].asInt();
    s.name = root["name"].asCString();
    s.grade = root["grade"].asDouble();
    s.c = root["c"].asInt(); // 没有asChar

    s.DebugPrint();
    return 0;
}

// int main()
// {
//     // 写
//     Json::Value root;
//     struct stu s = {1, "xp", 99.99, 'a'};
//     // 序列化
//     root["id"] = s.id;
//     root["name"] = s.name;
//     root["grade"] = s.grade;
//     root["c"] = s.c;
//     Json::FastWriter writer;
//     // Json::StyledWriter writer; -- 一样使用，但速度更慢
//     std::string str = writer.write(root);
//     std::ofstream ofs("./text.txt");
//     ofs << str;

//     return 0;
// }