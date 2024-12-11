#pragma once
#include <string>
#include <sys/epoll.h>
#include "Log.hpp"

std::string EventToString(uint32_t events)
{
    std::string str;
    if (events & EPOLLIN)//读事件
        str += "EPOLLIN ";
    if (events & EPOLLOUT)//写事件
        str += "EPOLLOUT ";
    if (events & EPOLLERR)//错误事件
        str += "EPOLLERR ";
    if (events & EPOLLHUP)//挂断事件
        str += "EPOLLHUP ";
    if (events & EPOLLRDHUP)//对端关闭连接
        str += "EPOLLRDHUP ";
    if (events & EPOLLPRI)//紧急事件
        str += "EPOLLPRI ";
    if (events & EPOLLONESHOT)//一次性事件
        str += "EPOLLONESHOT ";
    if (events & EPOLLET)//边缘触发
        str += "EPOLLET ";
    return str;
}

void Usage(std::string str)
{
    LogMessage(Fatal, "Usage: %s port\n", str.c_str());
}