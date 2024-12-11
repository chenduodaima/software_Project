#include <iostream>
#include "../util.hpp"
#include "../Log.hpp"

void CleanupTestFiles() {
    remove("test_index.html");
    remove("test_inbuffer.txt");
    remove("./live");
}

void FileUtilTest()
{
    using namespace ul;
    LogMessage(Debug, "\n=== File Util Test Begin ===\n");
    
    // 测试1：判断文件是否存在
    LogMessage(Debug, "\n=== Test 1: Check File Exists ===\n");
    // 创建测试文件
    {
        FILE* fp = fopen("test_index.html", "w");
        if (fp) {
            fprintf(fp, "Test Content");
            fclose(fp);
        }
    }

    FileUtil file("test_index.html");
    if (file.Exists()) {
        LogMessage(Debug, "\n✓ %s exists\n", "test_index.html");
    } else {
        LogMessage(Debug, "\n✗ Failed to create test file\n");
        return;
    }
    
    FileUtil file2("./not_exists_file");
    if (!file2.Exists()) {
        LogMessage(Debug, "\n✓ %s correctly reported as not existing\n", "./not_exists_file");
    } else {
        LogMessage(Debug, "\n✗ Unexpected file existence\n");
    }
    LogMessage(Debug, "\n\n\n");

    // 测试2：获取文件内容
    LogMessage(Debug, "\n=== Test 2: Get File Content ===\n");
    std::string content;
    if (file.GetContent(&content)) {
        LogMessage(Debug, "\n✓ Content retrieved: %s\n", content.c_str());
    } else {
        LogMessage(Debug, "\n✗ Failed to get content\n");
    }
    LogMessage(Debug, "\n\n\n");

    // 测试3：设置文件内容
    LogMessage(Debug, "\n=== Test 3: Set File Content ===\n");
    std::string new_content = "hello world";
    FileUtil file3("./test_inbuffer.txt");
    if (file3.SetContent(new_content)) {
        std::string buffer;
        if (file3.GetContent(&buffer)) {
            if (buffer == new_content) {
                LogMessage(Debug, "\n✓ Content written and verified: %s\n", buffer.c_str());
            } else {
                LogMessage(Debug, "\n✗ Content verification failed\n");
            }
        }
    } else {
        LogMessage(Debug, "\n✗ Failed to set content\n");
    }
    LogMessage(Debug, "\n\n\n");

    // 测试4：获取文件大小
    LogMessage(Debug, "\n=== Test 4: Get File Size ===\n");
    LogMessage(Debug, "\nFile 1 size: %lu bytes\n", file.Size());
    LogMessage(Debug, "\nFile 2 size: %lu bytes\n", file2.Size());
    LogMessage(Debug, "\nFile 3 size: %lu bytes\n", file3.Size());
    LogMessage(Debug, "\n\n\n");

    // 测试5：创建目录
    LogMessage(Debug, "\n=== Test 5: Create Directory ===\n");
    FileUtil dir("./live");
    if (dir.CreateDirectory()) {
        if (dir.Exists()) {
            LogMessage(Debug, "\n✓ Directory created successfully\n");
        } else {
            LogMessage(Debug, "\n✗ Directory creation reported success but doesn't exist\n");
        }
    } else {
        LogMessage(Debug, "\n✗ Failed to create directory\n");
    }
    
    LogMessage(Debug, "\n=== File Util Test End ===\n");
}

int main()
{
    // 清理可能存在的旧测试文件
    CleanupTestFiles();
    
    // 运行测试
    FileUtilTest();
    
    // 清理测试文件
    CleanupTestFiles();
    
    return 0;
}