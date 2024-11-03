#pragma once
#include <iostream>
#include <mariadb/mysql.h>
#include <jsoncpp/json/json.h>
#include "Log.hpp"

namespace my
{
#define HOST "127.0.0.1"
#define USER "root"
#define PASSWD "root"
#define DBNAME "software_project"

    static MYSQL *MySQLInit()
    {
        MYSQL *mysql = mysql_init(nullptr);
        if (mysql == nullptr)
            return nullptr;
        //初始化成功
        LogMessage(Info, "mysql_init success\n");
        if (mysql_real_connect(mysql, HOST, USER, PASSWD, DBNAME, 0, nullptr, 0) == nullptr)
        {
            LogMessage(Error, "connect mysql server failed!\n");
            LogMessage(Error, "%s\n", mysql_error(mysql));
            mysql_close(mysql);
            return nullptr;
        }
        mysql_set_character_set(mysql, "utf8");
        return mysql;
    }

    static void MysqlDestroy(MYSQL *mysql)
    {
        if (mysql == nullptr)
            return;
        mysql_close(mysql);
    }

    static bool Execute(MYSQL *mysql, std::string &sql)
    {
        int mysql_ret = mysql_real_query(mysql, sql.c_str(), sql.size());
        if (mysql_ret != 0)
        {
            LogMessage(Error, "Execute SQL failed: %s\n", mysql_error(mysql));
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
            _mysql = MySQLInit();
        }

        bool Insert(Json::Value &value)
        {
#define INSERT_VIDEO_SQL "insert into %s %s values %s;"
            Json::Value::iterator iter = value.begin();
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
                rightbuffer += "\"" + (*iter).asString() + "\"";
            }
            leftbuffer += ")";
            rightbuffer += ")";
            char inbuffer[1024];
            sprintf(inbuffer, INSERT_VIDEO_SQL, _table_name.c_str(), leftbuffer.c_str(), rightbuffer.c_str());
            LogMessage(Info, "Insert SQL: %s\n", inbuffer);
            std::string sql(inbuffer);
            return Execute(_mysql, sql);
        }

        bool Delete(int id)
        {
#define DELETE_VIDEO_SQL_1 "delete from %s where id = %d;"
            char inbuffer[1024];
            int n = sprintf(inbuffer, DELETE_VIDEO_SQL_1, _table_name.c_str(), id);
            if (n < 0)
            {
                LogMessage(Error, "write failed\n");
                return false;
            }
            else if (n == 0)
            {
                LogMessage(Error, "write failed\n");
                return false;
            }
            LogMessage(Info, "Delete SQL: %s\n", inbuffer);
            std::string sql(inbuffer);
            LogMessage(Info, "write success\n");
            return Execute(_mysql, sql);
        }

        bool Delete(Json::Value &value)
        {
            std::string sql;
            Json::Value::iterator iter = value.begin();
            for (; iter != value.end(); iter++)
            {
                if (iter != value.begin())
                    sql += " and ";
                sql += iter.key().asString() + " = " + "\"" + (*iter).asString() + "\"";
            }
#define DELETE_VIDEO_SQL_2 "delete from video where "
            sql = DELETE_VIDEO_SQL_2 + sql + ";";
            LogMessage(Info, "Delete SQL: %s\n", sql.c_str());
            return Execute(_mysql, sql);
        }
 
        bool SelectOne(int video_id, Json::Value *result)
        {
#define SELECT_VIDEO_SQL_ONE "select * from video where id = %d;"
            char sql[1024];
            sprintf(sql, SELECT_VIDEO_SQL_ONE, video_id);
            std::string sql_str(sql);
            if (!Execute(_mysql, sql_str))
            {
                return false;
            }
            if (!GetOneResult(*result))
            {
                return false;
            }
            return true;
        }

        bool SelectALL(Json::Value *result)
        {
#define SELECT_VIDEO_SQL_ALL "select * from video;"
            std::string sql(SELECT_VIDEO_SQL_ALL);
            if (!Execute(_mysql, sql))
            {
                return false;
            }
            if (!GetALLResult(result))
            {
                return false;
            }
            return true;
        }

        bool SelectLike(std::string &key, Json::Value *result)
        {
#define SELECT_VIDEO_SQL_LIKE "select * from video where name like '%%%s%%';"
            char sql[1024];
            sprintf(sql, SELECT_VIDEO_SQL_LIKE, key.c_str());
            std::string sql_str(sql);
            if (!Execute(_mysql, sql_str))
            {
                return false;
            }
            if (!GetALLResult(result))
            {
                return false;
            }
            return true;
        }

        bool Update(Json::Value &value, Json::Value &condition)
        {
#define UPDATE_VIDEO_SQL "update video set "
            std::string sql;
            Json::Value::iterator iter = value.begin();
            for (; iter != value.end(); iter++)
            {
                if (iter != value.begin())
                    sql += ", ";
                sql += iter.key().asString() + " = " + "\"" + (*iter).asString() + "\"";
            }
            sql = UPDATE_VIDEO_SQL + sql;
            sql += " where ";
            iter = condition.begin();
            for (; iter != condition.end(); iter++)
            {
                if (iter != condition.begin())
                    sql += " and ";
                sql += iter.key().asString() + " = " + "\"" + (*iter).asString() + "\"";
            }
            sql += ";";
            LogMessage(Info, "Update SQL: %s\n", sql.c_str());
            return Execute(_mysql, sql);
        }
        
        bool GetOneResult(Json::Value &video)
        {
            MYSQL_RES *result = mysql_store_result(_mysql);
            if (result == nullptr)
            {
                LogMessage(Error, "Get result failed: %s\n", mysql_error(_mysql));
                return false;
            }
            int row_num = mysql_num_rows(result);
            if (row_num != 1)
            {
                LogMessage(Error, "have no data!\n");
                mysql_free_result(result);
                return false;
            }
            MYSQL_ROW row = mysql_fetch_row(result);
            video["id"] = atoi(row[1]);
            video["name"] = row[1];
            video["author"] = row[2];
            video["info"] = row[3];
            video["video_path"] = row[4];
            video["image_path"] = row[5];
            mysql_free_result(result);
            return true;
        }

        bool GetALLResult(Json::Value *videos)
        {
            MYSQL_RES *result = mysql_store_result(_mysql);
            if (result == nullptr)
            {
                LogMessage(Error, "Get result failed: %s\n", mysql_error(_mysql));
                return false;
            }
            MYSQL_ROW row;
            uint num_fields = mysql_num_fields(result);
            while (row = mysql_fetch_row(result))
            {
                Json::Value video;
                video["id"] = atoi(row[0]);
                video["name"] = row[1];
                video["author"] = row[2];
                video["info"] = row[3];
                videos->append(video);
            }
            mysql_free_result(result);
            return true;
        }

        std::string GetTableName()
        {
            return _table_name;
        }

        ~Video()
        {
            MysqlDestroy(_mysql);
        }

    private:
        MYSQL *_mysql;
        std::string _table_name;
    };
}