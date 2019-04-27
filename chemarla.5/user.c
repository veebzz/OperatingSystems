
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include "p5Header.h"

#define BILLION  1e9
memTime checkSharedMemory(sem_t* sem);
message recieveMessage(int simPid);

int main(int argc, char **argv){
    int* sharedInt;
    char* outputFileName;
    memTime currentTime;
    int simPid;
    const char *semName = "/sem_Chem";
    sem_t* sem;


    simPid = atoi(argv[1]);
//    msgId = atoi(argv[2]);

    sem = sem_open(semName, O_CREAT, 0666, 1);
    if(sem == SEM_FAILED){
        perror("./master: sem_open error: ");
        exit(-1);
    }

    currentTime = checkSharedMemory(sem);
    printf("[%d] accessed clock at %d:%d\n", simPid, currentTime.seconds, currentTime.nseconds);
    //release
    sem_post(sem);

    exit(0);
}

memTime checkSharedMemory(sem_t* sem){
    int* sharedInt;
    memTime currentTime;
    unsigned int nextNano;
    unsigned int nextSeconds;


    //wait here to access time
    sem_wait(sem);
    sleep(2);

    int shmid = shmget(clockKey, 2 *sizeof(unsigned int), IPC_CREAT | 0666); /* return value from shmget() */

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

message recieveMessage(int simPid){
    message msg;
    int msgId = msgget(msgKey, 0666 | IPC_CREAT);
    if (msgId < 0) {
        perror("./user: msgget error: ");
        clearSharedMemory();

        exit(-1);
    }

    if (msgrcv(msgId, &msg, sizeof(msg.timeSlice), (simPid + 1) , 0) < -1) {
        perror("./user: msgrcv error");
        clearSharedMemory();

        exit(-1);
    }

    return msg;
}