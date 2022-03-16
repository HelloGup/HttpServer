#include <iostream>
#include "Protocol.hpp"
#include "TcpServer.hpp"

class HttpServer{
    private:
        int port;
        TcpServer* svr;
        bool stop;
    public:
        HttpServer(int _port):port(_port),svr(nullptr),stop(false){
        
        }

        void InitServer(){
            svr = TcpServer::GetInstance(port);
        }

        void Loop(){
            int listen_sock = svr->Sock();

            while(!stop){
                struct sockaddr_in peer;
                socklen_t len = sizeof(peer);
                int sock = accept(listen_sock,(struct sockaddr*)&peer,&len);
                if(sock < 0){
                    std::cerr << "accept error" << std::endl;
                    continue;
                }
                
                int *p = new int(sock);
                pthread_t tid;
                pthread_create(&tid,nullptr,Entrance::HanderRequest,p);
                pthread_detach(tid);
            }
        }

        ~HttpServer(){}
};
