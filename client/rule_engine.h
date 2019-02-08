#ifndef _RULE_ENGINE_H_
#define _RULE_ENGINE_H_

#include <sys/stat.h>
#include <stdio.h>
#include "infer_object_labels.h"

void fork_check(char* host_name, int uid, int child_pid, int parent_pid) {
    int host_id_index = get_host_index(host_name);
    int parent_sub_id_index = get_subject_id_index(host_id_index, uid, parent_pid);
    int child_sub_id_index = add_subject_id(host_id_index, uid, child_pid);
    if(parent_sub_id_index == -1) {
        int owner = get_user_id_index(host_id_index, uid);
        USER_SET readers = get_all_users(get_number_of_users());
        USER_SET writers = 0;
        add_user_to_label(owner, &writers);
        add_subject(child_sub_id_index, owner, readers, writers);
    }
    else {
        SUBJECT parent_subject = get_subject(parent_sub_id_index);
        
        int child_subject = add_subject(child_sub_id_index, parent_subject.owner, parent_subject.readers, parent_subject.writers);
        copy_subject_info(parent_sub_id_index, child_sub_id_index);
    }
}

int open_check(char * host_name, struct stat * file_info, int fd, int uid, int pid){
    int host_id_index = get_host_index(host_name);
    int object_id_index = get_object_id_index(host_id_index, file_info->st_dev, file_info->st_ino);
    
	if(object_id_index == -1) {
        object_id_index = add_object_id(host_id_index, file_info->st_dev, file_info->st_ino);
   
        OBJECT object;
        object.obj_id_index = object_id_index;
        object.owner = get_user_id_index(host_id_index, file_info->st_uid);
        infer_labels(&object, file_info, host_id_index);
        
        int res_add_obj = add_object(object_id_index, object.owner, object.readers, object.writers);

        int subject_id_index = get_subject_id_index(host_id_index, uid, pid);
        
        add_fd_mapping(subject_id_index, object_id_index, fd);
    }

    return 0;
}

int file_read_check(char * host_name, int uid, int pid, int fd) {
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
    int obj_id_index = get_obj_id_index_from_fd_map(sub_id_index, fd);

    //If fd is not a file fd
    if(obj_id_index == -1)
        return -1;

    SUBJECT subject = get_subject(sub_id_index);
    OBJECT object = get_object(obj_id_index);
	
    if(is_user_in_set(subject.owner, &object.readers)) {
        subject.readers = set_intersection(subject.readers, object.readers);
        subject.writers = set_union(subject.writers, object.writers);
        update_subject_label(sub_id_index, subject.readers, subject.writers);

        return 1;
    }

    return 0;
}

int file_write_check(char * host_name, int uid, int pid, int fd) {
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
    int obj_id_index = get_obj_id_index_from_fd_map(sub_id_index, fd);

    //If fd is not a file fd
    if(obj_id_index == -1)
        return -1;

    SUBJECT subject = get_subject(sub_id_index);
    OBJECT object = get_object(obj_id_index);

    if(is_user_in_set(subject.owner, &object.writers)
        && is_superset_of(subject.readers, object.readers)
        && is_subset_of(subject.writers, object.writers))
        return 1;

    return 0;
}

#endif
