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

#define print_result(res) res!=-1?printf("SUCCESS\n"):printf("FAILURE\n")

int main()
{
	int sockfd;
	struct sockaddr_in server_addr, client_addr;
	char buff[1024];
	socklen_t server_addr_sz, client_addr_sz;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(7777);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
	server_addr_sz = sizeof server_addr;

	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(7778);
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(client_addr.sin_zero, '\0', sizeof client_addr.sin_zero);
	client_addr_sz = sizeof client_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	printf("Socket: ");
	print_result(sockfd);
	getchar();
	int ret = connect(sockfd, (struct sockaddr *) &server_addr, server_addr_sz);
	printf("Connect");
	print_result(ret);
	printf("\n\nPress any key to rcv\n\n");
	getchar();
	ret = recv(sockfd, buff, 1024, 0);
	printf("Receive");
	print_result(ret);
	printf("Server said: %s",buff);
	close(sockfd);

	return 0;
}
