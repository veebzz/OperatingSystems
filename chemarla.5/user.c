
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include "p5Header.h"
#include "msg.h"
#include <time.h>

#define BILLION  1e9
#define NANO(t) (t*1000*1000)

memTime checkSharedMemory(sem_t *sem);

memTime getTimeElapsed(memTime startTime, memTime currentTime);

memTime buildElapsedTime(memTime current, long elapsed);

memTime getNextTime(memTime currentTime, int bound);
bool isElapsed(memTime currentTime, memTime boundTime);

void nsleep(int nano);

resourceDescriptor *getResourceDescriptor();
bool shouldTerminate();




int main(int argc, char **argv) {
    int *sharedInt;
    char *outputFileName;
    memTime currentTime;
    memTime startTime;
    memTime nextTime, terminateTime;
    int simPid;
    const char *semName = "/sem_Chem";
    sem_t *sem;
    long boundTime;
    bool acquire;
    int resourceId;
    bool heldResources[10];
    int i;
    msgStruct recvMsg;
    resourceDescriptor *rdp;
    msgStruct sendMsg;
    bool shouldWaitForMsg = false;

    srand(time(0));
    simPid = atoi(argv[1]);
    boundTime = atoi(argv[2]);
    printf("P%d bound time :%ld\n", simPid, boundTime);

    //CLOCK SEMAPHORE
    sem = sem_open(semName, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("./master: sem_open error: ");
        exit(-1);
    }
    // get current time
    currentTime = checkSharedMemory(sem);
    startTime = currentTime;
    terminateTime = currentTime;
    terminateTime.seconds += 1;
    rdp = getResourceDescriptor();

    printf("[%d] accessed clock at %d:%d\n", simPid, currentTime.seconds, currentTime.nseconds);
    memTime requestTick = buildElapsedTime(currentTime, rand() % boundTime + 1);
    memTime terminateTick = buildElapsedTime(currentTime, BILLION + NANO(250));

    // keep track of all the resources that are held, -1 means not holding currently
    for ( i = 0; i < 10; i++) {
        heldResources[i] = false;
    }

    while (true) {
        //atleast 1 second and
        if (isElapsed(startTime, terminateTick)) {
            if (shouldTerminate()) {
                printf("P%d terminate initiated\n", simPid);
                break;
            }
            startTime = currentTime;
            terminateTick = buildElapsedTime(currentTime, NANO(250));
        }
        if (!isElapsed(currentTime, requestTick)) {
            continue;
        }
        acquire = rand() & 1; // 50/50 chance to acquire or release a resource
        if (acquire) {
            resourceId = getNextAcquireResourceId(heldResources);
            rdp[resourceId].request[simPid]++;
            printf("P%d sending mesage requesting from oss for resource %d\n", simPid, recvMsg.resourceId);


            // send request to oss for grant
            sendMsg.type = 0;
            sendMsg.resourceId = resourceId;
            sendMsg.userProcessId = simPid;
            sendMessage(createMsgKey(0), sendMsg);
            shouldWaitForMsg = true;
        } else {
            resourceId = getNextReleaseResourceId();
            heldResources[resourceId] = false;
            rdp[resourceId].released[simPid]++;
            printf("P%d sending message about release of resource %d\n", simPid, recvMsg.resourceId);
            sendMsg.type = 2;
            sendMsg.resourceId = resourceId;
            sendMsg.userProcessId = simPid;

            sendMessage(createMsgKey(0), sendMsg);
            shouldWaitForMsg = false;
        }
        if (shouldWaitForMsg) {
            recvMsg = recieveMessage(createMsgKey(0));
            if (recvMsg.type == 1) { // permission granted
                printf("P%d Received permission from oss for resource %d\n", simPid, recvMsg.resourceId);
            } else if (recvMsg.type == 3){
                printf("P%d Received termination from oss\n", simPid);
                break;
            }
        }
        nsleep(25);
        currentTime = checkSharedMemory(sem);
        requestTick = buildElapsedTime(currentTime, rand() % boundTime + 1);
    }

    // send message of exit
    printf("P%d sending message about termination\n", simPid);
    sendMsg.type = 3;
    sendMsg.resourceId = 0;
    sendMsg.userProcessId = simPid;
    sendMessage(createMsgKey(0), sendMsg);

    //release
    sem_post(sem);

    exit(0);
}

memTime buildElapsedTime(memTime current, long elapsed){
    memTime future;
    future.seconds = current.seconds;
    future.nseconds = current.nseconds + elapsed;

    if (elapsed > BILLION) {
        elapsed = BILLION -1;
    }

    if (future.nseconds > BILLION){
        future.seconds++;
        future.nseconds -= BILLION;
    }
    return future;
}

void nsleep(int nano) {
    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = nano;

    nanosleep(&req, NULL);
}

resourceDescriptor *getResourceDescriptor() {
    resId = shmget(resKey, 20 * sizeof(resourceDescriptor), IPC_CREAT | 0666);
    if (resId < 0) {
        perror("./oss: resId error: ");
        clearSharedMemory();
        exit(1);
    }
    int *rdp;
    int i, j;
    //attach resource descriptor memory
    rdp = shmat(resId, NULL, 0);

    if (rdp == -1) {
        perror("./oss: rdp error: ");
        clearSharedMemory();
        exit(1);
    }

    return rdp;
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

bool shouldTerminate() {
    bool terminate = rand() & 1;
    return terminate;
}

bool isElapsed(memTime currentTime, memTime boundTime) {

    printf("time: %d:%d\n", currentTime.seconds, currentTime.nseconds);
    printf("bound time: %d:%d\n", boundTime.seconds, boundTime.nseconds);

    return currentTime.seconds > boundTime.seconds  ||
           (boundTime.seconds == currentTime.seconds &&  currentTime.nseconds > boundTime.nseconds);
}

memTime getTimeElapsed(memTime startTime, memTime currentTime) {
    memTime tempTime;
    tempTime.seconds = currentTime.seconds - startTime.seconds;
    tempTime.nseconds = currentTime.nseconds - startTime.nseconds;

    if (tempTime.nseconds < 0) {
        tempTime.seconds -= 1;
        tempTime.nseconds += BILLION;
    }
    return tempTime;

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

