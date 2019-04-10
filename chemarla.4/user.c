//
// Created by veebz on 3/2/2019.
//
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "p4Header.h"

#define BILLION  1e9

struct checkTime checkSharedMemory();

int main(int argc, char **argv) {
    long duration;
    long elapsed;
    int* sharedInt;
    char* outputFileName;
    struct checkTime currentTime;

    duration = atoi(argv[1]); // get the passed duration
    currentTime = checkSharedMemory();

    //add current simulated clock to duration
    duration += ((currentTime.seconds * BILLION) + currentTime.nseconds);

    while(1) {
        //update timePtr every while iteration
        currentTime = checkSharedMemory();

        elapsed = (currentTime.seconds * BILLION) + currentTime.nseconds;
        if (elapsed > duration) {
            printf("USER:[%d] terminated at time %d s:%d ns\n", getpid(), currentTime.seconds, currentTime.nseconds);
            break;
        }
    }
    exit(0);
}

struct checkTime checkSharedMemory(){
    key_t key = 102938;
    int* sharedInt;
    struct checkTime currentTime;
    int shmid = shmget(key, NULL, 0); /* return value from shmget() */

    if (shmid < 0) {
        perror("./oss: shmid error: ");
        exit(-1);
    }
    sharedInt = (int *) shmat(shmid, NULL, 0);      //assign shared memory to sharedInt pointer
    if (*sharedInt == -1) {
        perror("./user: shmat error: ");
        exit(-1);
    }

    currentTime.seconds = *(sharedInt+0);
    currentTime.nseconds = *(sharedInt+1);

    shmdt((void *)sharedInt);

    return currentTime;

}