/*
Name: Vibhav Chemarla
Date: 1/27/2019
CS4760 OS Project 1
Practice making system calls with the use of getopt, fork, and perror
*/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/wait.h>


int childFunction(FILE *in_file);
int inputProcessFile(FILE *in_file);

int main(int argc, char **argv) {
    int optionIndex;
    char *fileName = NULL;
    opterr = 0;

    while ((optionIndex = getopt(argc, argv, ":hi::o::")) != -1) {
        switch (optionIndex) {
            case 'h':
                printf("List of valid command line argument usage\n");
                printf("-h	      :  Lists all possible command line arguments, their explanation,and usage.\n");
                printf("-i <argument> :  This option will take a file name as an argument and open the given\n\t\t file and parse its contents to continue the program.\n\t\t If no argument is provided, (input.dat)will be used as\n\t\t a default argument.\n");
                printf("-o <argument> :  This option will output data from the program to the given file name\n\t\t argument. If no argument is provided, (output.dat) will be used as a\n\t\t default argument.\n");
                break;
            case 'i':
                if (optarg == NULL) {
                    optarg = "input.dat";
                }
                fileName = optarg;
                printf("Input File: %s\n", fileName);
                printf("Opening File...\n");
                FILE *in_file = fopen(fileName, "r");
                if (in_file == NULL) {
                    printf("Could not open file\n");
                    return 1;
                }
                if (in_file != NULL) {
                    inputProcessFile(in_file);
                }

                printf("Closing File...%d\n", getpid());
                fclose(in_file);
                break;

            case 'o':
                if (optarg == NULL) {
                    optarg = "output.dat";
                }
                fileName = optarg;
                printf("Output File: %s\n", fileName);
                break;
            default:
                printf("a.out: Error: Incomplete argument usage. Use '-h' argument for valid usage instructions.\n\n Example: ./a.out -h\n");
                break;
        }
    }

    return 0;
}


int childFunction(FILE *in_file) {
    char fileBuffer[256];
    int numbersToRead, i;
    const char space[2] = " ";
    char *token;

    printf("Child Process: %d \n", getpid());
    //first line
    fgets(fileBuffer, sizeof fileBuffer, in_file);
    numbersToRead = atoi(fileBuffer);
    int *processNumberArray = (int *)malloc(numbersToRead * sizeof(int));
    //second line
    fgets(fileBuffer, sizeof(fileBuffer), in_file);
    token = strtok(fileBuffer, space);

    i = 0;
    while (token != NULL) {
        *(processNumberArray+i) = atoi(token);
        i++;
        if (i > numbersToRead){
            break;
        }
        token = strtok(NULL, space);
    }
    printf("Numbers to read: %d\n", numbersToRead);

    for (i = 0; i < numbersToRead; i++) {

        printf("%d ", *(processNumberArray + (numbersToRead-i-1)));
    }
    printf("\n");
    //free(processNumberArray);
    return 0;
}

int inputProcessFile(FILE* in_file) {
    int i, numOfForks;
    char fileBuffer[256];
    pid_t child_pid, wpid;
    int status = 0;

    printf("File opened successfully!\n");

    fgets(fileBuffer, sizeof fileBuffer, in_file);
    numOfForks = atoi(fileBuffer);

    int *childPidArray = malloc(numOfForks * sizeof(int));
    printf("Number of Forks: %d\n", numOfForks);

    printf("Parent Process %d\n", getpid());

    for (i = 0; i < numOfForks; i++) {
        child_pid = fork();
        if (child_pid == 0) {
            //I am child
            childFunction(in_file);
            exit(0);
        } else {
            // parent
            // advance file reading by 2 lines
            *(childPidArray+i)= child_pid;
            fgets(fileBuffer, sizeof(fileBuffer), in_file);
            fgets(fileBuffer, sizeof(fileBuffer), in_file);
            while ((wpid = wait(&status)) == child_pid);
        }
    }

    printf("All children were:");
    for(i = 0; i < numOfForks; i++){
        printf("%d ", *(childPidArray+i));
    }
    printf("\n");
}




