#pragma once

#include <iostream>
#include <string>

#define INFO 1
#define WARNING 2
#define ERROR 3
#define FATAL 4

//我们需要显示等级名称，而不是宏值，使用#可以传递宏名字符串
//__FILE__是文件名的宏，__LINE__是当前所在行
#define LOG(level,message) Log(#level,message,__FILE__,__LINE__)

//[日志等级][时间戳][日志信息][错误文件名][行数]
void Log(std::string level,std::string message,std::string fileName,int line){
    std::cout << "[" << level << "][" << time(nullptr) << "][" << message;
    std::cout << "][" << fileName << "][" << line << "]" << std::endl;
}
