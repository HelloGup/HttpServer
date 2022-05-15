#pragma once

#include "Log.hpp"
#include <mysql.h>

MYSQL* ConnectMysql(){
    MYSQL* my = mysql_init(nullptr);
    if(!my){
        LOG(ERROR,"init mysql error");
        exit(1);
    }

    if(!mysql_real_connect(my,"127.0.0.1","**","****","**",3306,nullptr,0)){
        LOG(ERROR,"connect mysql error");
        exit(2);
    }

    return my;
}

void InsertMysql(MYSQL* my,const char* sql){
    if(mysql_query(my,sql)){
        LOG(WARNING,"insert mysql error.");
    }
}

void CloseMysql(MYSQL* my){
    if(my){
        mysql_close(my);
    }
}
