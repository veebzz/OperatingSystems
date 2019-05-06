
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include "oss.h"

#define BILLION  1e9
#define NANO(t) (t*1000*1000)

memTime checkSharedMemory();




int main(int argc, char **argv) {
    int *sharedInt;
    memTime currentTime, randTime;
    int simPid;
    const char *semName = "/sem_Chem";

    simPid = atoi(argv[1]);
    currentTime = checkSharedMemory();

    printf("IN Child %d \n", simPid);
    randTime.seconds = currentTime.seconds + 3;
    randTime.nseconds = currentTime.nseconds;
    while(true){
        currentTime = checkSharedMemory();

        if((currentTime.seconds * BILLION) + currentTime.nseconds >= (randTime.seconds * BILLION) + randTime.nseconds){
            break;
        }
    }
    printf("Child %d  Exiting\n", simPid);


    return 0;


}

memTime checkSharedMemory() {
    int *sharedInt;
    memTime currentTime;
    unsigned int nextNano;
    unsigned int nextSeconds;




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

