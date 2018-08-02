#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "database_helper_functions.h"

#define REQUEST_FIFO_PATH "./request_fifo"
#define RESPONSE_FIFO_PATH "./response_fifo"

#define REQUEST_DELIM " "
#define MAX_REQUEST_LENGTH 1024

#define ADD_HOST_OP 0
#define GET_HOST_INDEX_OP 1
#define ADD_USER_ID_OP 2
#define GET_USER_ID_INDEX_OP 3
#define ADD_GROUP_ID_OP 4
#define GET_GROUP_ID_INDEX_OP 5
#define ADD_OBJECT_ID_OP 6
#define GET_OBJECT_ID_INDEX_OP 7
#define ADD_SUBJECT_ID_OP 8
#define GET_SUBJECT_ID_INDEX_OP 9
#define ADD_OBJECT_OP 10
#define GET_OBJECT_INDEX_OP 11
#define UPDATE_OBJECT_LABEL 12
#define ADD_SUBJECT_OP 13
#define GET_SUBJECT_INDEX_OP 14
#define UPDATE_SUBJECT_LABEL 15
#define ADD_NEW_FD_MAPPING_OP 16
#define GET_OBJ_ID_FROM_FD_MAP_OP 17
#define GET_SUB_ID_FROM_FD_MAP_OP 18

int num_hosts = 0;
HOST * all_hosts = NULL;

int num_user_ids = 0;
USER_ID * all_user_ids = NULL;

int num_group_ids = 0;
GROUP_ID * all_group_ids = NULL;

int num_object_ids = 0;
OBJECT_ID * all_object_ids = NULL;

int num_subject_ids = 0;
SUBJECT_ID * all_subject_ids = NULL;

int num_objects = 0;
OBJECT * all_objects = NULL;

int num_subjects = 0;
SUBJECT * all_subjects = NULL;

int num_fd_maps = 0;
FD_MAP * fd_map = NULL;

int read_fifo_fd, write_fifo_fd;

void initialize_communication_channel() {
    mkfifo(REQUEST_FIFO_PATH, 0666);
    mkfifo(RESPONSE_FIFO_PATH, 0666);
    read_fifo_fd = open(REQUEST_FIFO_PATH, O_RDONLY);
    write_fifo_fd = open(RESPONSE_FIFO_PATH, O_WRONLY);
}

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

int start_daemon() {
    initialize_communication_channel();
    while(1) {
        char request[MAX_REQUEST_LENGTH];
        char response[MAX_REQUEST_LENGTH];

        read(read_fifo_fd, request, MAX_REQUEST_LENGTH);

        char **req_args = (char**)malloc(MAX_REQUEST_LENGTH * sizeof(char*));;
        int num_args = get_args_from_request(req_args, request);
        int operation = strtol(req_args[0], NULL, 10);

        switch(operation) {

        case ADD_HOST_OP:
            if(num_args!=2)
                return -1;
            add_host(req_args[1]);
            break;

        case GET_HOST_INDEX_OP:
            if(num_args != 2)
                return -1;
            sprintf(response, "%d", get_host_index(req_args[1]));
            write(write_fifo_fd, response, strlen(response));
            break;

        case ADD_USER_ID_OP:
            if(num_args != 3)
                return -1;
            USER_ID user_id_add;
            user_id_add.host_id_index = strtol(req_args[1], NULL, 10);
            user_id_add.uid = strtol(req_args[2], NULL, 10);
            add_user_id(user_id_add);
            break;

        case GET_USER_ID_INDEX_OP:
            if(num_args != 3)
                return -1;
            USER_ID user_id_get;
            user_id_get.host_id_index = strtol(req_args[1], NULL, 10);
            user_id_get.uid = strtol(req_args[2], NULL, 10);
            sprintf(response, "%d", get_user_id_index(user_id_get));
            write(write_fifo_fd, response, strlen(response));
            break;

        case ADD_GROUP_ID_OP:
            if(num_args != 3)
                return -1;
            GROUP_ID group_id_add;
            group_id_add.host_id_index = strtol(req_args[1], NULL, 10);
            group_id_add.gid = strtol(req_args[2], NULL, 10);
            add_group_id(group_id_add);
            break;

        case GET_GROUP_ID_INDEX_OP:
            if(num_args != 3)
                return -1;
            GROUP_ID group_id_get;
            group_id_get.host_id_index = strtol(req_args[1], NULL, 10);
            group_id_get.gid = strtol(req_args[2], NULL, 10);
            sprintf(response, "%d", get_group_id_index(group_id_get));
            write(write_fifo_fd, response, strlen(response));
            break;

        case ADD_OBJECT_ID_OP:
            if(num_args != 4)
                return -1;
            OBJECT_ID object_id_add;
            object_id_add.host_id_index = strtol(req_args[1], NULL, 10);
            object_id_add.device_id = strtol(req_args[2], NULL, 10);
            object_id_add.inode_number = strtol(req_args[3], NULL, 10);
            add_object_id(object_id_add);
            break;

        case GET_OBJECT_ID_INDEX_OP:
            if(num_args != 4)
                return -1;
            OBJECT_ID object_id_get;
            object_id_get.host_id_index = strtol(req_args[1], NULL, 10);
            object_id_get.device_id = strtol(req_args[2], NULL, 10);
            object_id_get.inode_number = strtol(req_args[3], NULL, 10);
            sprintf(response, "%d", get_object_id_index(object_id_get));
            write(write_fifo_fd, response, strlen(response));
            break;

        case ADD_SUBJECT_ID_OP:
            if(num_args != 4)
                return -1;
            SUBJECT_ID subject_id_add;
            subject_id_add.host_id_index = strtol(req_args[1], NULL, 10);
            subject_id_add.uid = strtol(req_args[2], NULL, 10);
            subject_id_add.pid = strtol(req_args[3], NULL, 10);
            add_subject_id(subject_id_add);
            break;

        case GET_SUBJECT_ID_INDEX_OP:
            if(num_args != 4)
                return -1;
            SUBJECT_ID subject_id_get;
            subject_id_get.host_id_index = strtol(req_args[1], NULL, 10);
            subject_id_get.uid = strtol(req_args[2], NULL, 10);
            subject_id_get.pid = strtol(req_args[3], NULL, 10);
            sprintf(response, "%d", get_subject_id_index(subject_id_get));
            write(write_fifo_fd, response, strlen(response));
            break;

        case ADD_OBJECT_OP:
            if(num_args != 5)
                return -1;
            OBJECT object_add;
            object_add.obj_id_index = strtol(req_args[1], NULL, 10);
            object_add.owner = strtol(req_args[2], NULL, 10);
            object_add.readers = strtoull(req_args[3], NULL, 10);
            object_add.writers = strtoull(req_args[4], NULL, 10);
            add_object(object_add);
            break;

        case GET_OBJECT_INDEX_OP:
            if(num_args != 2)
                return -1;
            int obj_id_index = strtol(req_args[1], NULL, 10);
            sprintf(response, "%d", get_object_from_obj_id_index(obj_id_index));
            write(write_fifo_fd, response, strlen(response));
            break;

        case UPDATE_OBJECT_LABEL:
            if(num_args != 4)
                return -1;
            int update_obj = strtol(req_args[1], NULL, 10);
            LABEL_SET new_obj_readers = strtoull(req_args[2], NULL, 10);
            LABEL_SET new_obj_writers = strtoull(req_args[2], NULL, 10);
            update_object_label(update_obj, new_obj_readers, new_obj_writers);
            break;

        case ADD_SUBJECT_OP:
            if(num_args != 5)
                return -1;
            SUBJECT subject_add;
            subject_add.sub_id_index = strtol(req_args[1], NULL, 10);
            subject_add.owner = strtol(req_args[2], NULL, 10);
            subject_add.readers = strtoull(req_args[3], NULL, 10);
            subject_add.writers = strtoull(req_args[4], NULL, 10);
            add_subject(subject_add);
            break;

        case GET_SUBJECT_INDEX_OP:
            if(num_args != 2)
                return -1;
            int sub_id_index = strtol(req_args[1], NULL, 10);
            sprintf(response, "%d", get_subject_from_sub_id_index(sub_id_index));
            write(write_fifo_fd, response, strlen(response));
            break;

        case UPDATE_SUBJECT_LABEL:
            if(num_args != 4)
                return -1;
            int update_sub = strtol(req_args[1], NULL, 10);
            LABEL_SET new_sub_readers = strtoull(req_args[2], NULL, 10);
            LABEL_SET new_sub_writers = strtoull(req_args[2], NULL, 10);
            update_subject_label(update_sub, new_sub_readers, new_sub_writers);
            break;

        case ADD_NEW_FD_MAPPING_OP:
            if(num_args != 4)
                return -1;
            FD_MAP fd_map;
            fd_map.sub_id_index = strtol(req_args[1], NULL, 10);
            fd_map.obj_id_index = strtol(req_args[2], NULL, 10);
            fd_map.fd = strtol(req_args[3], NULL, 10);
            add_new_mapping(fd_map);
            break;

        case GET_OBJ_ID_FROM_FD_MAP_OP:
            if(num_args != 3)
                return -1;
            int sub_id_ind = strtol(req_args[1], NULL, 10);
            int fd_obj = strtol(req_args[2], NULL, 10);
            sprintf(response, "%d", get_obj_id_index_from_sub_id_and_fd(sub_id_index, fd_obj));
            write(write_fifo_fd, response, strlen(response));

        case GET_SUB_ID_FROM_FD_MAP_OP:
            if(num_args != 3)
                return -1;
            int obj_id_ind = strtol(req_args[1], NULL, 10);
            int fd_sub = strtol(req_args[2], NULL, 10);
            sprintf(response, "%d", get_obj_id_index_from_sub_id_and_fd(obj_id_index, fd_sub));
            write(write_fifo_fd, response, strlen(response));
        }
    }
}

int main() {
    start_daemon();

    return 0;
}
