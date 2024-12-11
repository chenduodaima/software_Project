#include <iostream>
#include "http.hpp"
#include "Common.hpp"
#include <memory>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(Fatal);
    }
    // 清空日志
    lg.Clear();
    conf.SetLogStyle(ClassFile);
    uint16_t port = std::atoi(argv[1]);
    std::unique_ptr<aod::HttpServer> http_server(new aod::HttpServer(port));
    http_server->RunModule();
    return 0;
}