#ifndef _INFER_OBJECT_LABELS_H_
#define _INFER_OBJECT_LABELS_H_

#include "database_queries.h"
#include "user_set_manipulation_functions.h"
#include <sys/stat.h>

USER_SET get_all_users(int number_of_users) {
    USER_SET ans = 1;
    for(int i=0; i<number_of_users; i++) {
        ans *= 2;
    }
    return ans-1;
}

int infer_labels(OBJECT * object, struct stat * object_info, int host_id_index) {
    int user_id_index = get_user_id_index(host_id_index, object_info->st_uid);
    if(user_id_index==-1)
        return -1;
    
    if(object_info->st_mode & S_IRUSR)
        add_user_to_label(user_id_index, &(object->readers));   

    if(object_info->st_mode & S_IWUSR)
        add_user_to_label(user_id_index, &(object->writers));

    int is_user_reader = is_user_in_set(user_id_index, &(object->readers));
    int is_user_writer = is_user_in_set(user_id_index, &(object->writers));

    int group_id_index = get_group_id_index(host_id_index, object_info->st_gid);    
    USER_SET group_members = get_members_from_group_id(group_id_index);
    
    if(object_info->st_mode & S_IRGRP)
        object->readers |= group_members;

    if(object_info->st_mode & S_IWGRP)
        object->writers |= group_members; 

    if(!is_user_reader)
        remove_user_from_set(user_id_index, &(object->readers));
    
    if(!is_user_writer)
        remove_user_from_set(user_id_index, &(object->writers));
    
    int number_of_users = get_number_of_users();
    USER_SET all_users = get_all_users(number_of_users);

    USER_SET other_members = all_users & (~group_members);
    other_members = remove_user_from_set(user_id_index, &other_members);
    
    if(object_info->st_mode & S_IROTH)
        object->readers |= other_members;

    if(object_info->st_mode & S_IWOTH)
        object->writers |= other_members;

    return 0;       
}

#endif
