#pragma once
#include <memory>
#include "httplib.h"
#include "mysql.hpp"
#include "util.hpp"

namespace aod
{
    class HttpServer
    {
        #define WEB_PATH "./web"//web根目录
        #define VIDEO_PATH "/video"
        #define IMG_PATH "/img"
    private:
        // 插入响应
        static void Insert(const httplib::Request &req, httplib::Response &res)
        {
            //要求上传：视频名称、视频作者、视频简介、视频路径、图片路径
            //视频文件数据、视频封面图片、
            httplib::MultipartFormData video_name_mul = req.get_file_value("name");//视频名称
            httplib::MultipartFormData video_author_mul = req.get_file_value("author");//视频作者
            httplib::MultipartFormData video_info_mul = req.get_file_value("info");//视频简介
            httplib::MultipartFormData video_file_mul = req.get_file_value("file");//视频文件
            httplib::MultipartFormData image_mul = req.get_file_value("image");//视频封面
            std::string video_name = video_name_mul.content;
            std::string video_author = video_author_mul.content;
            std::string video_info = video_info_mul.content;
            //拼接文件存放路径
            std::string video_path = WEB_PATH + std::string(VIDEO_PATH) + video_name + video_file_mul.filename;
            std::string image_path = WEB_PATH + std::string(IMG_PATH) + video_name + image_mul.filename;

            if (ul::FileUtil(video_path).SetContent(video_file_mul.content) == false)//如果保存视频文件失败，返回服务器错误
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"视频文件上传失败"})";//添加正文数据
                res.set_header("Content-Type", "application/json");//添加报头字段：正文类型为json数据
                return;
            }
            if (ul::FileUtil(image_path).SetContent(image_mul.content) == false)//如果保存封面图片失败，返回服务器错误
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"封面图片上传失败"})";//添加正文数据
                res.set_header("Content-Type", "application/json");//添加报头字段：正文类型为json数据
                return;
            }
            //修改数据库
            Json::Value value;
            value["name"] = video_name;
            value["author"] = video_author;
            value["info"] = video_info;
            value["video_path"] = video_path;
            value["img_path"] = image_path;
            if (_video->Insert(value) == false)//数据库插入失败
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"视频文件上传失败"})";//添加正文数据
                res.set_header("Content-Type", "application/json");//添加报头字段：正文类型为json数据
                return;
            }
            //添加成功后，告诉浏览器重定向到主业
            res.set_redirect("/index.html", 303);
            return;
        }

        // 删除响应
        static void Delete(const httplib::Request &req, httplib::Response &res)
        {
            httplib::MultipartFormData video_name_mul = req.get_file_value("name");//视频名称
            httplib::MultipartFormData video_author_mul = req.get_file_value("author");//视频作者
            httplib::MultipartFormData video_info_mul = req.get_file_value("info");//视频简介
            httplib::MultipartFormData video_file_mul = req.get_file_value("file");//视频文件
            httplib::MultipartFormData image_mul = req.get_file_value("image");//视频封面
            std::string video_name = video_name_mul.content;
            std::string video_author = video_author_mul.content;
            std::string video_info = video_info_mul.content;
            std::string video_path = WEB_PATH + std::string(VIDEO_PATH) + video_name + video_file_mul.filename;
            std::string image_path = WEB_PATH + std::string(IMG_PATH) + video_name + image_mul.filename;
            //web目录删除
            Json::Value value;
            value["name"] = video_name;
            value["author"] = video_author;
            value["info"] = video_info;
            value["video_path"] = video_path;
            value["image_path"] = image_path;
            if (_video->Delete(value) == false)//数据库删除失败
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"视频删除失败"})";//添加正文数据
                res.set_header("Content-Type", "application/json");//添加报头字段：正文类型为json数据
                return;
            }
            //数据库删除
            res.set_redirect("/index.html", 303);
        }

        // 查询响应，通过视频id进行搜索，只要一个结果
        static void SelectOne(const httplib::Request &req, httplib::Response &res)
        {
            int video_id = std::stoi(req.matches[1]);
            Json::Value result;
            if (!_video->SelectOne(video_id, &result))//数据库查询失败
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"视频查询失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            //查询成功，结果在result中，对result进行序列化
            ul::JsonUtil::Serialize(result, &res.body);
            res.set_header("Content-Type", "application/json");
            return;
        }

        //根据是否有参数search判断是否进行模糊查询
        static void SelectALL(const httplib::Request &req, httplib::Response &res)
        {
            // /video?search="关键字"
            bool search_flag = true;//默认全部查询
            if (req.has_param("search"))//如果有search参数，则进行模糊查询
            {
                search_flag = false;
            }
            Json::Value result;
            if (search_flag == true)//全部查询
            {
                if (!_video->SelectALL(&result))//数据库查询失败
                {
                    res.status = 500;
                    res.body = R"({"result":false, "reason":"视频查询失败"})";
                    res.set_header("Content-Type", "application/json");
                    return;
                }
            }
            else//模糊查询
            {
                std::string key = req.get_param_value("search");
                if (!_video->SelectLike(key, &result))//数据库查询失败
                {
                    res.status = 500;
                    res.body = R"({"result":false, "reason":"视频查询失败"})";
                    res.set_header("Content-Type", "application/json");
                    return;
                }
            }
            //查询成功，结果在result中，对result进行序列化
            ul::JsonUtil::Serialize(result, &res.body);
            res.set_header("Content-Type", "application/json");
            return;
        }

        // 更新响应
        static void Update(const httplib::Request &req, httplib::Response &res)
        {
            int video_id = std::stoi(req.matches[1]);
            Json::Value value;
            if (ul::JsonUtil::Deserialize(value, req.body) == false)//反序列化失败，说明请求数据格式不正确
            {
                res.status = 400;
                res.body = R"({"result":false, "reason":"请求数据格式不正确"})";
                res.set_header("Content-Type", "application/json");
                return;
            }   
            //数据格式合法
            //修改数据库
            Json::Value condition;
            condition["id"] = video_id;
            if (!_video->Update(value, condition) == false)//数据库修改失败
            {
                res.status = 500;
                res.body = R"({"result":false, "reason":"视频修改失败"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            res.set_redirect("/index.html", 303);
            return;
        }

    public:
        HttpServer(uint16_t port) : _port(port)
        {}

        ~HttpServer()
        {
        }

        // 服务器启动
        bool RunModule()
        {
            //1.初始化我们的web资源目录
            ul::FileUtil(WEB_PATH).CreateDirectory();
            std::string web_path(WEB_PATH);
            std::string video_real_path = web_path + VIDEO_PATH;
            std::string image_real_path = web_path + IMG_PATH;
            ul::FileUtil(video_real_path).CreateDirectory();
            ul::FileUtil(image_real_path).CreateDirectory();
            //2. 初始化数据库
            _video = std::make_unique<my::Video>();
            //3.设置服务器静态资源路径
            _server.set_mount_point("/", WEB_PATH);
            //4.添加路由
            _server.Post("/video", Insert);
            _server.Delete("/video/(\\d+)", Delete);
            _server.Put("/video/(\\d+)", Update);
            _server.Get("/video/(\\d+)", SelectOne);
            _server.Get("/video", SelectALL);
            //4.启动服务器
            _server.listen("0.0.0.0", _port);
            return true;
        }

    private:
        uint16_t _port;       // 服务器监听端口
        httplib::Server _server; // 服务器
        static std::unique_ptr<my::Video> _video;//数据库实例
    };

    std::unique_ptr<my::Video> HttpServer::_video = nullptr;
}
