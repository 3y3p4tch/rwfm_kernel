/*
This file communicates with the database server using fifo path and adds new users and groups which it gets as arguments,
to the database.
add_users_and_groups communicates with the database_server using named pipe.It writes the request for adding user or groups to the database
at the write end the of the pipe using the 'write_request' function. And get the response back by using the 'read_response' function.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "database_macros.h"
#include "user_set_manipulation_functions.h"

#define USER 0
#define GROUP 1

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

int add_host(char *host) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d ", ADD_HOST_OP);
    strcat(request, host);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int add_user_id(int host_id_index, int uid) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d", ADD_USER_ID_OP, host_id_index, uid);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int get_user_id_index(int host_id_index, int uid) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d", GET_USER_ID_INDEX_OP, host_id_index, uid);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int add_group_id(int host_id_index, int gid, unsigned long long int member_set) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %llx", ADD_GROUP_ID_OP, host_id_index, gid, member_set);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int get_group_id_index(int host_id_index, int gid, unsigned long long int members) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %llx", GET_GROUP_ID_INDEX_OP, host_id_index, gid, members);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int main(int argc, char* argv[]) {
    if(argc<4)
        return -1;
    int host_index = add_host(argv[2]);
    if(host_index == -1)
        return -1;
    switch(strtol(argv[1], NULL, 10)) {
        case USER:
        {
            if(add_user_id(host_index, strtol(argv[3], NULL, 10))==-1)
                return -1;
            break;
        }
        case GROUP:
        {
            unsigned long long int members = 0;
            for(int i=4;i<argc;i++) {
                int user_to_add = get_user_id_index(host_index, strtol(argv[i], NULL, 10));
                if(add_user_to_label(user_to_add, &members) == -1)
                    return -1;
            }
            if(add_group_id(host_index, strtol(argv[3], NULL, 10), members)==-1)
                return -1;
            break;
        }
        default:
            return -1;
    }
    return 0;
}
