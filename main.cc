#include "httpServer.hpp"
#include <string>
#include <memory>
#include "Log.hpp"

void Usage(std::string proc){
    std::cout << "Usage:" << std::endl;
    std::cout << "\t" << proc << " port" << std::endl;
}

int main(int argc,char* argv[]){
    if(argc != 2){
        Usage(argv[0]);
        exit(4);
    }

    std::shared_ptr<HttpServer> httpServer(new HttpServer(atoi(argv[1])));
    httpServer->InitServer();
    httpServer->Loop();

    //LOG(INFO,"测试日志");
      
    return 0;
}
