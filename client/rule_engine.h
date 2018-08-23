#include <sys/stat.h>
#include "infer_object_labels.h"

int fork_check(int child_pid, int parent_pid) {
    //to be implemented
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
