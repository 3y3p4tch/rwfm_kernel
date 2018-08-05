#include "database_queries.h"
#include "user_set_manipulation_functions.h"
#define USER 0
#define GROUP 1

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
