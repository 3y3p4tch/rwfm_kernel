#include <sys/types.h>
#include <sys/stat.h>
#include <asm/ptrace.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <dlfcn.h>
#include <pwd.h>
#include <string.h>
//#include <sched.h>
//#include <fcntl.h>
#include "preload.h"
#include "socket.h"

#define TRUE 1
#define FALSE 0

#define O_APPEND	02000
#define O_CREAT		0100

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2
#define AT_SYMLINK_NOFOLLOW   0x100   /* Do not follow symbolic links.  */

int encountered_pipe_fds[1024];
int pipe_fd_count = 0;

int is_safe_to_close = TRUE;
int close_ref_count = 0;
char to_be_closed[1024][1024];

void toggle_is_safe_to_close() {
    if(is_safe_to_close == TRUE)
        is_safe_to_close = FALSE;
    else
        is_safe_to_close = TRUE;
}

int get_fd_from_close_buffer(char buf[]) {
    char last_tok[10];
    int no_space=0, i=0, j=0;

    //skip first 3 tokens delimited by space
    for(i=0;no_space<3;i++)
        if(buf[i]==' ')no_space++;

    //fourth token is fd
    while(buf[i])
        last_tok[j++]=buf[i++];
    last_tok[j]='\0';

    //return after converting to integer
    return atoi(last_tok);
}

void myexit(int);

void *get_libc() {
    static void *libc_handle = 0;
    if (!libc_handle) {
        libc_handle = dlopen(LIBC, RTLD_LAZY);
    }
    return libc_handle;
}

int myopen(const char *pathname, int flags) {
    static int (*underlying)(const char *pathname, int flags) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "open");
    }
    return (*underlying)(pathname, flags);
}

int mysocket(int domain, int type, int protocol) {
    static int (*underlying)(int , int , int ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "socket");
    }
    return (*underlying)(domain, type, protocol);
}

int myconnect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    static int (*underlying)(int ,const struct sockaddr* , socklen_t ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "connect");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

ssize_t mysend(int sockfd, const void *buf, size_t len, int flags) {
    static int (*underlying)(int ,const void * , size_t, int ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "send");
    }
    return (*underlying)(sockfd, buf, len, flags);
}

ssize_t myrecv(int sockfd, void *buf, size_t len, int flags) {
    static int (*underlying)(int ,void * , size_t, int ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "recv");
    }
    return (*underlying)(sockfd, buf, len, flags);
}

int myclose(int fd) {
    static int (*underlying)(int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "close");
    }
    return (*underlying)(fd);
}

void debuglog(char *log) {
    int logfd;
    logfd = myopen("/tmp/preload.log", 02000|02);
    write(logfd, log, strlen(log));
    myclose(logfd);
}

int skip(const char *pathname){
    char token[10];
    int i=0;
    token[0]='\0';
    while(pathname[0]=='/' || pathname[0]=='.')
        pathname++;

    while(pathname[0]!='\0' && pathname[0]!='/'){
        token[i++] = pathname[0];
        pathname++;
    }
    token[i]='\0';

    if(strcmp(token, "proc")==0 || strcmp(token, "sys")==0)
        return 1;
    return 0;
}

int rwfm_process(char *op){
    char buf[50];
    int ret=1;
    if(rwfm_connect()<0){
        printf("\nCould not connect %d", rwfm_sockfd);
        return 1;
        //myexit(1);
    }
    mysend(rwfm_sockfd, op, strlen(op), 0);
    myrecv(rwfm_sockfd, buf, MAXDATASIZE-1, 0);
    ret = atoi(buf);
    /*if(ret==1)
        printf("\nPass.\n");
    else{
        printf("\nFail.\n");
        exit(1);
    }*/
    myclose(rwfm_sockfd);
    return(ret);
}

int socket(int domain, int type, int protocol) {
    int fd;
    char buf[1024];
    static int (*underlying)(int , int , int ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "socket");
    }
    fd = (*underlying)(domain, type, protocol);
    if(fd==-1)
        return -1;

    sprintf(buf, "socket %ld %ld %d", (long)getuid(), (long)getpid(), fd);
#ifdef DEBUG
	debuglog(buf);
#endif
    if(!rwfm_process(buf))
        return -1;
    return fd;
}

int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    char buf[1024];
    char ip_addr_str[100];
    inet_ntop(addr->sa_family, &((struct sockaddr_in *)addr)->sin_addr, ip_addr_str, addrlen);
    sprintf(buf, "connect %ld %ld %d %s %d", (long)getuid(), (long)getpid(), sockfd, ip_addr_str, ntohs(((struct sockaddr_in *)addr)->sin_port));
#ifdef DEBUG
    debuglog(buf);
#endif
    if(!rwfm_process(buf))
        return -1;
    static int (*underlying)(int ,const struct sockaddr* , socklen_t ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "connect");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    char buf[1024];
    char ip_addr_str[100];
    inet_ntop(addr->sa_family, &((struct sockaddr_in *)addr)->sin_addr, ip_addr_str, addrlen);
    sprintf(buf, "bind %ld %ld %d %s %d", (long)getuid(), (long)getpid(), sockfd, ip_addr_str, ntohs(((struct sockaddr_in *)addr)->sin_port));
#ifdef DEBUG
    debuglog(buf);
#endif
    if(!rwfm_process(buf))
        return -1;
    static int (*underlying)(int ,const struct sockaddr* , socklen_t ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "bind");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

int accept(int sockfd, __SOCKADDR_ARG addr, socklen_t *__restrict addrlen) {
    char buf[1024];
    char ip_addr_str[100];
    static int (*underlying)(int ,const struct sockaddr * , socklen_t * ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "accept");
    }
    int new_fd = (*underlying)(sockfd, addr, addrlen);
    sprintf(buf, "accept %ld %ld %d %d", (long)getuid(), (long)getpid(), sockfd, new_fd);
#ifdef DEBUG
    debuglog(buf);
#endif
    if(!rwfm_process(buf))
        return -1;
    return new_fd;
}

ssize_t send(int sockfd, const void *buff, size_t len, int flags) {
    char buf[1024];
    static int (*underlying)(int ,const void * , size_t, int ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "send");
    }
    sprintf(buf, "send %ld %ld %d", (long)getuid(), (long)getpid(), sockfd);
#ifdef DEBUG
    debuglog(buf);
#endif
    if(!rwfm_process(buf))
        return -1;
    return (*underlying)(sockfd, buff, len, flags);
}

ssize_t recv(int sockfd, void *buff, size_t len, int flags) {
    char buf[1024];
    static int (*underlying)(int ,void * ,size_t ,int ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "recv");
    }
    sprintf(buf, "recv %ld %ld %d", (long)getuid(), (long)getpid(), sockfd);
#ifdef DEBUG
    debuglog(buf);
#endif
    if(!rwfm_process(buf))
        return -1;
    return (*underlying)(sockfd, buff, len, flags);
}

//pipe related calls

int pipe(int pipefd[2]) {
    char buf[1024];
    static int (*underlying)(int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "pipe");
    }
    sprintf(buf, "pipe %ld %ld %d %d", (long)getuid(), (long)getpid(), pipefd[0], pipefd[1]);
#ifdef DEBUG
    debuglog(buf);
#endif
    if(!rwfm_process(buf))
        return -1;
    int result = (*underlying)(pipefd[2]);
    encountered_pipe_fds[pipe_fd_count++] = pipefd[0];
    encountered_pipe_fds[pipe_fd_count++] = pipefd[1];
    return result;	
}

ssize_t read(int fd, void *buf, size_t count) {
    //yet to implement
    //need to distinguish bet file read and pipe read
    static ssize_t (*underlying)(int, void*, size_t) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "read");
    }
    return (*underlying)(fd, buf, count);
}

int mkfifo(const char *pathname, mode_t mode) {
    char buf[1024];
    static int (*underlying)(const char *, mode_t) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "mkfifo");
    }
    sprintf(buf, "mkfifo %ld %ld %s", (long)getuid(), (long)getpid(), pathname);
#ifdef DEBUG
    debuglog(buf);
#endif
    if(!rwfm_process(buf))
        return -1;
    return (*underlying)(pathname, mode);
}

pid_t fork(void) {
    pid_t p;
    char buf[50];
    static pid_t (*underlying)(void) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "fork");
    }
    p = (*underlying)();
    if(p==0){//child
#ifdef DEBUG
sprintf(buf, "\nfork \n\tUID : %ld \n\tPID : %ld \n\tPPID : %ld", (long)getuid(), (long)getpid(), (long)getppid());
debuglog(buf);
#endif
        sprintf(buf, "fork %ld %ld %ld", (long)getuid(), (long)getpid(), (long)getppid());
        rwfm_process(buf);
    }
    debuglog("inside fork");
    toggle_is_safe_to_close();
    if(is_safe_to_close == TRUE && close_ref_count>0){
        while(close_ref_count>0){
            close_ref_count--;
            rwfm_process(to_be_closed[close_ref_count]);
            myclose(get_fd_from_close_buffer(to_be_closed[close_ref_count]));
        }
    }
    return(p);
}

void exit_group(int status) {
    char buf[50];
    void (*underlying)(int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "exit_group");
    }
#ifdef DEBUG
    sprintf(buf, "\nexit_group \n\tUID : %ld \n\tPID : %ld", (long)getuid(), (long)getpid());
	debuglog(buf);
#endif
    sprintf(buf, "exit_group %ld %ld", (long)getuid(), (long)getpid());
    rwfm_process(buf);
    (*underlying)(status);
}

void exit(int status) {
    char buf[50];
    void (*underlying)(int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "exit");
    }
#ifdef DEBUG
    sprintf(buf, "\nexit \n\tUID : %ld \n\tPID : %ld", (long)getuid(), (long)getpid());
	debuglog(buf);
#endif
    sprintf(buf, "exit %ld %ld", (long)getuid(), (long)getpid());
    rwfm_process(buf);
    (*underlying)(status);
}

void _exit(int status){
    char buf[50];
    void (*underlying)(int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "_exit");
    }
#ifdef DEBUG
    sprintf(buf, "\n_exit \n\tUID : %ld \n\tPID : %ld", (long)getuid(), (long)getpid());
	debuglog(buf);
#endif
    sprintf(buf, "_exit %ld %ld", (long)getuid(), (long)getpid());
    rwfm_process(buf);
    (*underlying)(status);
}

void _Exit(int status){
    char buf[50];
    void (*underlying)(int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "_Exit");
    }
#ifdef DEBUG
    sprintf(buf, "\n_Exit \n\tUID : %ld \n\tPID : %ld", (long)getuid(), (long)getpid());
	debuglog(buf);
#endif
    sprintf(buf, "_Exit %ld %ld", (long)getuid(), (long)getpid());
    rwfm_process(buf);
    (*underlying)(status);
}

void myexit(int status) {
    void (*underlying)(int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "exit");
    }
    (*underlying)(status);
}

int close(int fd) {
    char buf[50];
    void (*underlying)(int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "close");
    }
#ifdef DEBUG
    sprintf(buf, "\nclose \n\tUID : %ld \n\tPID : %ld \n\tFD : %d", (long)getuid(), (long)getpid(), fd);
	debuglog(buf);
#endif
    sprintf(buf, "close %ld %ld %d", (long)getuid(), (long)getpid(), fd);
    if(is_safe_to_close == TRUE) {
        for(i = 1;i 
        rwfm_process(buf);
        (*underlying)(fd);
    } else {
        strcpy(to_be_closed[close_ref_count], buf);
        close_ref_count++;
    }
}

int open64(const char *pathname, int flags, mode_t mode) {
    struct stat sb;
    char buf[1024];
    int exists=0;
    int ret=0;
    static int (*underlying)(const char *pathname, int flags, mode_t mode) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "open64");
    }
#ifdef DEBUG
	sprintf(buf, "\nopen64 \n\tpathname:%s, \n\tflags:%d, \n\tmode:%lo", pathname, flags, (unsigned long)mode);
	debuglog(buf);
#endif
    char op[5];
    if(flags & O_RDWR)
        strcpy(op, "rw");
    else if(flags & O_WRONLY)
        strcpy(op, "w");
    else
        strcpy(op, "r");

    if(!skip(pathname) && stat(pathname, &sb)==0){
        exists=1;
        sprintf(buf, "open64 %s %ld %ld %ld %ld %ld %ld %lo", op,
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino,
                (long)sb.st_uid, (long)sb.st_gid, (unsigned long)sb.st_mode);
        if(!rwfm_process(buf))
            return -1;
    }

    ret = (*underlying)(pathname, flags, mode);
    if(ret==-1)
        return -1;

    if(!skip(pathname) && flags & O_CREAT && !exists){
        stat(pathname, &sb);
        sprintf(buf, "creat %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return ret;
}

int open(const char *pathname, int flags) {
    struct stat sb;
    char buf[1024];
    int exists=0;
    int ret=0;
    static int (*underlying)(const char *pathname, int flags) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "open");
    }
#ifdef DEBUG
	sprintf(buf, "\nopen \n\tpathname:%s, \n\tflags:%d", pathname, flags);
	debuglog(buf);
#endif
    char op[5];

    if(flags & O_RDWR)
        strcpy(op, "rw");
    else if(flags & O_WRONLY)
        strcpy(op, "w");
    else
        strcpy(op, "r");

    if(!skip(pathname) && stat(pathname, &sb)==0){
        exists=1;
        sprintf(buf, "open %s %ld %ld %ld %ld %ld %ld %lo", op,
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino,
                (long)sb.st_uid, (long)sb.st_gid, (unsigned long)sb.st_mode);
        if(!rwfm_process(buf))
            return -1;
    }

    ret = (*underlying)(pathname, flags);
    if(ret==-1)
        return -1;

    if(!skip(pathname) && flags & O_CREAT && !exists){
        stat(pathname, &sb);
        sprintf(buf, "creat %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return ret;
}

int openat(int dirfd, const char *pathname, int flags){
    struct stat sb;
    char buf[1024];
    int exists=0;
    int ret=0;
    static int (*underlying)(int dirfd, const char *pathname, int flags) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "openat");
    }
#ifdef DEBUG
	sprintf(buf, "\nopenat \n\tpathname:%s, \n\tflags:%d", pathname, flags);
	debuglog(buf);
#endif
    char op[5];

    if(flags & O_RDWR)
        strcpy(op, "rw");
    else if(flags & O_WRONLY)
        strcpy(op, "w");
    else
        strcpy(op, "r");

    if(fstatat(dirfd, pathname, &sb, 0)==0){
        exists=1;
        sprintf(buf, "open %s %ld %ld %ld %ld %ld %ld %lo", op,
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino,
                (long)sb.st_uid, (long)sb.st_gid, (unsigned long)sb.st_mode);
        if(!rwfm_process(buf))
            return -1;
    }

    ret = (*underlying)(dirfd, pathname, flags);
    if(ret==-1)
        return -1;

    if(flags & O_CREAT && !exists){
        fstatat(dirfd, pathname, &sb, 0);
        sprintf(buf, "creat %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return ret;
}

int creat(const char *pathname, mode_t mode){
    int newfd=0;
    char buf[1024];
    struct stat sb;
    static int (*underlying)(const char *pathname, mode_t mode) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "creat");
    }
#ifdef DEBUG
	sprintf(buf, "\ncreat \n\tpathname:%s,\n\tmode:%lo", pathname, (unsigned long)mode);
	debuglog(buf);
#endif
    newfd = (*underlying)(pathname, mode);
    if(newfd!=-1){
        stat(pathname, &sb);
        sprintf(buf, "creat %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return newfd;
}

int unlink(const char *pathname) {
    int ret=0;
    char buf[1024];
    struct stat sb;
    static int (*underlying)(const char *pathname) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "unlink");
    }
#ifdef DEBUG
	sprintf(buf, "\nunlink pathname:%s", pathname);
	debuglog(buf);
#endif
    if(stat(pathname, &sb)==0)
        sprintf(buf, "unlink %ld %ld", (long)sb.st_dev, (long)sb.st_ino);
    ret = (*underlying)(pathname);
    if(ret==0)
         rwfm_process(buf);
    return ret;
}

int unlinkat(int dirfd, const char *pathname, int flags){
    int ret=0;
    char buf[1024];
    struct stat sb;
    static int (*underlying)(int dirfd, const char *pathname, int flags) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "unlinkat");
    }
#ifdef DEBUG
	sprintf(buf, "\nunlinkat pathname:%s, flags:%d", pathname, flags);
	debuglog(buf);
#endif
    if(fstatat(dirfd, pathname, &sb, 0)==0)
        sprintf(buf, "unlinkat %ld %ld", (long)sb.st_dev, (long)sb.st_ino);
    ret = (*underlying)(dirfd, pathname, flags);
    if(ret==0)
         rwfm_process(buf);
    return ret;
}

int mkdir(const char *pathname, mode_t mode) {
    int ret=0;
    char buf[1024];
    struct stat sb;
    static int (*underlying)(const char *pathname, mode_t mode) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "mkdir");
    }
#ifdef DEBUG
	sprintf(buf, "\nmkdir \n\tpathname:%s, \n\tmode:%lo", pathname, (unsigned long)mode);
	debuglog(buf);
#endif
    ret = (*underlying)(pathname, mode);
    if(ret==0){
        if(stat(pathname, &sb)!=0)
            return -1;
        sprintf(buf, "mkdir %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return ret;
}

int mkdirat(int dirfd, const char *pathname, mode_t mode){
    int ret=0;
    char buf[1024];
    struct stat sb;
    static int (*underlying)(int dirfd, const char *pathname, mode_t mode) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "mkdirat");
    }
#ifdef DEBUG
	sprintf(buf, "\nmkdirat \n\tpathname:%s, \n\tmode:%lo", pathname, (unsigned long)mode);
	debuglog(buf);
#endif
    ret = (*underlying)(dirfd, pathname, mode);
    if(ret==0){
        if(fstatat(dirfd, pathname, &sb, 0)!=0)
            return -1;
        sprintf(buf, "mkdirat %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return ret;
}

int rmdir(const char *pathname) {
    int ret=0;
    char buf[1024];
    struct stat sb;
    static int (*underlying)(const char *pathname) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "rmdir");
    }
#ifdef DEBUG
	sprintf(buf, "\nrmdir pathname:%s", pathname);
	debuglog(buf);
#endif
    if(stat(pathname, &sb)==0)
        sprintf(buf, "rmdir %ld %ld", (long)sb.st_dev, (long)sb.st_ino);
    ret = (*underlying)(pathname);
    if(ret==0)
         rwfm_process(buf);
    return ret;
}

int get_operation_type(const char *mode, char *op){
    int create=0;
    if(strcmp(mode, "r")==0 || strcmp(mode, "rb")==0){
        strcpy(op, "r");
    }else if(strcmp(mode, "r+")==0 || strcmp(mode, "rb+")==0 ||
            strcmp(mode,"r+b")==0){
        strcpy(op, "rw");
    }else if(strcmp(mode, "w")==0 || strcmp(mode, "wb")==0 ||
            strcmp(mode, "a")==0 || strcmp(mode, "ab")==0){
        strcpy(op, "w");
        create = 1;
    }else if(strcmp(mode, "+w")==0 || strcmp(mode, "+wb")==0 ||
            strcmp(mode, "wb+")==0 || strcmp(mode, "w+b")==0 ||
            strcmp(mode, "a+")==0 || strcmp(mode, "ab+")==0 ||
            strcmp(mode, "a+b")==0){
        strcpy(op, "rw"); 
        create = 1;
    }else{
        strcpy(op, "undef"); 
    }
    return create;
}

FILE *fopen(const char *pathname, const char *mode) {
    struct stat sb;
    char buf[1024], op[10];
    int exists = 0;
    int create = 0;
    FILE *fp = NULL;
    static FILE * (*underlying)(const char *pathname, const char *mode) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "fopen");
    }
#ifdef DEBUG
	sprintf(buf, "\nfopen \n\tpathname:%s, \n\tmode:%s", pathname, mode);
	debuglog(buf);
#endif

    create = get_operation_type(mode, op);

    if(!skip(pathname) && stat(pathname, &sb)==0){
        exists=1;
        sprintf(buf, "fopen %s %ld %ld %ld %ld %ld %ld %lo", op,
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino,
                (long)sb.st_uid, (long)sb.st_gid, (unsigned long)sb.st_mode);
        if(!rwfm_process(buf))
            return NULL;
    }

    fp = (*underlying)(pathname, mode);
    if(fp==NULL)
        return NULL;

    if(!skip(pathname) && create && !exists){
        stat(pathname, &sb);
        sprintf(buf, "creat %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return NULL;
    }
    return fp;
}

FILE *fopen64(const char *pathname, const char *mode) {
    struct stat sb;
    char buf[1024];
    char op[10];
    int exists = 0;
    int create = 0;
    FILE *fp = NULL;
    static FILE * (*underlying)(const char *pathname, const char *mode) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "fopen64");
    }
#ifdef DEBUG
	sprintf(buf, "\nfopen64 \n\tpathname:%s, \n\tmode:%s", pathname, mode);
	debuglog(buf);
#endif

    create = get_operation_type(mode, op);

    if(!skip(pathname) && stat(pathname, &sb)==0){
        exists=1;
        sprintf(buf, "fopen64 %s %ld %ld %ld %ld %ld %ld %lo", op,
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino,
                (long)sb.st_uid, (long)sb.st_gid, (unsigned long)sb.st_mode);
        if(!rwfm_process(buf))
            return NULL;
    }

    fp = (*underlying)(pathname, mode);
    if(fp==NULL)
        return NULL;

    if(!skip(pathname) && create && !exists){
        stat(pathname, &sb);
        sprintf(buf, "creat %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return NULL;
    }
    return fp;
}

int link(const char *oldpath, const char *newpath){
    int ret=0;
    char buf[1024];
    struct stat sb;
    static int (*underlying)(const char *oldpath, const char *newpath) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "link");
    }
#ifdef DEBUG
	sprintf(buf, "\nlink \n\toldpath:%s, \n\tnewpath:%s", oldpath, newpath);
	debuglog(buf);
#endif

    ret = (*underlying)(oldpath, newpath);

    if(ret==0){
        if(stat(newpath, &sb)!=0)
            return -1;
        sprintf(buf, "link %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return ret;
}

int linkat(int olddirfd, const char *oldpath,
            int newdirfd, const char *newpath, int flags){
    int ret=0;
    char buf[1024];
    struct stat sb;
    static int (*underlying)(int, const char *, int, const char *, int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "linkat");
    }
#ifdef DEBUG
	sprintf(buf, "\nlinkat \n\toldpath:%s, \n\tnewpath:%s, flags:%d", oldpath, newpath, flags);
	debuglog(buf);
#endif

    ret = (*underlying)(olddirfd, oldpath, newdirfd, newpath, flags);

    if(ret==0){
        if(fstatat(newdirfd, newpath, &sb, 0)!=0)
            return -1;
        sprintf(buf, "linkat %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return ret;
}

int symlink(const char *oldpath, const char *newpath){
    int ret=0;
    char buf[1024];
    struct stat sb;
    static int (*underlying)(const char *oldpath, const char *newpath) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "symlink");
    }
#ifdef DEBUG
	sprintf(buf, "\nsymlink \n\toldpath:%s, \n\tnewpath:%s", oldpath, newpath);
	debuglog(buf);
#endif

    ret = (*underlying)(oldpath, newpath);

    if(ret==0){
        if(lstat(newpath, &sb)!=0)
            return -1;
        sprintf(buf, "symlink %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return ret;
}

int symlinkat(const char *oldpath, int newdirfd, const char *newpath){
    int ret=0;
    char buf[1024];
    struct stat sb;
    static int (*underlying)(const char *, int, const char *) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "symlinkat");
    }
#ifdef DEBUG
	sprintf(buf, "\nsymlinkat \n\toldpath:%s, \n\tnewpath:%s", oldpath, newpath);
	debuglog(buf);
#endif

    ret = (*underlying)(oldpath, newdirfd, newpath);

    if(ret==0){
        if(fstatat(newdirfd, newpath, &sb, AT_SYMLINK_NOFOLLOW)!=0)
            return -1;
        sprintf(buf, "symlinkat %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return ret;
}

int rename(const char *oldpath, const char *newpath) {
    int ret=0;
    char buf[1024];
    struct stat sb1, sb2;
    static int (*underlying)(const char *oldpath, const char *newpath) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "rename");
    }
#ifdef DEBUG
	sprintf(buf, "\nrename \n\toldpath:%s, \n\tnewpath:%s", oldpath, newpath);
	debuglog(buf);
#endif

    if(stat(oldpath, &sb1)!=0)
        return -1;

    ret = (*underlying)(oldpath, newpath);

    if(ret==0){
        if(stat(newpath, &sb2)!=0)
            return -1;
        sprintf(buf, "rename %ld %ld %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb1.st_dev, (long)sb1.st_ino,
                (long)sb2.st_dev, (long)sb2.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return ret;
}

int renameat(int olddirfd, const char *oldpath,
            int newdirfd, const char *newpath){
    int ret=0;
    char buf[1024];
    struct stat sb1, sb2;
    static int (*underlying)(int, const char *, int, const char *) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "renameat");
    }
#ifdef DEBUG
	sprintf(buf, "\nrenameat \n\toldpath:%s, \n\tnewpath:%s", oldpath, newpath);
	debuglog(buf);
#endif

    if(fstatat(olddirfd, oldpath, &sb1, 0)!=0)
        return -1;

    ret = (*underlying)(olddirfd, oldpath, newdirfd, newpath);

    if(ret==0){
        if(fstatat(newdirfd, newpath, &sb2, 0)!=0)
            return -1;
        sprintf(buf, "renameat %ld %ld %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb1.st_dev, (long)sb1.st_ino,
                (long)sb2.st_dev, (long)sb2.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return ret;
}

/*
int sigreturn(unsigned long __unused){
    char buf[50];
    int (*underlying)(unsigned long) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "sigreturn");
    }
    printf("\n### sigreturn\n");
    sprintf(buf, "exit_group %ld %ld", (long)getuid(), (long)getpid());
    //rwfm_process(buf);
    return (*underlying)(__unused);
}

int creat64(const char *pathname, mode_t mode){
    int newfd=0;
    char buf[1024];
    struct stat64 sb;
    static int (*underlying)(const char *pathname, mode_t mode) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "creat64");
    }
    printf("\n### creat64");
    newfd = (*underlying)(pathname, mode);
    if(newfd!=-1){
        stat64(pathname, &sb);
        sprintf(buf, "creat64 %ld %ld %ld %ld",
                (long)getuid(), (long)getpid(),
                (long)sb.st_dev, (long)sb.st_ino);
        if(!rwfm_process(buf))
            return -1;
    }
    return newfd;
}

int open(const char *pathname, int flags, mode_t mode) {
    static int (*underlying)(const char *pathname, int flags) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "open");
    }
    printf("\n### open with mode.");
    if(flags & O_CREAT)
        printf("\nCREAT\n");
    return (*underlying)(pathname, flags);
}

int execve(const char *filename, char *const argv[], char *const envp[]){
    char buf[50];
    static int (*underlying)(const char *, char * const*, char * const*)=0;
    printf("\n### execve\n");
    if (!underlying) {
        underlying = dlsym(get_libc(), "execve");
    }
    return (*underlying)(filename, argv, envp);
}

int chdir(const char *pathname) {
    static int (*underlying)(const char *pathname) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "chdir");
    }
    printf("\n### chdir\n");
    return (*underlying)(pathname);
}

int clone(int (*fn)(void *), void *child_stack, int flags, void *arg, ...){
    static int (*underlying)(int(*)(void *), void *, int, void *, ...) = 0;
    printf("\n### clone\n");
    if (!underlying) {
        underlying = dlsym(get_libc(), "clone");
    }
    va_list ap;
    va_start (ap, arg);
    return (*underlying)(fn, child_stack, flags, arg, ap);
}
*/

