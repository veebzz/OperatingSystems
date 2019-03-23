/*
Name: Vibhav Chemarla
Date: 3/19/2019
CS4760 OS Project 3
Semaphores and Operating System Simulator
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>


void postToSharedMemory(FILE *in_file, int maxForks, int shmid);

bool shouldCreateChild(int activeChildren, int maxActiveChildren, int maxForks, int activatedChildren);

int checkForTerminatedChildren(pid_t *array, int activated);

pid_t forkChild(int index);

bool shouldExit(pid_t *array, int activated, int maxForks);

static void interruptHandler();


int main(int argc, char **argv) {
    key_t key = 102938;
    const char * semName = "/sem_Chem";
    int optionIndex, maxForks = 10, maxActiveChildren = 20, activeChildren = 0;
    char *inputFileName = "input.txt";
    opterr = 0;
    int status = 0, i, index = 0;
    sem_t *sem;

    pid_t child_pid, wait_pid;


    //set default input file name
    inputFileName = "input.txt";
    while ((optionIndex = getopt(argc, argv, "hi:n:s:")) != -1) {
        switch (optionIndex) {
            case 'h':
                printf("List of valid command line argument usage\n");
                printf("-h	      :  Lists all possible command line arguments, theirw explanation,and usage.\n");
                printf("-i <argument> :  This option will take a file name as an argument and open the given\n");
                printf("\t\t file and parse its contents to continue the program.\n");
                printf("\t\t If no argument is provided, (input.txt)will be used as\n\t\t a default argument.\n");
                printf("-n <argument> : This option takes an positive integer argument, which specifies the maximum number of fork processes\n");
                printf("\t\t allowed in the program.");
                printf("\t\t NOTE: if this option is not given, the program will read all given input making this argument optional.");
                break;
            case 'i':
                if (optarg == NULL) {
                    printf("Caution: No file name provided. \nDefault file name [input.txt] will be assumed.\n");
                    optarg = "input.txt";
                }
                inputFileName = optarg;
                break;
            case 'n':
                maxForks = atoi(optarg);
                printf("Max number of forks is %d\n", maxForks);
                break;
            default:
                printf("%s: Argument Error: Incorrect argument usage.\n", argv[0]);
                printf("Use '-h' argument for valid usage instructions.\n\n Example: ./master -h\n\n");
                exit(-1);
        }
    }

    //open input file
    printf("\nInput File: %s\n", inputFileName);
    printf("Parent PID: %d\n", getpid());

    FILE *in_file = fopen(inputFileName, "r");
    if (in_file == NULL) {
        perror("./master: fileError: ");
        exit(-1);
    }
    //allocate shared memory
    int shmid = shmget(key, 100 * 100, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("./master: shmget error: ");
        exit(-1);
    }
    //start/post file data to shared memory segment
    postToSharedMemory(in_file, maxForks, shmid);
    //create named semaphore
    sem = sem_open(semName, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("./master: sem_open error: ");
        exit(-1);
    }


    int *childPidArray = malloc(maxForks * sizeof(int));
    int activatedChildren = 0;

    //Signal
    signal(SIGALRM, interruptHandler);//2 second interrupt
    signal(SIGINT, interruptHandler);//ctrl-c interrupt
    alarm(120);

    //master parent control
    while (true) {
        //check if a child should be spawned
        if (shouldCreateChild(activeChildren, maxActiveChildren, maxForks, activatedChildren)) {
            //spawn child
            child_pid = forkChild(index);
            index++;
            //add spawned child pid to childPidArray
            *(childPidArray + activatedChildren) = child_pid;
            //increment activatedChildren to keep track of how many children spawned
            activatedChildren++;
        }
        // check for terminated and get update for active children
        activeChildren = checkForTerminatedChildren(childPidArray, activatedChildren);
        //check termination conditions
        if (shouldExit(childPidArray, activatedChildren, maxForks)) {
            sem_close(sem);
            if (sem_unlink(semName) == -1) {
                perror("./master: sem_unlink error: ");
                exit(-1);
            }
            printf("Exit condition met\n");
            break;
        }
    }

    // release shared memory and file
    shmctl(key, IPC_RMID, NULL);
    fclose(in_file);
    free(childPidArray);
    printf("Parent[%d] Finished\n", getpid());
    return 0;

}

bool shouldCreateChild(int activeChildren, int maxActiveChildren, int maxForks, int activatedChildren) {
    //check if maxForks reached
    if (activatedChildren >= maxForks) {
        return false;
    }
    //check if current active children limit reached
    if (activeChildren >= maxActiveChildren) {
        return false;
    }
    return true;
}

int checkForTerminatedChildren(pid_t *array, int activated) {
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
                *(array + i) = -1;
            } else {
                active++;
            }
        }
    }
    //return current active processes
    return active;
}

bool shouldExit(pid_t *array, int activated, int maxForks) {
    int i = 0;
    for (i = 0; i < activated; i++) {
        //if the child pid array has an active child
        if (*(array + i) > 0) {
            return false;
        }
    }
    //if maxforks have not reach end yet
    if (activated < maxForks) {
        return false;
    }
    //exit requirement met
    return true;
}

pid_t forkChild(int index) {
    pid_t child_pid = fork();
    char arg[3];

    if (child_pid < 0) {
        perror("./oss : forkError");
        exit(-1);
    } else if (child_pid == 0) {
        //i am child
        //exec arguments
        sprintf(arg, "%d", index);
        char *arguments[3] = {"./palin", arg, NULL};
        execvp("./palin", arguments);
        perror("./master: exec error: ");
        exit(0);
    }
    return child_pid;
}

void postToSharedMemory(FILE* in_file, int maxForks, int shmid) {
    char fileBuffer[100];
    char (*palinArray)[100][100];
    int i;

    //attach sharedInt pointer to shared memory
    palinArray = shmat(shmid, NULL, 0);
    if (*palinArray == -1) {
        perror("./master: shmat error: ");
        exit(-1);
    }
    for (i = 0; i < maxForks; i++) {
        //add input strings to shared memory
        if(fgets(fileBuffer, sizeof(fileBuffer), in_file) != NULL) {
            strcpy((*palinArray)[i], fileBuffer);
        }
    }
    shmdt((void *) palinArray); //detach shared memory

}

static void interruptHandler(){
    key_t key = 102938;
    const char * semName = "/sem_Chem";
    int shmid = shmget(key, 100 * 100 , IPC_CREAT | 0666);
    fprintf(stderr,"Program Interrupted");
    sem_unlink(semName);
    shmctl(shmid, IPC_RMID, NULL); //delete shared memory
    kill(0, SIGKILL); // kill child process
    exit(0);
}

