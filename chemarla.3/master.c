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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>



void postToSharedMemory(FILE *in_file, int maxForks);
bool shouldCreateChild(int activeChildren, int maxActiveChildren, int maxForks, int activatedChildren);
int checkForTerminatedChildren(pid_t *array, int activated);
pid_t forkChild(int index);
bool shouldExit(pid_t *array, int activated, int maxForks);
//static void interruptHandler();


int main(int argc, char **argv) {
    int optionIndex, maxForks = 10, maxActiveChildren = 20, activeChildren = 0;
    char *inputFileName = "input.txt";
    opterr = 0;
    int status = 0, i;
    sem_t *sem;

    pid_t child_pid, wait_pid;

    //set default input file name
    inputFileName = "input.txt";
    while ((optionIndex = getopt(argc, argv, "hi:n:s:")) != -1) {
        switch (optionIndex) {
            case 'h':
                printf("List of valid command line argument usage\n");
                printf("-h	      :  Lists all possible command line arguments, their explanation,and usage.\n");
                printf("-i <argument> :  This option will take a file name as an argument and open the given\n");
                printf("\t\t file and parse its contents to continue the program.\n");
                printf("\t\t If no argument is provided, (input.txt)will be used as\n\t\t a default argument.\n");
                printf("-n <argument> : This option takes an positive integer argument, which specifies the maximum number of fork processes\n");
                printf("\t\t allowed in the program.");
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
    printf("Opening input file...\n");

    FILE *in_file = fopen(inputFileName, "r");
    if (in_file == NULL) {
        perror("./master: fileError: ");
        exit(-1);
    }

    //start/post file data to shared memory segment
    postToSharedMemory(inputFileName, maxForks);
    //create named semaphore
    sem = sem_open("semName", O_CREAT, 0644, 0);
    if(sem == SEM_FAILED){
        perror("./master: sem_open error: ");
        exit(-1);
    }


    int *childPidArray = malloc(maxForks * sizeof(int));
    int activatedChildren = 0;

    //Signal
//    signal(SIGALRM, interruptHandler);//2 second interrupt
//    signal(SIGINT, interruptHandler);//ctrl-c interrupt
//    alarm(2);

    //master parent control
    while (true) {
        //check if a child should be spawned
        if (shouldCreateChild(activeChildren, maxActiveChildren, maxForks, activatedChildren)) {
            //spawn child
            child_pid = forkChild(activatedChildren);
            printf("Child[%d] started\n", child_pid);
            //add spawned child pid to childPidArray
            *(childPidArray + activatedChildren) = child_pid;
            //increment activatedChildren to keep track of how many children spawned
            activatedChildren++;
        }
        // check for terminated and get update for active children
        activeChildren = checkForTerminatedChildren(childPidArray, activatedChildren);
        //check termination conditions
        if (shouldExit(childPidArray, activatedChildren, maxForks)) {
            if(sem_unlink(sem) == -1){
                perror("./master: sem_unlink error: ");
                exit(-1);
            }
            printf("Exit condition met\n");
            break;
        }
    }

    // release shared memory and file
    shmctl(123, IPC_RMID, NULL);
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
                printf("Child[%d] terminated", *(array + i));
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
    printf("asking to exit\n");
    return true;
}

pid_t forkChild(int index) {
    pid_t child_pid = fork();

    if (child_pid < 0) {
        perror("./oss : forkError");
        exit(-1);
    } else if (child_pid == 0) {
        //i am child
        //exec arguments
        char *arguments[3] = {"./palin", index, NULL};
        execvp("./palin", arguments);
        exit(0);
    }
    return child_pid;
}

void postToSharedMemory(FILE *in_file, int maxForks) {
    char* palinArray[maxForks], fileBuffer[100];
    int i;

    key_t key = 123;
    //allocate shared memory
    int shmid = shmget(key, 1024, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("./master: shmid error: ");
        exit(-1);
    }
    //attach sharedInt pointer to shared memory
    *palinArray = (char *) shmat(shmid, NULL, 0);

    if (*palinArray == -1) {
        perror("./master: shmat error: ");
        exit(-1);
    }
    for (i = 0; i < maxForks; i++) {
        fgets(fileBuffer, sizeof(fileBuffer), in_file);
        *(palinArray + i) = fileBuffer;
    }
    shmdt((void *) palinArray); //detach shared memory

}

//static void interruptHandler(){
//    key_t key = 123;
//    char* myList;
//    int shmid = shmget(key, 2 * sizeof(char), IPC_CREAT | 0666);
//    sharedInt = (int *) shmat(shmid, NULL, 0);
//    fprintf(out_file, "Program Interrupt at %d s: %d ns\n", *(sharedInt+0), *(sharedInt+1));
//    fclose(out_file);
//    shmctl(shmid, IPC_RMID, NULL); //delete shared memory
//    kill(0, SIGKILL); // kill child process
//    exit(0);
//}

