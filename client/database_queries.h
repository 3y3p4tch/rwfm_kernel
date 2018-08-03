#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
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

void add_host(char *host) {
    char request[MAX_REQUEST_LENGTH];
    sprintf(request, "%d ", ADD_HOST_OP);
    strcat(request, host);
    write_request(request);
}

int get_host_index(char *host) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d ", GET_HOST_INDEX_OP);
    strcat(request, host);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

void add_user_id(int host_id_index, int uid) {
    char request[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d", ADD_USER_ID_OP, host_id_index, uid);
    write_request(request);
}

int get_user_id_index(int host_id_index, int uid) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d", GET_USER_ID_INDEX_OP, host_id_index, uid);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

void add_group_id(int host_id_index, int gid) {
    char request[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d", ADD_GROUP_ID_OP, host_id_index, gid);
    write_request(request);
}

int get_group_id_index(int host_id_index, int gid) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d", GET_GROUP_ID_INDEX_OP, host_id_index, gid);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

void add_object_id(int host_id_index, int device_id, int inode_num) {
    char request[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %d", ADD_OBJECT_ID_OP, host_id_index, device_id, inode_num);
    write_request(request);
}

int get_object_id_index(int host_id_index, int device_id, int inode_num) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %d", GET_OBJECT_ID_INDEX_OP, host_id_index, device_id, inode_num);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

void add_subject_id(int host_id_index, int uid, int pid) {
    char request[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %d", ADD_SUBJECT_ID_OP, host_id_index, uid, pid);
    write_request(request);
}

int get_subject_id_index(int host_id_index, int uid, int pid) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %d", GET_SUBJECT_ID_INDEX_OP, host_id_index, uid, pid);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

void add_object(int obj_id_index, int owner, unsigned long long readers, unsigned long long writers) {
    char request[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %llu %llu", ADD_OBJECT_OP, obj_id_index, owner, readers, writers);
    write_request(request);
}

int get_object_index(int obj_id_index, int owner, unsigned long long readers, unsigned long long writers) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %llu %llu", GET_OBJECT_INDEX_OP, obj_id_index, owner, readers, writers);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

void update_object_label(int obj_index, unsigned long long readers, unsigned long long writers) {
    char request[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %llu %llu", UPDATE_OBJECT_LABEL_OP, obj_index, readers, writers);
    write_request(request);
}

void add_subject(int sub_id_index, int owner, unsigned long long readers, unsigned long long writers) {
    char request[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %llu %llu", ADD_SUBJECT_OP, sub_id_index, owner, readers, writers);
    write_request(request);
}

int get_subject_index(int sub_id_index, int owner, unsigned long long readers, unsigned long long writers) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %llu %llu", GET_SUBJECT_INDEX_OP, sub_id_index, owner, readers, writers);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

void update_subject_label(int sub_index, unsigned long long readers, unsigned long long writers) {
    char request[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %llu %llu", UPDATE_SUBJECT_LABEL_OP, sub_index, readers, writers);
    write_request(request);
}

void add_fd_mapping(int sub_id_index, int obj_id_index, int fd) {
    char request[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %d", ADD_NEW_FD_MAPPING_OP, sub_id_index, obj_id_index, fd);
    write_request(request);
}

int get_obj_id_index_from_fd_map(int sub_id_index, int fd) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d", GET_OBJ_ID_FROM_FD_MAP_OP, sub_id_index, fd);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int get_sub_id_index_from_fd_map(int obj_id_index, int fd) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d", GET_SUB_ID_FROM_FD_MAP_OP, obj_id_index, fd);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}
