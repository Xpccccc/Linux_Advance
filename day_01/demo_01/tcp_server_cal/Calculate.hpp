#pragma once

#include <iostream>
#include <string>
#include "Protocol.hpp"

using namespace protocol_ns;

// 应用层
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
            resptr->_equation = std::to_string(req._x) + " " + req._oper + " " + std::to_string(req._y) + " = " + std::to_string(resptr->_result);
            break;

        case '-':
            resptr->_result = req._x - req._y;
            resptr->_equation = std::to_string(req._x) + " " + req._oper + " " + std::to_string(req._y) + " = " + std::to_string(resptr->_result);
            break;

        case '*':
            resptr->_result = req._x * req._y;
            resptr->_equation = std::to_string(req._x) + " " + req._oper + " " + std::to_string(req._y) + " = " + std::to_string(resptr->_result);
            break;
        case '/':
        {
            if (req._y == 0)
            {
                resptr->_flag = 1;
                resptr->_equation = "除0错误";
            }
            else
            {
                resptr->_result = req._x / req._y;
                resptr->_equation = std::to_string(req._x) + " " + req._oper + " " + std::to_string(req._y) + " = " + std::to_string(resptr->_result);
            }
            break;
        }

        case '%':
        {
            if (req._y == 0)
            {
                resptr->_flag = 2;
                resptr->_equation = "模0错误";
            }
            else
            {
                resptr->_result = req._x % req._y;
                resptr->_equation = std::to_string(req._x) + " " + req._oper + " " + std::to_string(req._y) + " = " + std::to_string(resptr->_result);
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