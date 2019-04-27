/*
Name: Vibhav Chemarla
Date: 4/23/2019
CS4760 OS Project
Resource Management
 */
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <math.h>
#include "p5Header.h"
#include <semaphore.h>
#include <fcntl.h>
#include "oss.h"

#define BILLION  1e9
#define MILLION  1e6
#define MAXTIMEBETWEENNEWPROCSSECS 1
#define MAXTIMEBETWEENNEWPROCSNSECS 500000000
#define BASETIMEQUANTUM 10000


FILE *out_file;

int startSharedMemory();
memTime incrementSharedMemory(int value);
bool shouldCreateChild(int maxForks, int activatedChildren, memTime currentTime, memTime nextProcessTime, int simPidArray[]);
pid_t forkChild(int simPid);
int getOpenSimPid(int *simPidArray, int maxForks);
bool shouldExit(int activated, int maxForks, memTime currentTime, int active);
int dispatchMessage(int simPid, int priority, memTime currentTime, int);
int checkForTerminatedChildren(pid_t *array, int activated, memTime currentTime);
static void interruptHandler();
memTime getNextProcessSpawnTime(memTime currentTime);
pcbStruct getPCB(memTime currentTime, int simPid);
memTime *sharedClockPtr;


int main(int argc, char **argv) {
    int optionIndex, maxForks = 18, activeChildren = 0;
    char *outputFileName = "log.txt";
    opterr = 0;
    int status = 0, i;
    int incrementValue = 10000;
    int openPid;
    const char* semName = "/sem_Chem";
    sem_t *sem;


    pid_t child_pid, wait_pid;
    //check if arguments are given
    while ((optionIndex = getopt(argc, argv, "h:o:n:")) != -1) {
        switch (optionIndex) {
            case 'h':
                printf("List of valid command line argument usage\n");
                printf("-h	      :  Lists all possible command line arguments, their explanation,and usage.\n");
                printf("\t\t If no argument is provided, (input.dat)will be used as\n\t\t a default argument.\n");
                printf("-o <argument> :  This option will output data from the program to the given file name\n");
                printf("\t\t argument. If no argument is provided, (output.txt) will be used as a\n\t\t default argument.\n");
                printf("-n <argument> : This option takes an positive integer argument, which specifies the maximum number of fork processes\n");
                printf("\t\t allowed in the program.");
                printf("-s <argument> : This option takes an positive integer argument, which specifies the maximum number of active fork processes\n");
                printf("\t\t allowed in the program. Maximum active allowed is 20, if argument is above 20, it will default to 20.\n");
                break;

            case 'o':
                //set output file to output.txt
                // if no argument given
                if (optarg == NULL) {
                    optarg = "output.txt";
                }
                outputFileName = optarg;

                printf("Output File: %s\n", outputFileName);

                break;

            case 'n':
                maxForks = atoi(optarg);
                printf("Max number of forks is %d\n", maxForks);

                if (maxForks > 18) {
                    printf("Max processes argument cannot be over 18. Default of 18 is set.");
                    maxForks = 18;
                }
                break;


            default:
                printf("%s: Argument Error: Incorrect argument usage.\n", argv[0]);
                printf("Use '-h' argument for valid usage instructions.\n\n Example: ./oss -h\n\n");
                exit(0);
        }
    }


    //***************
    out_file = fopen(outputFileName, "a+");
    if (out_file == NULL) {
        perror("./oss: File Error: ");
        return 1;
    }
    int *childPidArray = malloc(maxForks * sizeof(int));
    int simPidArray[maxForks];
    for (i = 0; i < maxForks; i++) {
        //set all elements to 1 to indicate availability
        simPidArray[i] = 1;
    }
    int activatedChildren = 0;
    memTime currentTime;
    //get msgid from starting shared memory segment
    msgId = startSharedMemory();
    //create named semaphore
    sem = sem_open(semName, O_CREAT, 0666, 1);
    if(sem == SEM_FAILED){
        perror("./master: sem_open error: ");
        exit(-1);
    }
    //create randTime  for random process spawn
    memTime randTime;
    randTime.seconds = 0;
    randTime.nseconds = 0;
//    //creating queues
//    qStruct *roundRobinQueue = createQueue(maxForks);
//    qStruct *MLFQueue = createQueue(maxForks);

    //Signal
    signal(SIGALRM, interruptHandler);
    signal(SIGINT, interruptHandler);//ctrl-c interrupt
    alarm(100);

    //oss parent control
    while (true) {
        //increment time
        currentTime = incrementSharedMemory(incrementValue);
        //check if a child should be spawned
        if (shouldCreateChild(maxForks, activatedChildren, currentTime, randTime, simPidArray)) {
            //get open an open pid
            openPid = getOpenSimPid(simPidArray, maxForks);
            simPidArray[openPid] = 0;

            //fork a child
            child_pid = forkChild(openPid);
//            printf("Child[%d] started at time %d s:%d ns\n", child_pid, currentTime.seconds, currentTime.nseconds);
            //add spawned child pid to childPidArray
            *(childPidArray + activatedChildren) = child_pid;
            //increment activatedChildren to keep track of how many children spawned
            activatedChildren++;
            randTime = getNextProcessSpawnTime(currentTime);
        }

        // check for terminated/get update for active children
        activeChildren = checkForTerminatedChildren(childPidArray, activatedChildren, currentTime);



        if (shouldExit(activatedChildren, maxForks, currentTime, activeChildren)) {
            sem_close(sem);
            if(sem_unlink(semName) == -1){
                perror("./master: sem_unlink error: ");
                exit(-1);
            }
            printf("Exit condition met\n");
            break;
        }

    }

    // release shared memory and file
    shmctl(clockShmId, IPC_RMID, NULL);
    msgctl(msgId, IPC_RMID, NULL);
    fclose(out_file);
    free(childPidArray);
    printf("Parent[%d] Finished\n", getpid());
    return 0;

}


bool shouldCreateChild(int maxForks, int activatedChildren, memTime currentTime, memTime nextProcessTime, int simPidArray[]) {

    int i, isOpen;

    for (i = 0; i < maxForks; i++) {
        if (simPidArray[i] == 1) {
            isOpen = i;
            break;
        } else {
            isOpen = -1;
        }
    }
    if (isOpen == -1) {
        return false;
    }

    if (((currentTime.seconds * BILLION) + currentTime.nseconds) <
        ((nextProcessTime.seconds * BILLION) + nextProcessTime.nseconds)) {
        return false;
    }
    //check if maxForks reached
    if (activatedChildren >= maxForks) {
        return false;
    }

    return true;
}

int checkForTerminatedChildren(pid_t *array, int activated, memTime currentTime) {
    int i = 0;
    int status;
    pid_t checkId;
    int active = 0;
    // check through the activated children
    for (i = 0; i < activated; i++) {
        //only look at childPidArray elements with previously known active children(only elements with PID)
        if (*(array + i) > 0) {
            //if checkID = 0 its still running, if -1 error, if checkID = pid it is dead
            checkId = waitpid(*(array + i), &status, WNOHANG);
            //if it is dead, mark them as -1 to not check this anymore
            if (checkId == *(array + i)) {
                fprintf(out_file, "Child[%d] terminated at %d:%d\n", *(array + i), currentTime.seconds,
                        currentTime.nseconds);
                *(array + i) = -1;
            } else {
                active++;
            }
        }
    }
    //return current active processes
    return active;
}

bool shouldExit(int activated, int maxForks, memTime currentTime, int active) {
    if(active > 0){
        return false;
    }

    if(activated > 20){
        return true;
    }


    //if maxforks have not reach end yet
    if (activated < maxForks) {
        return false;
    }
    printf("asking to exit\n");
    return true;
}

pid_t forkChild(int simPid) {
    printf("in forkChild\n");
    pid_t child_pid = fork();
    char simPidStr[10], msgIdStr[10];
    //turn simulated pid, and message ID into strings
    sprintf(simPidStr, "%d", simPid);
//    sprintf(msgIdStr, "%d", msgId);
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

//memory allocation for clock, pcbtable, and message queue
int startSharedMemory(memTime *sharedClockPtr, pcbStruct *pcbStructTable) {

    //store clock id from shmget
    clockShmId = shmget(clockKey, 2* sizeof(unsigned int), IPC_CREAT | 0666);
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
    (*sharedClockPtr).seconds = 0;
    (*sharedClockPtr).nseconds = 0;
    //detach
    shmdt(sharedClockPtr);
    //message Queue shared memory allocation (GfG)
    int msgId = msgget(msgKey, 0666 | IPC_CREAT);
    if (msgId < 0) {
        perror("./oss: msgget error: ");
        clearSharedMemory();
        exit(1);
    }
    return msgId;
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


int dispatchMessage(int simPid, int priority, memTime currentTime, int msgId) {
    message msg;
    double result;
    //create message with unique type(id), and its timeslice based on priority

    msg.msgType = simPid + 1;
    result = BASETIMEQUANTUM * pow(2.0, priority);
    msg.timeSlice = result;
    printf("[%d] sent message: %d\n", msg.msgType, msg.timeSlice);
    //send the message
    if (msgsnd(msgId, &msg, sizeof(msg.timeSlice), 0) < 0) {
        perror("./oss: msgsnd error:");
        msgctl(msgId, IPC_RMID, NULL);
        clearSharedMemory();
        exit(-1);
    }

    return 0;

}
static void interruptHandler(){
    key_t clockKey = 102938;
    key_t pcbKey = 382910;
    key_t msgKey = 291038;

    int clockId = shmget(clockKey, sizeof(memTime), IPC_CREAT | 0666);
    int pcbId = shmget(pcbKey, sizeof(pcbStruct), IPC_CREAT | 0666);
    int msgId = msgget(msgKey, 0666 | IPC_CREAT);
    fclose(out_file);
    const char * semName = "/sem_Chem";
    sem_unlink(semName);

    shmctl(clockId, IPC_RMID, NULL); //delete shared memory
    msgctl(msgId, IPC_RMID, NULL);
    kill(0, SIGKILL); // kill child process
    exit(0);
}

//get the next open simulated pid
int getOpenSimPid(int *simPidArray, int maxForks) {
    int i;
    for (i = 0; i < maxForks; i++) {
        if (simPidArray[i] == 1) {
            return i;
        }
    }
    printf("all taken\n");
    return -1;
}

memTime getNextProcessSpawnTime(memTime currentTime) {
    memTime randomInterval;
    srand(time(NULL));
    randomInterval.seconds = (rand() % MAXTIMEBETWEENNEWPROCSSECS + 1) + (currentTime.seconds);
    randomInterval.nseconds = (rand() % MAXTIMEBETWEENNEWPROCSNSECS + 1) + (currentTime.nseconds);
    return randomInterval;
}

pcbStruct getPCB(memTime currentTime, int simPid) {
    pcbStruct pcb;
    int priority;
    pcb.totalCpuTime.nseconds = 0;
    pcb.totalCpuTime.seconds = 0;
    pcb.totalTimeInSystem.nseconds = 0;
    pcb.totalTimeInSystem.seconds = 0;
    pcb.elapsedBurstTime.nseconds = 0;
    pcb.elapsedBurstTime.seconds = 0;
    pcb.simPid = simPid;

    priority = rand() % 100 + 1;
    //heavily weighted towards priority 1( only 1% chance to get real time class)
    if (priority < 25) {
        //real time class
        pcb.priority = 0;
    } else {
        //user process class
        pcb.priority = 1;
    }

    return pcb;
}
