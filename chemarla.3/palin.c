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

void set_time(char *str);

char *strrev(char *str);

void isPalindrome(char str[]);

key_t key = 102938;
const char *semName = "/sem_Chem";


int main(int argc, char **argv) {
    int palinIndex, i, pid;
    char *palinString = malloc(sizeof(char) * 100);
    char *revPalinString = malloc(sizeof(char) * 100);
    char *outputFileName, token;
    FILE *out_file;
    sem_t *sem;
    time_t now;
    now = time(NULL);
    struct tm *ts;
    char (*palinArray)[100][100];
    char buf[80];
    ts = localtime(&now);

    // get the passed index
    palinIndex = atoi(argv[1]);
    srand(palinIndex);
    pid = getpid();

    printf("CHILD[%d] STARTED\n", getpid());
    //start shared memory in child
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
    //get the string at index
    strcpy(palinString, (*palinArray)[palinIndex]);
    printf("nor: %s",palinString);
    isPalindrome(palinString);



//    isPalindrome((*palinArray)[palinIndex]);

//    strcpy(revPalinString,strrev(palinString));
//    printf("rev: %s",revPalinString);

//    fprintf(stderr, "rev: %s\n", revPalinString);


    //compare strings to see if it is a palindrome
    if ((strcmp(palinString, revPalinString)) == 0) {
        outputFileName = "palin.out";
        fprintf(stderr, "%s\n", outputFileName);
    } else if ((strcmp(palinString, revPalinString)) == 1) {
        outputFileName = "nopalin.out";
        fprintf(stderr, "%s\n", outputFileName);
    }
    //semaphore
    sem = sem_open(semName, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        fprintf(stderr, "./palin: sem_open error: ");
        perror("./palin: sem_open error: ");
        exit(-1);
    }

    for (i = 0; i < 5; i++) {
        //sleep between 0 - 3 seconds
        sleep(rand() % 4);
        //entry section
        set_time(buf);
        fprintf(stderr, "Child[%d] STARTING to enter critical section at %s\n", pid, buf);
        //sem block
        sem_wait(sem);
        set_time(buf);
        //critical section
        fprintf(stderr, "Child[%d] is INSIDE critical section at %s\n", pid, buf);
        out_file = fopen(outputFileName, "a+");
        if (out_file == NULL) {
            perror("./palin: fileError: ");
            exit(-1);
        }
        sleep(2);
        //print to output file
        fprintf(out_file, "%d   %d   %s  \n", pid, palinIndex, palinString);
        fclose(out_file);
        sleep(2);
        set_time(buf);
        //execute code to exit from critical section
        fprintf(stderr, "Child[%d] is EXITING critical section at %s\n", pid, buf);
        sem_post(sem);
    }
    free(palinString);
    free(revPalinString);
    shmdt((void *) palinArray);
    printf("CHILD[%d] TERMINATED\n", pid);

    exit(0);
}


void set_time(char *buf) {
    time_t now;
    struct tm *ts;
    time(&now);
    ts = localtime(&now);
    sprintf(buf, "%d:%d:%d", ts->tm_hour, ts->tm_min, ts->tm_sec);
    return;
}


char *strrev(char *str) {
    int l, i;
    char *begin_ptr, *end_ptr, ch;

    // Get the length of the string
    l = strlen(str);

    // Set the begin_ptr and end_ptr
    // initially to start of string
    begin_ptr = str;
    end_ptr = str;

    // Move the end_ptr to the last character
    for (i = 0; i < l - 1; i++)
        end_ptr++;

    // Swap the char from start and end
    // index using begin_ptr and end_ptr
    for (i = 0; i < l / 2; i++) {

        // swap character
        ch = *end_ptr;
        *end_ptr = *begin_ptr;
        *begin_ptr = ch;

        // update pointers positions
        begin_ptr++;
        end_ptr--;
    }
}

void isPalindrome(char str[]) {

    int l = 0;
    int h = strlen(str) - 3;


    while (h > l) {

        if (str[l++] != str[h--]) {
            fprintf(stderr, "%d Not Palindrome\n", strlen(str));
            return;
        }

    }
    fprintf(stderr, "%s is palindrome\n", str);
}