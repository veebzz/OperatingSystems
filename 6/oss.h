

#ifndef OPERATINGSYSTEMSP6_OSS_H
#define OPERATINGSYSTEMSP6_OSS_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdbool.h>
#include <sys/msg.h>
#include <semaphore.h>

#define MAXUSERPROCESSES 18

//globals
key_t clockKey = 382910;
key_t msgKey = 000111;

int clockShmId;
int msgId;

//32k total, page =1k each, up to 32 pages per process
typedef struct pageStruct {
    int pages[32];
} pageStruct;

typedef struct frameStruct {
    int pid;
    int referenceByte;
    int dirtyBit;
} frameStruct;

struct msgQ {
    long type;
    char referenceNumber[100];
    frameStruct frame;
} message;

struct msgQ2{
    long type;
    int response;
};
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

void setupInitial(memTime randTime, bool pidArray[], int maxActiveChildren, frameStruct frameTable[], pageStruct pageTable[]);

int checkForTerminatedChildren(int *array, bool simArray[], int maxActiveChildren);

bool shouldExit(int *array, memTime currentTime);

static void interruptHandler();


void clearSharedMemory() {
    msgctl(msgId, IPC_RMID, NULL);
    shmctl(clockShmId, IPC_RMID, NULL);
}

#endif //OPERATINGSYSTEMSP6_OSS_H
