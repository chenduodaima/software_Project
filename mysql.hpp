#pragma once
#include <iostream>
#include <mariadb/mysql.h>
#include <jsoncpp/json/json.h>
#include <vector>
#include <pthread.h>
#include "Log.hpp"

namespace my
{
#define HOST "127.0.0.1"
#define USER "root"
#define PASSWD "root"
#define DBNAME "software_project"

    // 视频信息结构体
    struct VideoInfo
    {
        int id;                 // 视频id
        std::string name;       // 视频名称
        std::string author;     // 作者
        std::string info;       // 视频简介
        std::string video_path; // 视频资源路径
        std::string img_path;   // 视频封面资源路径
        int views;              // 观看量
        int likes;              // 点赞数
        int favorites;          // 收藏数
        int comments;           // 评论数
        std::string create_at;  // 创建时间
        std::string update_at;  // 更新时间
        int user_id;            // 用户id
        int heat;               // 视频热度
        float rating;           // 视频评分(0.0-5.0)
        std::string duration;   // 视频时长
    };
    // 打印视频信息，只打印视频名称、作者、视频简介、视频资源路径、视频封面路径
    static void PrintVideoInfo(const VideoInfo &video)
    {
        LogMessage(Info, "VideoInfo: name: %s, author: %s, info: %s, video_path: %s, img_path: %s\n",
                   video.name.c_str(), video.author.c_str(), video.info.c_str(), video.video_path.c_str(), video.img_path.c_str());
    }

    // MySQL初始化函数
    static MYSQL *MySQLInit()
    {
        MYSQL *mysql = mysql_init(nullptr);
        if (mysql == nullptr)
            return nullptr;
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
        LogMessage(Info, "Execute SQL: %s\n", sql.c_str());
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
        Video(std::string table_name = TABLE_NAME)
            : _table_name(table_name),
              _heat_video(8), _max_heat_video(6), _newest_video(6),
              _most_viewed_video(6), _most_commented_video(6), _most_liked_video(6),
              _most_favorited_video(6), _top_rated_video(6),
              _running(true), _db_busy(false)
        {
            _mysql = MySQLInit();

            // 初始化所有互斥锁
            pthread_mutex_init(&_heat_mutex, nullptr);
            pthread_mutex_init(&_max_heat_mutex, nullptr);
            pthread_mutex_init(&_newest_mutex, nullptr);
            pthread_mutex_init(&_viewed_mutex, nullptr);
            pthread_mutex_init(&_commented_mutex, nullptr);
            pthread_mutex_init(&_liked_mutex, nullptr);
            pthread_mutex_init(&_favorited_mutex, nullptr);
            pthread_mutex_init(&_rated_mutex, nullptr);

            // 初始化数据库互斥锁和条件变量
            pthread_mutex_init(&_db_mutex, nullptr);
            pthread_cond_init(&_db_cond, nullptr);

            if (_mysql)
            {
                // 启动更新线程
                _update_threads.emplace_back(&Video::UpdateHeatVideos, this);
                _update_threads.emplace_back(&Video::UpdateMaxHeatVideos, this);
                _update_threads.emplace_back(&Video::UpdateNewestVideos, this);
                _update_threads.emplace_back(&Video::UpdateMostViewedVideos, this);
                _update_threads.emplace_back(&Video::UpdateMostCommentedVideos, this);
                _update_threads.emplace_back(&Video::UpdateMostLikedVideos, this);
                _update_threads.emplace_back(&Video::UpdateMostFavoritedVideos, this);
                _update_threads.emplace_back(&Video::UpdateTopRatedVideos, this);
            }
        }

        ~Video()
        {
            _running = false;

            // 等待所有线程结束
            for (auto &thread : _update_threads)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }

            // 销毁所有互斥锁
            pthread_mutex_destroy(&_heat_mutex);
            pthread_mutex_destroy(&_max_heat_mutex);
            pthread_mutex_destroy(&_newest_mutex);
            pthread_mutex_destroy(&_viewed_mutex);
            pthread_mutex_destroy(&_commented_mutex);
            pthread_mutex_destroy(&_liked_mutex);
            pthread_mutex_destroy(&_favorited_mutex);
            pthread_mutex_destroy(&_rated_mutex);

            // 销毁数据库互斥锁和条件变量
            pthread_mutex_destroy(&_db_mutex);
            pthread_cond_destroy(&_db_cond);

            MysqlDestroy(_mysql);
        }

        bool Insert(Json::Value &value)
        {
            if (!AcquireDBLock())
            {
                return false;
            }
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
            bool ret = Execute(_mysql, sql);
            ReleaseDBLock();
            return ret;
        }
        bool Delete(int id)
        {
            if (!AcquireDBLock())
            {
                return false;
            }
#define DELETE_VIDEO_SQL_1 "delete from %s where id = %d;"
            char inbuffer[1024];
            int n = sprintf(inbuffer, DELETE_VIDEO_SQL_1, _table_name.c_str(), id);
            if (n <= 0)
            {
                LogMessage(Error, "write failed\n");
                ReleaseDBLock();
                return false;
            }
            LogMessage(Info, "Delete SQL: %s\n", inbuffer);
            std::string sql(inbuffer);
            bool ret = Execute(_mysql, sql);
            ReleaseDBLock();
            return ret;
        }

        bool Delete(Json::Value &value)
        {
            if (!AcquireDBLock())
            {
                return false;
            }
            std::string sql;
            Json::Value::iterator iter = value.begin();
            for (; iter != value.end(); iter++)
            {
                if (iter != value.begin())
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
#define DELETE_VIDEO_SQL_2 "delete from video where "
            sql = DELETE_VIDEO_SQL_2 + sql + ";";
            LogMessage(Info, "Delete SQL: %s\n", sql.c_str());
            bool ret = Execute(_mysql, sql);
            ReleaseDBLock();
            return ret;
        }

        bool SelectOne(int id, Json::Value *result)
        {
            if (!AcquireDBLock())
            {
                return false;
            }
#define SELECT_VIDEO_SQL_ONE "select * from video where id = %d;"
            char sql[1024];
            sprintf(sql, SELECT_VIDEO_SQL_ONE, id);
            std::string sql_str(sql);
            bool ret = Execute(_mysql, sql_str) && GetOneResult(*result);
            ReleaseDBLock();
            return ret;
        }

        bool SelectALL(Json::Value *result)
        {
            if (!AcquireDBLock())
            {
                return false;
            }
#define SELECT_VIDEO_SQL_ALL "select * from video;"
            std::string sql(SELECT_VIDEO_SQL_ALL);
            bool ret = Execute(_mysql, sql) && GetALLResult(result);
            ReleaseDBLock();
            return ret;
        }

        bool SelectLike(std::string &key, Json::Value *result)
        {
            if (!AcquireDBLock())
            {
                return false;
            }
#define SELECT_VIDEO_SQL_LIKE "select * from video where name like '%%%s%%';"
            char sql[1024];
            sprintf(sql, SELECT_VIDEO_SQL_LIKE, key.c_str());
            std::string sql_str(sql);
            bool ret = Execute(_mysql, sql_str) && GetALLResult(result);
            ReleaseDBLock();
            return ret;
        }

        bool Update(Json::Value &value, Json::Value &condition)
        {
            if (!AcquireDBLock())
            {
                return false;
            }
#define UPDATE_VIDEO_SQL "update video set "
            std::string sql;
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
            bool ret = Execute(_mysql, sql);
            ReleaseDBLock();
            return ret;
        }

        // 获取缓存的视频列表
        bool GetHeatVideos(Json::Value *result)
        {
            if (result == nullptr)
            {
                LogMessage(Error, "result pointer is nullptr\n");
                return false;
            }

            if (pthread_mutex_trylock(&_heat_mutex) != 0)
            {
                LogMessage(Warning, "heat mutex is locked, try later\n");
                return false;
            }

            for (const auto &video : _heat_video)
            {
                Json::Value video_json;
                video_json["id"] = video.id;
                video_json["name"] = video.name;
                video_json["author"] = video.author;
                video_json["info"] = video.info;
                video_json["video_path"] = video.video_path;
                video_json["img_path"] = video.img_path;
                video_json["views"] = video.views;
                video_json["likes"] = video.likes;
                video_json["favorites"] = video.favorites;
                video_json["comments"] = video.comments;
                video_json["create_at"] = video.create_at;
                video_json["update_at"] = video.update_at;
                video_json["user_id"] = video.user_id;
                video_json["heat"] = video.heat;
                video_json["rating"] = video.rating;
                video_json["duration"] = video.duration;
                result->append(video_json);
            }
            pthread_mutex_unlock(&_heat_mutex);
            return true;
        }

        bool GetMaxHeatVideos(Json::Value *result)
        {
            if (result == nullptr)
            {
                LogMessage(Error, "result pointer is nullptr\n");
                return false;
            }

            if (pthread_mutex_trylock(&_max_heat_mutex) != 0)
            {
                LogMessage(Warning, "max heat mutex is locked, try later\n");
                return false;
            }

            for (const auto &video : _max_heat_video)
            {
                Json::Value video_json;
                video_json["id"] = video.id;
                video_json["name"] = video.name;
                video_json["author"] = video.author;
                video_json["info"] = video.info;
                video_json["video_path"] = video.video_path;
                video_json["img_path"] = video.img_path;
                video_json["views"] = video.views;
                video_json["likes"] = video.likes;
                video_json["favorites"] = video.favorites;
                video_json["comments"] = video.comments;
                video_json["create_at"] = video.create_at;
                video_json["update_at"] = video.update_at;
                video_json["user_id"] = video.user_id;
                video_json["heat"] = video.heat;
                video_json["rating"] = video.rating;
                video_json["duration"] = video.duration;
                result->append(video_json);
            }
            pthread_mutex_unlock(&_max_heat_mutex);
            return true;
        }

        bool GetNewestVideos(Json::Value *result)
        {
            if (result == nullptr)
            {
                LogMessage(Error, "result pointer is nullptr\n");
                return false;
            }

            if (pthread_mutex_trylock(&_newest_mutex) != 0)
            {
                LogMessage(Warning, "newest mutex is locked, try later\n");
                return false;
            }

            for (const auto &video : _newest_video)
            {
                Json::Value video_json;
                video_json["id"] = video.id;
                video_json["name"] = video.name;
                video_json["author"] = video.author;
                video_json["info"] = video.info;
                video_json["video_path"] = video.video_path;
                video_json["img_path"] = video.img_path;
                video_json["views"] = video.views;
                video_json["likes"] = video.likes;
                video_json["favorites"] = video.favorites;
                video_json["comments"] = video.comments;
                video_json["create_at"] = video.create_at;
                video_json["update_at"] = video.update_at;
                video_json["user_id"] = video.user_id;
                video_json["heat"] = video.heat;
                video_json["rating"] = video.rating;
                video_json["duration"] = video.duration;
                result->append(video_json);
            }
            pthread_mutex_unlock(&_newest_mutex);
            return true;
        }

        bool GetMostViewedVideos(Json::Value *result)
        {
            if (result == nullptr)
            {
                LogMessage(Error, "result pointer is nullptr\n");
                return false;
            }

            if (pthread_mutex_trylock(&_viewed_mutex) != 0)
            {
                LogMessage(Warning, "viewed mutex is locked, try later\n");
                return false;
            }

            for (const auto &video : _most_viewed_video)
            {
                Json::Value video_json;
                video_json["id"] = video.id;
                video_json["name"] = video.name;
                video_json["author"] = video.author;
                video_json["info"] = video.info;
                video_json["video_path"] = video.video_path;
                video_json["img_path"] = video.img_path;
                video_json["views"] = video.views;
                video_json["likes"] = video.likes;
                video_json["favorites"] = video.favorites;
                video_json["comments"] = video.comments;
                video_json["create_at"] = video.create_at;
                video_json["update_at"] = video.update_at;
                video_json["user_id"] = video.user_id;
                video_json["heat"] = video.heat;
                video_json["rating"] = video.rating;
                video_json["duration"] = video.duration;
                result->append(video_json);
            }
            pthread_mutex_unlock(&_viewed_mutex);
            return true;
        }

        bool GetMostCommentedVideos(Json::Value *result)
        {
            if (result == nullptr)
            {
                LogMessage(Error, "result pointer is nullptr\n");
                return false;
            }

            if (pthread_mutex_trylock(&_commented_mutex) != 0)
            {
                LogMessage(Warning, "commented mutex is locked, try later\n");
                return false;
            }

            for (const auto &video : _most_commented_video)
            {
                Json::Value video_json;
                video_json["id"] = video.id;
                video_json["name"] = video.name;
                video_json["author"] = video.author;
                video_json["info"] = video.info;
                video_json["video_path"] = video.video_path;
                video_json["img_path"] = video.img_path;
                video_json["views"] = video.views;
                video_json["likes"] = video.likes;
                video_json["favorites"] = video.favorites;
                video_json["comments"] = video.comments;
                video_json["create_at"] = video.create_at;
                video_json["update_at"] = video.update_at;
                video_json["user_id"] = video.user_id;
                video_json["heat"] = video.heat;
                video_json["rating"] = video.rating;
                video_json["duration"] = video.duration;
                result->append(video_json);
            }
            pthread_mutex_unlock(&_commented_mutex);
            return true;
        }

        bool GetMostLikedVideos(Json::Value *result)
        {
            if (result == nullptr)
            {
                LogMessage(Error, "result pointer is nullptr\n");
                return false;
            }

            if (pthread_mutex_trylock(&_liked_mutex) != 0)
            {
                LogMessage(Warning, "liked mutex is locked, try later\n");
                return false;
            }

            for (const auto &video : _most_liked_video)
            {
                Json::Value video_json;
                video_json["id"] = video.id;
                video_json["name"] = video.name;
                video_json["author"] = video.author;
                video_json["info"] = video.info;
                video_json["video_path"] = video.video_path;
                video_json["img_path"] = video.img_path;
                video_json["views"] = video.views;
                video_json["likes"] = video.likes;
                video_json["favorites"] = video.favorites;
                video_json["comments"] = video.comments;
                video_json["create_at"] = video.create_at;
                video_json["update_at"] = video.update_at;
                video_json["user_id"] = video.user_id;
                video_json["heat"] = video.heat;
                video_json["rating"] = video.rating;
                video_json["duration"] = video.duration;
                result->append(video_json);
            }
            pthread_mutex_unlock(&_liked_mutex);
            return true;
        }

        bool GetMostFavoritedVideos(Json::Value *result)
        {
            if (result == nullptr)
            {
                LogMessage(Error, "result pointer is nullptr\n");
                return false;
            }

            if (pthread_mutex_trylock(&_favorited_mutex) != 0)
            {
                LogMessage(Warning, "favorited mutex is locked, try later\n");
                return false;
            }

            for (const auto &video : _most_favorited_video)
            {
                Json::Value video_json;
                video_json["id"] = video.id;
                video_json["name"] = video.name;
                video_json["author"] = video.author;
                video_json["info"] = video.info;
                video_json["video_path"] = video.video_path;
                video_json["img_path"] = video.img_path;
                video_json["views"] = video.views;
                video_json["likes"] = video.likes;
                video_json["favorites"] = video.favorites;
                video_json["comments"] = video.comments;
                video_json["create_at"] = video.create_at;
                video_json["update_at"] = video.update_at;
                video_json["user_id"] = video.user_id;
                video_json["heat"] = video.heat;
                video_json["rating"] = video.rating;
                video_json["duration"] = video.duration;
                result->append(video_json);
            }
            pthread_mutex_unlock(&_favorited_mutex);
            return true;
        }

        bool GetTopRatedVideos(Json::Value *result)
        {
            if (result == nullptr)
            {
                LogMessage(Error, "result pointer is nullptr\n");
                return false;
            }

            if (pthread_mutex_trylock(&_rated_mutex) != 0)
            {
                LogMessage(Warning, "rated mutex is locked, try later\n");
                return false;
            }

            for (const auto &video : _top_rated_video)
            {
                Json::Value video_json;
                video_json["id"] = video.id;
                video_json["name"] = video.name;
                video_json["author"] = video.author;
                video_json["info"] = video.info;
                video_json["video_path"] = video.video_path;
                video_json["img_path"] = video.img_path;
                video_json["views"] = video.views;
                video_json["likes"] = video.likes;
                video_json["favorites"] = video.favorites;
                video_json["comments"] = video.comments;
                video_json["create_at"] = video.create_at;
                video_json["update_at"] = video.update_at;
                video_json["user_id"] = video.user_id;
                video_json["heat"] = video.heat;
                video_json["rating"] = video.rating;
                video_json["duration"] = video.duration;
                result->append(video_json);
            }
            pthread_mutex_unlock(&_rated_mutex);
            return true;
        }

    private:
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
            video["id"] = atoi(row[0]);
            video["name"] = row[1];
            video["author"] = row[2];
            video["info"] = row[3];
            video["video_path"] = row[4];
            video["img_path"] = row[5];
            video["views"] = atoi(row[6]);
            video["likes"] = atoi(row[7]);
            video["favorites"] = atoi(row[8]);
            video["comments"] = atoi(row[9]);
            video["create_at"] = row[10];
            video["update_at"] = row[11];
            video["user_id"] = atoi(row[12]);
            video["heat"] = atoi(row[13]);
            video["rating"] = atof(row[14]);
            video["duration"] = row[15];

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
            while ((row = mysql_fetch_row(result)))
            {
                Json::Value video;
                video["id"] = atoi(row[0]);
                video["name"] = row[1];
                video["author"] = row[2];
                video["info"] = row[3];
                video["video_path"] = row[4];
                video["img_path"] = row[5];
                video["views"] = atoi(row[6]);
                video["likes"] = atoi(row[7]);
                video["favorites"] = atoi(row[8]);
                video["comments"] = atoi(row[9]);
                video["create_at"] = row[10];
                video["update_at"] = row[11];
                video["user_id"] = atoi(row[12]);
                video["heat"] = atoi(row[13]);
                video["rating"] = atof(row[14]);
                video["duration"] = row[15];

                videos->append(video);
            }
            mysql_free_result(result);
            return true;
        }

        bool AcquireDBLock()
        {
            pthread_mutex_lock(&_db_mutex);
            while (_db_busy)
            {
                pthread_cond_wait(&_db_cond, &_db_mutex);
            }
            _db_busy = true;
            pthread_mutex_unlock(&_db_mutex);
            return true;
        }

        void ReleaseDBLock()
        {
            pthread_mutex_lock(&_db_mutex);
            _db_busy = false;
            pthread_cond_signal(&_db_cond);
            pthread_mutex_unlock(&_db_mutex);
        }

        void UpdateHeatVideos()
        {
            while (_running)
            {
                if (AcquireDBLock())
                {
                    LogMessage(Debug, "我抢到数据库锁了\n");
                    std::string sql = "SELECT * FROM video ORDER BY heat DESC LIMIT 8;";
                    if (Execute(_mysql, sql))
                    {
                        LogMessage(Info, "更新热门视频\n");
                        std::vector<VideoInfo> temp_videos;
                        MYSQL_RES *result = mysql_store_result(_mysql);
                        if (result)
                        {
                            MYSQL_ROW row;
                            while ((row = mysql_fetch_row(result)))
                            {
                                VideoInfo video;
                                video.id = atoi(row[0]);
                                video.name = row[1];
                                video.author = row[2];
                                video.info = row[3];
                                video.video_path = row[4];
                                video.img_path = row[5];
                                video.views = atoi(row[6]);
                                video.likes = atoi(row[7]);
                                video.favorites = atoi(row[8]);
                                video.comments = atoi(row[9]);
                                video.create_at = row[10];
                                video.update_at = row[11];
                                video.user_id = atoi(row[12]);
                                video.heat = atoi(row[13]);
                                video.rating = atof(row[14]);
                                video.duration = row[15];
                                temp_videos.push_back(video);
                                PrintVideoInfo(video);
                            }
                            mysql_free_result(result);

                            pthread_mutex_lock(&_heat_mutex);
                            LogMessage(Debug, "我抢到更新热门视频缓冲区锁了\n");
                            _heat_video = std::move(temp_videos);
                            pthread_mutex_unlock(&_heat_mutex);
                        }
                    }
                    ReleaseDBLock();
                }
                std::this_thread::sleep_for(std::chrono::hours(1));
            }
        }

        void UpdateMaxHeatVideos()
        {
            while (_running)
            {
                LogMessage(Debug, "我抢到数据库锁了\n");
                if (AcquireDBLock())
                {
                    std::string sql = "SELECT * FROM video ORDER BY heat DESC LIMIT 6;";
                    if (Execute(_mysql, sql))
                    {
                        LogMessage(Info, "更新最热门视频\n");
                        std::vector<VideoInfo> temp_videos;
                        MYSQL_RES *result = mysql_store_result(_mysql);
                        if (result)
                        {
                            MYSQL_ROW row;
                            while ((row = mysql_fetch_row(result)))
                            {
                                VideoInfo video;
                                video.id = atoi(row[0]);
                                video.name = row[1];
                                video.author = row[2];
                                video.info = row[3];
                                video.video_path = row[4];
                                video.img_path = row[5];
                                video.views = atoi(row[6]);
                                video.likes = atoi(row[7]);
                                video.favorites = atoi(row[8]);
                                video.comments = atoi(row[9]);
                                video.create_at = row[10];
                                video.update_at = row[11];
                                video.user_id = atoi(row[12]);
                                video.heat = atoi(row[13]);
                                video.rating = atof(row[14]);
                                video.duration = row[15];
                                temp_videos.push_back(video);
                                PrintVideoInfo(video);
                            }
                            mysql_free_result(result);

                            pthread_mutex_lock(&_max_heat_mutex);
                            LogMessage(Debug, "我抢到更新最热门视频缓冲区锁了\n");
                            _max_heat_video = std::move(temp_videos);
                            pthread_mutex_unlock(&_max_heat_mutex);
                        }
                    }
                    ReleaseDBLock();
                }
                std::this_thread::sleep_for(std::chrono::hours(1));
            }
        }

        void UpdateNewestVideos()
        {
            while (_running)
            {
                LogMessage(Debug, "我抢到数据库锁了\n");
                if (AcquireDBLock())
                {
                    std::string sql = "SELECT * FROM video ORDER BY create_at DESC LIMIT 6;";
                    if (Execute(_mysql, sql))
                    {
                        LogMessage(Info, "更新最新视频\n");
                        std::vector<VideoInfo> temp_videos;
                        MYSQL_RES *result = mysql_store_result(_mysql);
                        if (result)
                        {
                            MYSQL_ROW row;
                            while ((row = mysql_fetch_row(result)))
                            {
                                VideoInfo video;
                                video.id = atoi(row[0]);
                                video.name = row[1];
                                video.author = row[2];
                                video.info = row[3];
                                video.video_path = row[4];
                                video.img_path = row[5];
                                video.views = atoi(row[6]);
                                video.likes = atoi(row[7]);
                                video.favorites = atoi(row[8]);
                                video.comments = atoi(row[9]);
                                video.create_at = row[10];
                                video.update_at = row[11];
                                video.user_id = atoi(row[12]);
                                video.heat = atoi(row[13]);
                                video.rating = atof(row[14]);
                                video.duration = row[15];
                                temp_videos.push_back(video);
                                PrintVideoInfo(video);
                            }
                            mysql_free_result(result);

                            pthread_mutex_lock(&_newest_mutex);
                            LogMessage(Debug, "我抢到更新最新视频缓冲区锁了\n");
                            _newest_video = std::move(temp_videos);
                            pthread_mutex_unlock(&_newest_mutex);
                        }
                    }
                    ReleaseDBLock();
                }
                std::this_thread::sleep_for(std::chrono::hours(1));
            }
        }

        void UpdateMostViewedVideos()
        {
            while (_running)
            {
                LogMessage(Debug, "我抢到数据库锁了\n");
                if (AcquireDBLock())
                {
                    std::string sql = "SELECT * FROM video ORDER BY views DESC LIMIT 6;";
                    if (Execute(_mysql, sql))
                    {
                        LogMessage(Info, "更新最多观看视频\n");
                        std::vector<VideoInfo> temp_videos;
                        MYSQL_RES *result = mysql_store_result(_mysql);
                        if (result)
                        {
                            MYSQL_ROW row;
                            while ((row = mysql_fetch_row(result)))
                            {
                                VideoInfo video;
                                video.id = atoi(row[0]);
                                video.name = row[1];
                                video.author = row[2];
                                video.info = row[3];
                                video.video_path = row[4];
                                video.img_path = row[5];
                                video.views = atoi(row[6]);
                                video.likes = atoi(row[7]);
                                video.favorites = atoi(row[8]);
                                video.comments = atoi(row[9]);
                                video.create_at = row[10];
                                video.update_at = row[11];
                                video.user_id = atoi(row[12]);
                                video.heat = atoi(row[13]);
                                video.rating = atof(row[14]);
                                video.duration = row[15];
                                temp_videos.push_back(video);
                                PrintVideoInfo(video);
                            }
                            mysql_free_result(result);

                            pthread_mutex_lock(&_viewed_mutex);
                            LogMessage(Debug, "我抢到更新最多观看视频缓冲区锁了\n");
                            _most_viewed_video = std::move(temp_videos);
                            pthread_mutex_unlock(&_viewed_mutex);
                        }
                    }
                    ReleaseDBLock();
                }
                std::this_thread::sleep_for(std::chrono::hours(1));
            }
        }

        void UpdateMostCommentedVideos()
        {
            while (_running)
            {
                LogMessage(Debug, "我抢到数据库锁了\n");
                if (AcquireDBLock())
                {
                    std::string sql = "SELECT * FROM video ORDER BY comments DESC LIMIT 6;";
                    if (Execute(_mysql, sql))
                    {
                        LogMessage(Info, "更新最多评论视频\n");
                        std::vector<VideoInfo> temp_videos;
                        MYSQL_RES *result = mysql_store_result(_mysql);
                        if (result)
                        {
                            MYSQL_ROW row;
                            while ((row = mysql_fetch_row(result)))
                            {
                                VideoInfo video;
                                video.id = atoi(row[0]);
                                video.name = row[1];
                                video.author = row[2];
                                video.info = row[3];
                                video.video_path = row[4];
                                video.img_path = row[5];
                                video.views = atoi(row[6]);
                                video.likes = atoi(row[7]);
                                video.favorites = atoi(row[8]);
                                video.comments = atoi(row[9]);
                                video.create_at = row[10];
                                video.update_at = row[11];
                                video.user_id = atoi(row[12]);
                                video.heat = atoi(row[13]);
                                video.rating = atof(row[14]);
                                video.duration = row[15];
                                temp_videos.push_back(video);
                                PrintVideoInfo(video);
                            }
                            mysql_free_result(result);

                            pthread_mutex_lock(&_commented_mutex);
                            LogMessage(Debug, "我抢到更新最多评论视频缓冲区锁了\n");
                            _most_commented_video = std::move(temp_videos);
                            pthread_mutex_unlock(&_commented_mutex);
                        }
                    }
                    ReleaseDBLock();
                }
                std::this_thread::sleep_for(std::chrono::hours(1));
            }
        }

        void UpdateMostLikedVideos()
        {
            while (_running)
            {
                LogMessage(Debug, "我抢到数据库锁了\n");
                if (AcquireDBLock())
                {
                    std::string sql = "SELECT * FROM video ORDER BY likes DESC LIMIT 6;";
                    if (Execute(_mysql, sql))
                    {
                        LogMessage(Info, "更新最多点赞视频\n");
                        std::vector<VideoInfo> temp_videos;
                        MYSQL_RES *result = mysql_store_result(_mysql);
                        if (result)
                        {
                            MYSQL_ROW row;
                            while ((row = mysql_fetch_row(result)))
                            {
                                VideoInfo video;
                                video.id = atoi(row[0]);
                                video.name = row[1];
                                video.author = row[2];
                                video.info = row[3];
                                video.video_path = row[4];
                                video.img_path = row[5];
                                video.views = atoi(row[6]);
                                video.likes = atoi(row[7]);
                                video.favorites = atoi(row[8]);
                                video.comments = atoi(row[9]);
                                video.create_at = row[10];
                                video.update_at = row[11];
                                video.user_id = atoi(row[12]);
                                video.heat = atoi(row[13]);
                                video.rating = atof(row[14]);
                                video.duration = row[15];
                                temp_videos.push_back(video);
                                PrintVideoInfo(video);
                            }
                            mysql_free_result(result);

                            pthread_mutex_lock(&_liked_mutex);
                            LogMessage(Debug, "我抢到更新最多点赞视频缓冲区锁了\n");
                            _most_liked_video = std::move(temp_videos);
                            pthread_mutex_unlock(&_liked_mutex);
                        }
                    }
                    ReleaseDBLock();
                }
                std::this_thread::sleep_for(std::chrono::hours(1));
            }
        }

        void UpdateMostFavoritedVideos()
        {
            while (_running)
            {
                LogMessage(Debug, "我抢到数据库锁了\n");
                if (AcquireDBLock())
                {
                    std::string sql = "SELECT * FROM video ORDER BY favorites DESC LIMIT 6;";
                    if (Execute(_mysql, sql))
                    {
                        LogMessage(Info, "更新最多收藏视频\n");
                        std::vector<VideoInfo> temp_videos;
                        MYSQL_RES *result = mysql_store_result(_mysql);
                        if (result)
                        {
                            MYSQL_ROW row;
                            while ((row = mysql_fetch_row(result)))
                            {
                                VideoInfo video;
                                video.id = atoi(row[0]);
                                video.name = row[1];
                                video.author = row[2];
                                video.info = row[3];
                                video.video_path = row[4];
                                video.img_path = row[5];
                                video.views = atoi(row[6]);
                                video.likes = atoi(row[7]);
                                video.favorites = atoi(row[8]);
                                video.comments = atoi(row[9]);
                                video.create_at = row[10];
                                video.update_at = row[11];
                                video.user_id = atoi(row[12]);
                                video.heat = atoi(row[13]);
                                video.rating = atof(row[14]);
                                video.duration = row[15];
                                temp_videos.push_back(video);
                                PrintVideoInfo(video);
                            }
                            mysql_free_result(result);

                            pthread_mutex_lock(&_favorited_mutex);
                            LogMessage(Debug, "我抢到更新最多收藏视频缓冲区锁了\n");
                            _most_favorited_video = std::move(temp_videos);
                            pthread_mutex_unlock(&_favorited_mutex);
                        }
                    }
                    ReleaseDBLock();
                }
                std::this_thread::sleep_for(std::chrono::hours(1));
            }
        }

        void UpdateTopRatedVideos()
        {
            while (_running)
            {
                LogMessage(Debug, "我抢到数据库锁了\n");
                if (AcquireDBLock())
                {
                    std::string sql = "SELECT * FROM video ORDER BY rating DESC LIMIT 6;";
                    if (Execute(_mysql, sql))
                    {
                        LogMessage(Info, "更新最高评分视频\n");
                        std::vector<VideoInfo> temp_videos;
                        MYSQL_RES *result = mysql_store_result(_mysql);
                        if (result)
                        {
                            MYSQL_ROW row;
                            while ((row = mysql_fetch_row(result)))
                            {
                                VideoInfo video;
                                video.id = atoi(row[0]);
                                video.name = row[1];
                                video.author = row[2];
                                video.info = row[3];
                                video.video_path = row[4];
                                video.img_path = row[5];
                                video.views = atoi(row[6]);
                                video.likes = atoi(row[7]);
                                video.favorites = atoi(row[8]);
                                video.comments = atoi(row[9]);
                                video.create_at = row[10];
                                video.update_at = row[11];
                                video.user_id = atoi(row[12]);
                                video.heat = atoi(row[13]);
                                video.rating = atof(row[14]);
                                video.duration = row[15];
                                temp_videos.push_back(video);
                                PrintVideoInfo(video);
                            }
                            mysql_free_result(result);

                            pthread_mutex_lock(&_rated_mutex);
                            LogMessage(Debug, "我抢到更新最高评分视频缓冲区锁了\n");
                            _top_rated_video = std::move(temp_videos);
                            pthread_mutex_unlock(&_rated_mutex);
                        }
                    }
                    ReleaseDBLock();
                }
                std::this_thread::sleep_for(std::chrono::hours(1));
            }
        }

    private:
        // MySQL句柄
        MYSQL *_mysql;
        // 视频信息表名
        std::string _table_name;

        // 缓存视频信息表
        std::vector<VideoInfo> _heat_video;           // 热门视频(8个)
        std::vector<VideoInfo> _max_heat_video;       // 最热门视频(6个)
        std::vector<VideoInfo> _newest_video;         // 最新视频(6个)
        std::vector<VideoInfo> _most_viewed_video;    // 最多观看(6个)
        std::vector<VideoInfo> _most_commented_video; // 最多评论(6个)
        std::vector<VideoInfo> _most_liked_video;     // 最多点赞(6个)
        std::vector<VideoInfo> _most_favorited_video; // 最多收藏(6个)
        std::vector<VideoInfo> _top_rated_video;      // 最高评分(6个)

        // 互斥锁
        pthread_mutex_t _heat_mutex;      // 热门视频
        pthread_mutex_t _max_heat_mutex;  // 最热门视频
        pthread_mutex_t _newest_mutex;    // 最新视频
        pthread_mutex_t _viewed_mutex;    // 最多观看视频
        pthread_mutex_t _commented_mutex; // 最多评论视频
        pthread_mutex_t _liked_mutex;     // 最多点赞视频
        pthread_mutex_t _favorited_mutex; // 最多收藏视频
        pthread_mutex_t _rated_mutex;     // 最高评分视频

        // 数据库操作互斥锁和条件变量
        pthread_mutex_t _db_mutex;
        pthread_cond_t _db_cond;
        bool _db_busy;

        // 更新线程数组
        std::vector<std::thread> _update_threads;
        // 线程运行标志
        bool _running;
    };
}