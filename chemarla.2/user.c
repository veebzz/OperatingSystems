//
// Created by veebz on 3/2/2019.
//
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

int main(int argc, char **argv) {

    key_t key = ftok(".", 'x');
    int* sharedInt1;
    int shmid = shmget(key, 2* sizeof(int), 0666); /* return value from shmget() */

    if (shmid < 0) {
        perror("./oss: shmid error: ");
       return 1;
    }
   sharedInt1 = (int *) shmat(shmid, NULL, 0);
    if (*sharedInt1 == -1) {
        perror("./user: shmat error: ");
       return 1;
    }

    printf("Date read from memory %d\n", sharedInt1[0]);

    shmdt((void *)sharedInt1);
    printf("shared memory detached\n");
    shmctl(shmid,IPC_RMID,NULL);
    printf("shared memory erased\n");

    return 0;
}