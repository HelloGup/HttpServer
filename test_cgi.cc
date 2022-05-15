#include "Util.hpp"
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <unistd.h>
#include "mysqlUtil.hpp"

//获取参数
bool GetQueryString(std::string& query_string){
    //特别注意，子进程已经将标准输入输出重定向到了管道，使用cout是无法输出的
    //要使用cerr
    //std::cerr << "debug test:" << getenv("METHOD") << std::endl;
    
    //获取方法
    std::string method = getenv("METHOD");

    //获取参数
    if(method == "GET"){
        query_string = getenv("QUERY_STRING");
        //std::cerr << "debug GET query_string:" << query_string << std::endl;
        return true;
    }
    else if(method == "POST"){
        int content_length = 0;
        std::stringstream st;
        st << getenv("CONTENT_LENGTH");
        st >> content_length;
        
        char ch = 0;
        while(content_length--){
            ssize_t s = read(0,&ch,1);
            if(s > 0){
                query_string += ch;    
            }
            else if(s == 0){

            }
            else{
                
            }
        }
        //std::cerr << "debug POST query_string:" << query_string << std::endl;

        return true;
    }
    else{
        //other method
        return false;
    }
}

//分割参数
int CutString(std::string& target,std::string& out1,std::string& out2,const std::string& sep){
    size_t pos = target.find(sep);
    if(pos != std::string::npos){
        out1 = target.substr(0,pos);
        out2 = target.substr(pos+sep.size());
        return 0;
    }
    else{
        return -1;
    }
}


int main(){
    //获取参数
    //a=100&c=200
    std::string query_string; 
    GetQueryString(query_string);
    //std::cerr << query_string << std::endl;
    query_string = Util::UrlDecode(query_string.c_str());

    //解析为多个参数
    std::string str1;
    std::string str2;
    CutString(query_string,str1,str2,"&");

    //解析为键值对
    std::string key1;
    std::string val1;
    CutString(str1,key1,val1,"=");

    std::string key2;
    std::string val2;
    CutString(str2,key2,val2,"=");

    //解码
    //标准错误打印测试

    //std::cerr << key1 << ":" << val1 << std::endl;
    //std::cerr << key2 << ":" << val2 << std::endl;
   
    
    MYSQL* my = ConnectMysql();

    std::string sql = "insert into httpMsg (name,msg,msgTime) values (\'";
    sql += val1;
    sql += "\',\'";
    sql += val2;
    sql += "\',";
    sql += "now());";

    InsertMysql(my,sql.c_str());

    CloseMysql(my);

    std::string echo = "<html>\
                        <head>\
                        <meta charset=\"UTF-8\">\
                        </head>\
                        <body>\
                        <h1>谢谢，我已收到您的留言，并尽快给您回复</h1>\
                        </body>\
                        </html>";

    //给客户端发送：子进程的标准输出已经重定向为管道了 此时cout就是往管道里写数据
    std::cout << echo << std::endl;

    return 0;
}
