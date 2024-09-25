#include <iostream>
#include "data.hpp"
#include "util.hpp"

//工具类测试用例
//文件工具类测试
void FileUtilTest()
{
    #define LIVE_TEST_FILE_PATH "./test/test_index.html"
    using namespace ul;
    //实例化一个file工具对象
    FileUtil file(LIVE_TEST_FILE_PATH);
    //测试1：判断文件是否存在
    std::cout << "这是测试1" << std::endl;
    //（1）文件存在
    if (file.Exists())
    {
        std::cout << "file exists" << std::endl;
    }
    //（2）文件不存在
    FileUtil file2("./test/not_exists");
    file2.Exists();
    std::cout << std::endl << "这是测试2" << std::endl;

    //测试2：获取文件内容
    std::string content;    
    if (file.GetContent(&content))
    {
        std::cout << content << std::endl;
    }
    else
    {
        std::cout << "get content failed" << std::endl;
    }
    std::cout << std::endl << "这是测试3" << std::endl;

    //测试3：设置文件内容
    std::string new_content = "hello world";   
    FileUtil file3("./test/test_inbuffer.txt");
    if (file3.SetContent(new_content))
    {
        std::string buffer;
        file3.GetContent(&buffer);
        std::cout << buffer << std::endl;
    }
    else
    {
        std::cout << "set content failed" << std::endl;
    }
    std::cout << std::endl << "这是测试4" << std::endl;

    //测试4：获取文件大小
    std::cout << file.Size() << std::endl;
    std::cout << file2.Size() << std::endl;
    std::cout << file3.Size() << std::endl;
    std::cout << std::endl << "这是测试5" << std::endl;

    //测试5：创建目录
    #define LIVE_DIR "./test/live"
    FileUtil dir(LIVE_DIR);
    if (dir.CreateDirectory())
    {
        std::cout << "create dir success" << std::endl;
    }
    else
    {
        std::cout << "create dir failed" << std::endl;
    }
}

//测试JsonUtil工具类
void JsonUtilTest()
{
    using namespace ul;
    
    //测试1：序列化
    std::cout << "这是测试1" << std::endl;
    std::string inbuffer;
    Json::Value root;
    root["name"] = "Tomi";
    root["age"] = 18;
    root["sex"] = "male";
    root["hobby"] = Json::arrayValue;
    root["hobby"].append("football");
    JsonUtil::Serialize(root, &inbuffer);
    std::cout << inbuffer << std::endl;
    //测试2：反序列化
    std::cout << std::endl << "这是测试2" << std::endl;
    Json::Value outroot;
    std::string outbuffer = R"({"name":"Tomi","age":18,"sex":"male","hobby":["football"]})";
    JsonUtil::Deserialize(outroot, outbuffer);
    std::cout << outroot["name"].asString() << std::endl;
    std::cout << outroot["age"].asInt() << std::endl;
    std::cout << outroot["sex"].asString() << std::endl;
    Json::Value hobby = outroot["hobby"];
    for (int i = 0; i < hobby.size(); i++)
    {
        std::cout << hobby[i].asString() << std::endl;
    }
}

//测试MySQL封装类
void MySQLTest()
{
    using namespace my;
    Video video;
    Json::Value value;
    value["id"] = 1;
    value["name"] = "test";
    
}


int main()
{
    //FileUtilTest();
    //JsonUtilTest();
    return 0;
}