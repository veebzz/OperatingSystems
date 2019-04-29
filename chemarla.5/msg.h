
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
}msgStruct;

void sendMessage(msgStruct message);
key_t createMsgKey(int simPid);
msgStruct recieveMessage();


void createMsgId(int simPid){
    msgId = msgget(msgKey, IPC_CREAT | 0666);


}

void sendMessage(msgStruct message){
    msgsnd(msgId, &message, sizeof(msgStruct), 0);
}

msgStruct recieveMessage(){
    msgStruct message;
    int msgId;
    int recv;
    msgId = msgget(msgKey, IPC_CREAT | 0666);
    recv = msgrcv(msgId, &message, sizeof(msgStruct), 1, IPC_NOWAIT);
    if (recv == -1){
        message.type = -1;
    }
    return message;
}


#endif //OPERATINGSYSTEMSP5_MSG_H
