#include <iostream>
#include "TcpServer.hpp"
#include <arpa/inet.h>
#include <signal.h>
#include "Task.hpp"
#include "ThreadPool.hpp"

class HttpServer{
    private:
        int port;
        bool stop;//是否退出

    public:
        HttpServer(int _port):port(_port),stop(false){
        
        }

        //初始化服务器
        void InitServer(){
            //需要忽略SIGPIPE信号，如果不忽略，在写入时可能客户端关闭链接
            //操作系统会通过发送SIGPIPE来终止写端进程,造成服务器奔溃
            signal(SIGPIPE,SIG_IGN);
        }

        void Loop(){

            LOG(INFO,"Loop Begin");

            //tcp服务器
            TcpServer* svr = TcpServer::GetInstance(port);
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

                char buf[16];
                inet_ntop(AF_INET,(void*)&peer.sin_addr,buf,sizeof(buf)-1); 

                //打印ip port
                std::string ip_addr = "ip:";
                ip_addr += buf;

                LOG(INFO,"Get a new Link." + ip_addr); 

                //构建任务
                //使用线程池处理任务
                Task* t = new Task(sock);
                ThreadPool::GetInstance()->Push(t);

                /*
                //创建线程处理请求
                int *p = new int(sock);
                pthread_t tid;
                pthread_create(&tid,nullptr,Entrance::HanderRequest,p);
                pthread_detach(tid);
                */
            }
        }

        ~HttpServer(){}
};



