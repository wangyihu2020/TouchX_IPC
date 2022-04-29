//
// Created by wangyihu2020 on 2022/4/28.
//
#include "src/Device.h"
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#define SHM_BUFF  50
void terminate(){
    key_t key = ftok("/usr/bin", 2022);
    int shmid = shmget(key, SHM_BUFF, IPC_CREAT|0666);
    char* shmadd = (char*)shmat(shmid, 0, 0);
    shmctl(shmid, IPC_RMID, NULL);
}

int main(){
    Device a ;
    a.initDevice();
    a.createShm();
    atexit(terminate);
    while(1){
        a.m_ShmData->write();
    }
    // release
//    std::string b = a.getFromDevice();

}
