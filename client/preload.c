/*
All the libc calls for file system,sockets,pipe etc are channelized to preload. This preload library communicates with the rwfm rule_engine for an below mentioned
function call to check whether the function call should be allowed or not in the rwfm context and gets back the result from the rule_engine. If it's allowed preload calls
the actual libc call for that function using the 'underlying_libc_functions.h'. Otherwise preload will deny the function call.
*/

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <stdarg.h>
#include "rule_engine.h"

void debuglog(char *log) {
    int logfd = underlying_open("/tmp/preload.log", 02000|02);
    underlying_write(logfd, log, strlen(log));
    underlying_close(logfd);
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

    struct stat file_info;
    fstat(fd, &file_info);

    if(is_rwfm_enabled()) {
        if(((file_info.st_mode & S_IFMT) == S_IFREG) && file_read_check(host_name, getuid(), getpid(), fd) != 1) {
            return -1;
        }
    }

    return underlying_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    char host_name[1024];
    sprintf(host_name, HOSTNAME);

    struct stat file_info;
    fstat(fd, &file_info);

    if(is_rwfm_enabled()) {
        if(((file_info.st_mode & S_IFMT) == S_IFREG) && file_write_check(host_name, getuid(), getpid(), fd) != 1) {
            return -1;
        }
    }

    return underlying_write(fd, buf, count);
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    char host_name[1024];
    sprintf(host_name, HOSTNAME);

    int ret = underlying_connect(sockfd, addr, addrlen);
    if(!is_rwfm_enabled()) {
		return ret;
	}
	
	if(connect_check(host_name, getuid(), getpid(), sockfd, (struct sockaddr_in *)addr) == 1)
		return ret;

	return -1;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	int new_sockfd = underlying_accept(sockfd, addr, addrlen);
	if(!is_rwfm_enabled() || new_sockfd == -1) {
		return new_sockfd;
	}

	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	if(accept_check(host_name, getuid(), getpid(), new_sockfd) != 1) {
		return -1;
    }

	return new_sockfd;
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
	if(is_rwfm_enabled()) {
		char host_name[1024];
		sprintf(host_name, HOSTNAME);

		if(send_check(host_name, getuid(), getpid(), sockfd) != 1)
			return -1;
	}

	return underlying_send(sockfd, buf, len, flags);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	ssize_t ret = underlying_recv(sockfd, buf, len, flags);

	if(!is_rwfm_enabled()) {
		return ret;
	}

	if(recv_check(host_name, getuid(), getpid(), sockfd) != 1) {
		memset(buf, '\0', len);
		buf = NULL;
		return -1;
	}

	return ret;
}

int close(int fd) {
    char host_name[1024];
    sprintf(host_name, HOSTNAME);
    struct stat file_info;
    if(fstat(fd, &file_info) == -1)
        return -1;
    if((file_info.st_mode & S_IFMT) == S_IFREG)
        file_close_check(host_name, getuid(), getpid(), fd);
    else if((file_info.st_mode & S_IFMT) == S_IFSOCK)
        socket_close_check(host_name, getuid(), getpid(), fd);

    return underlying_close(fd);
}
