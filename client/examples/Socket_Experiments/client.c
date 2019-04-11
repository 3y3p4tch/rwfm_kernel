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

int main(int argc, char *argv[])
{
    const int CHUNK_SIZE = atoi(argv[2]);
	int sockfd;
	struct sockaddr_in server_addr;
	char buff[CHUNK_SIZE];
	socklen_t server_addr_sz;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
	server_addr_sz = sizeof server_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int ret = connect(sockfd, (struct sockaddr *) &server_addr, server_addr_sz);
    
    struct timeval time;

    while((ret = recv(sockfd, buff, CHUNK_SIZE, 0))>0);

    gettimeofday(&time, NULL);
    printf("Time taken client read: %ld sec\n",time.tv_sec);
    printf("Time taken client read: %ld microsec\n",time.tv_usec);

	close(sockfd);

	return 0;
}
