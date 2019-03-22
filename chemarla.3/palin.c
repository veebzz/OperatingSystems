/*
Name: Vibhav Chemarla
Date: 3/19/2019
CS4760 OS Project 3
Semaphores and Operating System Simulator
*/
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>


char* getSharedMemory(int palinIndex, int maxForks);
char* isPalindrome(char str1[100]);
char* strrev(char *str);

key_t key = 102938;

int main(int argc, char **argv) {

    int palinIndex, i, maxForks;
    char* palinString = malloc(sizeof(char) * 100);
    char* revPalinString = malloc(sizeof(char) * 100);
    char* outputFileName, semName = "semName";
    FILE* out_file;
    sem_t* sem;
    time_t now;
    srand(now);
    now = time(NULL);
    int random = rand() % 4;
    struct tm *ts;
    char (*palinArray)[100][100];
    char buf[80];

    ts = localtime(&now);


    palinIndex = atoi(argv[1]); // get the passed index
    maxForks = atoi(argv[2]);


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

    palinString = (*palinArray)[palinIndex - 1];
    palinString = "racecar";
   //use C++ style string reverse
    strcpy(revPalinString,palinString);
    strrev(revPalinString);
    //compare strings to see if it is a palindrome
    if(strcmp(palinString, revPalinString) == 0){
        outputFileName = "palin.out";
        fprintf(stderr, "%s\n", outputFileName);
    }else {
        outputFileName = "nopalin.out";
        fprintf(stderr, "%s\n", outputFileName);
    }
    shmdt((void *) palinArray);

    printf("ERror 1\n");

    //semaphore
    sem = sem_open(semName, 0);
    if(sem == SEM_FAILED) {
        perror("./palin: sem_open error: ");
        exit(-1);
    }
    printf("ERror 2\n");

    for(i = 0; i < 5; i++){
        //sleep between 0 - 3 seconds
        sleep(rand);
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
        fprintf(out_file, "%d\t%d\t%s\n", getpid(), palinIndex, palinString);
        fclose(out_file);
        sleep(2);
        //execute code to exit from critical section
        sem_post(sem);
    }
    exit(0);
}

char* getSharedMemory(int palinIndex, int maxForks){
    char (*palinArray)[100][100];
    char* palinString;

    //get string from index
    strcpy(palinString, (*palinArray)[palinIndex]);
    //detach shared memory
    shmdt((void *) palinArray);

    return palinString;

}

//char* isPalindrome(char* str1){
//    char* outFileName;
//    char* str2;
//
//    strcpy(str2, str1);
//    strrev(str2);
//
//    if(strcmp(str1, str2) == 0){
//        outFileName = "palin.out";
//        fprintf(stderr, "%s\n", outFileName);
//    }else {
//        outFileName = "nopalin.out";
//        fprintf(stderr, "%s\n", outFileName);
//    }
//    return outFileName;
//}

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