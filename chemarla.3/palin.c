/*
Name: Vibhav Chemarla
Date: 3/19/2019
CS4760 OS Project 3
Semaphores and Operating System Simulator
*/
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


char* getSharedMemory(int palinIndex);
char* isPalindrome(char str[]);

int main(int argc, char **argv) {

    int palinIndex, i;
    char* palinString;
    char* outputFileName;
    FILE* out_file;

    palinIndex = atoi(argv[1]); // get the passed index
    //get string in shared memory stored at the passed index
    palinString = getSharedMemory(palinIndex);
    //assess if string is a palindrome and get the output file name;
    outputFileName = isPalindrome(palinString);

    for(i = 0; i < 5; i++){
        //sleep
        //execute code to enter critical section
        /*critical section*/
        //execute code to exit from critical section
    }
    exit(0);
}

char* getSharedMemory(int palinIndex){
    char *palinArray, palinString;

    key_t key = 123;

    //allocate shared memory
    int shmid = shmget(key,1024, IPC_CREAT | 0666);
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
