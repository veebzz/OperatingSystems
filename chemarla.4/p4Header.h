#ifndef OPERATINGSYSTEMSP4_P4HEADER_H
#define OPERATINGSYSTEMSP4_P4HEADER_H
#include <sys/types.h>

struct checkTime{
    int seconds;
    int nseconds;
};

struct PCB{
    struct checkTime totalCpuTime;
    struct checkTime totalTimeInSystem;
    struct checkTime elapsedBurstTime;
    int processClass;
    pid_t pid;
    int simPid;
    int priority;
};

struct sharedResources{
    struct PCB controlTable[18];
    struct checkTime time;
};
#endif //OPERATINGSYSTEMSP4_P4HEADER_H
