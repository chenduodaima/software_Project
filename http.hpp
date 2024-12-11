#pragma once
#include <memory>
#include <ctime>
#include <pthread.h>
#include <regex>
#include <string>
#include "httplib.h"
#include "mysql.hpp"
#include "util.hpp"

namespace aod
{
    typedef struct Session
    {
    private:
        long long unsigned int _session_id;
        int _uid;
        std::string _username;
        std::string _password;
        time_t _expire_time;

    public:
        Session() : _username(""), _password(""), _session_id(0), _expire_time(0), _uid(0)
        {
        }

        Session(std::string username, std::string password, int uid, time_t expire_time = time(nullptr) + 3600 * 2)
            : _username(username), _password(password), _uid(uid), _expire_time(expire_time)
        {
            // 生成session_id，根据当前时间戳%账号id
            time_t timestamp = time(nullptr);
            // 将时间戳转换为整数
            long long unsigned int timestamp_int = static_cast<long long unsigned int>(timestamp);
            _session_id = timestamp_int % 20040710;
        }

        // 检测Session是否过期
        bool IsExpired() const
        {
            return time(nullptr) > _expire_time;
        }

        // 返回过期时间
        time_t GetExpireTime() const
        {
            return _expire_time;
        }

        int GetUid() const
        {
            return _uid;
        }

        // 设置过期时间
        void SetExpireTime(time_t expire_time)
        {
            _expire_time = expire_time;
        }

        // 返回session_id
        long long unsigned int GetSessionId()
        {
            return _session_id;
        }

        const std::string &GetUserName()
        {
            return _username;
        }

        const std::string &GetPassWord()
        {
            return _password;
        }

    } Session;

    class HttpServer
    {
#define WEB_PATH "./web"     // web根目录
#define VIDEO_PATH "/video/" // 音频路径
#define VIDEO_IMG_PATH "/image/video_img/"   // 封面图片路径
#define USER_HEAD_PORTRAIT_PATH "/image/user_head/" // 用户头像路径

    private:
        // 视频模块
        // 插入响应，测试通过
        static void Insert(const httplib::Request &req, httplib::Response &res)
        {
            // 验证登录
            Session *session = nullptr;
            if (!IsLogin(req, session))
            {
                // 没有登录，告知用户登录
                res.status = 400;
                res.body = R"({"result":false, "reason":"您还未登录"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "有未登录用户访问服务器\n");
                return;
            }
            LogMessage(Debug, "登录成功，开始上传视频\n");
            // 要求上传：视频名称、视频简介、视频类型id、视频文件、封面图片文件
            httplib::MultipartFormData video_name_mul = req.get_file_value("video_name"); // 视频名称
            httplib::MultipartFormData video_info_mul = req.get_file_value("video_info"); // 视频简介
            httplib::MultipartFormData video_type_id_mul = req.get_file_value("video_type_id");
            httplib::MultipartFormData video_mul = req.get_file_value("video"); // 视频文件
            httplib::MultipartFormData image_mul = req.get_file_value("image"); // 视频封面
            httplib::MultipartFormData duration_mul = req.get_file_value("duration");   // 视频时长
            int uid = session->GetUid();
            std::string video_name = video_name_mul.content;
            std::string video_info = video_info_mul.content;
            int video_type_id = std::stoi(video_type_id_mul.content);
            std::string duration = duration_mul.content;
            // 拼接文件存放路径
            std::string video_path = WEB_PATH + std::string(VIDEO_PATH) + video_name + video_mul.filename;
            std::string image_path = WEB_PATH + std::string(VIDEO_IMG_PATH) + video_name + image_mul.filename;
            // std::string duration = duration_mul.content;
            if (ul::FileUtil(video_path).SetContent(video_mul.content) == false) // 如果保存视频文件失败，返回服务器错误
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"视频文件上传失败"})"; // 添加正文数据
                res.set_header("Content-Type", "application/json");            // 添加报头字段：正文类型为json数据
                LogMessage(Debug, "服务器保存视频文件失败, uid : %d, video_name : %s, video_path : %s\n", uid, video_name.c_str(), video_path.c_str());
                return;
            }
            if (ul::FileUtil(image_path).SetContent(image_mul.content) == false) // 如果保存封面图片失败，返回服务器错误
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"封面图片上传失败"})"; // 添加正文数据
                res.set_header("Content-Type", "application/json");            // 添加报头字段：正文类型为json数据
                LogMessage(Debug, "服务器保存封面图片文件失败, uid : %d, image_name : %s, video_path : %s\n", uid, video_name.c_str(), video_path.c_str());
                return;
            }
            // 修改数据库
            Json::Value value;
            value["uid"] = uid;
            value["video_name"] = video_name;
            value["video_info"] = video_info;
            value["video_type_id"] = video_type_id;
            value["video_path"] = std::string(VIDEO_PATH) + video_name + video_mul.filename;
            value["image_path"] = std::string(VIDEO_IMG_PATH) + video_name + image_mul.filename;
            value["duration"] = duration;
            if (!_video->Insert(value)) // 数据库插入失败
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"视频文件上传失败"})"; // 添加正文数据
                res.set_header("Content-Type", "application/json");            // 添加报头字段：正文类型为json数据
                LogMessage(Debug, "数据库插入视频失败, uid : %d, image_name : %s, video_path : %s\n", uid, video_name.c_str(), video_path.c_str());
                return;
            }
            res.status = 200;
            res.body = R"({"result":true, "reason":"视频文件上传成功"})";
            res.set_header("Content-Type", "application/json");
            // 插入成功
            LogMessage(Debug, "用户上传视频成功，uid : %d, video_path : %s, image_path : %s\n", uid, video_path.c_str(), image_path.c_str());
        }

        // 删除视频
        static void Delete(const httplib::Request &req, httplib::Response &res)
        {
            Session *session = nullptr;
            if (!IsLogin(req, session))
            {
                // 没有登录，告知用户登录
                res.status = 400;
                res.body = R"({"result":false, "reason":"您还未登录"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "有未登录用户访问服务器\n");
                return;
            }
            LogMessage(Debug, "登录成功，开始删除视频\n");
            int video_id = std::stoi(req.matches[1]);
            // web目录删除
            Json::Value value;
            if (!_video->SelectOne(video_id, &value)) // 数据库查询失败
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"视频删除失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "数据库删除视频不存在, video_id : %d\n", video_id);
                return;
            }
            int uid = std::stoi(value["uid"].asString());
            //判断权限
            if (!HasRole(uid, *session))
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"你没有权限删除该视频"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "登录用户没有权限删除该视频, 该视频的uid : %d, video_id : %d\n", uid, video_id);
                return;
            }
            LogMessage(Debug, "登录用户有权限删除该视频，该视频的uid : %d, video_id : %d\n", uid, video_id);
            //本地删除
            std::string video_path = WEB_PATH + value["video_path"].asString();
            std::string image_path = WEB_PATH + value["image_path"].asString();
            if (!ul::FileUtil(video_path).Delete() || !ul::FileUtil(image_path).Delete())
            {
                LogMessage(Debug, "文件系统删除文件失败，video_path : %s, image_path : %s\n", video_path.c_str(), image_path.c_str());
                res.status = 500;
                res.body = R"({"result":false, "reason":"视频删除失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            LogMessage(Debug, "文件系统删除文件成功，video_path : %s, image_path : %s\n", video_path.c_str(), image_path.c_str());
            if (!_video->Delete(video_id)) // 数据库删除失败
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"视频删除失败"})"; // 添加正文数据
                res.set_header("Content-Type", "application/json");        // 添加报头字段：正文类型为json数据
                LogMessage(Debug, "数据库删除视频失败, video_id : %d\n", video_id);
                return;
            }
            // 数据库删除
            LogMessage(Debug, "数据库删除视频成功, video_path : %s, image_path : %s\n", video_path.c_str(), image_path.c_str());
        }

        // 查询响应，通过视频id进行搜索，只要一个结果
        static void SelectOne(const httplib::Request &req, httplib::Response &res)
        {
            Session *session = nullptr;
            if (!IsLogin(req, session))
            {
                // 没有登录，告知用户登录
                res.status = 400;
                res.body = R"({"result":false, "reason":"您还未登录"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "有未登录用户访问服务器\n");
                return;
            }
            int video_id = std::stoi(req.matches[1]);
            Json::Value result;
            if (!_video->SelectOne(video_id, &result)) // 数据库查询失败
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"视频查询失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "数据库查询视频失败, video_id = %d\n", video_id);
                return;
            }
            // 查询成功，结果在result中，对result进行序列化
            ul::JsonUtil::Serialize(result, &res.body);
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "数据库查询视频成功, video_id = %d\n", video_id);
            return;
        }

        // 根据是否有参数search判断是否进行模糊查询
        static void SelectALL(const httplib::Request &req, httplib::Response &res)
        {
            Session *session = nullptr;
            if (!IsLogin(req, session))
            {
                // 没有登录，告知用户登录
                res.status = 400;
                res.body = R"({"result":false, "reason":"您还未登录"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "有未登录用户访问服务器");
                return;
            }
            // /video?search="关键字"
            bool search_flag = true;     // 默认全部查询
            if (req.has_param("search")) // 如果有search参数，则进行模糊查询
            {
                search_flag = false;
            }
            Json::Value result;
            if (search_flag) // 全部查询
            {
                if (!_video->SelectALL(&result)) // 数据库查询失败
                {
                    res.status = 500;
                    res.body = R"({"result":false, "reason":"视频查询全部视频失败"})";
                    res.set_header("Content-Type", "application/json");
                    LogMessage(Debug, "数据库查询全部视频失败\n");
                    return;
                }
                // 查询成功，结果在result中，对result进行序列化
                if (!ul::JsonUtil::Serialize(result, &res.body))
                {
                    res.status = 500;
                    res.body = R"({"result":false, "reason":"视频查询全部视频序列化失败"})";
                    res.set_header("Content-Type", "application/json");
                    LogMessage(Debug, "视频查询全部视频序列化失败\n");
                    return;
                }
                LogMessage(Debug, "数据库查询全部视频成功\n");
            }
            else // 模糊查询
            {
                std::string key = req.get_param_value("search");
                if (!_video->SelectLike(key, &result)) // 数据库查询失败
                {
                    res.status = 500;
                    res.body = R"({"result":false, "reason":"视频模糊查询失败"})";
                    res.set_header("Content-Type", "application/json");
                    LogMessage(Debug, "数据库模糊查询视频失败, key : %s\n", key.c_str());
                    return;
                }
                // 查询成功，结果在result中，对result进行序列化
                if (!ul::JsonUtil::Serialize(result, &res.body))
                {
                    res.status = 500;
                    res.body = R"({"result":false, "reason":"视频模糊查询序列化失败"})";
                    res.set_header("Content-Type", "application/json");
                    LogMessage(Debug, "视频模糊查询序列化失败, key : %s\n", key.c_str());
                    return;
                }
                LogMessage(Debug, "数据库模糊查询视频成功, key : %s\n", key.c_str());
            }
            res.status = 200;
            res.set_header("Content-Type", "application/json");
        }

        // 更新响应
        static void Update(const httplib::Request &req, httplib::Response &res)
        {
            Session *session = nullptr;
            if (!IsLogin(req, session))
            {
                // 没有登录，告知用户登录
                res.status = 400;
                res.body = R"({"result":false, "reason":"您还未登录"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "有未登录用户访问服务器\n");
                return;
            }
            int video_id = std::stoi(req.matches[1]);
            Json::Value value;
            if (!_video->SelectOne(video_id, &value))
            {
                LogMessage(Debug, "数据库搜索视频出错， video_id : %d\n", video_id);
                res.status = 500;
                res.body = R"({"result":false, "reason":"数据库没有找到视频"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            // 数据格式合法
            // 修改数据库
            int uid = std::stoi(value["uid"].asString());   
            if (!HasRole(uid, *session))
            {
                LogMessage(Debug, "登录用户没有权限修改该视频, uid : %d, video_id : %d\n", uid, video_id);
                res.status = 400;
                res.body = R"({"result":false, "reason":"你没有权限修改该视频"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            LogMessage(Debug, "有权限修改视频，修改视频操作开始, uid : %d, video_id : %d\n", uid, video_id);
            Json::Value condition;
            condition["video_id"] = video_id;
            Json::Value root;
            if (!ul::JsonUtil::Deserialize(root, req.body))
            {
                LogMessage(Debug, "更新视频反序列化失败, uid : %d, video_id : %d\n", uid, video_id);
                res.status = 400;
                res.body = R"({"result":false, "reason":"更新视频反序列化失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            if (!_video->Update(root, condition, video_id)) // 数据库修改失败
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"视频修改失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "数据库更新视频失败, video_id : %d\n", video_id);
                return;
            }
            res.status = 200;
            res.body = R"({"result":true, "content":"视频修改成功"})";
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "数据库更新视频成功, video_id : %d\n", video_id);
        }
        
        // 获取某种类型的视频
        static void GetVideoOfType(const httplib::Request &req, httplib::Response &res)
        {
            int type_id = std::stoi(req.get_param_value("type_id"));
            int number = std::stoi(req.get_param_value("number"));
            Json::Value video_array;
            if (!_video->GetVideoOfType(type_id, number, &video_array))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"获取视频失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            if (!ul::JsonUtil::Serialize(video_array, &res.body))
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"数据库数据序列化失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            res.status = 200;
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器获取某种类型的视频成功, type_id : %d, number : %d\n", type_id, number);
        }

        // 浏览器渲染，不需要登录验证
        // 获取排行榜数据，使用video数据库
        static void GetVideoTopK(const httplib::Request &req, httplib::Response &res)
        {
            // 获取请求报头中的Type
            Json::Value video_array;
            std::string type = req.get_header_value("Type");
            int k = std::stoi(req.get_header_value("TopK"));
            LogMessage(Debug, "获取到一个TopK请求，type : %s, topK : %d\n", type.c_str(), k);
            // 如果没获取到
            if (!_video->GetTopK(type, k, &video_array))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"服务端获取数据失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            // 序列化
            if (!ul::JsonUtil::Serialize(video_array, &res.body))
            {
                // 序列化失败
                LogMessage(Debug, "数据库数据序列化失败，type : %s, topK : %d\n", type.c_str(), k);
                res.status = 500;
                res.body = R"({"result":false, "reason":"服务端数据库数据序列化失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            // 成功
            res.status = 200;
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器获取TopK数据成功, type : %s, topK : %d\n", type.c_str(), k);
        }

        static void GetTotal(const httplib::Request &req, httplib::Response &res)
        {
            std::string table_name = req.get_param_value("table_name");
            Json::Value value;
            //获取视频类型总数
            if (table_name == "type")
            {
                if (!_type->GetTotal(&value))
                {   
                    res.status = 400;
                    res.body = R"({"result":false, "reason":"获取数据总数失败"})";
                    res.set_header("Content-Type", "application/json");
                    LogMessage(Debug, "获取数据总数失败, table_name : %s\n", table_name.c_str());
                    return;
                }
            }
            //获取视频标签总数
            else if (table_name == "tag")
            {
                if (!_tag->GetTotal(&value))
                {
                    res.status = 400;
                    res.body = R"({"result":false, "reason":"获取数据总数失败"})";
                    res.set_header("Content-Type", "application/json");
                    LogMessage(Debug, "获取数据总数失败, table_name : %s\n", table_name.c_str());
                    return;
                }
            }
            else
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"获取数据总数失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "获取数据总数失败, table_name : %s\n", table_name.c_str());
                return;
            }
            //序列化
            if (!ul::JsonUtil::Serialize(value, &res.body))
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"数据库数据序列化失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "数据库数据序列化失败, table_name : %s\n", table_name.c_str());
                return;
            }
            res.status = 200;
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器获取数据总数成功, table_name : %s\n", table_name.c_str());
        }
        
        static void UpdateVideoRelation(const httplib::Request &req, httplib::Response &res)
        {
            //根据参数进行用户操作
            //登录验证
            Session *session = nullptr;
            if (!IsLogin(req, session))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"您还未登录"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            int uid = session->GetUid();
            int video_id = std::stoi(req.get_param_value("video_id"));
            bool is_like = std::stoi(req.get_param_value("is_liked"));
            bool dislike = std::stoi(req.get_param_value("dislike"));
            bool favorite = std::stoi(req.get_param_value("favorite"));
            std::string grade = req.get_param_value("grade");
            Json::Value value;
            value["is_liked"] = is_like;
            value["dislike"] = dislike;
            value["favorite"] = favorite;
            if (!_video->UpdateVideoRelation(uid, video_id, grade, value))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"用户操作视频失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            res.status = 200;
            res.body = R"({"result":true, "reason":"用户操作视频成功"})";
            res.set_header("Content-Type", "application/json");
        }
        // 用户模块
        // 修改用户信息，需要登录验证，使用user数据库
        // 修改操作包括：修改个人头像，修改
        static void UpdateUserInfo(const httplib::Request &req, httplib::Response &res)
        {
            Session *session = nullptr;
            if (!IsLogin(req, session))
            {
                // 没有登录，告知用户登录
                res.status = 400;
                res.body = R"({"result":false, "reason":"您还未登录"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "有未登录用户访问服务器");
                return;
            }
            Json::Value value;
            std::string path("");
            if (req.has_file("name"))
                value["name"] = req.get_file_value("name").content;
            if (req.has_file("username"))
                value["username"] = req.get_file_value("username").content;
            if (req.has_file("password"))
                value["password"] = req.get_file_value("password").content;
            if (req.has_file("user_head_portrait"))
            {
                path = std::string(USER_HEAD_PORTRAIT_PATH) + req.get_file_value("user_head_portrait").filename;
                value["user_head_portrait_path"] = path;
            }
            int uid = session->GetUid();
            // 操作失败
            if (!_user->UpdateUserInfo(value, uid))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"登录用户操作失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "用户操作失败，uid : %d", uid);
                return;
            }
            //服务器上传头像
            if (path != "")
            {
                std::string head = req.get_file_value("user_head_portrait").content;
                path = WEB_PATH + std::string(USER_HEAD_PORTRAIT_PATH) + req.get_file_value("user_head_portrait").filename;
                if (!ul::FileUtil(path).SetContent(head))
                {
                    LogMessage(Debug, "服务器上传头像失败, uid : %d\n", uid);
                    res.status = 200;
                    res.body = R"({"result":false, "reason":"头像上传失败"})";
                    res.set_header("Content-Type", "application/json");
                    return;
                }
            }
            // 操作成功
            res.status = 200;
            res.body = R"({"result":true, "reason":"修改成功"})";
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "登录用户修改成功, uid : %d\n", uid);
        }

        // 获取用户和视频的关系
        static void GetVideoRelation(const httplib::Request &req, httplib::Response &res)
        {
            //获取uid
            Session *session = nullptr;
            if (!IsLogin(req, session))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"您还未登录"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            int uid = session->GetUid();
            int video_id = std::stoi(req.get_param_value("video_id"));
            Json::Value value;
            if (!_video->GetVideoRelation(uid, video_id, &value))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"获取用户和视频的关系失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            if (!ul::JsonUtil::Serialize(value, &res.body))
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"数据库数据序列化失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            res.status = 200;
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器获取用户和视频的关系成功, video_id : %d, uid : %d\n", video_id, uid);
        }

        static void GetUserInfo(const httplib::Request &req, httplib::Response &res)
        {
            Session *session = nullptr;
            if (!IsLogin(req, session))
            {
                // 没有登录，告知用户登录
                res.status = 400;
                res.body = R"({"result":false, "reason":"您还未登录"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "有未登录用户访问服务器\n");
                return;
            }
            int uid = session->GetUid();
            Json::Value value;
            if (!_user->GetUserInfo(&value, uid))
            {
                LogMessage(Debug, "服务器获取用户信息失败, uid : %d\n", uid);
                res.status = 400;
                res.body = R"({"result":false, "reason":"获取用户信息失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            if (!ul::JsonUtil::Serialize(value, &res.body))
            {
                // 序列化失败
                LogMessage(Debug, "数据库数据序列化失败，uid : %d", uid);
                res.status = 500;
                res.body = R"({"result":false, "reason":"服务端数据库数据序列化失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            res.status = 200;
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器获取用户信息成功，uid : %d\n", uid);
        }

        // 获取视频用户信息
        static void GetVideoUserInfo(const httplib::Request &req, httplib::Response &res)
        {
            // 获取视频id
            int video_id = std::stoi(req.get_param_value("id"));
            Json::Value value;
            if (!_user->GetVideoUserInfo(video_id, &value))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"获取视频用户信息失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "获取视频用户信息失败, video_id : %d\n", video_id);
                return;
            }
            if (!ul::JsonUtil::Serialize(value, &res.body))
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"数据库数据序列化失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            res.status = 200;
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器获取视频用户信息成功, video_id : %d\n", video_id);
        }

        // 获取视频评论用户信息
        static void GetCommentUserInfo(const httplib::Request &req, httplib::Response &res)
        {
            int id = std::stoi(req.get_param_value("id"));
            Json::Value value;
            if (!_user->GetCommentUserInfo(id, &value))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"获取视频评论用户信息失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "获取视频评论用户信息失败, video_id : %d\n", id);
                return;
            }
            if (!ul::JsonUtil::Serialize(value, &res.body))
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"数据库数据序列化失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            res.status = 200;
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器获取视频评论用户信息成功, video_id : %d\n", id);
        }

        // 登录验证。通过查看浏览器的Cookie:session=session_id
        // 如果存在，就检测是否有效
        // 如果不存在，就算作没登录
        static bool IsLogin(const httplib::Request &req, Session *&session)
        {
            LogMessage(Debug, "验证用户登陆...\n");
            if (!req.has_header("Cookie"))
            {
                LogMessage(Debug, "请求没有携带Cookie\n");
                return false;
            }
            std::string cookie = req.get_header_value("Cookie");
            std::regex session_id_regex(R"(session_id=(\d+))");
            std::smatch matches;
            std::regex_search(cookie, matches, session_id_regex);
            long long unsigned int session_id = std::stoll(matches[1].str());
            LogMessage(Debug, "获取到一个session_id : %lld\n", session_id);
            if (_session_map.find(session_id) == _session_map.end() || _session_map[session_id].IsExpired())
            {
                // 销毁Session
                if (_session_map.find(session_id) != _session_map.end())
                    _session_map.erase(_session_map.find(session_id));
                LogMessage(Debug, "session_id : %lld无效\n", session_id);
                return false;
            }
            LogMessage(Debug, "session_id : %lld有效\n", session_id);
            session = &_session_map[session_id];
            return true;
        }

        static void Login(const httplib::Request &req, httplib::Response &res)
        {
            Json::Value body;
            if (!ul::JsonUtil::Deserialize(body, req.body))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"反序列化失败，登录失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "用户登录反序列化失败\n");
                return;
            }
            std::string username = body["username"].asString();
            std::string password = body["password"].asString();
            LogMessage(Debug, "用户登录, username : %s, password : %s\n", username.c_str(), password.c_str());
            int uid;
            // 登录失败
            if (!_user->Login(username, password, &uid))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"用户名或者密码错误"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "用户登录失败, username : %s, password : %s\n", username.c_str(), password.c_str());
                return;
            }
            // 登录成功
            //遍历session_map，如果有相同uid，只需要更新过期时间即可
            bool is_update = false;
            long long unsigned int session_id = 0;
            for (auto &item : _session_map)
            {
                if (item.second.GetUid() == uid)
                {
                    item.second.SetExpireTime(time(nullptr) + 3600);
                    is_update = true;
                    session_id = item.first;
                    break;
                }
            }
            // 如果session_map中没有相同uid，则创建新的Session
            if (!is_update)
            {
                Session session(username, password, uid);
                session_id = session.GetSessionId();
                _session_map[session_id] = session;
            }
            res.status = 200;
            res.set_header("Set-Cookie", "session_id=" + std::to_string(session_id) + "; path=/");
            res.body = R"({"result":true, "reason":"用户登录成功", "success": true})";
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "用户登录成功, session_id : %lld, username : %s, password : %s\n", session_id, username.c_str(), password.c_str());
            return;
        }

        static void Register(const httplib::Request &req, httplib::Response &res)
        {
            Json::Value body;
            if (!ul::JsonUtil::Deserialize(body, req.body))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"反序列化失败，登录失败"})";
                LogMessage(Debug, "用户注册反序列化失败\n");
                return;
            }
            std::string name = body["name"].asString();
            std::string username = body["username"].asString();
            std::string password = body["password"].asString();
            LogMessage(Debug, "新用户注册, name : %s, username : %s, password : %s\n", name.c_str(), username.c_str(), password.c_str());
            if (!_user->Register(name, username, password))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"用户名已存在"})";
                LogMessage(Debug, "用户注册失败, name : %s, username : %s, password : %s\n", name.c_str(), username.c_str(), password.c_str());
                return;
            }
            res.status = 200;
            res.body = R"({"result":true, "reason":"用户注册成功"})";
            LogMessage(Debug, "用户注册成功, username : %s, password : %s\n", name.c_str(), username.c_str(), password.c_str());
            return;
        }

        // 判断是否有权限修改视频
        static bool HasRole(int video_uid, Session &session)
        {
            return session.GetUid() == video_uid;
        }

        // 标签模块
        // 获取视频标签
        static void GetVideoTag(const httplib::Request &req, httplib::Response &res)
        {
            int video_id = std::stoi(req.matches[1]);
            Json::Value value;
            if (!_tag->GetVideoTag(video_id, &value))
            {
                res.status = 204;
                res.body = R"({"result":false, "reason":"获取视频标签失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "获取视频标签失败, video_id : %d\n", video_id);
                return;
            }
            if (!ul::JsonUtil::Serialize(value, &res.body))
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"数据库数据序列化失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "数据库数据序列化失败, video_id : %d\n", video_id);
                return;
            }
            res.status = 200;
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器获取视频标签成功, video_id : %d\n", video_id);
        }

        // 获取用户所有视频
        static void GetUserVideo(const httplib::Request &req, httplib::Response &res)
        {
            //登录
            Session *session = nullptr;
            if (!IsLogin(req, session))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"您还未登录"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            //通过user_id获取到video里获取对应的所有视频信息
            int uid = session->GetUid();
            Json::Value value;
            if (!_user->GetUserVideo(uid, &value))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"获取用户所有视频失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "获取用户所有视频失败, uid : %d\n", uid);
                return;
            }
            if (!ul::JsonUtil::Serialize(value, &res.body))
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"数据库数据序列化失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "数据库数据序列化失败, uid : %d\n", uid);
                return;
            }
            res.status = 200;
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器获取用户所有视频成功, uid : %d\n", uid);
        }

        // 获取用户收藏
        //通过uid获取到user_video_relation表中所有favorite为1的视频，并到video表中获取视频信息
        static void GetFavorite(const httplib::Request &req, httplib::Response &res)
        {
            //登录
            Session *session = nullptr;
            if (!IsLogin(req, session))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"您还未登录"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            int uid = session->GetUid();
            Json::Value value;
            if (!_user->GetFavorite(uid, &value))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"获取用户收藏失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "获取用户收藏失败, uid : %d\n", uid);
                return;
            }
            if (!ul::JsonUtil::Serialize(value, &res.body))
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"数据库数据序列化失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "数据库数据序列化失败, uid : %d\n", uid);
                return;
            }
            res.status = 200;
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器获取用户收藏成功, uid : %d\n", uid);
        }

        // 类型模块
        // 获取视频类型
        static void GetVideoType(const httplib::Request &req, httplib::Response &res)
        {
            int video_id = std::stoi(req.matches[1]);
            Json::Value value;
            if (!_type->GetVideoType(video_id, &value))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"获取视频类型失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "获取视频类型失败, video_id : %d\n", video_id);
                return;
            }
            if (!ul::JsonUtil::Serialize(value, &res.body))
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"数据库数据序列化失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "数据库数据序列化失败, video_id : %d\n", video_id);
                return;
            }
            res.status = 200;
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器获取视频类型成功, video_id : %d\n", video_id);
        }

        // 评论模块
        //获取视频评论
        static void GetComment(const httplib::Request &req, httplib::Response &res)
        {
            int id = std::stoi(req.get_param_value("id"));
            Json::Value value;
            if (!_comment->GetComment(id, &value))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"获取视频评论失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "获取视频评论失败, video_id : %d\n", id);
                return;
            }
            if (!ul::JsonUtil::Serialize(value, &res.body))
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"数据库数据序列化失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "数据库数据序列化失败, video_id : %d\n", id);
                return;
            }
            res.status = 200;
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器获取视频评论成功, video_id : %d\n", id);
        }

        // 插入视频评论
        static void SendComment(const httplib::Request &req, httplib::Response &res)
        {
            int video_id = std::stoi(req.get_param_value("id"));
            Session *session = nullptr;
            if (!IsLogin(req, session))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"您还未登录"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "插入视频评论时用户未登录, video_id : %d\n", video_id);
                return;
            }
            int sender_uid = session->GetUid();  
            //反序列化获得评论
            Json::Value body;
            if (!ul::JsonUtil::Deserialize(body, req.body))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"反序列化失败，插入视频评论失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "插入视频评论反序列化失败, video_id : %d\n", video_id);
                return;
            }
            std::string comment = body["comment"].asString();
            if (!_comment->SendComment(video_id, sender_uid, comment))
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"插入视频评论失败"})";
                res.set_header("Content-Type", "application/json");
                LogMessage(Debug, "插入视频评论失败, video_id : %d, comment : %s\n", video_id, comment.c_str());
                return;
            }
            res.status = 200;
            res.body = R"({"result":true, "reason":"插入视频评论成功"})";
            res.set_header("Content-Type", "application/json");
            LogMessage(Debug, "服务器插入视频评论成功, video_id : %d, comment : %s\n", video_id, comment.c_str());
        }
    public: 
        HttpServer(uint16_t port)
            : _port(port)
        {
            pthread_mutex_init(&_db_mutex, nullptr);
        }

        ~HttpServer()
        {
            pthread_mutex_destroy(&_db_mutex);
        }

        // 服务器启动
        bool RunModule()
        {
            // 1.初始化我们的web资源目录
            ul::FileUtil(WEB_PATH).CreateDirectory();
            std::string web_path(WEB_PATH);
            std::string video_real_path = web_path + VIDEO_PATH;
            std::string image_real_path = web_path + VIDEO_IMG_PATH;
            ul::FileUtil(video_real_path).CreateDirectory();
            ul::FileUtil(image_real_path).CreateDirectory();
            // 2. 初始化数据库
            _video = std::make_unique<my::Video>(_db_mutex);
            _user = std::make_unique<my::User>(_db_mutex);
            _tag = std::make_unique<my::Tag>(_db_mutex);
            _type = std::make_unique<my::Type>(_db_mutex);
            _comment = std::make_unique<my::Comment>(_db_mutex);
            // 3.设置服务器静态资源路径
            _server.set_mount_point("/", WEB_PATH);
            // 4.添加路由
            //视频模块
            _server.Post("/video", Insert);              // 上传视频
            _server.Delete("/video/(\\d+)", Delete);     // 删除视频
            _server.Put("/video/(\\d+)", Update);        // 修改视频
            _server.Get("/video/(\\d+)", SelectOne);     // 查询单个视频
            _server.Get("/video/select", SelectALL);     // 查询所有视频
            _server.Get("/video/getTopK", GetVideoTopK); // 获取topk视频
            _server.Get("/video/getVideoOfType", GetVideoOfType); // 获取某种类型的视频
            _server.Get("/getTotal", GetTotal); // 获取视频属性总数
            _server.Post("/video/updateVideoRelation", UpdateVideoRelation); // 用户操作视频
            _server.Get("/video/getVideoRelation", GetVideoRelation); // 获取用户和视频的关系
            //用户模块
            // 登录
            _server.Post("/user/login", Login);//登录
            // 注册
            _server.Post("/user/register", Register);//注册
            //修改用户信息
            _server.Post("/user/updateUserInfo", UpdateUserInfo);//修改用户信息
            //获取用户信息
            _server.Get("/user/getUserInfo", GetUserInfo);//获取用户信息
            //获取视频用户信息
            _server.Get("/user/getVideoUserInfo", GetVideoUserInfo);//获取视频用户信息
            //获取视频评论用户信息
            _server.Get("/user/getCommentUserInfo", GetCommentUserInfo);//获取视频评论用户信息
            //获取用户所有视频
            _server.Get("/user/getVideo", GetUserVideo);//获取用户所有视频
            //获取用户收藏
            _server.Get("/user/getFavorite", GetFavorite);//获取用户收藏

            //标签模块
            _server.Get("/tag/(\\d+)", GetVideoTag); // 获取视频标签

            //类型模块
            _server.Get("/type/(\\d+)", GetVideoType); // 获取视频类型

            //评论模块
            _server.Get("/comment/getComment", GetComment); // 获取视频评论
            _server.Post("/comment/sendComment", SendComment); // 插入视频评论

            // 4.启动服务器
            _server.listen("0.0.0.0", _port);
            return true;
        }

    private:
        uint16_t _port;          // 服务器监听端口
        httplib::Server _server; // 服务器

        // 数据库管理
        static std::unique_ptr<my::Video> _video; // 数据库实例
        static std::unique_ptr<my::User> _user;   // 用户数据库实例
        static std::unique_ptr<my::Tag> _tag;   // 标签数据库实例
        static std::unique_ptr<my::Type> _type;   // 类型数据库实例
        static std::unique_ptr<my::Comment> _comment; // 评论数据库实例
        pthread_mutex_t _db_mutex;                // 数据库互斥锁

        // 会话管理
        static std::unordered_map<long long unsigned int, Session> _session_map; // Session管理
    };

    std::unique_ptr<my::Video> HttpServer::_video = nullptr;
    std::unique_ptr<my::User> HttpServer::_user = nullptr;
    std::unique_ptr<my::Tag> HttpServer::_tag = nullptr;
    std::unique_ptr<my::Type> HttpServer::_type = nullptr;
    std::unique_ptr<my::Comment> HttpServer::_comment = nullptr;
    std::unordered_map<long long unsigned int, Session> HttpServer::_session_map;
}