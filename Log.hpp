#pragma once

#include <iostream>
#include <string>

#define INFO 1
#define WARNING 2
#define ERROR 3
#define FATAL 4

//我们需要显示等级名称，而不是宏值，使用#使在#后的首个参数返回为一个带引号的字符串，这样传递的就是宏名
//__FILE__是当前行所属的源文件名，__LINE__是当前所在源文件的行数
#define LOG(level,message) Log(#level,message,__FILE__,__LINE__)

//__DATE__是当前日期，__TIME__是当前时间
//[日志等级][时间戳][日志信息][错误文件名][行数]
void Log(std::string level,std::string message,std::string fileName,int line){
    std::cout << "[" << level << "][" << __DATE__ << " " << __TIME__ << "][" << message;
    std::cout << "][" << fileName << "][" << line << "]" << std::endl;
}
