//
// Created by veebz on 3/2/2019.
//
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

int main(int argc, char **argv) {
    key_t key = 123;
    int* sharedInt;
    int shmid = shmget(key, 2* sizeof(int), 0666); /* return value from shmget() */

    if (shmid < 0) {
        perror("./oss: shmid error: ");
       return 1;
    }
   sharedInt = (int *) shmat(shmid, NULL, 0);
    if (*sharedInt == -1) {
        perror("./user: shmat error: ");
       return 1;
    }

    printf("seconds read from memory %d\n", sharedInt[0]);
    printf("nanoseconds read from memory %d\n", sharedInt[0]);

    shmdt((void *)sharedInt);
    printf("shared memory detached\n");
    printf("shared memory erased\n");

    return 0;
}