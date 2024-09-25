#pragma once
#include <iostream>
#include <mariadb/mysql.h>
#include <jsoncpp/json/json.h>

namespace my
{
#define HOST "127.0.0.1"
#define USER "root"
#define PASSWD ""
#define DBNAME "software_project"

    // 初始化MySQL操作句柄
    // 设置客户端字符集
    static MYSQL *MysqlInit()
    {
        // 初始化MySQL操作句柄
        MYSQL *mysql = mysql_init(nullptr);
        // 连接数据库服务器
        // Args:操作句柄、主机ip、用户名、密码、数据库名、端口号、Unix 套接字文件名、客户端标志位
        mysql_real_connect(mysql, HOST, USER, PASSWD, DBNAME, 0, nullptr, 0);
        // 客户端设置字符集
        mysql_set_character_set(mysql, "utf8");
        return mysql;
    }

    // MySQL操作句柄销毁
    static void MysqlDestroy(MYSQL *mysql)
    {
        if (mysql == nullptr)
            return;
        mysql_close(mysql);
    }

    // 执行sql语句
    static bool Execute(MYSQL *mysql, std::string &sql)
    {
        int mysql_ret = mysql_real_query(mysql, sql.c_str(), sql.size());
        if (mysql_ret != 0)
        {
            std::cout << "Execute SQL failed: " << mysql_error(mysql) << std::endl;
            return false;
        }
        return true;
    }

    class Video
    {
#define TABLE_NAME "video"
    public:
        Video(std::string table_name = TABLE_NAME) : _table_name(table_name)
        {
            _mysql = MysqlInit();
        }

                // 执行SQL语句

        // 增删查改
        // 增加，通过把插入的值放入Value中
        bool Insert(Json::Value &value)
        {
#define INSERT_VIDEO_SQL "insert into %s %s values %s;"
            std::string sql;
            Json::Value::iterator iter = value.begin();
            //leftbuffer: (key1, key2, ...)
            //rightbuffer: (value1, value2, ...)
            std::string leftbuffer, rightbuffer;
            for (; iter != value.end(); iter++)
            {
                if (iter != value.begin())
                {
                    leftbuffer += ", ";
                    rightbuffer += ", ";
                }
                else
                {
                    leftbuffer += "(";
                    rightbuffer += "(";
                }
                leftbuffer += iter.key().asString();
                rightbuffer += (*iter).asString();
            }
            leftbuffer += ")";
            rightbuffer += ")";
            char inbuffer[20];
            sprintf(inbuffer, sql.c_str(), leftbuffer.c_str(), rightbuffer.c_str(), _table_name.c_str());
            std::cout << "Insert SQL: " << sql << std::endl;
            return Execute(_mysql, sql);
        }

        // 删除1，通过指定的视频id删除
        bool Delete(int id)
        {
            // Args：视频id
#define DELETE_VIDEO_SQL "delete from %s where id = %d;"
            char inbuffer[1024];
            int n = sprintf(inbuffer, DELETE_VIDEO_SQL, _table_name.c_str(), id);
            if (n < 0) // 错误
            {
                std::cout << "write failed" << std::endl;
                return false;
            }
            else if (n == 0)
            {
                std::cout << "write failed" << std::endl;
                return false;
            }
            std::cout << "Delete SQL: " << inbuffer << std::endl;
            std::string sql(inbuffer);
            std::cout << "write success" << std::endl;
            return Execute(_mysql, sql);
        }

        // 删除2，通过指定Json::Value中的属性进行筛选删除
        bool Delete(Json::Value &value)
        {
            // 把value中的属性拼接成 "key = value[key] "的形式
            std::string sql;
            Json::Value::iterator iter = value.begin();
            for (; iter != value.end(); iter++)
            {
                if (iter != value.begin())
                    sql += " and ";
                sql += iter.key().asString() + " = " + (*iter).asString();
            }
#define DELETE_VIDEO_SQL "delete from video where "
            sql = DELETE_VIDEO_SQL + sql + ";";
            std::cout << "Delete SQL: " << sql << std::endl;
            return Execute(_mysql, sql);
        }
 
        // 查询，通过指定Json::Value中的属性进行筛选查询
        bool Select(Json::Value &value, Json::Value &result)
        {
#define SELECT_VIDEO_SQL "select * from video where "
            std::string sql;
            Json::Value::iterator iter = value.begin();
            for (; iter != value.end(); iter++)
            {
                if (iter != value.begin())
                    sql += " and ";
                sql += iter.key().asString() + " = " + (*iter).asString();
            }
            sql = SELECT_VIDEO_SQL + sql + ";";
            std::cout << "Select SQL: " << sql << std::endl;
            return Execute(_mysql, sql);
        }

        // 更新，通过指定Json::Value中的属性进行筛选更新
        bool Update(Json::Value &value, Json::Value &condition)
        {
#define UPDATE_VIDEO_SQL "update video set "
            std::string sql;
            Json::Value::iterator iter = value.begin();
            for (; iter != value.end(); iter++)
            {
                if (iter != value.begin())
                    sql += ", ";
                sql += iter.key().asString() + " = " + (*iter).asString();
            }
            sql = UPDATE_VIDEO_SQL + sql;
            sql += " where ";
            iter = condition.begin();
            for (; iter != condition.end(); iter++)
            {
                if (iter != condition.begin())
                    sql += " and ";
                sql += iter.key().asString() + " = " + (*iter).asString();
            }
            sql += ";";
            std::cout << "Update SQL: " << sql << std::endl;
            return Execute(_mysql, sql);
        }

        // 获取结果集并打印
        bool GetResult()
        {
            MYSQL_RES *result = mysql_store_result(_mysql);
            if (result == nullptr)
            {
                std::cout << "Get result failed: " << mysql_error(_mysql) << std::endl;
                return false;
            }
            MYSQL_ROW row;
            uint num_fields = mysql_num_fields(result);
            while (row = mysql_fetch_row(result))
            {
                for (uint i = 0; i < num_fields; i++)
                {
                    std::cout << row[i] << " ";
                }
                std::cout << std::endl;
            }
            return true;
        }

        // 释放结果集
        ~Video()
        {
            // 关闭连接
            MysqlDestroy(_mysql);
        }

    private:
        MYSQL *_mysql;           // 数据库操作句柄
        std::string _table_name; // 视频类维护的表名
    };
}