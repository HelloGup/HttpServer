#pragma once

#include <iostream>
#include <unistd.h>

class Entrance{
    public:
        static void* HanderRequest(void* _sock){
            int sock = (*(int*)_sock);

            delete (int*)_sock;
            std::cout << "get a new link...:" << sock << std::endl;


            close(sock);
            return nullptr;
        }
};
