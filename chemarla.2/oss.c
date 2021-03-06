/*
Name: Vibhav Chemarla
Date: 1/27/2019
CS4760 OS Project 2
Concurrent UNIX processes and shared memory
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

#define BILLION  1e9

typedef struct memTime{
    int seconds;
    int nseconds;
    char* duration[20];
    bool valid;
}memTime;

FILE* out_file;


key_t startSharedMemory();
memTime incrementSharedMemory(key_t key, int value);
char* shouldCreateChild(int activeChildren, int maxActiveChildren, int maxForks,  int activatedChildren, memTime fileTime, memTime currentTime);
int checkForTerminatedChildren(pid_t* array, FILE* outfileHandler, int activated, memTime currentTime);
pid_t forkChild(char* duration, char* outputFileName);
bool shouldExit(pid_t* array, int activated, int maxForks, memTime fileTime);
memTime readNextLine(FILE* input_file);
static void interruptHandler();


int main(int argc, char **argv) {
    int optionIndex, maxForks = 4, maxActiveChildren = 2, activeChildren = 0;
    char *inputFileName = "input.txt";
    char *outputFileName = "output.txt";
    char *fileBuffer[256];
    const char space[2] = " ";
    char *token;
    char *duration;
    opterr = 0;
    int status = 0, i;
    int incrementValue;



    pid_t child_pid, wait_pid;
    //check if arguments are given
    while ((optionIndex = getopt(argc, argv, "hi:o:n:s:")) != -1) {
        switch (optionIndex) {
            case 'h':
                printf("List of valid command line argument usage\n");
                printf("-h	      :  Lists all possible command line arguments, their explanation,and usage.\n");
                printf("-i <argument> :  This option will take a file name as an argument and open the given\n");
                printf("\t\t file and parse its contents to continue the program.\n");
                printf("\t\t If no argument is provided, (input.dat)will be used as\n\t\t a default argument.\n");
                printf("-o <argument> :  This option will output data from the program to the given file name\n");
                printf("\t\t argument. If no argument is provided, (output.txt) will be used as a\n\t\t default argument.\n");
                printf("-n <argument> : This option takes an positive integer argument, which specifies the maximum number of fork processes\n");
                printf("\t\t allowed in the program.");
                printf("-s <argument> : This option takes an positive integer argument, which specifies the maximum number of active fork processes\n");
                printf("\t\t allowed in the program. Maximum active allowed is 20, if argument is above 20, it will default to 20.\n");
                break;
            case 'i':
                if (optarg == NULL) {
                    printf("Caution: No file name provided. \nDefault file name [input.txt] will be assumed.\n");
                    optarg = "input.txt";
                }
                inputFileName = optarg;

                if (outputFileName == NULL) { //set output file to default if user only provides input data file
                    outputFileName = "output.txt";
                }
                break;

            case 'o':
                //set output file to output.txt
                // if no argument given
                if (optarg == NULL) {
                    optarg = "output.txt";
                }
                outputFileName = optarg;

                printf("Output File: %s\n", outputFileName);
                //If only the output was given
                if (inputFileName == NULL) { //set input file to default if user only provides input data file
                    inputFileName = "input.txt";
                }
                break;

            case 'n':
                maxForks = atoi(optarg);
                printf("Max number of forks is %d\n", maxForks);
                break;

            case 's':
                maxActiveChildren = atoi(optarg);
                if(maxActiveChildren > 20){
                    printf("Maximum number of active child processes running on machine is 20. Exceeding argument will be set to 20.");
                    maxActiveChildren = 20;
                }else if(maxActiveChildren < 0){
                    printf("Invalid argument passed to -s option.\n");
                    printf("Use '-h' argument for valid usage instructions.\n\n Example: ./oss -h\n\n");
                    exit(-1);
                }
                printf("Max number of active children allowed is %d\n", maxActiveChildren);
                break;

            default:
                printf("%s: Argument Error: Incorrect argument usage.\n", argv[0]);
                printf("Use '-h' argument for valid usage instructions.\n\n Example: ./oss -h\n\n");
                exit(0);
        }
    }
    inputFileName = "input.txt";
    printf("Input File: %s\n", inputFileName);
    printf("Output File: %s\n", outputFileName);
    printf("Opening input file...\n");

    FILE *in_file = fopen(inputFileName, "r");
    if (in_file == NULL) {
        perror("./oss: fileError: ");
        return 1;
    }
    //***************
    out_file = fopen(outputFileName, "a+");
    if (out_file == NULL) {
        perror("./oss: File Error: ");
        return 1;
    }
    //get key from starting shared memory segment
    key_t key = startSharedMemory();

    // read first line
    fgets(fileBuffer, sizeof(fileBuffer), in_file);
    token = strtok(fileBuffer, space);
    incrementValue = atoi(fileBuffer);

    //read second line to start incrementing and store line contents in fileTime struct
    memTime fileTime;
    fileTime = readNextLine(in_file);
    //if empty file detected exit
    if(!fileTime.valid){
        exit(-1);
    }

    int *childPidArray = malloc(maxForks * sizeof(int));
    int activatedChildren = 0;
    memTime currentTime;
    currentTime.nseconds = 0;
    currentTime.seconds = 0;
    //Signal
    signal(SIGALRM, interruptHandler);//2 second interrupt
    signal(SIGINT, interruptHandler);//ctrl-c interrupt
    alarm(2);

    //oss parent control
    while (true){
        //increment time
        currentTime = incrementSharedMemory(key, incrementValue);
        //check if a child should be spawned
        if ((duration = shouldCreateChild(activeChildren, maxActiveChildren, maxForks, activatedChildren, fileTime, currentTime)) != NULL) {
            //spawn child
            child_pid = forkChild(duration, outputFileName);
            printf("Child[%d] started with duration %d at time %d s:%d ns\n", child_pid, atoi(duration), currentTime.seconds, currentTime.nseconds);
            fprintf(out_file, "Child[%d] started with duration %d at time %d s:%d ns\n", child_pid, atoi(duration), currentTime.seconds, currentTime.nseconds);
            //add spawned child pid to childPidArray
            *(childPidArray + activatedChildren) = child_pid;
            //increment activatedChildren to keep track of how many children spawned
            activatedChildren++;
            //read the next line and store into fileTime for next while loop iteration
            fileTime = readNextLine(in_file);
        }
        // check for terminated/get update for active children
        activeChildren = checkForTerminatedChildren(childPidArray, out_file, activatedChildren, currentTime);

        if (shouldExit(childPidArray, activatedChildren, maxForks, fileTime)){
            printf("Exit condition met\n");
            break;
        }
    }

    // release shared memory and file
    shmctl(key,IPC_RMID,NULL);
    fclose(in_file);
    fclose(out_file);
    free(childPidArray);
    printf("Parent[%d] Finished\n", getpid());
    return 0;

}

memTime readNextLine(FILE* input_file){
    char fileBuffer[256];
    memTime fileTime;
    fileTime.valid = false;
    const char space[2] = " ";
    char* token;

    if(fgets(fileBuffer, sizeof(fileBuffer), input_file) != NULL) {
        token = strtok(fileBuffer, space);
        fileTime.seconds = atoi(token);
        token = strtok(NULL, space);
        fileTime.nseconds = atoi(token);
        token = strtok(NULL, space);
        strcpy(fileTime.duration,token);
        fileTime.valid = true;
    }
    return fileTime;
}

char* shouldCreateChild(int activeChildren, int maxActiveChildren, int maxForks, int activatedChildren, memTime fileTime, memTime currentTime) {
    long int currentNanoTotal, fileNanoTotal;
    //check if maxForks reached
    if(activatedChildren >= maxForks){
        return NULL;
    }
    //check if current active children limit reached
    if(activeChildren >= maxActiveChildren){
        return NULL;
    }
    //Additional check if fgets took read a empty line
    if(!fileTime.valid){
        return NULL;
    }
    //change file and current(shared memory segment) to nano seconds
    fileNanoTotal = (fileTime.seconds * BILLION) + fileTime.nseconds;
    currentNanoTotal = (currentTime.seconds * BILLION) + currentTime.nseconds;
    //If simulated clock has past the entry time stated in the input file spawn the child
    if(currentNanoTotal > fileNanoTotal){
        return fileTime.duration;
    }
    else{
        return NULL;
    }
}

int checkForTerminatedChildren(pid_t* array, FILE* out_file, int activated, memTime currentTime) {
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
                fprintf(out_file, "Child[%d] terminated at %d:%d\n", *(array + i), currentTime.seconds, currentTime.nseconds);
                *(array + i) = -1;
            } else {
                active++;
            }
        }
    }
    //return current active processes
    return active;
}

bool shouldExit(pid_t* array, int activated, int maxForks, memTime fileTime){
    int i = 0;
    for (i = 0; i < activated; i++){
        //if the child pid array has an active child
        if (*(array+i) > 0){
            return false;
        }
    }
    //if fgets takes in empty lines
    if(!fileTime.valid){
        return true;
    }
    //if maxforks have not reach end yet
    if (activated < maxForks){
        return false;
    }
    printf("asking to exit\n");
    return true;
}

pid_t forkChild(char* duration, char* outputFileName){
    pid_t child_pid = fork();

    if (child_pid < 0) {                              //fork fail check
        perror("./oss : forkError");
        exit(-1);
    } else if (child_pid == 0) {
        //i am child
        //exec arguments
        char *arguments[3] = {"./user", duration, NULL};
        execvp("./user", arguments);
        exit(0);
    }
    return child_pid;
}

key_t startSharedMemory(){
    int *sharedInt;
    key_t key = 123;
    //allocate shared memory
    int shmid = shmget(key, 2 * sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("./user: shmid error: ");
        exit(1);
    }
    //attach sharedInt pointer to shared memory
    sharedInt = (int *) shmat(shmid, NULL, 0);

    if (*sharedInt == -1) {
        perror("./oss: shmat error: ");
        exit(1);
    }

    //initialize seconds and nanoseconds in shared memory
    *(sharedInt+0) = 0;   //seconds
    *(sharedInt+1) = 0;   //nanoseconds

    shmdt((void *) sharedInt); //detach shared memory
    return key;
}

memTime incrementSharedMemory(key_t key, int value) {
    int *sharedInt;
    unsigned int nextNano;
    unsigned int nextSeconds;
    memTime currentTime;
    //allocate shared memory
    int shmid = shmget(key, 2 * sizeof(int), IPC_CREAT | 0666);

    if (shmid < 0) {
        perror("./user: shmid error: ");
        exit(1);
    }
    //attach sharedInt pointer to shared memory
    sharedInt = (int *) shmat(shmid, NULL, 0);

    if (*sharedInt == -1) {
        perror("./oss: shmat error: ");
        exit(1);
    }
    //intialize seconds and nanoseconds in shared memory
    nextSeconds = *(sharedInt+0);   //seconds
    nextNano = *(sharedInt+1)+ value;   //nanoseconds
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
    shmdt((void *) sharedInt); //detach shared memory

    return currentTime;
}

static void interruptHandler(){
    key_t key = 123;
    int* sharedInt;
    int shmid = shmget(key, 2 * sizeof(int), IPC_CREAT | 0666);
    sharedInt = (int *) shmat(shmid, NULL, 0);
    fprintf(out_file, "Program Interrupt at %d s: %d ns\n", *(sharedInt+0), *(sharedInt+1));
    fclose(out_file);
    shmctl(shmid, IPC_RMID, NULL); //delete shared memory
    kill(0, SIGKILL); // kill child process
    exit(0);
}

