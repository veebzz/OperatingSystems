/*
Name: Vibhav Chemarla
Date: 3/19/2019
CS4760 OS Project 3
Semaphores and Operating System Simulator
*/
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>



char* strrev(char *str);

key_t key = 102938;
const char * semName = "/sem_Chem";

int main(int argc, char **argv) {
    int palinIndex, i;
    char* palinString = malloc(sizeof(char) * 100);
    char* revPalinString = malloc(sizeof(char) * 100);
    char* outputFileName;
    FILE* out_file;
    sem_t* sem;
    time_t now;
    now = time(NULL);
    int random = rand() % 4;
    struct tm *ts;
    char (*palinArray)[100][100];
    char buf[80];

    ts = localtime(&now);

    palinIndex = atoi(argv[1]); // get the passed index
    srand(palinIndex);

    printf("CHILD[%d] STARTED\n", getpid());
    //get string in shared memory stored at the passed index
    int shmid = shmget(key, 100 * 100, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("./master: shmget error: ");
        exit(-1);
    }
    //attach pointer to shared memory
    palinArray = shmat(shmid, NULL, 0);
    if (*palinArray == -1) {
        perror("./palin: shmat error: ");
        exit(1);
    }


    strcpy(palinString, (*palinArray)[palinIndex]);
    strtok(palinString, "\n");
    printf("palin %d\n", strlen(palinString));
   //use C++ style string reverse
    strcpy(revPalinString,strrev(palinString));
    printf("%s\n", revPalinString);
    printf("revpalin %d\n", strlen(revPalinString));


    //compare strings to see if it is a palindrome
    if(strcmp(palinString, revPalinString) == 0){
        outputFileName = "palin.out";
        fprintf(stderr, "%s\n", outputFileName);
    }else {
        outputFileName = "nopalin.out";
        fprintf(stderr, "%s\n", outputFileName);
    }
    shmdt((void *) palinArray);

    //semaphore
    sem = sem_open(semName, O_CREAT, 0666, 1);
    if(sem == SEM_FAILED) {
        fprintf(stderr, "./palin: sem_open error: ");
        perror("./palin: sem_open error: ");
        exit(-1);
    }

    for(i = 0; i < 5; i++){
        //sleep between 0 - 3 seconds
        sleep(1);
        //entry section
        strftime(buf, sizeof(buf),"%H:%M:%S", ts);
        fprintf(stderr, "Child[%d] starting to enter critical section at %s\n", getpid(), buf);

        sem_wait(sem);

        //critical section
        strftime(buf, sizeof(buf),"%H:%M:%S",ts);
        fprintf(stderr,"Child[%d] is inside CRITICAL SECTION at %s\n", getpid(), buf);
        out_file = fopen(outputFileName, "a+");
        if (out_file == NULL){
            perror("./palin: fileError: ");
            exit(-1);
        }
        sleep(2);
        printf("Printing [%d]\n", getpid());
        printf("Printing index %d\n", palinIndex);
        printf("Printing string %s\n", palinString);

        fprintf(out_file, "%d %d %s\n", getpid(), palinIndex, palinString);
//        fprintf(stderr, "%d %d %s\n", getpid(), palinIndex, palinString);

        printf("%d %d %s\n", getpid(), palinIndex, palinString);

        fclose(out_file);
        sleep(2);
        //execute code to exit from critical section
        sem_post(sem);
    }
    free(palinString);
    free(revPalinString);
    printf("CHILD[%d] TERMINATED\n", getpid());

    exit(0);
}



char* strrev(char* str)
{
    char *p1, *p2;

    if (! str || ! *str)
        return str;
    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
    {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return str;
}
//char * strrev(char* str)
//{
//    int l, i;
//    char *begin_ptr, *end_ptr, ch;
//
//    // Get the length of the string
//    l = strlen(str);
//
//    // Set the begin_ptr and end_ptr
//    // initially to start of string
//    begin_ptr = str;
//    end_ptr = str;
//
//    // Move the end_ptr to the last character
//    for (i = 0; i < l - 1; i++)
//        end_ptr++;
//
//    // Swap the char from start and end
//    // index using begin_ptr and end_ptr
//    for (i = 0; i < l / 2; i++) {
//
//        // swap character
//        ch = *end_ptr;
//        *end_ptr = *begin_ptr;
//        *begin_ptr = ch;
//
//        // update pointers positions
//        begin_ptr++;
//        end_ptr--;
//    }
//}