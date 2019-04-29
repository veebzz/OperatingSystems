#ifndef OPERATINGSYSTEMSP5_P5HEADER_H
#define OPERATINGSYSTEMSP5_P5HEADER_H
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdbool.h>
#include <sys/msg.h>
#include <semaphore.h>

#define NUM_USER_PROCESSES 1
//globals
key_t clockKey = 382910;
key_t pcbKey = 102938;
key_t resKey = 111000;
int msgKey = 000111;

int clockShmId;
int pcbShmId;
int resId;
int msgId;

void clearSharedMemory();

typedef struct processStruct {
    int simPid;
    pid_t pid;
} processStruct;

typedef struct memTime{
    unsigned int seconds;
    unsigned int nseconds;
}memTime;

typedef struct resourceDescriptor{
    bool shared;
    int resourceId;
    bool inUse;
    int currentOwner;
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
