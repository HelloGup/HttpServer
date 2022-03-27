#include <iostream>
#include "Protocol.hpp"
#include "TcpServer.hpp"

class HttpServer{
    private:
        int port;
        //tcp服务器
        TcpServer* svr;
        bool stop;//是否退出
    public:
        HttpServer(int _port):port(_port),svr(nullptr),stop(false){
        
        }

        //初始化服务器
        void InitServer(){
            svr = TcpServer::GetInstance(port);
        }

        void Loop(){

            LOG(INFO,"Loop Begin");

            int listen_sock = svr->Sock();

            while(!stop){
                //获取链接
                struct sockaddr_in peer;
                socklen_t len = sizeof(peer);
                int sock = accept(listen_sock,(struct sockaddr*)&peer,&len);
                if(sock < 0){
                    LOG(ERROR,"Accept Error");
                    continue;
                }

                LOG(INFO,"Get a new Link");
                
                //创建线程处理请求
                int *p = new int(sock);
                pthread_t tid;
                pthread_create(&tid,nullptr,Entrance::HanderRequest,p);
                pthread_detach(tid);
            }
        }

        ~HttpServer(){}
};



