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




int main(int argc, char **argv) {
    int optionIndex, maxForks = 0, maxActiveChildren = 0, activeChildren = 0;
    char *inputFileName = NULL;
    char *outputFileName = NULL;
    opterr = 0;
    int status = 0, i;
    int *sharedInt1;
    int *sharedInt2;

    pid_t child_pid, wait_pid;
    //check if arguments are given
    if (argc < 2) {
        inputFileName = "input.dat";
        outputFileName = "output.dat";
    }

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
    } else {                                                    //****************
       key_t key = ftok(".", 'x');
        //allocate shared memory
        int shmid = shmget(key, 2 * sizeof(int), IPC_CREAT | 0666);
        if (shmid < 0) {
            perror("./user: shmid error: ");
            return 1;
        }
        //attach shared memory
        sharedInt1 = (int *) shmat(shmid, NULL, 0);

        if (*sharedInt1 == -1) {
            perror("./oss: shmat error: ");
            return 1;
        }

        sharedInt1[0] = 9999;
        printf("Data written in memory: %d\n", sharedInt1[0]);

        shmdt((void *)sharedInt1);

        child_pid = fork();


        if (child_pid < 0) {                              //fork fail check
            perror("./oss : forkError");
            return 1;
        }
        else if(child_pid == 0){
            char *arguments[] = {NULL};
            execvp("./user", arguments);
        }

        printf("Parent is making the Kool Aid dranks!!\n");
        wait(NULL);
        printf("Parent Finished\n");
        return 0;
//        for (i = 0; i < maxForks; i++) {
//            printf("Number of Children Fork: %d \n", i + 1);
//            while (activeChildren <= maxActiveChildren) {
//                printf("Active child: %d\n", activeChildren);
//                activeChildren++;
//                i++;
//                child_pid = fork();
//
//
//                if (child_pid < 0) {                              //fork fail check
//                    perror("./oss : forkError");
//                    return 1;
//                }
//                if (child_pid == 0) {
//                    //I am child
//                    printf("I am child with pid: %d\n", getpid());
//                    printf("Child terminating\n");
//                    sleep(2);
//                    activeChildren--;
//                    exit(0);
//                }
//                    // exit here to stop child process from copying the rest of parent process
//                 else {
//                    //I am parent
//                    printf("parent waiting\n");
//                    while ((wait_pid = wait(&status)) == child_pid);
//
//                }
//            }
//
//        }

    }

    fclose(in_file);
    printf("\n__________________________________\n");
    printf("\nParent Process [%d] Finished.\n", getpid());

    return 0;
}



