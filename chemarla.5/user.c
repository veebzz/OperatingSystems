
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include "p5Header.h"
#include "msg.h"

#define BILLION  1e9

memTime checkSharedMemory(sem_t *sem);

message recieveMessage(int simPid);

memTime getTimeElapsed(memTime startTime, memTime currentTime);

memTime getNextTime(memTime currentTime, int bound);


int main(int argc, char **argv) {
    int *sharedInt;
    char *outputFileName;
    memTime currentTime;
    memTime startTime;
    memTime nextTime;
    int simPid;
    const char *semName = "/sem_Chem";
    sem_t *sem;
    long boundTime;
    bool acquire;
    int resourceId;
    bool heldResources[10];
    int i;
    msgStruct message;

    timeElapsed.nseconds = 0;
    timeElapsed.seconds = 0;

    simPid = atoi(argv[1]);
    boundTime = atoi(argv[2]);
    printf("bound time :%ld\n", boundTime);

    //CLOCK SEMAPHORE
    sem = sem_open(semName, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("./master: sem_open error: ");
        exit(-1);
    }
    // get current time
    currentTime = checkSharedMemory(sem);
    startTime = currentTime;

    printf("[%d] accessed clock at %d:%d\n", simPid, currentTime.seconds, currentTime.nseconds);
    long requestTick = (rand() % (boundTime + 1);

    // keep track of all the resources that are held, -1 means not holding currently
    for (int i = 0; i < 10; i++) {
        heldResources[i] = false;
    }

    while (true) {
        if (!isElapsed(currentTime, requestTick)) {
            continue;
        }
        acquire = rand() & 1; // 50/50 chance to acquire or release a resource
        if (acquire) {
            resourceId = getNextAcquireResourceId(heldResources);
        } else {
            resourceId = getNextReleaseResourceId();
            heldResources[resourceId] = false;

            message.type = 2;
            message.resourceId = resourceId;
            message.userProcessId = simPid;

            sendMessage(createMsgKey(simPid), );
        }
        currentTime = checkSharedMemory(sem);
        requestTick = (rand() % (boundTime + 1);
    }








    //release
    sem_post(sem);

    exit(0);
}

int getNextAcquireResourceId(bool held[]) {
    int resourceId = (rand() % 10);
    while (true) {
        if (held[resourceId]) {
            resourceId = (rand() % 10);
            continue;
        }
        break;
    }
    return resourceId;
}

int getNextReleaseResourceId(bool held[]) {
    int resourceId = (rand() % 10);
    while (true) {
        if (!held[resourceId]) {
            resourceId = (rand() % 10);
            continue;
        }
        break;
    }
    return resourceId;
}

bool isElapsed(memTime currentTime, int bound) {
    memTime tempTime = currentTime;

    tempTime.nseconds += bound;

    if (tempTime.nseconds > BILLION) {
        tempTime.seconds += 1;
        tempTime.nseconds -= BILLION;
    }
    return tempTime.seconds > currentTime.seconds ||
           (tempTime.seconds == currentTime.seconds && tempTime.nseconds > currentTime.nseconds);
}

memTime getTimeElapsed(memTime startTime, memTime currentTime) {
    memTime tempTime;
    tempTime.seconds = currentTime.seconds - startTime.seconds;
    tempTime.nseconds = currentTime.nseconds - startTime.nseconds;

    if (tempTime.nseconds < 0) {
        tempTime.seconds -= 1;
        tempTime.nseconds += BILLION;
    }


}

memTime checkSharedMemory(sem_t *sem) {
    int *sharedInt;
    memTime currentTime;
    unsigned int nextNano;
    unsigned int nextSeconds;


    //wait here to access time
    sem_wait(sem);

    int shmid = shmget(clockKey, 2 * sizeof(unsigned int), IPC_CREAT | 0666); /* return value from shmget() */

    if (shmid < 0) {
        perror("\n./user: shmid error: ");
        clearSharedMemory();
        exit(-1);
    }
    sharedInt = shmat(shmid, NULL, 0);      //assign shared memory to sharedInt pointer
    if (*sharedInt == -1) {
        perror("./user: shmat error: ");
        clearSharedMemory();
        exit(-1);
    }

    //initialize seconds and nanoseconds in shared memory
    nextSeconds = *(sharedInt + 0);   //seconds
    nextNano = *(sharedInt + 1);   //nanoseconds
    //check if nanoseconds reached over billion
    if (nextNano > BILLION) {
        nextSeconds++;          // increment seconds
        nextNano = (nextNano - BILLION);   //update nanoseconds
    }
    // update shareInt pointer in shared memory with changes
    *(sharedInt + 0) = nextSeconds;
    *(sharedInt + 1) = nextNano;

    currentTime.seconds = nextSeconds;
    currentTime.nseconds = nextNano;

    shmdt((void *) sharedInt);

    return currentTime;

}

