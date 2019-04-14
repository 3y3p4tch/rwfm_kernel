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
	int msgqid = msgget(key, 0666 | IPC_CREAT);
	
    struct timeval time;
    int n;

    for(n=0;n<(DATA_SIZE/CHUNK_SIZE);n++) {
        msgrcv(msgqid, &msg, sizeof(msg.msg_str), 0, 0);
    }

    gettimeofday(&time, NULL);
    printf("Time to receive: %ld sec\n",time.tv_sec);
    printf("Time to receive: %ld microsec\n",time.tv_usec);

	msgctl(msgqid, IPC_RMID, NULL);

	return 0;
}
