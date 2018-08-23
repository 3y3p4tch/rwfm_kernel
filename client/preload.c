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
    if(pid == 0) {
        char host_name[1024];
        sprintf(host_name, HOSTNAME);
        fork_check(host_name, getuid(), getpid(), getppid());
    }

    return pid;
}

int open(const char *path, int flags) {
    int fd = underlying_open(path, flags);
    if(fd == -1) 
        return -1;

    struct stat file_info;
	if(stat(path, &file_info) == -1)
        return -1;

    char host_name[1024];
    sprintf(host_name, HOSTNAME);

    open_check(host_name, &file_info, fd, getuid(), getpid());
}
