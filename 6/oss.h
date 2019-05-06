

#ifndef OPERATINGSYSTEMSP6_OSS_H
#define OPERATINGSYSTEMSP6_OSS_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdbool.h>
#include <sys/msg.h>
#include <semaphore.h>

//globals
key_t clockKey = 382910;
key_t pcbKey = 102938;
key_t resKey = 111000;
int msgKey = 000111;

int clockShmId;
int pcbShmId;
int resId;
int msgId;

typedef struct memTime {
    unsigned int seconds;
    unsigned int nseconds;
} memTime;

void clearSharedMemory();

memTime incrementSharedMemory(int value);

void argHandler(int optionIndex, char *outputFileName, int maxActiveChildren);

memTime startSharedMemory();

memTime getNextProcessSpawnTime(memTime currentTime);

bool shouldCreateChild(memTime currentTime, memTime nextProcessTime, int maxActiveChildren, int activeChildren);

int getOpenSimPid(int maxActiveChildren, bool pidArray[]);

void setupInitial(memTime randTime, bool pidArray[], int maxActiveChildren);

int checkForTerminatedChildren(int *array, bool simArray[], int maxActiveChildren);

bool shouldExit(int *array, memTime currentTime);



void clearSharedMemory() {
    shmctl(clockShmId, IPC_RMID, NULL);
}

#endif //OPERATINGSYSTEMSP6_OSS_H
