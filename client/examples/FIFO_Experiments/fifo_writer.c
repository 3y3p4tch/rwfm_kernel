#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define CHUNK_SIZE 16000

#define DATA_SIZE 10000000

int main() {
    char buff[CHUNK_SIZE];
    int n;
    struct timeval time;
    struct timeval create_start, create_end;

    gettimeofday(&create_start, NULL);
	int fifofd = open("tmp_fifo", O_WRONLY);
    gettimeofday(&create_end, NULL);

    unsigned long long t1 = 1000 * (create_end.tv_sec - create_start.tv_sec) + (create_end.tv_usec - create_start.tv_usec)/1000;
    unsigned long long t2 = 1000000 * (create_end.tv_sec - create_start.tv_sec) + (create_end.tv_usec - create_start.tv_usec);
    printf("Time taken to create fifo: %llu millisec\n",t1);
    printf("Time taken to create fifo: %llu microsec\n",t2);
    
    gettimeofday(&time, NULL);

    for(n=0;n<(DATA_SIZE/CHUNK_SIZE);n++)
    {
        strcpy(buff,"");
        for(int i=0;i<CHUNK_SIZE/sizeof(long int);i++)
        {
            char t[9];
            sprintf(t,"%ld", random());
            t[8]='\0';
            strcat(buff,t);
        }
        write(fifofd, buff, CHUNK_SIZE);
    }

    gettimeofday(&time, NULL);
    printf("Time taken fifo read: %llu sec\n",time.tv_sec);
    printf("Time taken fifo read: %llu microsec\n",time.tv_usec);

    close(fifofd);

	return 0;
}
