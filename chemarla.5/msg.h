
#ifndef OPERATINGSYSTEMSP5_MSG_H
#define OPERATINGSYSTEMSP5_MSG_H

#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include "p5Header.h"
#include "oss.h"
#include <fcntl.h>


typedef struct msgStruct {
    long type;
    int resourceId;
    int userProcessId;
    char text[100];
}msgStruct;

void sendMessage(key_t key, msgStruct message);
key_t createMsgKey(int simPid);
msgStruct recieveMessage(key_t key);


key_t createMsgKey(int simPid){
    key_t key = ftok("user", simPid);
    return key;

}

void sendMessage(key_t key, msgStruct message){
    int msgId = msgget(key, IPC_CREAT | 0666);
    msgsnd(msgId, &message, sizeof(int), 0);
}

msgStruct recieveMessage(key_t key){
    msgStruct message;
    int msgId;
    msgId = msgget(key, IPC_CREAT | 0666);
    msgrcv(msgId, &message, sizeof(int), 1, 0);

    return message;
}


#endif //OPERATINGSYSTEMSP5_MSG_H
