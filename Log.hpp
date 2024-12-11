#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <cstdarg>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// 枚举日志信息类别
enum
{
    Debug = 0,
    Info,
    Warning,
    Error,
    Fatal,
};

// 枚举日志输出流
enum
{
    Screen = 10,
    OneFile,
    ClassFile
};

// 根据枚举值返回对应字符串
std::string LevelToString(int level)
{
    switch (level)
    {
    case Debug:
        return "debug";
    case Info:
        return "info";
    case Warning:
        return "warning";
    case Error:
        return "error";
    case Fatal:
        return "fatal";
    default:
        return "unknown";
    }
}

const int defaultstyle = Screen;             // 默认日志输出流是显示屏
const std::string default_filename = "log."; // 默认日志文件的前缀都是log.
const std::string logdir = "log";            // 日志目录名称

// 定义一个日志类
class Log
{
public:
    Log() : _style(defaultstyle), _filename_prefix(default_filename)
    {
    }

    //清空日志
    void Clear()
    {
        //枚举所有日志文件，如果存在，就清空
        for (int i = Debug; i <= Fatal; ++i)
        {
            std::string logname = logdir;
            logname += "/";
            logname += _filename_prefix;
            logname += LevelToString(i);
            //清空但是不删除文件
            std::ofstream out(logname, std::ios::trunc);
            if (out.is_open())
                out.close();
        }
    }

    // 改变日志输出流
    void Enable(int sty) //
    {
        _style = sty;
        // 如果日志输出流为ClassFile，则创建日志目录
        if (_style == ClassFile)
        {
            mkdir(logdir.c_str(), 0775); // 创建日志目录,0775表示该目录的权限，0表示8进制，775表示的权限为rwxrwxr-x
        }
    }

    // 打印该日志信息的时间
    std::string TimeStampExLocalTime()
    {
        time_t currtime = time(nullptr);
        struct tm *curr = localtime(&currtime);
        char time_buffer[128];
        snprintf(time_buffer, sizeof(time_buffer), "%d-%d-%d %d:%d:%d",
                 curr->tm_year + 1900, curr->tm_mon + 1, curr->tm_mday,
                 curr->tm_hour, curr->tm_min, curr->tm_sec);
        return time_buffer;
    }

    void WriteLogToOneFile(const std::string &logname, const std::string &message)
    {
        umask(0);
        int fd = open(logname.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
        if (fd < 0)
            return;
        write(fd, message.c_str(), message.size());
        close(fd);
        // std::ofstream out(logname);
        // if (!out.is_open())
        //     return;
        // out.write(message.c_str(), message.size());
        // out.close();
    }
    void WriteLogToClassFile(const std::string &levelstr, const std::string &message)
    {
        std::string logname = logdir;
        logname += "/";
        logname += _filename_prefix;
        logname += levelstr;
        // log/log.all log/log.debug log/log.info log/log.warning log/log.error log/log.fatal
        // 写到文件中
        WriteLogToOneFile(logname, message);
    }

    void WriteLog(const std::string &levelstr, const std::string &message)
    {
        switch (_style)
        {
        case Screen:
            std::cout << message;
            break;
        case OneFile:
            WriteLogToClassFile("all", message);
            break;
        case ClassFile:
            WriteLogToClassFile(levelstr, message);
            break;
        default:
            break;
        }
    }

    void LogMessage(int level, const char *file, int line, const char *format, ...) // 类C的一个日志接口
    {
        char leftbuffer[1024];
        std::string levelstr = LevelToString(level);   // 获取日志级别
        std::string currtime = TimeStampExLocalTime(); // 获取当前时间
        std::string idstr = std::to_string(getpid());  // 获取进程ID
        char rightbuffer[1024];
        va_list args; // char *, void *
        va_start(args, format);
        // args 指向了可变参数部分
        vsnprintf(rightbuffer, sizeof(rightbuffer), format, args);
        va_end(args); // args = nullptr;
        snprintf(leftbuffer, sizeof(leftbuffer), "[%s][%s][%s][%s][%d] ",
                 levelstr.c_str(), currtime.c_str(), idstr.c_str(), file, line);

        std::string loginfo = leftbuffer;
        loginfo += rightbuffer;
        WriteLog(levelstr, loginfo);
    }

#define LogMessage(level, format, ...) \
    lg.LogMessage(level, __FILE__, __LINE__, format, ##__VA_ARGS__)

    // void operator()(int level, const char *format, ...)
    // {
    //     LogMessage(int level, const char *format, ...)
    // }

    ~Log() {}

private:
    int _style;
    std::string _filename_prefix; // 日志文件名前缀
};

Log lg;

class Conf
{
public:
    Conf()
    {
        lg.Enable(Screen);
    }

    void SetLogStyle(int style)
    {
        lg.Enable(style);
    }

    ~Conf()
    {
    }
};

Conf conf;


