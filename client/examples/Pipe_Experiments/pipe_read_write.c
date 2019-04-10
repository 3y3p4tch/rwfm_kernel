#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define CHUNK_SIZE 16000

#define DATA_SIZE 10000000

int main() {
	int pipefd[2], pid;

    struct timeval create_start, create_end;

    gettimeofday(&create_start, NULL);
	pipe(pipefd);
    gettimeofday(&create_end, NULL);
	
    unsigned long long t1 = 1000 * (create_end.tv_sec - create_start.tv_sec) + (create_end.tv_usec - create_start.tv_usec)/1000;
    unsigned long long t2 = 1000000 * (create_end.tv_sec - create_start.tv_sec) + (create_end.tv_usec - create_start.tv_usec);
    printf("Time taken to create pipe: %llu millisec\n",t1);
    printf("Time taken to create pipe: %llu microsec\n",t2);
    struct timeval start, end;

	if((pid = fork()) == 0) {
        int m;
		close(pipefd[1]);
		char buff[CHUNK_SIZE];
    
        while((m=read(pipefd[0], buff, CHUNK_SIZE))>0);
        
        gettimeofday(&end, NULL);
        unsigned long long t1 = 1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000;
        unsigned long long t2 = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        printf("Time taken: %llu millisec\n",t1);
        printf("Time taken: %llu microsec\n",t2);

		close(pipefd[0]);
	} else {
		close(pipefd[0]);
		char buff[CHUNK_SIZE];
        int n;

        gettimeofday(&start, NULL);

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
            write(pipefd[1], buff, CHUNK_SIZE);
        }

		close(pipefd[1]);
	}

    

	return 0;
}
