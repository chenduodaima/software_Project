# 欢迎来到我的个人项目——视频点播系统

[toc]

## 项目环境

- 操作系统：Ubuntu 24.04.1 LTS
- 编辑器：Cursor IDE
- 编译器：g++ (Ubuntu 9.5.0-6ubuntu2) 9.5.0
- 技术栈：C++11、STL、MySQL、多线程编程、多进程编程、html/css/js/jquery/ajax
- 第三方依赖库：
  - httplib：HTTP服务器库
  - libjsoncpp：JSON解析库
  - libmariadb：MySQL数据库操作库
  - libboost：Boost库

## 项目描述

该系统是一个视频点播系统，主要功能是提供基于Web的视频点播网站，让用户可以在线搜索观看视频，并进行管理。

## 项目结构

- 前端：

  - web：前端模块
- 后端：

  - Commen.hpp：公共头文件
  - Log.hpp：日志类
  - util.hpp：工具类
  - mysql.hpp：MySQL操作类
  - http.hpp：HTTP服务器类

## 项目架构：

- 前端：
  - 使用html/css/js/jquery/ajax等技术实现
  - 使用jquery实现页面交互
  - 使用ajax实现异步请求

## 项目功能

- 登录
- 注册
- 视频上传
- 视频删除
- 视频修改
- 视频搜索
- 视频排序
- 视频推荐
- 修改个人信息

## 项目使用

- aod：视频播放服务器，需要指定端口号port

## 总结

这个项目是我在学习C++11、STL、MySQL、多线程编程、多进程编程、html/css/js/jquery/ajax等技术栈时，为了巩固所学知识，自己动手实现的一个视频点播系统，目前暂未实现管理员模式，还有很大的提升空间。

## 致谢

感谢各位的阅读和使用，同时也感谢开源库的作者们，没有你们的辛勤工作，就没有这个项目。
