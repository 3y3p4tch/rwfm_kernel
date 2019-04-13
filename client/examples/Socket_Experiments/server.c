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

#define DATA_SIZE 10000000

int main(int argc, char *argv[])
{
    const int CHUNK_SIZE = atoi(argv[1]);
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
    
    struct timeval time;
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
        ret = send(connection_sockfd,buff,sizeof buff, 0);
    }

    printf("Time taken server write: %ld sec\n",time.tv_sec);
    printf("Time taken server write: %ld microsec\n",time.tv_usec);
    
	close(connection_sockfd);
	close(listen_sockfd);

	return 0;
}
