#ifndef OPERATINGSYSTEMSP4_P4HEADER_H
#define OPERATINGSYSTEMSP4_P4HEADER_H
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//globals
key_t clockKey = 382910;
key_t pcbKey = 102938;

struct checkTime{
    unsigned int seconds;
    unsigned int nseconds;
};

typedef struct PCB{
    struct checkTime totalCpuTime;
    struct checkTime totalTimeInSystem;
    struct checkTime elapsedBurstTime;
    int processClass;
    pid_t pid;
    int simPid;
    int priority;
}pcbStruct;

struct sharedResources{
    struct PCB controlTable[18];
    struct checkTime time;
};

typedef struct msgQ{
    long msgType;
    char msgText[100];
} message;
#endif //OPERATINGSYSTEMSP4_P4HEADER_H
