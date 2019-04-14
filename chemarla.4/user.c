
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "p4Header.h"

#define BILLION  1e9
memTime checkSharedMemory();

int main(int argc, char **argv) {

    int* sharedInt;
    char* outputFileName;
    memTime currentTime;
    int childPid, msgId;

    childPid = atoi(argv[1]);
    msgId = atoi(argv[2]);


    currentTime = checkSharedMemory();


    printf("Child PID: %d got msgID: %d\n", childPid, msgId);
    printf(" user: %d:%d\n", currentTime.seconds, currentTime.nseconds);


    exit(0);
}

memTime checkSharedMemory(){
    int* sharedInt;
    memTime currentTime;
    unsigned int nextNano;
    unsigned int nextSeconds;
    int shmid = shmget(clockKey, sizeof(memTime), IPC_CREAT | 0666); /* return value from shmget() */

    if (shmid < 0) {
        perror("\n./user: shmid error: ");
        exit(-1);
    }
    sharedInt = shmat(shmid, NULL, 0);      //assign shared memory to sharedInt pointer
    if (*sharedInt == -1) {
        perror("./user: shmat error: ");
        exit(-1);
    }

    //initialize seconds and nanoseconds in shared memory
    nextSeconds = *(sharedInt+0);   //seconds
    nextNano = *(sharedInt+1);   //nanoseconds
    //check if nanoseconds reached over billion
    if (nextNano > BILLION){
        nextSeconds++;          // increment seconds
        nextNano =  (nextNano - BILLION);   //update nanoseconds
    }
    // update shareInt pointer in shared memory with changes
    *(sharedInt+0) = nextSeconds;
    *(sharedInt+1) = nextNano;

    currentTime.seconds = nextSeconds;
    currentTime.nseconds = nextNano;

    shmdt((void *)sharedInt);

    return currentTime;

}