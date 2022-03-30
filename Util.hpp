#pragma once
#include <assert.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include "Log.hpp"

class Util{

    private:

    public:
        //读取一行
        static int ReadLine(int sock,std::string& out){
            char ch = 'x';
            while(ch != '\n'){
                ssize_t ss = recv(sock,&ch,1,0);
                if(ss > 0){
                    //http协议的行分割符可能是\r\n也能是\n或着\r,需要判断
                    //将\r和\r\n都转为\n
                    if(ch == '\r'){
                        //当判断'\r'后面是不是还有'\n'时，不能使用recv再往后读，否则假如不是'\n'的话
                        //就把下一行的第一个字符给取走了,所以只能查看而不取走
                        //所以就要使用到recv的一个选项MSG_PEEK,它的作用是从接收缓冲区中返回第一个字符但并不取走
                        //该功能叫作数据窥探功能
                        recv(sock,&ch,1,MSG_PEEK);
                        if(ch == '\n'){
                            //\r\n 转为 \n
                            //窥探成功 这个字符存在，那么就正常往后取一个字符
                            recv(sock,&ch,1,0);
                        }
                        else{
                            // \r 转为 \n
                            ch = '\n';
                        }
                    }

                    out += ch;
                }
                else if(ss == 0){ 
                    //对端关闭
                    return 0;
                }
                else{
                    //出错
                    return -1;
                }
            }

            //正常结束返回读取到的个数
            return out.size();
        }

        //切分字符串 不要改变源字符串，使用const
        static bool CutString(const std::string& target,std::string& out_key,std::string& out_value,std::string sep/*分隔符*/){
            //自己写
           // size_t index = 0;
           // size_t len = target.size();
           // while(index < len){
           //     //HTTP协议报头中的每行:后面还有一个空格
           //     if(target[index] == ':' && index + 1 < len && target[index+1] == ' '){
           //         index += 2;
           //         while(index < len){
           //             out_value += target[index];
           //             index++;
           //         }
           //         break;
           //     }

           //     out_key += target[index];
           //     index++;
           // }

            //库函数
            size_t pos = target.find(sep);
            if(pos != std::string::npos){
               out_key = target.substr(0,pos); 
               //substr不指定截取多少个，则默认截取到结尾
               out_value = target.substr(pos + sep.size());
               return true;
            }
            return false;
        }
        

        //4位二进制转16进制字符形式
        static unsigned char ToHex(unsigned char str)   
        {
            //大于9就是'A'-'F'，<=9就是'0'-'9'
            return str > 9 ? str + 55:str + 48;
        }


        //urlcode -> 4位二进制
        static unsigned char FromHex(unsigned char str)   
        {
            unsigned char y = 0;
            str = toupper(str);
            //A'-'F'转为对应十进制数字
            if(str >= 'A' && str <= 'F'){
                y = str - 'A' + 10;  
            }
            //'0'-'9'转为对应十进制数字
            else if(str >= '0' && str <='9'){
                y = str - '0';
            }
            //超过15的字符是错误字符，16进制一位不可能超过15
            else{
                LOG(ERROR,"data error.");
            }
            return y;
        }

        //编码规则：
        //1. 字母数字字符 "a" 到 "z"、"A" 到 "Z" 和 "0" 到 "9" 保持不变。
        //2. 特殊字符 "."、"-"、"*" 和 "_" 保持不变。
        //3. 空格字符 " " 转换为一个加号 "+"。
        //4. 所有其他字符都是不安全的，因此首先使用一些编码机制将它们转换为一个或多个字节。
        //然后每个字节用一个包含 3 个字符的字符串 "%xy" 表示，其中 xy 为该字节的两位十六进制表示形式。
        //推荐的编码机制是 UTF-8。
        
        //url编码
        static std::string UrlEncode(const std::string& str)  
        {
            std::string tmp;
            for(size_t i = 0;i < str.size();i++)
            {
                unsigned char ch = 0;
                //字母数字等不编码
                if(isalnum(str[i]) || str[i] == '.' || str[i] == '-' || str[i] == '*' || str[i] == '='){
                   tmp += ch; 
                }
                //空格替换为'+'
                else if(str[i] == ' '){
                    tmp += '+';
                }
                //其他字符编码为 %xx 类型字符串
                else{
                    tmp += '%';
                    //高四位编码
                    tmp += ToHex(str[i] >> 4);
                    //低四位编码
                    tmp += ToHex(str[i] & 0xf);
                }
            }
            return tmp;
        }

        //url解码
        static std::string UrlDecode(const std::string& str)
        {
            std::string tmp;
            for(size_t i = 0;i < str.size();i++){
                //遇% 后面的两位解码
                if(str[i] == '%'){
                    if(i + 2 >= str.size()){
                        return nullptr;
                    }
                    //解码第一个字符 是高4位的16进制字符
                    unsigned char h = FromHex(str[++i]) << 4;
                    //解码第二个字符 是低四位的16进制字符
                    unsigned char l = FromHex(str[++i]);
                    //合并为一个字符
                    tmp += h+l;
                } 
                //+替换为字符
                else if(str[i] == '+'){
                    tmp += ' ';
                }
                //其他都是不转码的 直接追加
                else{
                    tmp += str[i];
                }
            }

            return tmp;
        }
};
