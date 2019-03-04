//
// Created by veebz on 3/2/2019.
//
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>


#define BILLION  1e9

void checkSharedMemory();

int main(int argc, char **argv) {
    long duration;
    struct timespec start, current;
    long elapsed;

    duration = atoi(argv[1]);
    printf("child started: %d\n", getpid());
    if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
        perror( "clock gettime" );
        exit( -1 );
    }

    while(1) {
        checkSharedMemory();
        if (clock_gettime(CLOCK_REALTIME, &current) == -1) {
            perror("clock gettime");
            exit(-1);
        }

        elapsed = ( current.tv_sec - start.tv_sec ) * BILLION
                +  current.tv_nsec + start.tv_nsec  ;

        if (elapsed > duration) {
            printf("Reached the duration %d\n", duration);
            break;
        }
    }
    printf ("child done %d \n\n\n", getpid());
    exit(0);
}

void checkSharedMemory(){
    key_t key = 123;
    int* sharedInt;
    int shmid = shmget(key, NULL, 0); /* return value from shmget() */

    if (shmid < 0) {
        perror("./oss: shmid error: ");
        return 1;
    }
    sharedInt = (int *) shmat(shmid, NULL, 0);
    if (*sharedInt == -1) {
        perror("./user: shmat error: ");
        return 1;
    }


    printf("%d: time %d:%d\n", getpid(), *(sharedInt+0), *(sharedInt+1));

    shmdt((void *)sharedInt);
}