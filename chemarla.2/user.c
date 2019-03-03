//
// Created by veebz on 3/2/2019.
//
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

int main(int argc, char **argv) {

    key_t key = 999;
    int* sharedInt1;
    int shmid = shmget(key, 2* sizeof(int), 0666); /* return value from shmget() */

    if (shmid < 0) {
        perror("./oss: shmid error: ");
        exit(1);
    }
   sharedInt1 = (int *) shmat(shmid,NULL, 0);
    if ((int) sharedInt1 == -1) {
        perror("./user: shmat error: ");
        exit(1);
    }

    printf("Date read from memory %d\n", sharedInt1);

    shmdt((void *)sharedInt1);
    printf("shared memory detached\n");
    shmctl(shmid,IPC_RMID,NULL);
    printf("shared memory erased\n");

    return 0;
}