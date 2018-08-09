#include "database_model.h"

int add_user_to_label(int user_to_add, USER_SET * label) {
    unsigned long long int max = -1;
    if(*label == max)
        return -1;

    USER_SET tmp = 1;
    tmp <<= user_to_add;

    *label |= tmp;

    return 0;
}

int remove_user_from_set(int user_to_remove, USER_SET * set) {
    unsigned long long int max = -1;
    if(*set == max)
        return -1;

    USER_SET tmp = 1;
    tmp <<= user_to_remove;
    tmp = ~tmp;

    *set &= tmp;

    return 0;
}

int is_user_in_set(int user_id_index, USER_SET * set) {
    USER_SET tmp = 1;
    tmp <<= user_id_index;

    return (*set) & tmp;
}

