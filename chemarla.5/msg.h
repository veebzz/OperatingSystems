
#ifndef OPERATINGSYSTEMSP5_MSG_H
#define OPERATINGSYSTEMSP5_MSG_H

void sendMessage(key_t key, msgStruct message);
key_t createMsgKey(int simPid);
msgStruct recieveMessage(key_t key);




typedef struct msgStruct {
    long type;
    int resourceId;
    int userProcessId;
    char text[100];
}msgStruct;

key_t createMsgKey(int simPid){
    key_t key;
    int msgId;

    key = ftok("user", simPid);
    return key;

}

void sendMessage(key_t key, msgStruct message){
    msgId = msget(key, IPC_CREAT | 0666);
    msgsnd(msgId, &message, sizeof(int), 0);
}

msgStruct recieveMessage(key_t key){
    msgStruct message;
    msgId = msget(key, IPC_CREAT | 0666);
    msgrcv(msgId, &message, sizeof(int), 0);

    return message;
}


#endif //OPERATINGSYSTEMSP5_MSG_H
