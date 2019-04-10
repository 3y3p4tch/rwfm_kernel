#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

int main()
{
    struct timeval start, end;
    int i=0;
    
    gettimeofday(&start, NULL);
    for(i=0; i<10; i++) {
        if(fork()==0)
            return 0;
    }
    gettimeofday(&end, NULL);
    
    unsigned long long t1 = 1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000;
    unsigned long long t2 = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
    printf("Time taken: %llu millisec\n",t1);
    printf("Time taken: %llu microsec\n",t2);

    return 0;
}
