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


int childFunction(FILE *in_file, char *outputFileName);

int inputProcessFile(FILE *in_file, char *outputFileName);


int main(int argc, char **argv) {
    int optionIndex, maxForks, maxLivingChildren;
    char *inputFileName = NULL;
    char *outputFileName = NULL;
    opterr = 0;
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
                printf("-i <argument> :  This option will take a file name as an argument and open the given\n\t\t file and parse its contents to continue the program.\n\t\t If no argument is provided, (input.dat)will be used as\n\t\t a default argument.\n");
                printf("-o <argument> :  This option will output data from the program to the given file name\n\t\t argument. If no argument is provided, (output.dat) will be used as a\n\t\t default argument.\n");
                break;
            case 'i':
                if (optarg == NULL) {
                    printf("Caution: No file name provided. \nDefault file name [input.dat] will be assumed.\n");
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
                maxForks = optarg;
                printf("Max number of forks is %d\n", maxForks);
                break;

            case 's':
                maxLivingChildren = optarg;
                printf("Max number of living children allowed is %d\n", maxLivingChildren);
                break;

            default:
                printf("%s: Argument Error: Incorrect argument usage.\n Use '-h' argument for valid usage instructions.\n\n Example: ./syscall -h\n\n",
                       argv[0]);
                exit(0);
        }
    }
    printf("Input File: %s\n", inputFileName);
    printf("Output File: %s\n", outputFileName);
    printf("Opening input file...\n");

    FILE *in_file = fopen(inputFileName, "r");
    if (in_file == NULL) {
        perror("./syscall: fileError: ");
        return 1;
    } else {
        inputProcessFile(in_file, outputFileName);                                                     //***
    }
    fclose(in_file);
    printf("\n__________________________________\n");
    printf("\nParent Process [%d] Finished.\n", getpid());

    return 0;
}


int inputProcessFile(FILE *in_file, char *outputFileName) {
    int i, numOfForks = 0, status = 0, stopFlag;
    char fileBuffer[256];
    pid_t child_pid, wait_pid;

    printf("Input file opened successfully!\n");

    //Read number of forks from the first line
    fgets(fileBuffer, sizeof fileBuffer, in_file);
    numOfForks = atoi(fileBuffer);
    //Dynamically allocate integer array to store child PIDs
    int *childPidArray = malloc(numOfForks * sizeof(int));

    printf("Number of Forks: %d\n", numOfForks);
    printf("Parent Process %d\n", getpid());
    printf("__________________________________\n");



    // Parent/Child fork control
    for (i = 0; i < numOfForks; i++) {
        child_pid = fork();
        if (child_pid == 0) {
            //I am child
            childFunction(in_file, outputFileName);
            exit(0); // exit here to stop child process from copying the rest of parent process
        } else {
            //I am parent
            //advance 2 lines
            if (fgets(fileBuffer, sizeof(fileBuffer), in_file)) { // check if next line exists
                fgets(fileBuffer, sizeof(fileBuffer), in_file);
                *(childPidArray + i) = child_pid;
                while ((wait_pid = wait(&status)) == child_pid);
            } else {
                numOfForks = i;//else change numOfForks to output correct number of processes read
                stopFlag++;
            }
        }
    }

    //Parent writes to output file
    FILE *out_file = fopen(outputFileName, "a+");
    if (out_file == NULL) {
        perror("fileError: ");
        return 1;
    }
    for (i = 0; i < numOfForks; i++) {
        if (i == 0) {
            //Including Parent PID to show the parent is writing this
            fprintf(out_file, "Parent[%d] had %d child processes: %d", getpid(), numOfForks, *(childPidArray + i));
        } else if (i == numOfForks - 1) {
            //adding newline to EOF
            fprintf(out_file, " %d\n", *(childPidArray + i));
        } else {
            fprintf(out_file, " %d", *(childPidArray + i));
        }
    }
    fclose(out_file);
    free(childPidArray);
    return 0;
}

int childFunction(FILE *in_file, char *outputFileName) {
    char fileBuffer[256];
    int numbersToRead, i = 0;
    const char space[2] = " ";
    char *token;


    //first line
    if (fgets(fileBuffer, sizeof fileBuffer, in_file)) {//check to see if first line has data
        token = strtok(fileBuffer, space);
        numbersToRead = atoi(fileBuffer);
    } else { //if fgets returns NULL return to inputProcessFile

        return 0;
    }

    int *processNumberArray = (int *) malloc(numbersToRead * sizeof(int));
    //second line
    fgets(fileBuffer, sizeof(fileBuffer), in_file);
    token = strtok(fileBuffer, space);


    printf("\nChild Process: %d \n", getpid());

    while (token != NULL) {                         //while token is not null
        *(processNumberArray + i) = atoi(token);
        i++;                                        //increment pointer like an array
        //If there are more numbers than instructed to read then break
        if (i > numbersToRead) {
            break;
        }
        token = strtok(NULL, space);
    }

    printf("Numbers to read: %d\n", numbersToRead);

    //Open output file
    FILE *out_file = fopen(outputFileName, "a+");
    if (out_file == NULL) {
        perror("syscall: File Error: ");
        return 1;
    }
    //print to stream in correct order
    for (i = 0; i < numbersToRead; i++) {
        printf("%d ", *(processNumberArray + i));
    }
    printf("\n");
    //Reverse Numbers
    for (i = 0; i < numbersToRead; i++) {
        //print to stream
        printf("%d ", *(processNumberArray + (numbersToRead - i - 1)));
        //write to output file
        if (i == 0) {
            //Place PID in the beginning
            fprintf(out_file, "%d: %d", getpid(), *(processNumberArray + (numbersToRead - i - 1)));
        } else if (i == numbersToRead - 1) {
            //Place newline at the end of the last number
            fprintf(out_file, " %d\n", *(processNumberArray + (numbersToRead - i - 1)));
        } else {
            fprintf(out_file, " %d", *(processNumberArray + (numbersToRead - i - 1)));
        }
    }
    fclose(out_file);
    printf("\nChild Process [%d] Finished.\n", getpid());
    //If the last token is reached then free processNumberArray
    if (token == NULL) {
        free(processNumberArray);// causing error when trying to free memory by itself
    }

    return 0;
}




