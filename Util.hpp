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
        
        //二进制转进制
        static unsigned char ToHex(unsigned char x)   
        {   
            return  x > 9 ? x + 55 : x + 48;   
        }  
          
        static unsigned char FromHex(unsigned char x)   
        {   
            unsigned char y = 0;  
            if (x >= 'A' && x <= 'Z'){
                y = x - 'A' + 10;  
            } 
            else if (x >= 'a' && x <= 'z'){
                y = x - 'a' + 10;  
            } 
            else if (x >= '0' && x <= '9'){ 
                y = x - '0';  
            }
            else{
                assert(0);  
            } 
            return y;  
        }  
          
        static std::string UrlEncode(const std::string& str)  
        {  
            std::string strTemp = "";  
            size_t length = str.length();  
            for (size_t i = 0; i < length; i++)  
            {  
                if (isalnum((unsigned char)str[i]) ||   
                        (str[i] == '-') ||  
                        (str[i] == '_') ||   
                        (str[i] == '.') ||   
                        (str[i] == '~'))  
                    strTemp += str[i];  
                else if (str[i] == ' ')  
                    strTemp += "+";  
                else  
                {  
                    strTemp += '%';  
                    strTemp += ToHex((unsigned char)str[i] >> 4);  
                    strTemp += ToHex((unsigned char)str[i] % 16);  
                }  
            }  
            return strTemp;  
        }  

        static std::string UrlDecode(const std::string& str)  
        {  
            std::string strTemp = "";  
            size_t length = str.length();  
            for (size_t i = 0; i < length; i++)  
            {  
                if (str[i] == '+') strTemp += ' ';  
                else if (str[i] == '%')  
                {  
                    assert(i + 2 < length);  
                    unsigned char high = FromHex((unsigned char)str[++i]);  
                    unsigned char low = FromHex((unsigned char)str[++i]);  
                    strTemp += high*16 + low;  
                }  
                else strTemp += str[i];  
            }  
            return strTemp;  
        }
};
