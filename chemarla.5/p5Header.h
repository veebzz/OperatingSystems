#ifndef OPERATINGSYSTEMSP5_P5HEADER_H
#define OPERATINGSYSTEMSP5_P5HEADER_H
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


//globals
key_t clockKey = 382910;
key_t pcbKey = 102938;
key_t msgKey = 291038;
int clockShmId;
int pcbShmId;
int msgId;

void clearSharedMemory();

typedef struct memTime{
    unsigned int seconds;
    unsigned int nseconds;
}memTime;

typedef struct PCB{
    memTime totalCpuTime;
    memTime totalTimeInSystem;
    memTime elapsedBurstTime;
    int simPid;
    int priority;
}pcbStruct;

struct sharedResources{
    struct PCB controlTable[18];
    memTime time;
};

typedef struct msgQ{
    long msgType;
    int timeSlice;
}message;


void clearSharedMemory(){
    shmctl(clockShmId, IPC_RMID, NULL);
    shmctl(pcbShmId, IPC_RMID, NULL);
    msgctl(msgId, IPC_RMID, NULL);
}
#endif //OPERATINGSYSTEMSP5_P5HEADER_H
