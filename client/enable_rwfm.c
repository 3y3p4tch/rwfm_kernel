#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "database_macros.h"

int read_response(char *res) {
    int read_fifo_fd = open(RESPONSE_FIFO_PATH, O_RDONLY);
    int ret = read(read_fifo_fd, res, MAX_REQUEST_LENGTH);
    close(read_fifo_fd);
    return ret;
}

int write_request(char *req) {
    int write_fifo_fd = open(REQUEST_FIFO_PATH, O_WRONLY);
    int ret = write(write_fifo_fd, req, strlen(req)+1);
    close(write_fifo_fd);
    return ret;
}

int set_rwfm_enabled(int rwfm_enabled) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d", SET_RWFM_ENABLED_OP, rwfm_enabled);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int main(int argc, char *argv[]) {
    if(argc != 2)
        return -1;
    int res = set_rwfm_enabled(strtol(argv[1], NULL, 10));
    printf("RWFM set to %d\n", res);
    return 0;
}
