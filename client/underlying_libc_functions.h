#ifndef _UND_LIBC_FUN_H_
#define _UND_LIBC_FUN_H_

#include <dlfcn.h>
#include <sys/socket.h>
#include <semaphore.h>
#include "preload.h"

void *get_libc() {
    static void *libc_handle = 0;
    if (!libc_handle) {
        libc_handle = dlopen(LIBC, RTLD_LAZY);
    }
    return libc_handle;
}

pid_t underlying_fork(void) {
    static pid_t (*underlying)(void) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "fork");
    }
    return (*underlying)();
}

int underlying_open(const char *pathname, int flags) {
    static int (*underlying)(const char *pathname, int flags) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "open");
    }
    return (*underlying)(pathname, flags);
}

int underlying_read(int fd, void *buf, size_t count) {
    static int (*underlying)(int fd, void *buf, size_t count) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "read");
    }
    return (*underlying)(fd, buf, count);
}

int underlying_write(int fd, const void *buf, size_t count) {
    static int (*underlying)(int fd, const void *buf, size_t count) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "write");
    }
    return (*underlying)(fd, buf, count);
}

int underlying_close(int fd) {
    static int (*underlying)(int fd) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "close");
    }
    return (*underlying)(fd);
}

int underlying_socket(int domain, int type, int protocol) {
    static int (*underlying)(int , int , int ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "socket");
    }
    return (*underlying)(domain, type, protocol);
}

int underlying_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	static int (*underlying)(int, const struct sockaddr *, socklen_t) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "bind");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

int underlying_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    static int (*underlying)(int ,const struct sockaddr* , socklen_t ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "connect");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

int underlying_accept(int sockfd, struct sockaddr* addr, socklen_t *addrlen) {
    static int (*underlying)(int, struct sockaddr*, socklen_t* ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "accept");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

ssize_t underlying_send(int sockfd, const void *buf, size_t len, int flags) {
    static int (*underlying)(int ,const void * , size_t, int ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "send");
    }
    return (*underlying)(sockfd, buf, len, flags);
}

ssize_t underlying_recv(int sockfd, void *buf, size_t len, int flags) {
    static int (*underlying)(int ,void * , size_t, int ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "recv");
    }
    return (*underlying)(sockfd, buf, len, flags);
}

int underlying_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	static int (*underlying)(int, struct sockaddr *, socklen_t *) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "getsockname");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

int underlying_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	static int (*underlying)(int, struct sockaddr *, socklen_t *) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "getpeername");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

#endif
