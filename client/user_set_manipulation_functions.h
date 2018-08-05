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
