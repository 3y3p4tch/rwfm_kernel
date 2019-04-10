#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/time.h>

#define CHUNK_SIZE 16000

#define DATA_SIZE 10000000

int main()
{
	int listen_sockfd, connection_sockfd, n;
	struct sockaddr_in server_addr, client_addr;
	char buff[CHUNK_SIZE];
	socklen_t server_addr_sz, client_addr_sz;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(0);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
	server_addr_sz = sizeof server_addr;
	client_addr_sz = sizeof client_addr;

	listen_sockfd = socket(PF_INET, SOCK_STREAM, 0);
	int ret = bind(listen_sockfd, (struct sockaddr *) &server_addr, server_addr_sz);
	getsockname(listen_sockfd, (struct sockaddr *) &server_addr, &server_addr_sz);
	if(listen(listen_sockfd,5)==0)
	{
		printf("Listening on %s:%u\n",inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port));
	}
	else
	{
		printf("Error in listening!\n");
		perror("Listen");
		exit(1);
	}

	connection_sockfd = accept(listen_sockfd, (struct sockaddr *) &client_addr, &client_addr_sz);
    
    struct timeval start, end;
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
        ret = send(connection_sockfd,buff,sizeof buff, 0);
    }

    gettimeofday(&end, NULL);
    unsigned long long t1 = 1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000;
    unsigned long long t2 = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
    printf("Time taken server write: %llu millisec\n",t1);
    printf("Time taken server write: %llu microsec\n",t2);
    
	close(connection_sockfd);
	close(listen_sockfd);

	return 0;
}
