// #pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <condition_variable>
#include <thread>
#include <mariadb/mysql.h>
#include <jsoncpp/json/json.h>
#include <unordered_map>
#include "Log.hpp"
#include "util.hpp"
namespace my
{
    // video、user、tag
    static constexpr const char *HOST = "127.0.0.1";
    static constexpr const char *USER = "root";
    static constexpr const char *PASSWD = "root";
    static constexpr const char *DBNAME = "sw_pj";
    static constexpr const char *VIDEO_TABLE_NAME = "video";
    static constexpr const char *USER_TABLE_NAME = "user";
    static constexpr const char *TAG_TABLE_NAME = "tag";
    static constexpr const char *TYPE_TABLE_NAME = "type";
    static constexpr const char *COMMENT_TABLE_NAME = "comments";
    // Video信息表
    typedef struct VideoInfo
    {
        int video_id;           // video的唯一id
        int uid;                // 作者id
        std::string video_name; // 视频名称
        std::string video_info; // 视频简介
        int likes;              // 点赞数
        int dislike;            // 踩数
        int favorites;          // 收藏数
        int views;              // 观看数
        long long heat;         // 热度
        float grade;            // 评分
        time_t release_time;    // 发布时间
        int video_type_id;      // 视频类型id
        std::string video_path; // 视频存放路径
        std::string image_path; // 封面图片存放路径
    } VideoInfo;

    // 用户信息表
    typedef struct UserInfo
    {
        int uid;                             // 用户唯一id
        std::string name;                    // 用户昵称
        std::string username;                // 用户名
        std::string password;                // 密码
        std::string user_head_portrait_path; // 用户头像
    } UserInfo;

    typedef struct TagInfo
    {
        int tag_id;           // tag_id
        std::string tag_name; // tag的名称
        int tag_count;        // 拥有该标签的视频的总数
    } TagInfo;

    typedef struct VideoOfTagInfo
    {
        int id;       // 事件id
        int video_id; // 视频id
        int tag_id;   // 标签id
    } VideoOfTagInfo;

    typedef struct TypeInfo
    {
        int type_id;
        std::string type_name;
    } TypeInfo;
}

namespace my
{
    // 执行SQL语句
    static bool ExcuteSQL(MYSQL *mysql, const std::string &sql)
    {
        LogMessage(Info, "ExcuteSQL SQL: %s\n", sql.c_str());
        int mysql_ret = mysql_real_query(mysql, sql.c_str(), sql.size());
        if (mysql_ret != 0)
        {
            LogMessage(Error, "ExcuteSQL SQL failed: %s\n", mysql_error(mysql));
            return false;
        }
        return true;
    }

    // 获取结果，序列化到Json::Value
    static bool GetResult(MYSQL *mysql, Json::Value &result, bool isOne = true)
    {
        MYSQL_RES *res = mysql_store_result(mysql);
        if (res == nullptr)
        {
            LogMessage(Error, "Get result failed: %s\n", mysql_error(mysql));
            return false;
        }
        MYSQL_ROW row;
        MYSQL_FIELD *fileds = mysql_fetch_fields(res);
        int num_fileds = mysql_num_fields(res);
        int num_rows = mysql_num_rows(res);
        if (num_rows == 0)
        {
            LogMessage(Info, "数据库结果为空\n");
            return false;
        }
        if (isOne)
        {
            row = mysql_fetch_row(res);
            for (int i = 0; i < num_fileds; i++)
            {
                if (row[i] == nullptr)
                {
                    result[fileds[i].name] = "NULL";
                }
                else
                {
                    result[fileds[i].name] = row[i];
                }
            }
            LogMessage(Info, "获取一个结果成功\n");
        }
        else
        {
            while (row = mysql_fetch_row(res))
            {
                Json::Value value;
                for (unsigned int i = 0; i < num_fileds; i++)
                {
                    value[fileds[i].name] = row[i];
                }
                result.append(value);
            }
            LogMessage(Info, "获取全部结果成功\n");
        }
        mysql_free_result(res);
        return true;
    }

    // MySQL句柄初始化函数
    static MYSQL *MySQLInit()
    {
        MYSQL *mysql = mysql_init(nullptr);
        if (mysql == nullptr)
        {
            LogMessage(Fatal, "mysql init failed!\n");
            return nullptr;
        }
        LogMessage(Info, "mysql_init success\n");
        if (mysql_real_connect(mysql, HOST, USER, PASSWD, DBNAME, 0, nullptr, 0) == nullptr)
        {
            LogMessage(Fatal, "connect mysql server failed!, error : %s\n", mysql_error(mysql));
            mysql_close(mysql);
            return nullptr;
        }
        mysql_set_character_set(mysql, "utf8");
        return mysql;
    }

    // 销毁MySQL句柄
    static void MySQLDestroy(MYSQL *mysql)
    {
        if (mysql == nullptr)
        {
            LogMessage(Warning, "MySQL句柄为空\n");
        }
        mysql_close(mysql);
    }

    class Video
    {
    public:
        // 插入视频
        bool Insert(Json::Value &value)
        {
#define INSERT_VIDEO_SQL "insert into %s %s values %s;"
            pthread_mutex_lock(&_db_mutex);
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

                // 根据值的类型决定是否添加引号
                switch ((*iter).type())
                {
                case Json::intValue:
                case Json::uintValue:
                case Json::realValue:
                    rightbuffer += (*iter).asString(); // 数字类型不需要引号
                    break;
                case Json::nullValue:
                    rightbuffer += "NULL";
                    break;
                default:
                    rightbuffer += "\"" + (*iter).asString() + "\""; // 字符串类型需要引号
                    break;
                }
            }
            leftbuffer += ")";
            rightbuffer += ")";
            char inbuffer[4096];
            sprintf(inbuffer, INSERT_VIDEO_SQL, _table_name.c_str(), leftbuffer.c_str(), rightbuffer.c_str());
            LogMessage(Info, "Insert SQL: %s\n", inbuffer);
            std::string sql(inbuffer);
            if (!ExcuteSQL(_mysql, sql))
            {
                LogMessage(Error, "数据库插入视频失败\n");
                pthread_mutex_unlock(&_db_mutex);
                return false;
            }
            LogMessage(Info, "数据库插入视频成功\n");
#define UPDATE_TYPE_COUNT_SQL "update type set type_count = type_count + 1 where type_id = %d;"
            sprintf(inbuffer, UPDATE_TYPE_COUNT_SQL, value["video_type_id"].asInt());
            sql = inbuffer;
            if (!ExcuteSQL(_mysql, sql))
            {
                LogMessage(Error, "数据库更新视频类型失败\n");
                pthread_mutex_unlock(&_db_mutex);
                return false;
            }
            LogMessage(Info, "数据库更新视频类型成功\n");
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        bool Delete(int video_id)
        {
#define DELETE_VIDEO_SQL1 "DELETE FROM comments WHERE video_id = %d;"
#define DELETE_VIDEO_SQL2 "DELETE FROM user_video_relation WHERE video_id = %d;"
#define DELETE_VIDEO_SQL3 "DELETE FROM video_of_tag WHERE video_id = %d;"
#define DELETE_VIDEO_SQL4 "DELETE FROM video WHERE video_id = %d;"
#define UPDATE_VIDEO_TYPE_COUNT_SQL "UPDATE type SET type_count = type_count - 1 WHERE type_id = (SELECT video_type_id FROM video WHERE video_id = %d);"
            pthread_mutex_lock(&_db_mutex);

            // 删除相关评论
            char inbuffer[1024];
            sprintf(inbuffer, DELETE_VIDEO_SQL1, video_id);
            LogMessage(Info, "Delete SQL: %s\n", inbuffer);
            ExcuteSQL(_mysql, inbuffer);

            // 删除用户视频关系
            sprintf(inbuffer, DELETE_VIDEO_SQL2, video_id);
            LogMessage(Info, "Delete SQL: %s\n", inbuffer);
            ExcuteSQL(_mysql, inbuffer);

            // 删除视频标签关系
            sprintf(inbuffer, DELETE_VIDEO_SQL3, video_id);
            LogMessage(Info, "Delete SQL: %s\n", inbuffer);
            ExcuteSQL(_mysql, inbuffer);

            sprintf(inbuffer, UPDATE_VIDEO_TYPE_COUNT_SQL, video_id);
            LogMessage(Info, "Update SQL: %s\n", inbuffer);
            ExcuteSQL(_mysql, inbuffer);

            // 删除视频
            sprintf(inbuffer, DELETE_VIDEO_SQL4, video_id);
            LogMessage(Info, "Delete SQL: %s\n", inbuffer);
            ExcuteSQL(_mysql, inbuffer);

            LogMessage(Info, "数据库删除视频成功, video_id : %d\n", video_id);
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        bool SelectOne(int video_id, Json::Value *result)
        {
#define SELECT_VIDEO_SQL_ONE "select * from video where video_id = %d;"
            pthread_mutex_lock(&_db_mutex);
            char buffer[1024];
            sprintf(buffer, SELECT_VIDEO_SQL_ONE, video_id);
            std::string sql(buffer);
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result)))
            {
                LogMessage(Error, "数据库查询单个视频失败, video_id : %d\n", video_id);
                return false;
            }
            LogMessage(Info, "数据库查询单个视频成功，video_id : %d\n", video_id);
            // 更新视频观看数
#define ADD_VIDEO_VIEWS "update video set views = views + 1 where video_id = %d;"
            char add_views_buffer[1024];
            sprintf(add_views_buffer, ADD_VIDEO_VIEWS, video_id);
            std::string add_views_sql(add_views_buffer);
            ExcuteSQL(_mysql, add_views_sql);
            // 更新视频热度
            //热度算法：观看数*2+点赞数*1+评论数*3+收藏数*4-踩数*1
#define UPDATE_VIDEO_HEAT "update video set heat = %d where video_id = %d;"
            char update_heat_buffer[1024];
            int heat = (int)(atoi((*result)["views"].asString().c_str()) * 2 + atoi((*result)["likes"].asString().c_str()) * 1 + atoi((*result)["comments"].asString().c_str()) * 3 + atoi((*result)["favorites"].asString().c_str()) * 4 - atoi((*result)["dislikes"].asString().c_str()) * 1);
            sprintf(update_heat_buffer, UPDATE_VIDEO_HEAT, heat, video_id);
            std::string update_heat_sql(update_heat_buffer);
            ExcuteSQL(_mysql, update_heat_sql);
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        bool SelectALL(Json::Value *result)
        {
#define SELECT_VIDEO_SQL_ALL "select * from video;"
            pthread_mutex_lock(&_db_mutex);
            std::string sql(SELECT_VIDEO_SQL_ALL);
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result, false)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库查询全部视频失败\n");
                return false;
            }
            LogMessage(Info, "数据库查询全部视频成功\n");
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        bool SelectLike(std::string &key, Json::Value *result)
        {
#define SELECT_VIDEO_SQL_LIKE "select * from video where video_name like '%%%s%%';"
            pthread_mutex_lock(&_db_mutex);
            char sql[1024];
            sprintf(sql, SELECT_VIDEO_SQL_LIKE, key.c_str());
            std::string sql_str(sql);
            if (!(ExcuteSQL(_mysql, sql_str) && GetResult(_mysql, *result, false)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库模糊查询视频失败, key : %s\n", key.c_str());
                return false;
            }
            LogMessage(Info, "数据库模糊查询视频成功, key : %s\n", key.c_str());
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        bool Update(Json::Value &value, Json::Value &condition, int video_id)
        {
#define UPDATE_VIDEO_SQL "update video set "
            pthread_mutex_lock(&_db_mutex);
            std::string sql(UPDATE_VIDEO_SQL);
            Json::Value::iterator iter = value.begin();
            for (; iter != value.end(); iter++)
            {
                if (iter != value.begin())
                    sql += ", ";

                sql += iter.key().asString() + " = ";

                // 根据值的类型决定是否添加引号
                switch ((*iter).type())
                {
                case Json::intValue:
                case Json::uintValue:
                case Json::realValue:
                    sql += (*iter).asString(); // 数字类型不需要引号
                    break;
                case Json::nullValue:
                    sql += "NULL";
                    break;
                default:
                    sql += "\"" + (*iter).asString() + "\""; // 字符串类型需要引号
                    break;
                }
            }

            sql += " where ";
            iter = condition.begin();
            for (; iter != condition.end(); iter++)
            {
                if (iter != condition.begin())
                    sql += " and ";

                sql += iter.key().asString() + " = ";

                // 根据值的类型决定是否添加引号
                switch ((*iter).type())
                {
                case Json::intValue:
                case Json::uintValue:
                case Json::realValue:
                    sql += (*iter).asString(); // 数字类型不需要引号
                    break;
                case Json::nullValue:
                    sql += "NULL";
                    break;
                default:
                    sql += "\"" + (*iter).asString() + "\""; // 字符串类型需要引号
                    break;
                }
            }
            sql += ";";
            LogMessage(Info, "Update SQL: %s\n", sql.c_str());
            if (!ExcuteSQL(_mysql, sql))
            {
                LogMessage(Error, "数据库更新视频失败, video_id : %d\n", video_id);
                return false;
            }
            LogMessage(Info, "数据库更新视频成功, video_id : %d\n", video_id);
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        // 获取TopK数据
        bool GetTopK(std::string type, const int k, Json::Value *result)
        {
#define GET_MOST_SQL "select * from video order by %s desc limit %d;"
            char buffer[1024];
            sprintf(buffer, GET_MOST_SQL, type.c_str(), k);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!ExcuteSQL(_mysql, sql))
            {
                LogMessage(Error, "数据库获取资源出错, type : %s, topk : %d\n", type.c_str(), k);
                pthread_mutex_unlock(&_db_mutex);
                return false;
            }
            GetResult(_mysql, *result, false);
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        // 获取某种类型的视频
        bool GetVideoOfType(int type_id, int number, Json::Value *result)
        {
#define GET_VIDEO_OF_TYPE_SQL "select * from video where video_type_id = %d order by release_time desc limit %d;"
            char buffer[1024];
            sprintf(buffer, GET_VIDEO_OF_TYPE_SQL, type_id, number);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result, false)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库获取某种类型的视频失败, type_id : %d, number : %d\n", type_id, number);
                return false;
            }
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        // 用户操作视频
        bool UpdateVideoRelation(int uid, int video_id, std::string grade, Json::Value &value)
        {
            // 先检查是否已经有了用户和视频的映射
#define CHECK_USER_VIDEO_RELATION_SQL "select * from user_video_relation where uid = %d and video_id = %d;"
            char buffer[1024];
            sprintf(buffer, CHECK_USER_VIDEO_RELATION_SQL, uid, video_id);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!ExcuteSQL(_mysql, sql))
            {
                LogMessage(Error, "数据库检查用户和视频映射失败, uid : %d, video_id : %d\n", uid, video_id);
                pthread_mutex_unlock(&_db_mutex);
                return false;
            }
            // 如果有就更新，没有先插入再更新
            Json::Value result;
            if (!GetResult(_mysql, result) && result.size() == 0) // 没有映射
            {
#define INSERT_USER_VIDEO_RELATION_SQL "insert into user_video_relation (uid, video_id) values (%d, %d);"
                sprintf(buffer, INSERT_USER_VIDEO_RELATION_SQL, uid, video_id);
                std::string insert_sql(buffer);
                if (!ExcuteSQL(_mysql, insert_sql))
                {
                    LogMessage(Error, "数据库插入用户和视频映射失败, uid : %d, video_id : %d\n", uid, video_id);
                    pthread_mutex_unlock(&_db_mutex);
                    return false;
                }
                LogMessage(Info, "数据库插入用户和视频映射成功, uid : %d, video_id : %d\n", uid, video_id);
            }
            pthread_mutex_unlock(&_db_mutex);
            UpdateVideoInfo(value, grade, video_id, uid);
#define UPDATE_USER_VIDEO_RELATION_SQL "update user_video_relation set %s, grade = %s where uid = %d and video_id = %d;"
            std::string set_str = ul::StringUtil::replaceDoubleQuotesWithSingle(ul::StringUtil::JsonToSetStr(value));
            sprintf(buffer, UPDATE_USER_VIDEO_RELATION_SQL, set_str.c_str(), grade.c_str(), uid, video_id);
            sql = buffer;
            pthread_mutex_lock(&_db_mutex);
            if (!ExcuteSQL(_mysql, sql))
            {
                LogMessage(Error, "数据库更新用户和视频映射失败, uid : %d, video_id : %d\n", uid, video_id);
                pthread_mutex_unlock(&_db_mutex);
                return false;
            }
            // 根据该条信息进行更新
            LogMessage(Info, "数据库根据用户和视频映射更新成功, uid : %d, video_id : %d\n", uid, video_id);
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        bool UpdateVideoInfo(Json::Value &value, std::string grade, int video_id, int uid)
        {
            // 获取到视频关系表
            Json::Value relation;
            if (!GetVideoRelation(uid, video_id, &relation))
            {
                LogMessage(Error, "数据库获取用户和视频的关系失败, uid : %d, video_id : %d\n", uid, video_id);
                return false;
            }
            // 根据关系表和value的值进行更新
            int set_likes = value["is_liked"].asInt();
            int set_dislike = value["dislike"].asInt();
            int set_favorite = value["favorite"].asInt();
            int set_grade = std::stoi(grade);
            char buffer[1024];
            pthread_mutex_lock(&_db_mutex);
            if (std::atoi(relation["is_liked"].asString().c_str()) != set_likes)
            {
                // 更新likes
                if (set_likes == 1) // 说明之前是未点赞
                {
#define UPDATE_LIKES_SQL_PLUS "update video set likes = likes + 1 where video_id = %d;"
                    sprintf(buffer, UPDATE_LIKES_SQL_PLUS, video_id);
                    std::string sql(buffer);
                    if (!ExcuteSQL(_mysql, sql))
                    {
                        LogMessage(Error, "数据库更新视频点赞数失败, video_id : %d\n", video_id);
                        pthread_mutex_unlock(&_db_mutex);
                        return false;
                    }
                }
                else // 说明之前是点赞
                {
#define UPDATE_LIKES_SQL_MINUS "update video set likes = likes - 1 where video_id = %d;"
                    sprintf(buffer, UPDATE_LIKES_SQL_MINUS, video_id);
                    std::string sql(buffer);
                    if (!ExcuteSQL(_mysql, sql))
                    {
                        LogMessage(Error, "数据库更新视频点赞数失败, video_id : %d\n", video_id);
                        pthread_mutex_unlock(&_db_mutex);
                        return false;
                    }
                }
            }
            if (std::atoi(relation["dislike"].asString().c_str()) != set_dislike)
            {
                // 更新dislike
                if (set_dislike == 1) // 说明之前是未点踩
                {
#define UPDATE_DISLIKE_SQL_PLUS "update video set dislikes = dislikes + 1 where video_id = %d;"
                    sprintf(buffer, UPDATE_DISLIKE_SQL_PLUS, video_id);
                    std::string sql(buffer);
                    if (!ExcuteSQL(_mysql, sql))
                    {
                        LogMessage(Error, "数据库更新视频点踩数失败, video_id : %d\n", video_id);
                        pthread_mutex_unlock(&_db_mutex);
                        return false;
                    }
                }
                else // 说明之前是点踩
                {
#define UPDATE_DISLIKE_SQL_MINUS "update video set dislikes = dislikes - 1 where video_id = %d;"
                    sprintf(buffer, UPDATE_DISLIKE_SQL_MINUS, video_id);
                    std::string sql(buffer);
                    if (!ExcuteSQL(_mysql, sql))
                    {
                        LogMessage(Error, "数据库更新视频点踩数失败, video_id : %d\n", video_id);
                        pthread_mutex_unlock(&_db_mutex);
                        return false;
                    }
                }
            }
            if (std::atoi(relation["favorite"].asString().c_str()) != set_favorite)
            {
                if (set_favorite == 1) // 说明之前是未收藏
                {
#define UPDATE_FAVORITE_SQL_PLUS "update video set favorites = favorites + 1 where video_id = %d;"
                    sprintf(buffer, UPDATE_FAVORITE_SQL_PLUS, video_id);
                    std::string sql(buffer);
                    if (!ExcuteSQL(_mysql, sql))
                    {
                        LogMessage(Error, "数据库更新视频收藏数失败, video_id : %d\n", video_id);
                        pthread_mutex_unlock(&_db_mutex);
                        return false;
                    }
                }
                else // 说明之前是收藏
                {
#define UPDATE_FAVORITE_SQL_MINUS "update video set favorites = favorites - 1 where video_id = %d;"
                    sprintf(buffer, UPDATE_FAVORITE_SQL_MINUS, video_id);
                    std::string sql(buffer);
                    if (!ExcuteSQL(_mysql, sql))
                    {
                        LogMessage(Error, "数据库更新视频收藏数失败, video_id : %d\n", video_id);
                        pthread_mutex_unlock(&_db_mutex);
                        return false;
                    }
                }
            }
            // 更新grade，这个比较复杂，应当是视频的评分
            if (std::atoi(relation["grade"].asString().c_str()) != set_grade)
            {
#define UPDATE_GRADE_SQL "update video set grade = %d where video_id = %d;"
                sprintf(buffer, UPDATE_GRADE_SQL, set_grade, video_id);
                std::string sql(buffer);
                if (!ExcuteSQL(_mysql, sql))
                {
                    LogMessage(Error, "数据库更新视频评分失败, video_id : %d\n", video_id);
                    pthread_mutex_unlock(&_db_mutex);
                    return false;
                }
            }
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        // 获取用户和视频的关系
        bool GetVideoRelation(int uid, int video_id, Json::Value *result)
        {
            // 检查有没有，没有就创建
#define CHECK_VIDEO_RELATION_SQL "select * from user_video_relation where uid = %d and video_id = %d;"
            char buffer[1024];
            sprintf(buffer, CHECK_VIDEO_RELATION_SQL, uid, video_id);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!(ExcuteSQL(_mysql, sql)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库获取用户和视频的关系失败, uid : %d, video_id : %d\n", uid, video_id);
                return false;
            }
            // 有就返回，没有就创建
            if (GetResult(_mysql, *result) && result->size() != 0)
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Info, "数据库获取用户和视频的关系成功, uid : %d, video_id : %d\n", uid, video_id);
                return true;
            }
            else
            {
                // 创建
#define INSERT_VIDEO_RELATION_SQL "insert into user_video_relation (uid, video_id) values (%d, %d);"
                sprintf(buffer, INSERT_VIDEO_RELATION_SQL, uid, video_id);
                sql = buffer;
                if (!ExcuteSQL(_mysql, sql))
                {
                    pthread_mutex_unlock(&_db_mutex);
                    LogMessage(Error, "数据库创建用户和视频的关系失败, uid : %d, video_id : %d\n", uid, video_id);
                    return false;
                }
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Info, "数据库创建用户和视频的关系成功, uid : %d, video_id : %d\n", uid, video_id);
                return true;
            }
        }

        Video(pthread_mutex_t &db_mutex, std::string table_name = VIDEO_TABLE_NAME)
            : _db_mutex(db_mutex), _table_name(table_name)
        {
            _mysql = MySQLInit();
        }

        ~Video()
        {
            MySQLDestroy(_mysql);
        }

    private:
        std::string _table_name = VIDEO_TABLE_NAME; // 表名
        MYSQL *_mysql;                              // MySQL句柄

        pthread_mutex_t _db_mutex;
    };

    class User
    {
    public:
        User(pthread_mutex_t &db_mutex, std::string table_name = USER_TABLE_NAME)
            : _db_mutex(db_mutex), _table_name(table_name)
        {
            _mysql = MySQLInit();
        }

        ~User()
        {
            MySQLDestroy(_mysql);
        }

        // 登录
        bool Login(const std::string &username, const std::string &password, int *uid)
        {
#define CHECK_USER "select * from user where username = '%s';"
#define CHECK_USERNAME_AND_PASSWORD "select * from user where username = '%s' and password = '%s';"
            char buffer[1024];
            sprintf(buffer, CHECK_USER, username.c_str());
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!ExcuteSQL(_mysql, sql))
            {
                LogMessage(Error, "数据库执行检查用户存在出错\n");
                pthread_mutex_unlock(&_db_mutex);
                return false;
            }
            MYSQL_RES *res = mysql_store_result(_mysql);
            int num = mysql_num_rows(res);
            if (num != 1)
            {
                LogMessage(Info, "用户登录失败，数据库没有该用户, username : %s, password : %s\n", username.c_str(), password.c_str());
                pthread_mutex_unlock(&_db_mutex);
                mysql_free_result(res);
                return false;
            }
            MYSQL_ROW row = mysql_fetch_row(res);
            *uid = std::stoi(row[0]);
            LogMessage(Info, "用户登录成功, username : %s, password : %s, uid : %d\n", username.c_str(), password.c_str(), *uid);
            pthread_mutex_unlock(&_db_mutex);
            mysql_free_result(res);
            return true;
        }

        // 注册
        bool Register(const std::string &name, const std::string &username, const std::string &password)
        {
#define REGISTER_SQL "insert into user (name, username, password) values ('%s', '%s', '%s');"
            char buffer[1024];
            sprintf(buffer, CHECK_USER, username.c_str());
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!ExcuteSQL(_mysql, sql))
            {
                LogMessage(Error, "数据库执行检查用户存在出错\n");
                pthread_mutex_unlock(&_db_mutex);
                return false;
            }
            MYSQL_RES *res = mysql_store_result(_mysql);
            int num = mysql_num_rows(res);
            if (num != 0)
            {
                LogMessage(Info, "用户注册失败，数据库已有该用户名, name : %s, username : %s, password : %s\n", name.c_str(), username.c_str(), password.c_str());
                pthread_mutex_unlock(&_db_mutex);
                mysql_free_result(res);
                return false;
            }
            sprintf(buffer, REGISTER_SQL, name.c_str(), username.c_str(), password.c_str());
            sql = buffer;
            if (!ExcuteSQL(_mysql, sql))
            {
                LogMessage(Error, "数据库执行注册用户出错, name : %s, username : %s, password : %s\n", name.c_str(), username.c_str(), password.c_str());
                pthread_mutex_unlock(&_db_mutex);
                mysql_free_result(res);
                return false;
            }
            LogMessage(Info, "用户注册成功, name : %s, username : %s, password : %s\n", name.c_str(), username.c_str(), password.c_str());
            pthread_mutex_unlock(&_db_mutex);
            mysql_free_result(res);
            return true;
        }

        bool UpdateUserInfo(const Json::Value &body, const int uid)
        {
#define UPDATE_USER_INFO "update user set %s where uid = %d;"
            Json::Value::const_iterator it = body.begin();
            std::string leftstr = "";
            Json::ValueType type = Json::nullValue;
            while (it != body.end())
            {
                if (it != body.begin())
                    leftstr += ", ";
                type = it->type();
                leftstr += it.key().asString() + " = ";
                Json::Value value = *it;
                if (type == Json::stringValue)
                    leftstr += value.toStyledString();
                else
                    leftstr += value.asString();
                it++;
            }
            char buffer[1024];
            sprintf(buffer, UPDATE_USER_INFO, leftstr.c_str(), uid);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            LogMessage(Info, "更新用户信息, sql : %s\n", sql.c_str());
            if (!ExcuteSQL(_mysql, sql))
            {
                LogMessage(Error, "数据库执行更新用户信息失败, uid = %d\n", uid);
                pthread_mutex_unlock(&_db_mutex);
                return false;
            }
            LogMessage(Info, "数据库更新用户信息成功, uid = %d\n", uid);
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        bool GetUserInfo(Json::Value *result, int uid)
        {
#define GET_USER_INFO "select * from user where uid = %d;"
            char buffer[1024];
            sprintf(buffer, GET_USER_INFO, uid);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库获取用户信息失败, uid : %d\n", uid);
                return false;
            }
            pthread_mutex_unlock(&_db_mutex);
            LogMessage(Info, "数据库获取用户信息成功, uid : %d\n", uid);
            return true;
        }

        bool GetVideoUserInfo(int video_id, Json::Value *result)
        {
#define GET_VIDEO_USER_INFO "select * from user where uid = (select uid from video where video_id = %d);"
            char buffer[1024];
            sprintf(buffer, GET_VIDEO_USER_INFO, video_id);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result)))
            {
                pthread_mutex_unlock(&_db_mutex);
                return false;
            }
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        bool GetCommentUserInfo(int video_id, Json::Value *result)
        {
#define GET_COMMENT_USER_INFO "select * from user where uid in (select sender_uid from comments where video_id = %d);"
            char buffer[1024];
            sprintf(buffer, GET_COMMENT_USER_INFO, video_id);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result, false)))
            {
                pthread_mutex_unlock(&_db_mutex);
                return false;
            }
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        bool GetUserVideo(int uid, Json::Value *result)
        {
#define GET_USER_VIDEO "select * from video where uid = %d;"
            char buffer[1024];
            sprintf(buffer, GET_USER_VIDEO, uid);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result, false)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库获取用户所有视频失败, uid : %d\n", uid);
                return false;
            }
            pthread_mutex_unlock(&_db_mutex);
            LogMessage(Info, "数据库获取用户所有视频成功, uid : %d\n", uid);
            return true;
        }

        bool GetFavorite(int uid, Json::Value *result)
        {
#define GET_FAVORITE "select video_id from user_video_relation where uid = %d and favorite = 1;"
            char buffer[1024];
            sprintf(buffer, GET_FAVORITE, uid);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result, false)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库获取用户收藏失败, uid : %d\n", uid);
                return false;
            }
            std::vector<int> video_ids;
            for (int i = 0; i < result->size(); i++)
            {
                video_ids.push_back(std::stoi((*result)[i]["video_id"].asString()));
            }
            // 清空result
            result->clear();
#define GET_FAVORITE_VIDEO "select * from video where video_id in (%s);"
            sprintf(buffer, GET_FAVORITE_VIDEO, ul::StringUtil::VectorToString(video_ids).c_str());
            sql = buffer;
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result, false)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库获取用户收藏视频失败, uid : %d\n", uid);
                return false;
            }
            pthread_mutex_unlock(&_db_mutex);
            LogMessage(Info, "数据库获取用户收藏成功, uid : %d\n", uid);
            return true;
        }

    private:
        std::string _table_name = USER_TABLE_NAME; // 表名
        MYSQL *_mysql;                             // MySQL句柄

        pthread_mutex_t _db_mutex;
    };

    class Tag
    {
    public:
        Tag(pthread_mutex_t &db_mutex)
            : _db_mutex(db_mutex)
        {
            _mysql = MySQLInit();
        }

        ~Tag()
        {
            MySQLDestroy(_mysql);
        }

        bool GetTotal(Json::Value *result)
        {
#define GET_TAG_TOTAL "select * from tag;"
            std::string sql(GET_TAG_TOTAL);
            pthread_mutex_lock(&_db_mutex);
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result, false)))
            {
                pthread_mutex_unlock(&_db_mutex);
                return false;
            }
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        bool GetVideoTag(int video_id, Json::Value *result)
        {
#define GET_VIDEO_OF_TAG "select * from video_of_tag where video_id = %d;"
            char buffer[1024];
            sprintf(buffer, GET_VIDEO_OF_TAG, video_id);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            Json::Value value;
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, value, false)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库获取视频标签失败, video_id : %d\n", video_id);
                return false;
            }
            std::vector<int> tag_ids;
            for (int i = 0; i < value.size(); i++)
            {
                tag_ids.push_back(std::stoi(value[i]["tag_id"].asString()));
            }
#define GET_TAG_SQL "select * from tag where tag_id in (%s);"
            sprintf(buffer, GET_TAG_SQL, ul::StringUtil::VectorToString(tag_ids).c_str());
            sql = buffer;
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result, false)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库获取视频标签失败, video_id : %d\n", video_id);
                return false;
            }
            pthread_mutex_unlock(&_db_mutex);
            LogMessage(Info, "数据库获取视频标签成功, video_id : %d\n", video_id);
            return true;
        }

    private:
        std::string _table_name = TAG_TABLE_NAME; // 表名
        MYSQL *_mysql;                            // MySQL句柄

        pthread_mutex_t _db_mutex;
    };

    class Type
    {
    public:
        Type(pthread_mutex_t &db_mutex)
            : _db_mutex(db_mutex)
        {
            _mysql = MySQLInit();
        }

        ~Type()
        {
            MySQLDestroy(_mysql);
        }

        bool GetTotal(Json::Value *result)
        {
#define GET_TYPE_TOTAL "select * from type;"
            std::string sql(GET_TYPE_TOTAL);
            pthread_mutex_lock(&_db_mutex);
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result, false)))
            {
                pthread_mutex_unlock(&_db_mutex);
                return false;
            }
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        bool GetVideoType(int video_id, Json::Value *result)
        {
#define GET_VIDEO_TYPE "select video_type_id from video where video_id = %d;"
            char buffer[1024];
            sprintf(buffer, GET_VIDEO_TYPE, video_id);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            Json::Value value;
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, value)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库获取视频类型失败, video_id : %d\n", video_id);
                return false;
            }
            int type_id = std::stoi(value["video_type_id"].asString());
#define GET_TYPE_SQL "select * from type where type_id = %d;"
            sprintf(buffer, GET_TYPE_SQL, type_id);
            sql = buffer;
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库获取视频类型失败, video_id : %d\n", video_id);
                return false;
            }
            pthread_mutex_unlock(&_db_mutex);
            LogMessage(Info, "数据库获取视频类型成功, video_id : %d\n", video_id);
            return true;
        }

    private:
        std::string _table_name = TYPE_TABLE_NAME; // 表名
        MYSQL *_mysql;                             // MySQL句柄

        pthread_mutex_t _db_mutex;
    };

    class Comment
    {
    public:
        Comment(pthread_mutex_t &db_mutex)
            : _db_mutex(db_mutex)
        {
            _mysql = MySQLInit();
        }

        ~Comment()
        {
            MySQLDestroy(_mysql);
        }

        bool GetComment(int video_id, Json::Value *result)
        {
#define GET_COMMENT_SQL "select * from comments where video_id = %d;"
            char buffer[1024];
            sprintf(buffer, GET_COMMENT_SQL, video_id);
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!(ExcuteSQL(_mysql, sql) && GetResult(_mysql, *result, false)))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库获取视频评论失败, video_id : %d\n", video_id);
                return false;
            }
            pthread_mutex_unlock(&_db_mutex);
            LogMessage(Info, "数据库获取视频评论成功, video_id : %d\n", video_id);
            return true;
        }

        bool SendComment(int video_id, int sender_uid, const std::string &comment)
        {
#define SEND_COMMENT_SQL "insert into comments (video_id, sender_uid, comment_text) values (%d, %d, '%s');"
            char buffer[1024];
            sprintf(buffer, SEND_COMMENT_SQL, video_id, sender_uid, comment.c_str());
            std::string sql(buffer);
            pthread_mutex_lock(&_db_mutex);
            if (!ExcuteSQL(_mysql, sql))
            {
                pthread_mutex_unlock(&_db_mutex);
                LogMessage(Error, "数据库插入视频评论失败, video_id : %d, sender_uid : %d, comment : %s\n", video_id, sender_uid, comment.c_str());
                return false;
            }
            pthread_mutex_unlock(&_db_mutex);
            LogMessage(Info, "数据库插入视频评论成功, video_id : %d, sender_uid : %d, comment : %s\n", video_id, sender_uid, comment.c_str());
            return true;
        }

    private:
        std::string _table_name = COMMENT_TABLE_NAME; // 表名
        MYSQL *_mysql;                                // MySQL句柄

        pthread_mutex_t _db_mutex;
    };
}