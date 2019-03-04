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


key_t startSharedMemory();
void incrementSharedMemory(key_t key, int value);
char* shouldCreateChild(int activeChildren, int maxActiveChildren, int maxForks, int activatedChildren);
int checkForTerminatedChildren(pid_t* array, FILE* outfileHandler, int activated);
pid_t forkChild(char* duration, FILE* outfileHandler);
bool shouldExit(pid_t* array, int activated, int maxForks);


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
    while ((optionIndex = getopt(argc, argv, "::hi:o:n:s:")) != -1) {
        switch (optionIndex) {
            case 'h':
                printf("List of valid command line argument usage\n");
                printf("-h	      :  Lists all possible command line arguments, their explanation,and usage.\n");
                printf("-i <argument> :  This option will take a file name as an argument and open the given\n");
                printf("\t\t file and parse its contents to continue the program.\n");
                printf("\t\t If no argument is provided, (input.dat)will be used as\n\t\t a default argument.\n");
                printf("-o <argument> :  This option will output data from the program to the given file name\n");
                printf("\t\t argument. If no argument is provided, (output.txt) will be used as a\n\t\t default argument.\n");
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
                //set output file to output.dat if no argument given
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
    FILE *out_file = fopen(outputFileName, "a+");
    if (out_file == NULL) {
        perror("syscall: File Error: ");
        return 1;
    }
    key_t key = startSharedMemory();


    fgets(fileBuffer, sizeof(fileBuffer), in_file); // read first line
    token = strtok(fileBuffer, space);
    incrementValue = atoi(fileBuffer);


    int *childPidArray = malloc(maxForks * sizeof(int));
    int activatedChildren = 0;

    while (true){
        incrementSharedMemory(key, incrementValue);
        if ((duration = shouldCreateChild(activeChildren, maxActiveChildren, maxForks, activatedChildren)) != NULL) {
            child_pid = forkChild(duration, out_file);

            //keep the pid array
            *(childPidArray + activatedChildren) = child_pid;
            activatedChildren++;

        }

        activeChildren = checkForTerminatedChildren(childPidArray, out_file, activatedChildren);

        if (shouldExit(childPidArray, activatedChildren, maxForks)){
            break;
        }
    }

    // release memory
    shmctl(key,IPC_RMID,NULL);



    printf("Parent Finished\n");
    return 0;

}

char* shouldCreateChild(int activeChildren, int maxActiveChildren, int maxForks, int activatedChildren) {

    char* duration = "10000";
    if(activatedChildren >= maxForks){
        return NULL;
    }

    if(activeChildren >= maxActiveChildren){
        return NULL;
    }


    return duration;

}
int checkForTerminatedChildren(pid_t* array, FILE* outfileHandler, int activated) {
    int i = 0;
    int status;
    pid_t checkId;
    int active = 0;

    for (i = 0; i < activated; i++) {
        if (*(array + i) > 0) {
            //if checkID = 0 its still running, if -1 error, is = to its pid, it is dead
            checkId = waitpid(*(array + i), &status, WNOHANG);
            //if it is dead, mark to not check this anymore
            if (checkId == *(array + i)) {
                *(array + i) = -1;
            } else {
                active++;
            }
        }
    }
    return active;
}

bool shouldExit(pid_t* array, int activated, int maxForks){
    int i = 0;
    for (i = 0; i < activated; i++){
        if (*(array+i) > 0){
            return false;
        }
    }

    if (activated < maxForks){
        return false;
    }

    return true;
}

pid_t forkChild(char* duration, FILE* outfileHandler){
    printf("In forkChild function\n");
    pid_t child_pid = fork();

    if (child_pid < 0) {                              //fork fail check
        perror("./oss : forkError");
        exit(-1);
    } else if (child_pid == 0) {
        //i a child
        char *arguments[3] = {"./user", duration, NULL}; // exec arguments, replace NULL
        execvp("./user", arguments);
        exit(0);
    }
    return child_pid;
}


key_t startSharedMemory(){
    int *sharedInt;
    key_t key = 123;                        //unique key
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
    *(sharedInt+0) = 0;   //seconds
    *(sharedInt+1) = 0;   //nanoseconds

    shmdt((void *) sharedInt); //detach shared memory
    return key;
}


void incrementSharedMemory(key_t key, int value) {
    int *sharedInt;
    unsigned int nextNano;
    unsigned int nextSeconds;
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

    if (nextNano > 1e9){
        nextSeconds++;
        nextNano =  (nextNano - 1e9);
    }

    *(sharedInt+0) = nextSeconds;
    *(sharedInt+1) = nextNano;

//    printf("OSS: seconds written in memory: %d\n", nextSeconds);
//    printf("OSS: nanoseconds written in memory: %ld\n", nextNano);

    shmdt((void *) sharedInt); //detach shared memory
}
