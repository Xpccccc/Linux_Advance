#pragma once

#include <stdio.h>
#include <string>
#include <vector>

bool CheckCommand(std::string command)
{
    std::vector<std::string> cmd = {
        "kill",
        "rm",
        "dd",
        "top",
        "reboot",
        "shutdown",
        "mv",
        "cp",
        "halt",
        "unlink",
        "exit",
        "chmod"};
    for (auto &e : cmd)
    {
        if (command.find(e) != std::string::npos)
            return false;
    }

    return true;
}

std::string OnCommand(std::string command)
{
    if (!CheckCommand(command))
    {
        return "bad man!";
    }
    // FILE *popen(const char *command, const char *type);
    FILE *pp = popen(command.c_str(), "r");
    if (!pp)
    {
        return "popen error!";
    }

    std::string response;
    char buff[1024];
    while (true)
    {
        // char *fgets(char *s, int size, FILE *stream);
        char *s = fgets(buff, sizeof(buff), pp);
        if (!s)
            break;
        else
            response += buff;
    }
    pclose(pp);
    return response.empty() ? "not command" : response;
}