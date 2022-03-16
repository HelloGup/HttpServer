#pragma once

#include <iostream>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>

#define BACKLOG 5 //全连接队列长度

class TcpServer{

    private:
        int listen_sock = -1;
        int port;

        static TcpServer* instance;
        
        TcpServer(int p):port(p){};
        TcpServer(const TcpServer& tp) = delete;
    public:

        static TcpServer* GetInstance(int port){
            //静态全局锁样初始化即可，不需要使用动态锁的初始化和释放了
            static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 
            if(instance == nullptr){
                pthread_mutex_lock(&lock);
                if(instance == nullptr){
                    instance = new TcpServer(port);
                    instance->InitServer();
                }

                pthread_mutex_unlock(&lock);
            }
            return instance;
        }

        void InitServer(){
            Socket();
            Bind();
            Listen();
        }

        void Socket(){
            listen_sock = socket(AF_INET,SOCK_STREAM,0);
            if(listen_sock < 0){
                exit(1);
            }

            //设置端口复用
            int opt = 1;
            setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
        }

        void Bind(){
            struct sockaddr_in local;
            memset(&local,0,sizeof(local));

            local.sin_family = AF_INET;
            local.sin_addr.s_addr = INADDR_ANY;
            local.sin_port = htons(port);

            if(bind(listen_sock,(struct sockaddr*)&local,sizeof(local)) < 0){
                exit(2);
            }
        }

        void Listen(){
            if(listen(listen_sock,BACKLOG) < 0){
                exit(3);
            }
        }

        int Sock(){
            return listen_sock;
        }

};

TcpServer* TcpServer::instance = nullptr;
