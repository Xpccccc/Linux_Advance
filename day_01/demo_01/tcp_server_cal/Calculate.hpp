#pragma once

#include <iostream>
#include <string>
#include "Protocol.hpp"

using namespace protocol_ns;

class Calculate
{
public:
    Calculate()
    {
    }

    std::unique_ptr<Response> Execute(const Request &req)
    {
        std::unique_ptr<Response> resptr = std::make_unique<Response>();
        switch (req._oper)
        {
        case '+':
            resptr->_result = req._x + req._y;
            break;

        case '-':
            resptr->_result = req._x - req._y;
            break;

        case '*':
            resptr->_result = req._x * req._y;
            break;
        case '/':
        {
            if (req._y == 0)
            {
                resptr->_flag = 1;
            }
            else
            {
                resptr->_result = req._x / req._y;
            }
            break;
        }

        case '%':
        {
            if (req._y == 0)
            {
                resptr->_flag = 2;
            }
            else
            {
                resptr->_result = req._x % req._y;
            }
            break;
        }
        default:
            resptr->_flag = 3;
            break;
        }
        return resptr;
    }
    ~Calculate() {}

private:
};