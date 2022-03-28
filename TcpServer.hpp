#pragma once

#include <iostream>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include "Log.hpp"

#define BACKLOG 5 //全连接队列长度

class TcpServer{

    private:
        int listen_sock = -1;
        int port;

        //单例模式
        static TcpServer* instance;
        
        TcpServer(int p):port(p){};
        TcpServer(const TcpServer& tp) = delete;
    public:

        static TcpServer* GetInstance(int port){
            //静态全局锁初始化即可，不需要使用动态锁的初始化和释放了
            static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER; 
            if(instance == nullptr){
                pthread_mutex_lock(&mtx);
                if(instance == nullptr){
                    instance = new TcpServer(port);
                    instance->InitServer();
                }

                pthread_mutex_unlock(&mtx);
            }
            return instance;
        }

        void InitServer(){
            Socket();
            Bind();
            Listen();
            LOG(INFO,"Init Server Success");
        }

        void Socket(){
            listen_sock = socket(AF_INET,SOCK_STREAM,0);
            if(listen_sock < 0){
                LOG(FATAL,"Socket error");
                exit(1);
            }

            //设置端口复用
            int opt = 1;
            setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

            LOG(INFO,"Create Socket Success");
        }

        void Bind(){
            struct sockaddr_in local;
            memset(&local,0,sizeof(local));

            local.sin_family = AF_INET;
            local.sin_addr.s_addr = INADDR_ANY;
            local.sin_port = htons(port);

            if(bind(listen_sock,(struct sockaddr*)&local,sizeof(local)) < 0){
                LOG(FATAL,"Bind error");
                exit(2);
            }

            LOG(INFO,"Bind Success");
        }

        void Listen(){
            if(listen(listen_sock,BACKLOG) < 0){
                LOG(FATAL,"Listen error");
                exit(3);
            }

            LOG(INFO,"Listen Success");
        }

        //返回监听套接字
        int Sock(){
            return listen_sock;
        }

        ~TcpServer(){
            if(listen_sock >= 0){
                close(listen_sock);
            }
        }

};

//static成员类外初始化
TcpServer* TcpServer::instance = nullptr;







