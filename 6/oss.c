/*
Name: Vibhav Chemarla
Date: 5/02/2019
CS4760 OS Project
Memory Management
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <math.h>
#include <semaphore.h>
#include <fcntl.h>
#include "oss.h"


#define BILLION  1e9
#define MILLION  1e6
#define MAXTIMEBETWEENNEWPROCSSECS 1
#define MAXTIMEBETWEENNEWPROCSNSECS 500000000
#define BASETIMEQUANTUM 10000


FILE *out_file;



int main(int argc, char **argv) {
    int optionIndex, activeChildren = 0, maxActiveChildren = 5;
    char *outputFileName = "log.txt";
    opterr = 0;
    int status = 0, i;
    int incrementValue = 100000;
    int openPid;
    pid_t child_pid, wait_pid;
    memTime currentTime, randTime;
    bool pidArray[maxActiveChildren]; // holds simulated pids


    //check if arguments are given
    while ((optionIndex = getopt(argc, argv, "h:o:n:")) != -1) {
      argHandler(optionIndex, outputFileName, maxActiveChildren);
    }
    int *childPidArray = malloc(maxActiveChildren * sizeof(int)); // holds real pids


    //Open output file
    out_file = fopen(outputFileName, "a+");
    if (out_file == NULL) {
        perror("./oss: File Error: ");
        return 1;
    }
    //start clock in shared mem
    currentTime = startSharedMemory();
    //set intial random spawn time to 0 amd set up array keep track of active processes
    setupInitial(randTime, pidArray, maxActiveChildren);

    //OSS control
    while(true){
        //increment time
        currentTime = incrementSharedMemory(incrementValue);
        //check if a child should be spawned
        if(shouldCreateChild(currentTime,randTime, maxActiveChildren, activeChildren)){
            activeChildren++;
            //update that active process is taken
            openPid = getOpenSimPid(maxActiveChildren, pidArray);
            pidArray[openPid] = false;
            printf("Forking Child %d at %d : %d\n", openPid, currentTime.seconds, currentTime.nseconds);
            //fork the child
            child_pid = forkChild(openPid);
            //store the process pid
            *(childPidArray + openPid) = child_pid;
            //get random spawn interval time
            randTime = getNextProcessSpawnTime(currentTime);
        }
        //check for terminated children
        activeChildren = checkForTerminatedChildren(childPidArray, pidArray, maxActiveChildren);

        if(shouldExit(childPidArray, currentTime)){
            break;
        }

    }

    clearSharedMemory();
    free(childPidArray);
    printf("Parent[%d] Finished\n", getpid());
    return 0;
}

//memory allocation for clock,and
memTime startSharedMemory() {
    memTime *sharedClockPtr;

    //store clock id from shmget
    clockShmId = shmget(clockKey, 2 * sizeof(unsigned int), IPC_CREAT | 0666);
    if (clockShmId < 0) {
        perror("./oss: clockShmId error: ");
        clearSharedMemory();
        exit(1);
    }
    //attach shared memory to clock pointer
    sharedClockPtr = shmat(clockShmId, NULL, 0);

    if (sharedClockPtr == -1) {
        perror("./oss: sharedClockPtr error: ");
        clearSharedMemory();
        exit(1);
    }
    //set starting clock to 0
    (*sharedClockPtr).seconds = 1;
    (*sharedClockPtr).nseconds = 0;
    //detach
    shmdt(sharedClockPtr);
}

memTime incrementSharedMemory(int value) {
    int *sharedInt;
    unsigned int nextNano;
    unsigned int nextSeconds;
    memTime currentTime;
    //get previously allocated shared memory's id
    int shmid = shmget(clockKey, sizeof(memTime), IPC_CREAT | 0666);

    if (shmid < 0) {
        perror("./user: shmid error: ");
        clearSharedMemory();
        exit(1);
    }
    //attach sharedInt pointer to shared memory
    sharedInt = shmat(shmid, NULL, 0);

    if (*sharedInt == -1) {
        perror("./oss: shmat error: ");
        clearSharedMemory();
        exit(1);
    }
    //initialize seconds and nanoseconds in shared memory
    nextSeconds = *(sharedInt + 0);   //seconds
    nextNano = *(sharedInt + 1) + value;   //nanoseconds
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


    shmdt(sharedInt); //detach shared memory

    return currentTime;
}

void argHandler(int optionIndex, char* outputFileName,int maxActiveChildren){
    switch (optionIndex) {
        case 'h':
            printf("List of valid command line argument usage\n");
            printf("-h	      :  Lists all possible command line arguments, their explanation,and usage.\n");
            printf("-o <argument> :  This option will output data from the program to the given file name\n");
            printf("\t\t argument. If no argument is provided, (output.txt) will be used as a\n\t\t default argument.\n");
            printf("-n <argument> : This option takes an positive integer argument, which specifies the maximum number of active fork processes\n");
            printf("\t\t allowed in the program. Maximum active processes allowed is 18, if argument is above 18, it will default to 18.\n");
            break;

        case 'o':
            //set output file to output.txt
            // if no argument given
            if (optarg == NULL) {
                optarg = "log.txt";
            }
            outputFileName = optarg;

            printf("Output File: %s\n", outputFileName);

            break;

        case 'n':
            maxActiveChildren = atoi(optarg);
            if (maxActiveChildren > 18) {
                printf("Max processes argument cannot be over 18. Default of 18 is set.");
                maxActiveChildren = 18;
            }
            printf("Concurrent active process chosen is %d\n", maxActiveChildren);
            break;

        default:
            printf("./oss: Argument Error: Incorrect argument usage.\n");
            printf("Use '-h' argument for valid usage instructions.\n\n Example: ./oss -h\n\n");
            exit(0);
    }
}

bool shouldCreateChild(memTime currentTime, memTime nextProcessTime, int maxActiveChildren, int activeChildren) {
    if((activeChildren < maxActiveChildren) && ((currentTime.seconds * BILLION) + currentTime.nseconds) >= ((nextProcessTime.seconds * BILLION) + nextProcessTime.nseconds)){
        if(getOpenSimPid < 0){
            return false;
        }
        return true;
    }

    return false;
}

memTime getNextProcessSpawnTime(memTime currentTime) {
    memTime randomInterval;
    srand(time(NULL));
    randomInterval.seconds = (currentTime.seconds);
    randomInterval.nseconds = (rand() % MAXTIMEBETWEENNEWPROCSNSECS + 1) + (currentTime.nseconds);
    return randomInterval;
}

int getOpenSimPid(int maxActiveChildren, bool pidArray[]) {
    int i;
    for (i = 0; i < maxActiveChildren; i++) {
        //if true return array element location
        if (pidArray[i]) {
            printf("[%d] is open\n", i);
            return i;
        }
    }
    return 0;
}

void setupInitial(memTime randTime, bool pidArray[], int maxActiveChildren){
    int i;

    for (i = 0; i < maxActiveChildren; i++) {
        //set all elements to true to indicate availability
        pidArray[i] = true;
    }

    randTime.seconds = 0;
    randTime.nseconds = 0;

}

pid_t forkChild(int openPid) {
    pid_t child_pid = fork();
    char simPidStr[10];
    //turn simulated pid, and message ID into strings
    sprintf(simPidStr, "%d", openPid);
    //fork fail check
    if (child_pid < 0) {
        perror("./oss : forkError");
        clearSharedMemory();

        exit(-1);
    } else if (child_pid == 0) {
        //i am child
        //exec arguments
        char *arguments[3] = {"./user", simPidStr, NULL};
        execvp("./user", arguments);
        exit(0);
    }
    return child_pid;
}
int checkForTerminatedChildren(int* array, bool simArray[], int maxActiveChildren) {
    int i = 0;
    int status;
    pid_t checkId;
    int active = 0;
    // check through the activated children
    for (i = 0; i < maxActiveChildren; i++) {
        //only look at childPidArray elements with previously known active children(only elements with PID)
        if (*(array + i) > 0) {
            //if checkID = 0 its still running, if -1 error, if checkID = pid it is dead
            checkId = waitpid(*(array + i), &status, WNOHANG);
            //if it is dead, mark them as -1 to not check this anymore
            if (checkId == *(array + i)) {
                *(array + i) = -1;
                printf("Process %d died, clearing...\n", i);
                simArray[i] = true;
            } else {
                active++;
            }
        }
    }
    //return current active processes
    return active;
}

bool shouldExit(int *array, memTime currentTime) {
    int i = 0;
//    for (i = 0; i < activated; i++) {
//        //if the child pid array has an active child
//        if (*(array + i) > 0) {
//            return false;
//        }
//    }
//
    if(currentTime.seconds >= 30){
        return true;
    }

    //exit requirement met
    return false;
}