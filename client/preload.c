#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "rule_engine.h"

void debuglog(char *log) {
    int logfd = underlying_open("/tmp/preload.log", 02000|02);
    underlying_write(logfd, log, strlen(log));
    close(logfd);
}

pid_t fork(void) {
    pid_t pid = underlying_fork();
    if(is_rwfm_enabled()) {
        if(pid == 0) {
            char host_name[1024];
            sprintf(host_name, HOSTNAME);
            fork_check(host_name, getuid(), getpid(), getppid());
        }
    }

    return pid;
}

int open(const char *path, int flags) {
    int fd = underlying_open(path, flags);
    if(is_rwfm_enabled()) {
        if(fd == -1) 
            return -1;

        struct stat file_info;
	    if(stat(path, &file_info) == -1)
            return -1;

        char host_name[1024];
        sprintf(host_name, HOSTNAME);

        open_check(host_name, &file_info, fd, getuid(), getpid());
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    char host_name[1024];
    sprintf(host_name, HOSTNAME);

    if(is_rwfm_enabled()) {
        if(file_read_check(host_name, getuid(), getpid(), fd) == 0) {
            return -1;
        }
    }

    return underlying_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    char host_name[1024];
    sprintf(host_name, HOSTNAME);

    if(is_rwfm_enabled()) {
        if(file_write_check(host_name, getuid(), getpid(), fd) == 0) {
            return -1;
        }
    }

    return underlying_write(fd, buf, count);
}

int socket(int domain, int type, int protocol) {
    int fd = underlying_socket(domain, type, protocol);
	if(fd == -1)
        return -1;

	char host_name[1024];
    sprintf(host_name, HOSTNAME);

    if(socket_check(host_name, getuid(), getpid(), fd) != 1)
		return -1;

    return fd;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	int ret = underlying_bind(sockfd, addr, addrlen);
	if(ret == -1)
		return -1;

	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	if(bind_check(host_name, getuid(), getpid(), sockfd) != 1)
		return -1;

	return ret;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	int ret = underlying_connect(sockfd, addr, addrlen);
	if(ret == -1)
		return -1;

	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	if(connect_check(host_name, getuid(), getpid(), sockfd) != 1)
		return -1;

	return ret;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	int ret = underlying_accept(sockfd, addr, addrlen);
	if(ret == -1)
		return -1;

	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	if(accept_check(host_name, getuid(), getpid(), sockfd) != 1)
		return -1;

	return ret;
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	if(send_check(host_name, getuid(), getpid(), sockfd) != 1)
		return -1;

	return underlying_send(sockfd, buf, len, flags);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	ssize_t ret = underlying_recv(sockfd, buf, len, flags);

	if(recv_check(host_name, getuid(), getpid(), sockfd) != 1) {
		buf = NULL;
		return -1;
	}

	return ret;
}
