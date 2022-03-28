#pragma once

#include <iostream>
#include <pthread.h>
#include <queue>
#include "Task.hpp"

class ThreadPool{
    private:
        //任务队列
        std::queue<Task*> task_q; 
        //互斥锁
        pthread_mutex_t mtx;
        //条件变量
        pthread_cond_t cod;
        int max_num = 8;

        static ThreadPool* instance;

        ThreadPool(){}
        ThreadPool(const ThreadPool& tp) = delete;
        ThreadPool& operator=(const ThreadPool& tp) = delete;

        static void* Routine(void* arg){
            ThreadPool* tp = (ThreadPool*)arg;

            while(true){
                pthread_mutex_lock(&tp->mtx);

                //防止假唤醒
                while(tp->task_q.empty()){
                   pthread_cond_wait(&tp->cod,&tp->mtx); 
                }

                Task* t = tp->Get();
                pthread_mutex_unlock(&tp->mtx);

                t->PrcessOn();
                delete t;
            }

            return nullptr;
        }

        Task* Get(){
            Task* tmp = task_q.front();
            task_q.pop();

            return tmp;
        }

        bool ThreadPoolInit(){
            pthread_mutex_init(&mtx,nullptr);
            pthread_cond_init(&cod,nullptr);

            LOG(INFO,"Init success.");

            pthread_t tid;
            for(int i = 0;i < max_num;i++){
                if(pthread_create(&tid,nullptr,Routine,this) != 0){
                    LOG(FATAL,"ThreadPool Create Error.");
                    return false;
                }
                else{
                    LOG(INFO,"Thread Create Success.");
                }
            }

            return true;
        }

    public:
        
        static ThreadPool* GetInstance(){
            static pthread_mutex_t instance_mtx = PTHREAD_MUTEX_INITIALIZER; 

            if(instance == nullptr){
                pthread_mutex_lock(&instance_mtx);
                if(instance == nullptr){
                    instance = new ThreadPool();
                    instance->ThreadPoolInit();
                }
                pthread_mutex_unlock(&instance_mtx);
            }
            return instance;
        }

        void Push(Task* t){
            pthread_mutex_lock(&mtx);
            task_q.push(t);
            pthread_mutex_unlock(&mtx);

            //有任务后唤醒一个线程处理
            pthread_cond_signal(&cod);
        }

        ~ThreadPool(){
            pthread_mutex_destroy(&mtx);
            pthread_cond_destroy(&cod);
        }
};

ThreadPool* ThreadPool::instance = nullptr;




