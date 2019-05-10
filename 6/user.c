
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

int randTermination();
frameStruct request(int simPid);


int main(int argc, char **argv) {
    int *sharedInt;
    memTime currentTime, randTime;
    int simPid, referenceCounter, terminateCondition;

    srand(getpid());

    simPid = atoi(argv[1]);
    currentTime = checkSharedMemory();

    terminateCondition = randTermination();
    printf("%d IN Child %d \n", getpid(), simPid);
    randTime.seconds = currentTime.seconds + 3;
    randTime.nseconds = currentTime.nseconds;
    if ((msgId = msgget(msgKey, 0666 | IPC_CREAT)) == -1) {
        perror("./user: msgget Error: ");
        exit(1);
    }


    message.type = 1;
    sprintf(message.referenceNumber, "%d", simPid);


    while(true){
        //get up to date time
        currentTime = checkSharedMemory();
        //prepare a memory reference request
        message.frame = request(simPid);
        //count references
        referenceCounter++;
        //send message to oss
        if ((msgsnd(msgId, &message, sizeof(message), 0))== -1) {
            perror("./user: msgsnd error: ");
            exit(1);
        }

        //wait for oss to send page hit or page fault
        msgrcv(msgId, &message, sizeof(message), simPid, 0);
        //if memory reference 1000 +/- 100
        if(referenceCounter == terminateCondition){
            break;
        }
    }

//    //send message
//    if ((msgsnd(msgId, &message, sizeof(message), 0))== -1) {
//        perror("./user: msgsnd error: ");
//        exit(1);
//    }


    printf("Child %d  Exiting: sending %d\n", simPid, getpid());

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
        clearSharedMemory(msgId);
        exit(-1);
    }
    sharedInt = shmat(shmid, NULL, 0);      //assign shared memory to sharedInt pointer
    if (*sharedInt == -1) {
        perror("./user: shmat error: ");
        clearSharedMemory(msgId);
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

frameStruct request(int simPid){
    int readWrite;
    frameStruct frameInstance;
    srand(simPid + getpid());
    frameInstance.pid = simPid;
    //memory reference 0 to less than 32 k
    frameInstance.referenceByte = rand() % (32000-1);
    // READ or WRITE
    readWrite = rand() % 101; //0-100

    if(readWrite > 50){
        //Write
        frameInstance.dirtyBit = 1;
    }else if (readWrite <= 50){
        //Read
        frameInstance.dirtyBit = 0;
    }


}

int randTermination(){
    int number, sign;
    srand(getpid());
    sign = rand() % 1;
    number = rand() % 101;

    if(sign == 1){
       //plus
       number = 1000 + number;
    }else{
        number = 1000 - number;
    }

    return number;
}