#pragma once
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

class Util{

    private:

    public:
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
};
