#ifndef OPERATINGSYSTEMSP5_P5HEADER_H
#define OPERATINGSYSTEMSP5_P5HEADER_H
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdbool.h>
#include <sys/msg.h>

#define NUM_USER_PROCESSES 18
//globals
key_t clockKey = 382910;
key_t pcbKey = 102938;
key_t resKey = 111000;

int clockShmId;
int pcbShmId;
int resId;

void clearSharedMemory();


typedef struct memTime{
    unsigned int seconds;
    unsigned int nseconds;
}memTime;

typedef struct resourceDescriptor{
    bool shared;
    sem_t* sem;
    int resourceId;
    int request[NUM_USER_PROCESSES];
    int allocated[NUM_USER_PROCESSES];
    int released[NUM_USER_PROCESSES];
}resourceDescriptor;

typedef struct msgQ{
    long msgType;
    int timeSlice;
}message;


void clearSharedMemory(){
    shmctl(clockShmId, IPC_RMID, NULL);
    shmctl(resId, IPC_RMID,NULL);
}
#endif //OPERATINGSYSTEMSP5_P5HEADER_H
