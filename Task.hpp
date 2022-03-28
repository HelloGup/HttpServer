#ifndef __TASK_H__
#define __TASK_H__

#include <iostream>
#include "Protocol.hpp"

class Task{
    int sock;
    CallBack handler;//回调对象

    public:
    Task(int _sock):sock(_sock){
    }

    //调用回调函数
    void PrcessOn(){
        LOG(INFO,"task call callback.");
        
        handler(sock);
    }

    ~Task(){
    }
};

#endif
