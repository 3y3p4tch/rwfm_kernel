#include <sys/stat.h>
#include "infer_object_labels.h"

int fork_check(char* host_name, int uid, int child_pid, int parent_pid) {
    int host_id_index = get_host_index(host_name);
    int parent_sub_id_index = get_subject_id(host_id_index, uid, parent_pid);
    SUBJECT parent_subject = get_subject(parent_sub_id_index);

    int child_sub_id_index = add_subject_id(host_id_index, uid, child_pid);
    int child_subject = add_subject(child_sub_id_index, parent_subject.owner, parent_subject.readers, parent_subject.writers);
    copy_subject_info(parent_sub_id_index, child_sub_id_index);
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
        
        add_object(object_id_index, object.owner, object.readers, object.writers);

        int subject_id_index = get_subject_id_index(host_id_index, uid, pid);
        
        add_fd_mapping(subject_id_index, object_id_index, fd);
    }

    return 0;
}
