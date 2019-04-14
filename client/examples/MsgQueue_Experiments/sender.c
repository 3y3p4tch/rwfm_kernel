#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 10000000

int main(int argc, char *argv[]) {
    const int CHUNK_SIZE = atoi(argv[1]);

    struct msgbuff {
	    long type;
	    char msg_str[CHUNK_SIZE];
    };

	key_t key = ftok("./tmp_file", 1);
	struct msgbuff msg;
	msg.type = 1;
	int msgqid = msgget(key, 0666 | IPC_CREAT);
    int n;

    struct timeval time;
    gettimeofday(&time, NULL);

    for(n=0;n<(DATA_SIZE/CHUNK_SIZE);n++)
    {
        strcpy(msg.msg_str,"");
        for(int i=0;i<CHUNK_SIZE/9;i++)
        {
            char t[9];
            sprintf(t,"%ld", random());
            t[8]='\0';
            strcat(msg.msg_str,t);
        }
        msgsnd(msgqid, &msg, sizeof(msg.msg_str), 0);
    }

    printf("Time of sender: %ld sec\n",time.tv_sec);
    printf("Time of sender: %ld microsec\n",time.tv_usec);

	return 0;
}
