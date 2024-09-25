#include <jsoncpp/json.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <unistd.h>
#include <sys/stat.h>
#include <string>

namespace ul
{
    class FileUtil
    {
    public:
        FileUtil(std::string path) : _filepath(path)
        {}

        ~FileUtil()
        {}
        
        //检查文件是否存在
        bool Exists()
        {
            int ret = access(_filepath, F_OK);
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
            if (this->Exists == false)
                return 0;
            struct stat st;
            int ret = stat(_filepath, st);
            if (ret != 0)
            {
                std::cout << "get file stat failed" << std::endl;
                return 0;
            }
            return st.st_size;
        }

        //获取文件到字符串
        bool GetContent(std::string *str)
        {
            std::ifstream ifs;
            ifs.open(_filepath, std::ios::binary);
            if (!ifs.is_open())
            {
                std::cerr << "open file failed" << std::endl;
                return false;
            }
            size_t size = Size();
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

        //读取字符串到文件
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

        bool CreateDirectory()
        {
            if (Exists(_filepath))
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
}