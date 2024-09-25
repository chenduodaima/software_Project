#pragma once
#include <jsoncpp/json/json.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <fstream>

namespace ul
{
    // 文件操作类
    class FileUtil
    {
    public:
        FileUtil(std::string path) : _filepath(path)
        {}

        ~FileUtil()
        {}
        
        //判断文件是否存在
        bool Exists()
        {
            int ret = access(_filepath.c_str(), F_OK);
            if (ret != 0)
            {
                std::cout << "file is not exists" << std::endl;
                return false;
            }
            return true;
        }

        //获取文件大小
        size_t Size()
        {
            if (this->Exists() == false)
                return 0;
            struct stat st;
            //stat函数获取文件信息
            int ret = stat(_filepath.c_str(), &st);
            if (ret != 0)
            {
                std::cout << "get file stat failed" << std::endl;
                return 0;
            }
            return st.st_size;
        }

        //读取文件
        bool GetContent(std::string *str)
        {
            std::ifstream ifs;
            ifs.open(_filepath, std::ios::binary);
            if (!ifs.is_open())
            {
                std::cerr << "open file failed" << std::endl;
                return false;
            }
            size_t size = this->Size();
            str->resize(size);
            ifs.read(&(*str)[0], size);
            if (ifs.good() == false)
            {
                std::cerr << "read file failed" << std::endl;
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }

        //写入字符串到文件
        bool SetContent(std::string &str)
        {
            std::ofstream out;
            out.open(_filepath, std::ios::binary);//以二进制打开
            if (!out.is_open())
            {
                std::cerr << "open file failed" << std::endl;
                return false;
            }
            out.write(str.c_str(), str.size());
            if (out.good() == false)
            {
                std::cerr << "write file failed" << std::endl;
                out.close();
                return false;
            }
            out.close();
            return true;
        }

        //创建目录
        bool CreateDirectory()
        {
            if (this->Exists())
                return true;
            int ret = mkdir(_filepath.c_str(), 0777);
            if (ret != 0)
            {
                std::cerr << "mkdir dir failed" << std::endl;
                return false;
            }
            return true;
        }
    private:
        std::string _filepath;//文件路径
    };

    class JsonUtil
    {
    public:
        // 反序列化
        static bool Deserialize(Json::Value &root, const std::string &str)
        {
            Json::CharReaderBuilder crb;
            std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
            Json::String errs;
            bool ret = cr->parse(str.c_str(), str.c_str() + str.size(), &root, &errs);
            if (ret == false)
            {
                std::cout << errs << std::endl;
                return false;
            }
            return true;
        }

        // 序列化
        static bool Serialize(Json::Value &root, std::string *str)
        {
            Json::StreamWriterBuilder swb;
            std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
            std::stringstream ssr;
            int ret = sw->write(root, &ssr);
            if (ret != 0)
            {
                std::cout << "json write failed" << std::endl;
                return false;
            }
            *str = ssr.str();
            return true;
        }
    };
}