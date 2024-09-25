#include <mysql/mysql.h>

namespace my
{
    #define HOST "127.0.0.1"
    #define USER "root"
    #define PASSWD ""
    #define DBNAME "software_project"
    
    //初始化MySQL操作句柄
    //设置客户端字符集
    static MYSQL *MysqlInit()
    {
        //初始化MySQL操作句柄
        MYSQL *mysql = mysql_init(nullptr);
        //连接数据库服务器
        //Args:操作句柄、主机ip、用户名、密码、数据库名、
        mysql_real_connect(mysql, HOST, USER, PASSWD, DBNAME, 0, nullptr, 0);
        //客户端设置字符集
        mysql_set_character_set(mysql, "utf8");
        //选择数据库
        mysql_select_db(mysql, DBNAME);
        return mysql;
    }

    
}