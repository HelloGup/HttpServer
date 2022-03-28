#pragma once

#include <iostream>
#include <unistd.h>
#include "Util.hpp"
#include <string>
#include <stdlib.h>
#include <vector>
#include "Log.hpp" 
#include <sstream>
#include <unordered_map>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <algorithm>

//行分割符
#define LINE_END "\r\n"
//报头键值对分隔符，HTTP协议报头中的:后还有一个空格
#define SEP ": "

//状态码
#define OK 200 //正常
#define NOT_FOUND 404 //资源不存在
#define BAD_REQUEST 400 //非法请求
#define SERVER_ERROR 500 //服务器错误

//web根目录
#define WEB_ROOT "wwwroot"
//首页
#define HOME_PAGE "index.html"

//HTTP版本
#define HTTP_VERSION "HTTP/1.0"

//404页面
#define PAGE_404 "404.html"

//状态码描述映射
static std::string Code2Desc(int code){
    std::string desc;
    switch(code){
        case 200:
            desc = "OK";
            break;
        case 404:
            desc = "Not Found";
            break;
        default:
            break;
    }

    return desc;
}

//content-type映射
static std::string Suffix2Desc(std::string& suffix){
    //静态变量只会被初始化一次 不要使用insert，否则调用函数多次插入
    static std::unordered_map<std::string,std::string> suffix_map = {
        {".html","text/html"},
        {".ico","image/x-icon"},
        {".css","text/css"},
        {".js",".js application/x-javascript"}
    };

    auto it = suffix_map.find(suffix);
    if(it != suffix_map.end()){
        return it->second;
    }

    //默认返回.html对应的网页格式
    return suffix_map[".html"]; 
} 

class HttpRequest{
    public:
        std::string request_line;//请求行
        std::vector<std::string> request_header;//请求报头
        std::string blank;//空行
        std::string request_body;//请求正文

        //解析完毕后的结果
        std::string method;//请求方法
        std::string uri;//请求的资源路径 可能会有两部分构成，请求的资源路径?参数
        std::string path;//请求的资源路径
        std::string suffix;//请求的资源后缀
        std::string query_string;//传递的参数
        std::string version;//HTTP版本

        //请求报头键值对
        std::unordered_map<std::string,std::string> header_map;

        //正文长度
        int content_length = 0;

        //要发送的资源文件大小
        int size;

        //是否使用CGI机制
        bool cgi = false;
};

class HttpResponse{
    public:
        std::string status_line;
        std::vector<std::string> response_header;
        std::string blank = LINE_END;//初始化空行
        std::string response_body;

        //状态码
        int status_code = OK;

        //要发送的资源文件描述符
        int fd;
};

//负责读取请求，分析请求，构建响应，完成IO通信
class EndPoint{
    private:
        int sock;
        HttpRequest http_Request;
        HttpResponse http_Response;

        //是否退出
        bool stop = false;

        //读取请求行
        bool RecvRequestLine(){
            auto& line = http_Request.request_line;
            if(Util::ReadLine(sock,line) > 0){
                //删除掉最后的'\n'，没有用
                line.resize(line.size()-1);
            }
            //读取出错就退出
            else{
                stop = true;
            }

            LOG(INFO,line);
            return stop;
        }

        //读请求报头
        bool RecvRequestHeader(){
            std::string line;
            while(true){
                //每次循环开始清空上次的数据
                line.clear();

                //每次读一行 且换行符被替换为'\n'
                if(Util::ReadLine(sock,line) <= 0){
                    //出错就退出
                    stop = true;
                    break;
                }
                //读到空行退出
                if(line == "\n"){
                    http_Request.blank = line;
                    break;
                }

                //不是空行添加进请求报头数组中
                //去除'\n'
                line.resize(line.size()-1);
                http_Request.request_header.push_back(line);
            }

            return stop;
        }

        //分析请求行
        void ParseRequestLine(){
            //请求行
            auto& line = http_Request.request_line;

            //stringstream 进行流的输入输出操作，可将任意类型转为string，或string转为任意类型 
            //使用流进行多次类型转换时每次结束要clear()清空流
            //也可用来切分字符串，切分时默认以空格切分
            std::stringstream ss;
            ss << line;//可将任意类型放入 
            //可将流中数据输出为任意类型，从ss中以空格切分分别赋值
            ss >> http_Request.method >> http_Request.uri >> http_Request.version; 

            //用户请求的方法可能是GET,Get,get等大小写不一致的情况，而我们统一使用GET判断的
            //所以还需要将方法统一转为全部大写,全局toupper()函数用于将小写字母转换为大写字母
            //可以使用algorithm里的transform函数来对每个元素调用toupper()函数
            //对指定范围中的每个元素调用某个函数以改变元素的值,存放到result的位置
            //OutputIterator transform (InputIterator first1, InputIterator last1,OutputIterator result, UnaryOperator op)
            //返回一个指向转换范围末端的迭代器  ::全局函数
            std::transform(http_Request.method.begin(),http_Request.method.end(),http_Request.method.begin(),::toupper);
        }

        //分析请求报头
        void ParseRequestHeader(){
            std::string key;
            std::string value;

            //分析每行为键值对
            for(auto& line : http_Request.request_header){
                if(Util::CutString(line,key,value,SEP)){
                    http_Request. header_map.insert(std::pair<std::string,std::string>(key,value));
                }
            }
        }


        //是否需要读取正文 这里设计的是只有POST方法才读正文
        bool IsNeedRecvRequestBody(){
            if(http_Request.method == "POST"){
                //报头中查找是否有正文 如有将长度拿出
                auto it = http_Request.header_map.find("Content-Length");
                if(it != http_Request.header_map.end()){
                    http_Request.content_length = atoi(it->second.c_str());
                    return true;
                }
            }
            return false;
        }

        //读取正文
        bool RecvRequestBody(){
            //判断需要读
            if(IsNeedRecvRequestBody()){
                //正文长度
                int count = http_Request.content_length;
                //正文
                auto& body = http_Request.request_body;

                //一个一个读
                char ch = 0;
                while(count--){
                    ssize_t ss = recv(sock,&ch,1,0);
                    if(ss > 0){
                        body.push_back(ch);
                    }
                    else{
                        stop = true;
                        break;
                    }
                }
            }

            return stop;
        }


        //调用CGI程序处理 使用exec*系列函数
        int ProcessCGI(){
            int code = OK;
            //此时要请求的资源就是要执行的程序

            //环境变量
            std::string query_string_env;
            std::string method_env;
            std::string content_length_env;

            //建立匿名管道
            //以父进程的角度看
            int input[2];
            int output[2];
            if(pipe(input) < 0){
                LOG(ERROR,"Input Pipe Build Error.");
                code = SERVER_ERROR;
                return code;
            }
            if(pipe(output) < 0){
                LOG(ERROR,"Output Pipe Build Error.");
                code = SERVER_ERROR;
                return code;
            }

            //创建子进程进行程序替换，直接替换会将httpServer替换掉
            pid_t pid = fork();
            if(pid == 0){
                //child
                //子进程关闭2条管道的读和写
                close(input[0]);
                close(output[1]);

                //导入环境变量
                method_env = "METHOD=";
                method_env += http_Request.method;
                putenv((char*)method_env.c_str());//c++11字符串是const的,强转一下

                //按照方法导入各自需要的环境变量
                if(http_Request.method == "GET"){
                    query_string_env = "QUERY_STRING=";
                    query_string_env += http_Request.query_string;

                    putenv((char*)query_string_env.c_str());
                }
                else if(http_Request.method == "POST"){
                    //子进程处理POST方法时,需要知道正文长度,使用环境变量传递
                    content_length_env = "CONTENT_LENGTH="; 
                    content_length_env += std::to_string(http_Request.content_length);
                    putenv((char*)content_length_env.c_str());
                }
                else{
                    //Do Nothing
                }

                //防止替换后找不到管道，读0写1即可
                //写  --  1  --  input[1]
                //读  --  0  --  output[0];
                //dup2(oldfd,newfd),新的是旧的的一份拷贝，并适时关闭新的
                dup2(input[1],1);
                dup2(output[0],0);

                //程序替换
                execl(http_Request.path.c_str(),http_Request.path.c_str(),nullptr);
                //替换失败退出
                exit(1);
            }
            else if(pid < 0){
                LOG(ERROR,"Fork Error.");
                code = SERVER_ERROR;
                return code;
            }
            else{//parent
                //父进程关闭不用的读写端
                close(input[1]);
                close(output[0]);

                //传递参数
                //根据方法判断参数是正文还是请求变量中
                if(http_Request.method == "POST"){
                    //为防止参数长度大于管道长度，需要写多次才能写完，就要处理一下每次写的内容
                    int len = http_Request.request_body.size(); 
                    const char* body_text = http_Request.request_body.c_str();
                    int total = 0;//已经写入的个数
                    int size = 0;//本次写入的个数
                    while(total < http_Request.content_length && (size = write(output[1],body_text + total,len - total))){
                        total += size;
                    }
                }

                //读取CGI程序处理后的数据
                char ch = 0;
                while(read(input[0],&ch,1) > 0){
                    //不能直接send发，要保证响应行和响应报头、空行都发完了才能发

                    http_Response.response_body += ch;
                }

                //是当前线程在阻塞等，不会阻塞主线程
                int status = 0;
                pid_t ret = waitpid(pid,&status,0);
                if(ret == pid){
                    //正常退出
                    if(WIFEXITED(status)){
                        //正常退出结果正确
                        if(WEXITSTATUS(status) == 0){
                            code = OK;
                        }
                        //结果不正确
                        else{
                            code = BAD_REQUEST;
                        }
                    }
                    //非正常退出
                    else{
                        code = SERVER_ERROR;
                    }
                }

                //通信完毕，关闭管道
                close(input[0]);
                close(output[1]);
            }
            return code;
        }

        //非CGI 返回静态资源
        int ProcessNonCGI(){
            //以只读方式打开请求的资源文件 为发送正文做准备
            http_Response.fd = open(http_Request.path.c_str(),O_RDONLY);
            LOG(INFO,"File: " + std::to_string(http_Response.fd) + " is open.");
            //只有当请求的资源正常打开了，才构建响应，打开出错，就没必要构建了
            if(http_Response.fd >= 0){
                return OK;
            }
            return NOT_FOUND;
        }

        //处理错误请求的响应
        void HandlerError(const std::string& page){
            //为防止CGI程序里出错，后续发送还根据CGI是否为真按CGI的方式发正文就会出错
            //此时应该发送的是静态页面，所以只要出错就将cgi设为false
            http_Request.cgi = false;

            //打开文件
            http_Response.fd = open(page.c_str(),O_RDONLY);
            if(http_Response.fd >= 0){
                //获取属性
                struct stat st;
                stat(page.c_str(),&st);

                //构建响应报头
                std::string line = "Content-Type: text/html";
                line += LINE_END;
                http_Response.response_header.push_back(line);

                line = "Content-Length: ";
                line += std::to_string(st.st_size);
                line += LINE_END;
                http_Response.response_header.push_back(line);

                //更新该页面的大小
                http_Request.size = st.st_size;
            }
        }

        //构建正确的请求方法响应
        void BuildOkResponse(){
            //构建响应报头
            std::string line = "Content-Type: ";
            line += Suffix2Desc(http_Request.suffix);
            line += LINE_END;
            http_Response.response_header.push_back(line);

            line = "Content-Length: ";
            //CGI返回的大小是正文的大小
            if(http_Request.cgi){
                line += std::to_string(http_Response.response_body.size());//POST
            }
            //非CGI返回的是静态网页，大小就是文件的大小
            else{
                line += std::to_string(http_Request.size);//GET
            }
            line += LINE_END;
            http_Response.response_header.push_back(line);
        }

        //构建错误请求的响应
        void BuildHttpResponseHelper(){
            //构建响应行
            auto& code = http_Response.status_code;
            auto& status_line = http_Response.status_line;
            status_line = HTTP_VERSION;
            status_line += " ";
            status_line += std::to_string(code);
            status_line += " ";
            status_line += Code2Desc(code);
            status_line += LINE_END;

            //用web根目录拼接错误响应页面的路径
            std::string error_path = WEB_ROOT;
            error_path += "/";

            //根据不同错误码构建报头和正文
            switch(code){
                case OK:
                    BuildOkResponse();
                    break;
                case NOT_FOUND:
                    error_path += PAGE_404;
                    HandlerError(error_path);
                    break;

                    //以下选项未单独做页面，暂时全使用404页面
                case SERVER_ERROR:
                    error_path += PAGE_404;
                    HandlerError(error_path);
                    break;
                case BAD_REQUEST:
                    error_path += PAGE_404;
                    HandlerError(error_path);
                    break;

                default:
                    break;
            }
        }
    public:
        EndPoint(int _sock):sock(_sock){}

        //读取请求
        void RecvRequest(){
            //读取请求行和报头都不出错 才开始解析和读正文
            if(!RecvRequestLine() && !RecvRequestHeader()){
                //解析请求行
                ParseRequestLine();
                //解析请求报头
                ParseRequestHeader();
                //读取正文
                RecvRequestBody();
            }
        }

        bool Stop(){
            return stop;
        }

        //构建响应
        void BuildResponse(){
            //path添加web根目录前缀
            std::string tmp_path = WEB_ROOT;
            //查找后缀
            size_t ret = 0; 

            auto& code = http_Response.status_code;

            //非法请求
            if(http_Request.method != "GET" && http_Request.method != "POST"){
                LOG(WARNING,"Method is not right");
                code = BAD_REQUEST;

                goto END;
            }

            //处理GET请求
            if(http_Request.method == "GET"){
                size_t pos = http_Request.uri.find('?');
                //带参数分离路径和参数
                if(pos != std::string::npos){
                    Util::CutString(http_Request.uri,http_Request.path,http_Request.query_string,"?");
                    //如果GET方法带参数了需要使用CGI机制
                    http_Request.cgi = true;
                }
                //不带参数请求的路径就是uri
                else{
                    http_Request.path = http_Request.uri;
                }
            }
            //POST方法
            else if(http_Request.method == "POST"){
                //使用CGI机制处理
                http_Request.cgi = true;
                //post方法请求的uri就是资源路径
                http_Request.path = http_Request.uri;
            }
            else{
                //其他方法 暂无
            }

            //添加web根目录前缀 将请求路径转移到web根目录
            //注意这里临时变量在goto和END之间定义有可能会越过这个语句，所以编译会报错，应该定义在goto和END外
            //std::string tmp_path = WEB_ROOT;
            tmp_path += http_Request.path;
            http_Request.path = tmp_path;

            //如果请求的路径是根目录，则拼接首页资源，默认返回首页
            if(http_Request.path[http_Request.path.size()-1] == '/'){
                http_Request.path += HOME_PAGE;
            }

            struct stat st;
            //如请求的资源存在
            if(stat(http_Request.path.c_str(),&st) == 0){
                //如果请求的资源是一个目录，则返回当前目录的index.html
                if(S_ISDIR(st.st_mode)){
                    //这里需要先拼接一个'/',因为结尾为'/'的情况上面已经处理
                    //到这里的路径后面一定没有'/'，直接拼接资源路径就出错了
                    http_Request.path += '/';
                    http_Request.path += HOME_PAGE; 

                    //拼接首页资源后，文件属性就应该更新为首页资源的属性了
                    stat(http_Request.path.c_str(),&st);
                } 

                //如果请求的是一个可执行程序,即三个用户组任意一个具有可执行权限就是可执行程序
                if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)){
                    //请求的是可执行程序需要使用CGI机制处理
                    http_Request.cgi = true;
                }

                //获取文件大小
                http_Request.size = st.st_size;
            }

            //资源不存在
            else{
                LOG(WARNING,http_Request.path + " Path Not Found.");
                code = NOT_FOUND;
                goto END;
            }

            //执行到这里 说明路径合法 请求的资源一定存在

            //提取文件后缀 防止文件名有多个后缀，应从后往前找使用rfind
            ret = http_Request.path.rfind(".");
            if(ret == std::string::npos){
                //没找到后缀则默认设为.html文件
                http_Request.suffix = ".html"; 
            }
            else{
                http_Request.suffix = http_Request.path.substr(ret);
            }

            //判断是否需要CGI处理
            if(http_Request.cgi){
                code =  ProcessCGI();
            }
            //不需要CGI处理
            else{
                //静态网页返回
                //说明一定是不带参的GET方法，就是简单的返回网页资源
                code = ProcessNonCGI();
            }

            //为什么要用goto 因为http中每一步都可能出错，会导致出现大量if判断语句，使用goto会简洁一些
END://构建响应
            BuildHttpResponseHelper();

        }

        //发送响应
        void SendResponse(){
            //1.发送响应行
            send(sock,http_Response.status_line.c_str(),http_Response.status_line.size(),0);
            //2.发送响应报头
            for(std::string& line : http_Response.response_header){
                send(sock,line.c_str(),line.size(),0);
            }
            //3.发送空行
            send(sock,http_Response.blank.c_str(),http_Response.blank.size(),0);
            //4.发送正文

            //CGI方式处理的发正文
            if(http_Request.cgi){
                auto& response_body = http_Response.response_body;

                //为防止正文长度过大，将发送缓冲区填满，所以设计循环一直发直达发完
                size_t size = 0;
                size_t total = 0;
                while(total < response_body.size() && (size = send(sock,response_body.c_str()+total,response_body.size()-total,0))){
                    total += size;
                }
            }
            //非CGI方式处理的发静态网页
            else{
                //页面size要注意返回错误响应页面时要重新计算大小
                if(http_Response.fd >= 0){
                    sendfile(sock,http_Response.fd,nullptr,http_Request.size);
                    //发送完应将打开的资源文件关闭
                    close(http_Response.fd);
                    LOG(INFO,"File: " + std::to_string(http_Response.fd) + " is close.");
                }
            }
        }

        ~EndPoint(){
            close(sock);
        }
};

//#define DEBUG 1;

//回调
class CallBack{
    public:

        void operator()(int sock){
            HandlerRequest(sock);
        }
        
        //处理请求函数
        void HandlerRequest(int sock){
            LOG(INFO,"Hander Request Begin");

            //处理请求
            EndPoint* ep = new EndPoint(sock);
            ep->RecvRequest();

            //只有读取没有出错，才构建响应发送
            if(!ep->Stop()){
                LOG(INFO,"Recv Right,Begin Build And Send.");

                ep->BuildResponse();
                ep->SendResponse();
            }
            else{
                LOG(WARNING,"Recv Error,Stop Build And Send.");
            }

            delete ep;

            LOG(INFO,"Hander Request End");
        }

        ~CallBack(){

        }
};
