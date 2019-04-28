struct mesg_buffer {
    long mesg_type;
    char mesg_text[100];
} message;

key_t createMsgKey(int simPid){
    key_t key;
    int msgId;

    key = ftok("user", simPid);
    return key;

}

void sendMessage(key_t key, int message){
    msgId = msget(key, IPC_CREAT | 0666);
    msgsnd(msgId, &message, sizeof(int), 0);
}

void recieveMessage(key_t key){

    msgId = msget(key, IPC_CREAT | 0666);
    msgrcv(msgId, &message, sizeof(int), 0);
}

