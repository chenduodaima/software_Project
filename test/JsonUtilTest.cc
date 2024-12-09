#include <iostream>
#include "../util.hpp"
#include "../Log.hpp"
//测试JsonUtil工具类
void JsonUtilTest()
{
    using namespace ul;
    LogMessage(Debug, "\n=== Json Util Test Begin ===\n");
    //测试1：序列化
    LogMessage(Debug, "\n=== Test 1: Serialize ===\n");
    std::string inbuffer;
    Json::Value root;
    root["name"] = "Tomi";
    root["age"] = 18;
    root["sex"] = "male";
    root["hobby"] = Json::arrayValue;
    root["hobby"].append("football");
    JsonUtil::Serialize(root, &inbuffer);
    LogMessage(Debug, "inbuffer: %s", inbuffer.c_str());
    LogMessage(Debug, "\n\n\n");



    //测试2：反序列化
    LogMessage(Debug, "\n=== Test 2: Deserialize ===\n");
    Json::Value outroot;
    std::string outbuffer = R"({"name":"Tomi","age":18,"sex":"male","hobby":["football"]})";
    JsonUtil::Deserialize(outroot, outbuffer);
    LogMessage(Debug, "name: %s\n", outroot["name"].asString().c_str());
    LogMessage(Debug, "age: %d\n", outroot["age"].asInt());
    LogMessage(Debug, "sex: %s\n", outroot["sex"].asString().c_str());
    Json::Value hobby = outroot["hobby"];
    for (int i = 0; i < hobby.size(); i++)
    {
        LogMessage(Debug, "hobby[%d]: %s\n", i, hobby[i].asString().c_str());
    }
    LogMessage(Debug, "\n=== Json Util Test End ===\n");
}

int main()
{
    JsonUtilTest();
    return 0;
}