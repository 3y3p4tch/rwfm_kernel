#ifndef _DB_QUERIES_H_
#define _DB_QUERIES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "underlying_libc_functions.h"
#include "database_macros.h"
#include "database_model.h"

#define	O_RDONLY	0x0000		/* open for reading only */
#define	O_WRONLY	0x0001		/* open for writing only */
#define	O_RDWR		0x0002		/* open for reading and writing */

//Converting space seperated request string into arguments array
int get_args_from_request(char **args, char *req) {
    memset(args, '\0', sizeof(char*) * MAX_REQUEST_LENGTH);
    char *curToken = strtok(req, " ");
    int i;
    for (i = 0; curToken != NULL; i++) {
      args[i] = strdup(curToken);
      curToken = strtok(NULL, " ");
    }

    return i;
}

int read_response(char *res) {
    int read_fifo_fd = underlying_open(RESPONSE_FIFO_PATH, O_RDONLY);
    int ret = underlying_read(read_fifo_fd, res, MAX_REQUEST_LENGTH);
    underlying_close(read_fifo_fd);
    return ret;
}

int write_request(char *req) {
    int write_fifo_fd = underlying_open(REQUEST_FIFO_PATH, O_WRONLY);
    int ret = underlying_write(write_fifo_fd, req, strlen(req)+1);
    underlying_close(write_fifo_fd);
    return ret;
}

int is_rwfm_enabled() {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d", IS_RWFM_ENABLED_OP);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int add_host(char *host) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d ", ADD_HOST_OP);
    strcat(request, host);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int get_host_index(char *host) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d ", GET_HOST_INDEX_OP);
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

int get_number_of_users() {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d", GET_NUMBER_OF_USERS_OP);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int add_group_id(int host_id_index, int gid, unsigned long long int member_set) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %llu", ADD_GROUP_ID_OP, host_id_index, gid, member_set);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int get_group_id_index(int host_id_index, int gid) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d", GET_GROUP_ID_INDEX_OP, host_id_index, gid);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

unsigned long long int get_members_from_group_id(int group_id_index) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d", GET_MEMBERS_FROM_GROUP_ID_OP, group_id_index);
    write_request(request);
    read_response(response);

    return strtoull(response, NULL, 10);
}

int add_object_id(int host_id_index, int device_id, int inode_num) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %d", ADD_OBJECT_ID_OP, host_id_index, device_id, inode_num);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int get_object_id_index(int host_id_index, int device_id, int inode_num) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %d", GET_OBJECT_ID_INDEX_OP, host_id_index, device_id, inode_num);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int add_object(int obj_id_index, int owner, unsigned long long readers, unsigned long long writers) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %llu %llu", ADD_OBJECT_OP, obj_id_index, owner, readers, writers);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

OBJECT get_object(int obj_id_index) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d", GET_OBJECT_OP, obj_id_index);
    write_request(request);
    read_response(response);
    char **arguments = (char**)malloc(MAX_REQUEST_LENGTH * sizeof(char*));
    get_args_from_request(arguments, response);
    OBJECT object;
    object.owner = strtoul(arguments[0], NULL, 10);
    object.readers = strtoull(arguments[1], NULL, 10);
    object.writers = strtoull(arguments[2], NULL, 10);

    return object;
}

int update_object_label(int obj_id_index, unsigned long long readers, unsigned long long writers) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %llu %llu", UPDATE_OBJECT_LABEL_OP, obj_id_index, readers, writers);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int add_subject_id(int host_id_index, int uid, int pid) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %d", ADD_SUBJECT_ID_OP, host_id_index, uid, pid);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int get_subject_id_index(int host_id_index, int uid, int pid) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %d", GET_SUBJECT_ID_INDEX_OP, host_id_index, uid, pid);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int add_subject(int sub_id_index, int owner, unsigned long long readers, unsigned long long writers) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %llu %llu", ADD_SUBJECT_OP, sub_id_index, owner, readers, writers);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

SUBJECT get_subject(int sub_id_index) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d", GET_SUBJECT_OP, sub_id_index);
    write_request(request);
    read_response(response);
    char **arguments = (char**)malloc(MAX_REQUEST_LENGTH * sizeof(char*));
    get_args_from_request(arguments, response);
    SUBJECT subject;
    subject.owner = strtoul(arguments[0], NULL, 10);
    subject.readers = strtoull(arguments[1], NULL, 10);
    subject.writers = strtoull(arguments[2], NULL, 10);

    return subject;
}

int update_subject_label(int sub_id_index, unsigned long long readers, unsigned long long writers) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %llu %llu", UPDATE_SUBJECT_LABEL_OP, sub_id_index, readers, writers);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

int add_fd_mapping(int sub_id_index, int obj_id_index, int fd) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d %d", ADD_NEW_FD_MAPPING_OP, sub_id_index, obj_id_index, fd);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
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

int copy_subject_info(int src_sub_id_index, int dstn_sub_id_index) {
    char request[MAX_REQUEST_LENGTH], response[MAX_REQUEST_LENGTH];
    sprintf(request, "%d %d %d", COPY_SUBJECT_FDS_OP, src_sub_id_index, dstn_sub_id_index);
    write_request(request);
    read_response(response);

    return strtol(response, NULL, 10);
}

#endif
