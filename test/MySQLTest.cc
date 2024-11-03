#include <iostream>
#include "../mysql.hpp"
#include "../util.hpp"
#include "../Log.hpp"

void MySQLTest()
{
    using namespace my;
    LogMessage(Debug, "\n=== MySQL Test Begin ===\n");
    Video video;
    // 测试1：插入测试
    LogMessage(Debug, "\n=== Test 1: Insert Test ===\n");
    Json::Value value;
    value["name"] = "test1";
    value["author"] = "tomi";
    value["video_path"] = "/video/test1.mp4";
    value["image_path"] = "/img/test1.jpg";
    value["info"] = "hello world";

    Json::Value value1;
    value1["name"] = "test2";
    value1["author"] = "love";
    value1["video_path"] = "/video/test2.mp4";
    value1["image_path"] = "/img/test2.jpg";
    value1["info"] = "奇怪的人";
    
    Json::Value value2;
    value2["name"] = "test3";
    value2["author"] = "动作大师";
    value2["video_path"] = "/video/test3.mp4";
    value2["image_path"] = "/img/test3.jpg";
    value2["info"] = "爱情动作片";

    
    //插入test1、test2、test3
    if (video.Insert(value))
    {
        std::string insert_str;
        ul::JsonUtil::Serialize(value, &insert_str);
        LogMessage(Debug, "\n✓ Insert successful: %s\n", insert_str.c_str());
    }
    else
    {
        LogMessage(Debug, "\n✗ Insert failed\n");
    }
    if (video.Insert(value1))
    {
        std::string insert_str;
        ul::JsonUtil::Serialize(value1, &insert_str);
        LogMessage(Debug, "\n✓ Insert successful: %s\n", insert_str.c_str());
    }
    else
    {
        LogMessage(Debug, "\n✗ Insert failed\n");
    }
    if (video.Insert(value2))
    {
        std::string insert_str;
        ul::JsonUtil::Serialize(value2, &insert_str);
        LogMessage(Debug, "\n✓ Insert successful: %s\n", insert_str.c_str());
    }
    else
    {
        LogMessage(Debug, "\n✗ Insert failed\n");
    }   
    LogMessage(Debug, "\n\n\n");



    //测试2：删除测试
    LogMessage(Debug, "\n=== Test 2: Delete Test ===\n");
    //删除test1
    int id1 = 8;
    if (video.Delete(id1))
    {
        LogMessage(Debug, "\n✓ Delete successful\n");
    }
    else
    {
        LogMessage(Debug, "\n✗ Delete failed\n");
    }
    LogMessage(Debug, "\n\n\n");



    // 测试3：更新测试
    LogMessage(Debug, "\n=== Test 3: Update Test ===\n");
    //更新test2
    Json::Value condition;
    condition["name"] = "test2";
    Json::Value update;
    update["name"] = "test222";
    
    if (video.Update(update, condition))
    {
        LogMessage(Debug, "\n✓ Update successful\n");
    }
    else
    {
        LogMessage(Debug, "\n✗ Update failed\n");
    }
    LogMessage(Debug, "\n\n\n");



    // 测试4：模糊查询测试
    LogMessage(Debug, "\n=== Test 4: Select Like Test ===\n");
    //模糊查询test2，得到test222
    std::string key = "test2";
    Json::Value result;
    if (video.SelectLike(key, &result))
    {
        std::string result_str;
        ul::JsonUtil::Serialize(result, &result_str);
        LogMessage(Debug, "\n✓ Select Like successful: %s\n", result_str.c_str());
    }
    else
    {
        LogMessage(Debug, "\n✗ Select Like failed\n");
    }
    LogMessage(Debug, "\n\n\n");



    // 测试5：全部查询测试
    //查询所有，得到test222、test3
    LogMessage(Debug, "\n=== Test 5: Select All Test ===\n");
    Json::Value result1;
    if (video.SelectALL(&result1))
    {
        std::string result_str;
        ul::JsonUtil::Serialize(result1, &result_str);
        LogMessage(Debug, "\n✓ Select All successful: %s\n", result_str.c_str());
    }
    else
    {
        LogMessage(Debug, "\n✗ Select All failed\n");
    }
    LogMessage(Debug, "\n\n\n");



    // 测试6：单条查询测试
    //查询id为9的，得到test3
    LogMessage(Debug, "\n=== Test 6: Select One Test ===\n");
    int id2 = 9;
    Json::Value result2;
    if (video.SelectOne(id2, &result2))
    {
        std::string result_str;
        ul::JsonUtil::Serialize(result2, &result_str);
        LogMessage(Debug, "\n✓ Select One successful: %s\n", result_str.c_str());
    }
    else
    {
        LogMessage(Debug, "\n✗ Select One failed\n");
    }
    LogMessage(Debug, "\n\n\n");
    
    LogMessage(Debug, "\n=== MySQL Test End ===\n");
}

int main()
{
    MySQLTest();
    return 0;
}