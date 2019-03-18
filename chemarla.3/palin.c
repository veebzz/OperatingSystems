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


char* getSharedMemory(int palinIndex);
char* isPalindrome(char str[]);

int main(int argc, char **argv) {

    int palinIndex, i;
    char* palinString;
    char* outputFileName;
    FILE* out_file;
    sem_t* sem;
    time_t now;
    srand(now);
    now = time(NULL);
    int random = rand() % 4;
    struct tm *ts;
    char buf[80];

    ts = localtime(&now);

    palinIndex = atoi(argv[1]); // get the passed index
    //get string in shared memory stored at the passed index
    palinString = getSharedMemory(palinIndex);
    //assess if string is a palindrome and get the output file name;
    outputFileName = isPalindrome(palinString);


    for(i = 0; i < 5; i++){
        //sleep between 0 - 3 seconds
        sleep(rand);
        //entry section
        strftime(buf, sizeof(buf),"%H:%M:%S", ts);
        printf("Child[%d] starting to enter critical section at %s\n", getpid(), buf);
        while(sem_wait(sem) == -1){
            if(errno != EINTR){
                perror("./palin: sem_wait error: ");
                exit(-1);
            }
        }
        //critical section
        strftime(buf, sizeof(buf),"%H:%M:%S",ts);
        printf("Child[%d] is inside CRITICAL SECTION at %s\n", getpid(), buf);
        out_file = fopen(outputFileName, "a+");
        if (out_file == NULL){
            perror("./palin: fileError: ");
            exit(-1);
        }
        sleep(2);
        fprintf(out_file, "%d\t%d\t%s\n", getpid(), palinIndex, palinString);
        sleep(2);
        //execute code to exit from critical section
        if(sem_post(sem) == -1){
            perror("./palin: sem_post error: ");
        }
    }
    exit(0);
}

char* getSharedMemory(int palinIndex){
    char *palinArray, palinString;

    key_t key = 123;

    //allocate shared memory
    int shmid = shmget(key, 1024, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("./palin: shmid error: ");
        exit(1);
    }
    //attach sharedInt pointer to shared memory
    palinArray = (char *) shmat(shmid, NULL, 0);

    if (*palinArray == -1) {
        perror("./palin: shmat error: ");
        exit(1);
    }
    //get string from index
    palinString = *(palinArray + palinIndex);

    shmdt((void *) palinArray); //detach shared memory

    return palinString;

}

char* isPalindrome(char str[]){
    char* outFileName;
    int left = 0;
    int right = strlen(str) - 1;

    while(right > left){
        if(str[left++] != str[right--]){
            outFileName = "nopalin.out";
            return outFileName;

        }
    }
    outFileName = "palin.out";
    return outFileName;
}
